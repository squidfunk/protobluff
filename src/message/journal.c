/*
 * Copyright (c) 2013-2015 Martin Donath <martin.donath@squidfunk.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/allocator.h"
#include "message/buffer.h"
#include "message/common.h"
#include "message/journal.h"

/* ----------------------------------------------------------------------------
 * Internal functions
 * ------------------------------------------------------------------------- */

/*!
 * Add an entry to a journal.
 *
 * If allocation fails, the journal's internal state is fully recoverable.
 *
 * \param[in,out] journal Journal
 * \param[in]     origin  Origin
 * \param[in]     offset  Offset
 * \param[in]     delta   Delta
 * \return                Error code
 */
static pb_error_t
push(
    pb_journal_t *journal, size_t origin, size_t offset, ptrdiff_t delta) {
  assert(journal && origin <= offset && delta);
  assert(pb_journal_valid(journal));
  pb_allocator_t *allocator = pb_buffer_allocator(&(journal->buffer));
  if (unlikely_(allocator == &allocator_zero_copy))
    return PB_ERROR_ALLOC;

  /* Grow journal and append entry */
  pb_journal_entry_t *data = pb_allocator_resize(allocator,
    journal->entry.data, sizeof(pb_journal_entry_t)
      * (journal->entry.size + 1));
  if (data) {
    journal->entry.data = data;
    journal->entry.data[journal->entry.size++] = (pb_journal_entry_t){
      .origin = origin,
      .offset = offset,
      .delta  = delta
    };
    return PB_ERROR_NONE;
  }
  return PB_ERROR_ALLOC;
}

/* LCOV_EXCL_START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*!
 * Remove the latest entry from the journal.
 *
 * \param[in,out] journal Journal
 */
static void
pop(pb_journal_t *journal) {
  assert(journal);
  assert(pb_journal_valid(journal));
  if (journal->entry.size)
    --journal->entry.size;
}

/* LCOV_EXCL_STOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a journal.
 *
 * A journal is basically just a wrapper around a buffer which keeps track
 * of all changes, so structures that refer to the buffer can sync up.
 *
 * \param[in] data[] Raw data
 * \param[in] size   Raw data size
 * \return           Journal
 */
extern pb_journal_t
pb_journal_create(const uint8_t data[], size_t size) {
  return pb_journal_create_with_allocator(&allocator_default, data, size);
}

/*!
 * Create a journal using a custom allocator.
 *
 * \warning A journal does not take ownership of the provided allocator, so the
 * caller must ensure that the allocator is not freed during operations.
 *
 * \param[in,out] allocator Allocator
 * \param[in]     data[]    Raw data
 * \param[in]     size      Raw data size
 * \return                  Journal
 */
extern pb_journal_t
pb_journal_create_with_allocator(
    pb_allocator_t *allocator, const uint8_t data[], size_t size) {
  assert(allocator && data && size);
  pb_journal_t journal = {
    .buffer = pb_buffer_create_with_allocator(allocator, data, size),
    .entry  = {
      .data = NULL,
      .size = 0
    }
  };
  return journal;
}

/*!
 * Create an empty journal.
 *
 * \return Journal
 */
extern pb_journal_t
pb_journal_create_empty(void) {
  return pb_journal_create_empty_with_allocator(&allocator_default);
}

/*!
 * Create an empty journal using a custom allocator.
 *
 * \warning A journal does not take ownership of the provided allocator, so the
 * caller must ensure that the allocator is not freed during operations.
 *
 * \param[in,out] allocator Allocator
 * \return                  Journal
 */
extern pb_journal_t
pb_journal_create_empty_with_allocator(pb_allocator_t *allocator) {
  assert(allocator);
  pb_journal_t journal = {
    .buffer = pb_buffer_create_empty_with_allocator(allocator),
    .entry  = {
      .data = NULL,
      .size = 0
    }
  };
  return journal;
}

