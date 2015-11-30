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

#include "core/allocator.h"

/* ----------------------------------------------------------------------------
 * System-default allocator callbacks
 * ------------------------------------------------------------------------- */

/*!
 * Allocate a memory block of given size.
 *
 * \param[in,out] data Internal allocator data
 * \param[in]     size Bytes to be allocated
 * \return             Memory block
 */
static void *
allocator_allocate(void *data, size_t size) {
  assert(!data && size);
  return malloc(size);
}

/*!
 * Change the size of a previously allocated memory block.
 *
 * \param[in,out] data  Internal allocator data
 * \param[in,out] block Memory block to be resized
 * \param[in]     size  Bytes to be allocated
 * \return              Memory block
 */
static void *
allocator_resize(void *data, void *block, size_t size) {
  assert(!data && size);
  return realloc(block, size);
}

/*!
 * Free an allocated memory block.
 *
 * \param[in,out] data  Internal allocator data
 * \param[in,out] block Memory block to be freed
 */
static void
allocator_free(void *data, void *block) {
  assert(!data && block);
  free(block);
}

/* ----------------------------------------------------------------------------
 * Allocators
 * ------------------------------------------------------------------------- */

/*! System-default allocator */
pb_allocator_t
allocator_default = {
  .proc = {
    .allocate = allocator_allocate,
    .resize   = allocator_resize,
    .free     = allocator_free
  },
  .data = NULL
};

/*! Zero-copy fake-allocator */
pb_allocator_t
allocator_zero_copy = {};