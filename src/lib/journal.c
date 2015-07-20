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
#include <stdlib.h>

#include "lib/allocator.h"
#include "lib/common.h"
#include "lib/journal.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a journal to keep track of changes within a binary.
 *
 * \param[in] bulk Bulk
 * \return         Journal
 */
extern pb_journal_t
pb_journal_create(size_t bulk) {
  return pb_journal_create_with_allocator(&allocator_default, bulk);
}

/*!
 * Create a journal using a custom allocator.
 *
 * \warning The initial size of the journal must be a power of two, so growing
 * it at a later point is simple and efficient.
 *
 * \param[in,out] allocator Allocator
 * \param[in]     bulk      Bulk
 * \return                  Journal
 */
extern pb_journal_t
pb_journal_create_with_allocator(pb_allocator_t *allocator, size_t bulk) {
  assert(allocator && bulk && !(bulk & (bulk - 1)));
  pb_journal_entry_t *data = pb_allocator_alloc(
    allocator, sizeof(pb_journal_entry_t) * bulk);
  pb_journal_t journal = (pb_journal_t){
    .allocator = allocator,
    .entry     = {
      .data = data,
      .size = 0,
      .bulk = bulk
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
  if (journal->entry.data) {
    pb_allocator_free(journal->allocator, journal->entry.data);
    journal->entry.data = NULL;
  }
}

/*!
 * Retrieve the internal error state of a journal.
 *
 * \param[in] journal Journal
 * \return            Error code
 */
extern pb_error_t
pb_journal_error(const pb_journal_t *journal) {
  assert(journal);
  return journal->entry.data
    ? PB_ERROR_NONE
    : PB_ERROR_ALLOC;
}

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
extern pb_error_t
pb_journal_log(
    pb_journal_t *journal,
    size_t origin, size_t offset, ptrdiff_t delta) {
  assert(journal && origin <= offset && delta);
  if (__unlikely(!pb_journal_valid(journal)))
    return PB_ERROR_INVALID;

  /* If no bulk slot is available, grow bulk */
  if (!journal->entry.bulk) {
    pb_journal_entry_t *new_data = pb_allocator_realloc(
      journal->allocator, journal->entry.data,
        sizeof(pb_journal_entry_t) * journal->entry.size << 1);
    if (new_data) {
      journal->entry.data = new_data;
      journal->entry.bulk = journal->entry.size;
    } else {
      return PB_ERROR_ALLOC;
    }
  }

  /* Create and append a new journal entry */
  pb_journal_entry_t entry = {
    .origin = origin,
    .offset = offset,
    .delta  = delta
  };
  journal->entry.data[journal->entry.size++] = entry;
  journal->entry.bulk--;
  return PB_ERROR_NONE;
}

/*!
 * Remove the latest entry from the journal.
 *
 * \param[in,out] journal Journal
 */
extern void
pb_journal_revert(pb_journal_t *journal) {
  assert(journal);
  if (pb_journal_valid(journal) && journal->entry.size) {
    --journal->entry.size;
    ++journal->entry.bulk;
  }
}

/*!
 * Align an offset of a specific version according to a journal.
 *
 * This is the core of the journaling functionality, as it allows several
 * different parts to co-exist at the same time, reading or writing values
 * from or to the underlying binary. However, it's very tricky to get right,
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
  if (__unlikely(!pb_journal_valid(journal)))
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
      for (size_t d = 0; d < 3; d++)
        if (entry->offset > offset->start + *(diff[d]) - entry->delta)
          *(diff[d]) -= entry->delta;

    /* Change happened within current part: resize or clear */
    } else if (entry->origin >= offset->start + offset->diff.origin &&
               entry->offset <= offset->end) {

      /* Change happened inside current part: resize */
      if (entry->origin >= offset->start) {
        offset->end += entry->delta;

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