/*!
 * Create a zero-copy journal.
 *
 * \param[in,out] data[] Raw data
 * \param[in]     size   Raw data size
 * \return               Journal
 */
extern pb_journal_t
pb_journal_create_zero_copy(uint8_t data[], size_t size) {
  assert(data && size);
  pb_journal_t journal = {
    .buffer = pb_buffer_create_zero_copy(data, size),
    .entry  = {
      .data = NULL,
      .size = 0
    }
  };
  return journal;
}

/*!
 * Destroy a journal.
 *
 * \param[in,out] journal Journal
 */
extern void
pb_journal_destroy(pb_journal_t *journal) {
  assert(journal);
  if (pb_journal_valid(journal)) {
    if (journal->entry.data) {
      pb_allocator_t *allocator = pb_buffer_allocator(&(journal->buffer));
      pb_allocator_free(allocator, journal->entry.data);

      /* Clear entries */
      journal->entry.data = NULL;
      journal->entry.size = 0;
    }
    pb_buffer_destroy(&(journal->buffer));
  }
}

/*!
 * Write data to a journal.
 *
 * \warning The lines excluded from code coverage cannot be triggered within
 * the tests, as they are masked through push().
 *
 * \param[in,out] journal Journal
 * \param[in]     origin  Origin
 * \param[in]     start   Start offset
 * \param[in]     end     End offset
 * \param[in]     data[]  Raw data
 * \param[in]     size    Raw data size
 * \return                Error code
 */
