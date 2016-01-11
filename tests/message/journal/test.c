/*
 * Copyright (c) 2013-2016 Martin Donath <martin.donath@squidfunk.com>
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
#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "core/allocator.h"
#include "message/common.h"
#include "message/journal.h"

/* ----------------------------------------------------------------------------
 * System-default allocator callback overrides
 * ------------------------------------------------------------------------- */

/*!
 * Allocator with failing allocation.
 *
 * \param[in,out] data Internal allocator data
 * \param[in]     size Bytes to be allocated
 * \return             Memory block
 */
static void *
allocator_allocate_fail(void *data, size_t size) {
  assert(!data && size);
  return NULL;
}

/*!
 * Allocator with failing reallocation.
 *
 * \param[in,out] data  Internal allocator data
 * \param[in,out] block Memory block to be resized
 * \param[in]     size  Bytes to be allocated
 * \return              Memory block
 */
static void *
allocator_resize_fail(void *data, void *block, size_t size) {
  assert(!data && size);
  return NULL;
}

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a journal.
 */
START_TEST(test_create) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create journal */
  pb_journal_t journal = pb_journal_create(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(size, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert same contents but different location */
  fail_if(memcmp(data, pb_journal_data(&journal), size));
  ck_assert_ptr_ne(data, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create an empty journal.
 */
START_TEST(test_create_empty) {
  pb_journal_t journal = pb_journal_create_empty();

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Assert journal size and version */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert empty journal */
  ck_assert_ptr_eq(NULL, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST


/*
 * Create a zero-copy journal.
 */
START_TEST(test_create_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create journal */
  pb_journal_t journal = pb_journal_create_zero_copy(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(size, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert same contents and location */
  fail_if(memcmp(data, pb_journal_data(&journal), size));
  ck_assert_ptr_eq(data, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create an invalid journal.
 */
START_TEST(test_create_invalid) {
  pb_journal_t journal = pb_journal_create_invalid();

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Assert journal size and version */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert empty journal */
  ck_assert_ptr_eq(NULL, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a journal for which allocation fails.
 */
START_TEST(test_create_invalid_allocate) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_allocate_fail,
      .resize   = allocator_default.proc.resize,
      .free     = allocator_default.proc.free
    }
  };

  /* Create journal */
  pb_journal_t journal =
    pb_journal_create_with_allocator(&allocator, data, size);

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Assert journal size and version */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert empty journal */
  ck_assert_ptr_eq(NULL, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write data to a journal.
 */
START_TEST(test_write) {
  pb_journal_t journal = pb_journal_create_empty();

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Iteratively grow journal: "" => "HELP I'M TRAPPED ..." */
  uint8_t data[] = "HELP I'M TRAPPED IN A UNIVERSE FACTORY";
  for (size_t s = 1; s < 38; s++) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_write(&journal, 0, 0,
      pb_journal_size(&journal), data, s));

    /* Assert journal size and version */
    fail_if(pb_journal_empty(&journal));
    ck_assert_uint_eq(s, pb_journal_size(&journal));
    ck_assert_uint_eq(s, pb_journal_version(&journal));

    /* Assert same contents but different location */
    fail_if(memcmp(data, pb_journal_data(&journal), s));
    ck_assert_ptr_ne(data, pb_journal_data(&journal));
  }

  /* Iteratively shrink journal: "HELP ... UNIVERSE FACTORY" => "FACTORY" */
  for (size_t s = 30; s > 0; s--) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_write(&journal, 0, 0,
      pb_journal_size(&journal) - 8, data, s));

    /* Assert journal size and version */
    fail_if(pb_journal_empty(&journal));
    ck_assert_uint_eq(s + 8, pb_journal_size(&journal));
    ck_assert_uint_eq(38 + (30 - s), pb_journal_version(&journal));

    /* Assert same contents but different location */
    fail_if(memcmp(data, pb_journal_data(&journal), s));
    ck_assert_ptr_ne(data, pb_journal_data(&journal));
  }

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write data of the same length to a zero-copy journal.
 */
START_TEST(test_write_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create journal */
  pb_journal_t journal = pb_journal_create_zero_copy(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Update journal: "SOME DATA" => "NEW STUFF" */
  uint8_t new_data[] = "NEW STUFF";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_write(&journal, 0, 0, size, new_data, size));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(size, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert same contents but different location */
  fail_if(memcmp(new_data, pb_journal_data(&journal), size));
  ck_assert_ptr_ne(new_data, pb_journal_data(&journal));
  ck_assert_ptr_eq(data, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write data to an invalid journal.
 */
START_TEST(test_write_invalid) {
  pb_journal_t journal = pb_journal_create_invalid();

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Try to update journal */
  uint8_t data[] = "THIS WON'T WORK ANYWAY";
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_write(&journal, 0, 0, 0, data, 22));

  /* Assert journal validity and error again */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Assert journal size and version */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert empty journal */
  ck_assert_ptr_eq(NULL, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write data to a journal for which allocation fails.
 */
START_TEST(test_write_invalid_allocate) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_allocate_fail,
      .resize   = allocator_default.proc.resize,
      .free     = allocator_default.proc.free
    }
  };

  /* Create journal */
  pb_journal_t journal =
    pb_journal_create_with_allocator(&allocator, data, size);

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Try to update journal */
  uint8_t new_data[] = "THIS WILL FAIL";
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_write(&journal, 0, 0, size, new_data, 14));

  /* Assert journal validity and error again */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Assert journal size and version */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert empty journal */
  ck_assert_ptr_eq(NULL, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write data to a journal for which reallocation fails.
 */
START_TEST(test_write_invalid_resize) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_resize_fail,
      .free     = allocator_default.proc.free
    }
  };

  /* Create journal */
  pb_journal_t journal =
    pb_journal_create_with_allocator(&allocator, data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Iteratively grow journal */
  uint8_t new_data[] = "THIS WILL FAIL";
  for (size_t s = 10; s < 14; s++) {
    ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_write(&journal, 0, 0,
      pb_journal_size(&journal), new_data, s));

    /* Assert journal size and version */
    fail_if(pb_journal_empty(&journal));
    ck_assert_uint_eq(size, pb_journal_size(&journal));
    ck_assert_uint_eq(0, pb_journal_version(&journal));
  }

  /* Assert journal validity and error again */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Iteratively shrink journal */
  for (size_t s = 8; s > 0; s--) {
    ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_write(&journal, 0, 0,
      pb_journal_size(&journal), new_data, s));

    /* Assert journal size and version */
    fail_if(pb_journal_empty(&journal));
    ck_assert_uint_eq(size, pb_journal_size(&journal));
    ck_assert_uint_eq(0, pb_journal_version(&journal));
  }

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write invalid data of the same length to a zero-copy journal.
 */
START_TEST(test_write_invalid_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create journal */
  pb_journal_t journal = pb_journal_create_zero_copy(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Update journal: "SOME DATA" => "MORE DATA" */
  uint8_t new_data[] = "AWE";
  ck_assert_uint_eq(PB_ERROR_ALLOC,
    pb_journal_write(&journal, 0, 0, 0, new_data, 3));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(9, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear data from a journal.
 */
START_TEST(test_clear) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create journal */
  pb_journal_t journal = pb_journal_create(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Clear journal: "SOME DATA" => "DATA" */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_clear(&journal, 0, 0, 5));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(4, pb_journal_size(&journal));
  ck_assert_uint_eq(1, pb_journal_version(&journal));

  /* Assert same contents but different location */
  fail_if(memcmp("DATA", pb_journal_data(&journal), size - 5));
  ck_assert_ptr_ne(data, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear data from a zero-copy journal.
 */
START_TEST(test_clear_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create journal */
  pb_journal_t journal = pb_journal_create_zero_copy(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Try to clear data from journal */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_clear(&journal, 0, 0, 0));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(9, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert same contents but different location */
  fail_if(memcmp(data, pb_journal_data(&journal), size));
  ck_assert_ptr_eq(data, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear data from an invalid journal.
 */
START_TEST(test_clear_invalid) {
  pb_journal_t journal = pb_journal_create_invalid();

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Try to clear data from journal */
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_clear(&journal, 0, 0, 0));

  /* Assert journal validity and error again */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert empty journal */
  ck_assert_ptr_eq(NULL, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear data from a journal for which allocation fails.
 */
START_TEST(test_clear_invalid_allocate) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_allocate_fail,
      .resize   = allocator_default.proc.resize,
      .free     = allocator_default.proc.free
    }
  };

  /* Create journal */
  pb_journal_t journal =
    pb_journal_create_with_allocator(&allocator, data, size);

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Try to clear data from journal */
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_clear(&journal, 0, 0, size));

  /* Assert journal validity and error again */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Assert journal size and version */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Assert empty journal */
  ck_assert_ptr_eq(NULL, pb_journal_data(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear data from a journal for which reallocation fails.
 */
START_TEST(test_clear_invalid_resize) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_resize_fail,
      .free     = allocator_default.proc.free
    }
  };

  /* Create journal */
  pb_journal_t journal =
    pb_journal_create_with_allocator(&allocator, data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Try to clear data from journal */
  ck_assert_uint_eq(PB_ERROR_ALLOC,
    pb_journal_clear(&journal, 0, 0, size));

  /* Assert journal validity and error again */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(size, pb_journal_size(&journal));
  ck_assert_uint_eq(0, pb_journal_version(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset of a specific version according to a journal.
 */
START_TEST(test_align) {
  pb_journal_t journal = pb_journal_create_empty();

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Iteratively grow journal: "" => "HELP I'M TRAPPED ..." */
  uint8_t data[] = "HELP I'M TRAPPED IN A UNIVERSE FACTORY";
  for (size_t s = 1; s < 38; s++) {
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_journal_write(&journal, 0, 0, pb_journal_size(&journal), data, s));

    /* Assert journal size and version */
    fail_if(pb_journal_empty(&journal));
    ck_assert_uint_eq(s, pb_journal_size(&journal));
    ck_assert_uint_eq(s, pb_journal_version(&journal));

    /* Assert same contents but different location */
    fail_if(memcmp(data, pb_journal_data(&journal), s));
    ck_assert_ptr_ne(data, pb_journal_data(&journal));
  }

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 0, 0 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(37, version);
  ck_assert_uint_eq(0,  offset.start);
  ck_assert_uint_eq(37, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(0, offset.diff.origin);
  ck_assert_int_eq(0, offset.diff.tag);
  ck_assert_int_eq(0, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update on a preceding part.
 */
START_TEST(test_align_move) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create journal */
  pb_journal_t journal = pb_journal_create(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Insert data to into journal */
  uint8_t new_data[] = "AMAZING ";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_write(&journal, 0, 5, 5, new_data, 8));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(17, pb_journal_size(&journal));
  ck_assert_uint_eq(1, pb_journal_version(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 8, 9 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(16, offset.start);
  ck_assert_uint_eq(17, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(0, offset.diff.origin);
  ck_assert_int_eq(0, offset.diff.tag);
  ck_assert_int_eq(0, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update on a length prefix.
 */
START_TEST(test_align_move_length) {
  const uint8_t data[] = { 16, 2, 0, 0 };
  const size_t  size   = 4;

  /* Create journal */
  pb_journal_t journal = pb_journal_create(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Insert data to into journal */
  uint8_t new_data[] = { 255, 1 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_write(&journal, 0, 1, 2, new_data, 2));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(5, pb_journal_size(&journal));
  ck_assert_uint_eq(1, pb_journal_version(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 3, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(3, offset.start);
  ck_assert_uint_eq(4, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(-3, offset.diff.origin);
  ck_assert_int_eq(-3, offset.diff.tag);
  ck_assert_int_eq(-2, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update growing the current part.
 */
START_TEST(test_align_grow) {
  const uint8_t data[] = { 16, 2, 0, 0 };
  const size_t  size   = 4;

  /* Create journal */
  pb_journal_t journal = pb_journal_create(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Insert data to into journal */
  uint8_t new_data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_write(&journal, 2, 2, 3, new_data, 8));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(11, pb_journal_size(&journal));
  ck_assert_uint_eq(1, pb_journal_version(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 3, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(2, offset.start);
  ck_assert_uint_eq(10, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(-2, offset.diff.origin);
  ck_assert_int_eq(-2, offset.diff.tag);
  ck_assert_int_eq(-1, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update shrinking the current part.
 */
START_TEST(test_align_shrink) {
  const uint8_t data[] = { 16, 4, 0, 0, 0, 0 };
  const size_t  size   = 6;

  /* Create journal */
  pb_journal_t journal = pb_journal_create(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Insert data to into journal */
  uint8_t new_data[] = { 0, 0 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_write(&journal, 2, 2, 6, new_data, 2));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(4, pb_journal_size(&journal));
  ck_assert_uint_eq(1, pb_journal_version(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 6, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(2, offset.start);
  ck_assert_uint_eq(4, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(-2, offset.diff.origin);
  ck_assert_int_eq(-2, offset.diff.tag);
  ck_assert_int_eq(-1, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update clearing the whole part.
 */
START_TEST(test_align_clear) {
  const uint8_t data[] = { 16, 4, 0, 0 };
  const size_t  size   = 4;

  /* Create journal */
  pb_journal_t journal = pb_journal_create(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Clear data from journal */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_clear(&journal, 0, 0, 4));

  /* Assert journal size and version */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));
  ck_assert_uint_eq(1, pb_journal_version(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 4, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(SIZE_MAX, version);
  ck_assert_uint_eq(0, offset.start);
  ck_assert_uint_eq(0, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(0, offset.diff.origin);
  ck_assert_int_eq(0, offset.diff.tag);
  ck_assert_int_eq(0, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update clearing the outer part.
 */
START_TEST(test_align_clear_outside) {
  const uint8_t data[] = { 16, 4, 0, 0, 0, 0 };
  const size_t  size   = 6;

  /* Create journal */
  pb_journal_t journal = pb_journal_create(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Clear data from journal */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_clear(&journal, 0, 0, 6));

  /* Assert journal size and version */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));
  ck_assert_uint_eq(1, pb_journal_version(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 4, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(SIZE_MAX, version);
  ck_assert_uint_eq(0, offset.start);
  ck_assert_uint_eq(0, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(0, offset.diff.origin);
  ck_assert_int_eq(0, offset.diff.tag);
  ck_assert_int_eq(0, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update happening after the current part.
 */
START_TEST(test_align_clear_after) {
  const uint8_t data[] = { 16, 4, 0, 0, 0, 0 };
  const size_t  size   = 6;

  /* Create journal */
  pb_journal_t journal = pb_journal_create(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Clear data from journal */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_clear(&journal, 0, 4, 6));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(4, pb_journal_size(&journal));
  ck_assert_uint_eq(1, pb_journal_version(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 4, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(2, offset.start);
  ck_assert_uint_eq(4, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(-2, offset.diff.origin);
  ck_assert_int_eq(-2, offset.diff.tag);
  ck_assert_int_eq(-1, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update clearing the whole part.
 */
START_TEST(test_align_clear_multiple) {
  const uint8_t data[] = { 16, 4, 0, 0, 0, 0 };
  const size_t  size   = 6;

  /* Create journal */
  pb_journal_t journal = pb_journal_create(data, size);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Clear data from and write data to a journal */
  uint8_t new_data[] = { 255 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_clear(&journal, 2, 2, 6));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_write(&journal, 0, 0, 0, new_data, 1));

  /* Assert journal size and version */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(3, pb_journal_size(&journal));
  ck_assert_uint_eq(2, pb_journal_version(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 4, 6, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(SIZE_MAX, version);
  ck_assert_uint_eq(3, offset.start);
  ck_assert_uint_eq(3, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(0, offset.diff.origin);
  ck_assert_int_eq(0, offset.diff.tag);
  ck_assert_int_eq(0, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset of a specific version according to an invalid journal.
 */
START_TEST(test_align_invalid) {
  pb_journal_t journal = pb_journal_create_invalid();

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 0, 0 };
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(0, version);
  ck_assert_uint_eq(0, offset.start);
  ck_assert_uint_eq(0, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(0, offset.diff.origin);
  ck_assert_int_eq(0, offset.diff.tag);
  ck_assert_int_eq(0, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/* ----------------------------------------------------------------------------
 * Program
 * ------------------------------------------------------------------------- */

/*
 * Create a test suite for all registered test cases and run it.
 *
 * Tests must be run sequentially (in no-fork mode) or code coverage
 * cannot be determined properly.
 */
int
main(void) {
  void *suite = suite_create("protobluff/message/journal"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_empty);
  tcase_add_test(tcase, test_create_zero_copy);
  tcase_add_test(tcase, test_create_invalid);
  tcase_add_test(tcase, test_create_invalid_allocate);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "write" */
  tcase = tcase_create("write");
  tcase_add_test(tcase, test_write);
  tcase_add_test(tcase, test_write_zero_copy);
  tcase_add_test(tcase, test_write_invalid);
  tcase_add_test(tcase, test_write_invalid_allocate);
  tcase_add_test(tcase, test_write_invalid_resize);
  tcase_add_test(tcase, test_write_invalid_zero_copy);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "clear" */
  tcase = tcase_create("clear");
  tcase_add_test(tcase, test_clear);
  tcase_add_test(tcase, test_clear_zero_copy);
  tcase_add_test(tcase, test_clear_invalid);
  tcase_add_test(tcase, test_clear_invalid_allocate);
  tcase_add_test(tcase, test_clear_invalid_resize);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "align" */
  tcase = tcase_create("align");
  tcase_add_test(tcase, test_align);
  tcase_add_test(tcase, test_align_move);
  tcase_add_test(tcase, test_align_move_length);
  tcase_add_test(tcase, test_align_grow);
  tcase_add_test(tcase, test_align_shrink);
  tcase_add_test(tcase, test_align_clear);
  tcase_add_test(tcase, test_align_clear_outside);
  tcase_add_test(tcase, test_align_clear_after);
  tcase_add_test(tcase, test_align_clear_multiple);
  tcase_add_test(tcase, test_align_invalid);
  suite_add_tcase(suite, tcase);

  /* Create a test suite runner in no-fork mode */
  void *runner = srunner_create(suite);
  srunner_set_fork_status(runner, CK_NOFORK);

  /* Execute test suite runner */
  srunner_run_all(runner, CK_NORMAL);
  int failed = srunner_ntests_failed(runner);
  srunner_free(runner);

  /* Exit with status code */
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}