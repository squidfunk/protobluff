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

#ifndef PB_INCLUDE_ALLOCATOR_H
#define PB_INCLUDE_ALLOCATOR_H

#include <assert.h>
#include <stddef.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef void *
(*pb_allocator_alloc_f)(
  void *data,                          /*!< Internal allocator data */
  size_t size);                        /*!< Bytes to be allocated */

typedef void *
(*pb_allocator_realloc_f)(
  void *data,                          /*!< Internal allocator data */
  void *block,                         /*!< Memory block to be resized */
  size_t size);                        /*!< Bytes to be allocated */

typedef void
(*pb_allocator_free_f)(
  void *data,                          /*!< Internal allocator data */
  void *block);                        /*!< Memory block to be freed */

/* ------------------------------------------------------------------------- */

typedef struct pb_allocator_t {
  struct {
    pb_allocator_alloc_f alloc;        /*!< Allocate a memory block */
    pb_allocator_realloc_f realloc;    /*!< Resize a memory block */
    pb_allocator_free_f free;          /*!< Free a memory block */
  } proc;
  void *data;                          /*!< Internal allocator data */
} pb_allocator_t;

/* ----------------------------------------------------------------------------
 * Extern declarations
 * ------------------------------------------------------------------------- */

extern pb_allocator_t
allocator_default;                     /*!< System-default allocator */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Allocate a memory block of given size.
 *
 * \param[in,out] allocator Allocator
 * \param[in]     size      Bytes to be allocated
 * \return                  Memory block
 */
#define pb_allocator_alloc(allocator, size) \
  (assert((allocator) && (size)), \
    (allocator)->proc.alloc((allocator)->data, (size)))

/*!
 * Change the size of a previously allocated memory block.
 *
 * \param[in,out] allocator Allocator
 * \param[in,out] block     Memory block to be resized
 * \param[in]     size      Bytes to be allocated
 * \return                  Memory block
 */
#define pb_allocator_realloc(allocator, block, size) \
  (assert((allocator) && (size)), \
    (allocator)->proc.realloc((allocator)->data, (block), (size)))

/*!
 * Free an allocated memory block.
 *
 * \param[in,out] allocator Allocator
 * \param[in,out] block     Memory block to be freed
 */
#define pb_allocator_free(allocator, block) \
  (assert((allocator) && (block)), \
    (allocator)->proc.free((allocator)->data, (block)))

#endif /* PB_INCLUDE_ALLOCATOR_H */