extern pb_error_t
pb_journal_write(
    pb_journal_t *journal, size_t origin, size_t start, size_t end,
    const uint8_t data[], size_t size) {
  assert(journal && data && size);
  if (unlikely_(!pb_journal_valid(journal)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* Perform journaled write if size changed */
  ptrdiff_t delta = size - (end - start);
  if (delta) {
    if (likely_(!(error = push(journal, origin, end, delta)))) {
      error = pb_buffer_write(&(journal->buffer), start, end, data, size);
      if (unlikely_(error))
        pop(journal);                                      /* LCOV_EXCL_LINE */
    }

  /* Otherwise perform immediate write */
  } else {
    error = pb_buffer_write(&(journal->buffer), start, end, data, size);
  }
  return error;
}

/*!
 * Clear data from a journal.
 *
 * \warning The lines excluded from code coverage cannot be triggered within
 * the tests, as they are masked through push().
 *
 * \param[in,out] journal Journal
 * \param[in]     origin  Origin
 * \param[in]     start   Start offset
 * \param[in]     end     End offset
 * \return                Error code
 */
extern pb_error_t
pb_journal_clear(
    pb_journal_t *journal, size_t origin, size_t start, size_t end) {
  assert(journal);
  if (unlikely_(!pb_journal_valid(journal)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* Perform journaled clear if size changed */
  ptrdiff_t delta = start - end;
  if (delta) {
    if (likely_(!(error = push(journal, origin, end, delta)))) {
      error = pb_buffer_clear(&(journal->buffer), start, end);
      if (unlikely_(error))
        pop(journal);                                      /* LCOV_EXCL_LINE */
    }

  /* Otherwise perform immediate clear */
  } else {
    error = pb_buffer_clear(&(journal->buffer), start, end);
  }
  return error;
}

/*!
 * Align an offset of a specific version according to a journal.
 *
 * This is the core of the journaling functionality, as it allows several
 * different parts to co-exist at the same time, reading or writing values
 * from or to the underlying buffer. However, it's very tricky to get right,
 * as there are three major scenarios as well as some edge cases:
 *
 * -# The change happened before the current part, so the part must be moved by
 *    the given delta. There are two cases which can trigger this scenario:
 *
 *    -# Update on a preceding part, either located in the current message, in
 *       the preceding message or in a parent message.
 *
 *    -# Update on the length prefix of the current part, which is done when
 *       altering the contents of a length-prefixed field or message.
 *
 *    \code{.unparsed}
 *      ... [ T ][ L ][ ... ] ...
 *        ^                       (a)
 *        ^^^^^^^^^^^^^^^^
 *                 ^              (b)
 *    \endcode
 *
 * -# The change happened within the current part, so the part must either be
 *    resized or cleared completely:
 *
 *    -# The update happened somewhere inside the current part, which makes it
 *       necessary to resize the current part by the given delta.
 *
 *    -# The update involves the whole part from start to end and a negative
 *       delta is given which matches its size, so the part was cleared.
 *
 *    \code{.unparsed}
 *      ... [ T ][ L ][ ... ] ...
 *                      ^         (a)
 *          ^^^^^^^^^^^^^^^^^     (b)
 *    \endcode
 *
 * -# The change happened outside the current part and the delta is negative,
 *    which means that the current part must be cleared completley.
 *
 *    \code{.unparsed}
 *      ... [ T ][ L ][ ... ] ...
 *        ^^^^^^^^^^^^^^^^^^^^^
 *    \endcode
 *
 * If a parent message is resized, nothing needs to be done, as the current
 * part is contained and seemingly not affected by the update, so this is a
 * case that doesn't need to be explicitly handled. This is repeated until
 * the provided version matches the current version of the journal.
 *
 * \param[in]     journal Journal
 * \param[in,out] version Version
 * \param[in,out] offset  Offset
 * \return                Error code
 */
extern pb_error_t
pb_journal_align(
    const pb_journal_t *journal, pb_version_t *version, pb_offset_t *offset) {
  assert(journal && offset);
  if (unlikely_(!pb_journal_valid(journal)))
    return PB_ERROR_INVALID;

  /* Iterate journal entries until we're up-to-date */
  uint8_t invalid = 0;
  while (*version < journal->entry.size) {
    const pb_journal_entry_t *entry = &journal->entry.data[*version];

    /* Change happened before current part: move */
    if (entry->origin < offset->start &&
        entry->offset < offset->end) {
      offset->start += entry->delta;
      offset->end   += entry->delta;

      /* Apply potential update on relative offsets */
      ptrdiff_t *diff[3] = {
        &(offset->diff.origin),
        &(offset->diff.tag),
        &(offset->diff.length)
      };
      for (size_t d = 0; d < 3; ++d)
        if (entry->offset > offset->start + *(diff[d]) - entry->delta)
          *(diff[d]) -= entry->delta;

    /* Change happened within current part: resize or clear */
    } else if (entry->origin >= offset->start + offset->diff.origin &&
               entry->offset <= offset->end) {

      /* Change happened inside current part: resize */
      if (entry->origin >= offset->start) {
        offset->end += entry->delta;

        /* Current part is a packed field: invalidate */
        if (offset->diff.origin && !offset->diff.tag &&
            offset->start == offset->end)
          invalid = 1;

      /* Current part was cleared: clear */
      } else if ((offset->start + offset->diff.origin) -
                 (offset->end   + entry->delta) == 0) {
        offset->start      += offset->diff.origin;
        offset->end        += entry->delta;
        offset->diff.origin = 0;
        offset->diff.tag    = 0;
        offset->diff.length = 0;

        /* Invalidate after alignment */
        invalid = 1;
      }

    /* Change happened outside current part: clear */
    } else if (entry->origin <= offset->start + offset->diff.origin &&
               entry->origin == entry->offset + entry->delta) {
      offset->start       = entry->origin;
      offset->end         = entry->origin;
      offset->diff.origin = 0;
      offset->diff.tag    = 0;
      offset->diff.length = 0;

      /* Invalidate after alignment */
      invalid = 1;
    }
    (*version)++;
  }

  /* Invalidate part */
  if (invalid)
    *version = SIZE_MAX;
  return *version == SIZE_MAX
    ? PB_ERROR_INVALID
    : PB_ERROR_NONE;
}