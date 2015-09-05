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

#include "lib/allocator/chunk.h"
#include "lib/common.h"

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_allocator_chunk_t {
  struct pb_allocator_chunk_t *next;   /*!< Next linked chunk */
  size_t capacity;                     /*!< Capacity */
} pb_allocator_chunk_t;

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the memory block from an allocated chunk.
 *
 * \param[in] chunk Allocated chunk
 * \return          Memory block
 */
#define block_from_chunk(chunk) \
  (assert(chunk), (void *)((uint8_t *)(chunk) + \
    sizeof(pb_allocator_chunk_t)))

/*!
 * Retrieve the allocated chunk for a memory block.
 *
 * \param[in] block Memory block
 * \return          Allocated chunk
 */
#define chunk_from_block(block) \
  (assert(block), (void *)((uint8_t *)(block) - \
    sizeof(pb_allocator_chunk_t)))

/* ----------------------------------------------------------------------------
 * Allocator callbacks
 * ------------------------------------------------------------------------- */

/*!
 * Allocate a memory block of given size.
 *
 * Memory blocks are stored in pre-allocated chunks to account for later growth
 * by allocating more space than is needed upfront. If the block grows inside
 * the corresponding chunk's capacity, the same block is returned. Otherwise,
 * the chunk is doubled in size to make room for more growth.
 *
 * Chunks are managed through a linked list. New chunks are always appended to
 * the end of the list. If a chunk is freed, it is removed from the list and
 * its predecessor and successor are linked. The first chunk does not store
 * any data, it just holds the initial capacity and the head of the list.
 *
 * \warning The lines excluded from code coverage cannot be triggered within
 * the tests, as we're not going to wrap malloc for that.
 *
 * \param[in,out] data Internal allocator data
 * \param[in]     size Bytes to be allocated
 * \return             Memory block
 */
static void *
allocator_alloc(void *data, size_t size) {
  assert(size);
  if (unlikely_(!data))
    return NULL;

  /* Search to end of chunk list */
  pb_allocator_chunk_t *base = data;
  pb_allocator_chunk_t *head = data;
  while (head->next)
    head = head->next;

  /* Calculate necessary capacity */
  size_t capacity = base->capacity * (size / base->capacity + 1);

  /* Allocate chunk and link in predecessor */
  pb_allocator_chunk_t *chunk = malloc(
    sizeof(pb_allocator_chunk_t) + capacity);
  if (chunk) {
    *chunk = (pb_allocator_chunk_t){
      .next     = NULL,
      .capacity = capacity
    };
    head->next = chunk;

    /* Return freshly allocated block */
    return block_from_chunk(head->next);
  }

  /* It's not for you, mate */
  return NULL;                                             /* LCOV_EXCL_LINE */
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
allocator_realloc(void *data, void *block, size_t size) {
  assert(size);
  if (unlikely_(!data))
    return NULL;

  /* Change size of given memory block */
  if (block) {
    pb_allocator_chunk_t *base    = data;
    pb_allocator_chunk_t *current = chunk_from_block(block);

    /* Check if chunk's capacity is exhausted */
    if (unlikely_(size > current->capacity)) {
      pb_allocator_chunk_t *head = data;
      while (head->next && head->next != current)
        head = head->next;

      /* Calculate new capacity */
      size_t capacity = base->capacity * (size / base->capacity + 1);

      /* Grow chunk and link in predecessor */
      pb_allocator_chunk_t *chunk = realloc(head->next,
        sizeof(pb_allocator_chunk_t) + capacity);
      if (chunk) {
        head->next = chunk;
        head->next->capacity = capacity;

        /* Return resized block */
        return block_from_chunk(head->next);
      }
    }

  /* Allocate new block */
  } else {
    return allocator_alloc(data, size);
  }

  /* Nothing to be done */
  return block;
}

/*!
 * Free an allocated memory block.
 *
 * \param[in,out] data  Internal allocator data
 * \param[in,out] block Memory block to be freed
 */
static void
allocator_free(void *data, void *block) {
  assert(block);
  if (unlikely_(!data))
    return;

  /* Search current chunk in chunk list */
  pb_allocator_chunk_t *head    = data;
  pb_allocator_chunk_t *current = chunk_from_block(block);
  while (head->next && head->next != current)
    head = head->next;

  /* Link predecessor and successor and free current chunk */
  head->next = current->next;
  free(current);
}

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a chunk allocator.
 *
 * \return Chunk allocator
 */
extern pb_allocator_t
pb_allocator_chunk_create() {
  return pb_allocator_chunk_create_with_capacity(4096);
}

/*!
 * Create a chunk allocator with a minimum capacity.
 *
 * \param[in] capacity Capacity
 * \return             Chunk allocator
 */
extern pb_allocator_t
pb_allocator_chunk_create_with_capacity(size_t capacity) {
  assert(capacity);
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_alloc,
      .realloc = allocator_realloc,
      .free    = allocator_free
    },
    .data = NULL
  };
  pb_allocator_chunk_t *base = malloc(sizeof(pb_allocator_chunk_t));
  if (base) {
    *base = (pb_allocator_chunk_t){
      .next     = NULL,
      .capacity = capacity
    };
    allocator.data = base;
  }
  return allocator;
}

/*!
 * Destroy a chunk allocator.
 *
 * The chunk allocator will automatically free all memory blocks that are
 * still allocated upon destruction.
 *
 * \param[in,out] allocator Chunk allocator
 */
extern void
pb_allocator_chunk_destroy(pb_allocator_t *allocator) {
  assert(allocator);
  pb_allocator_chunk_t *base = allocator->data;
  if (base) {
    pb_allocator_chunk_t *head;
    while ((head = base->next) && base->next) {
      base->next = base->next->next;
      free(head);
    }
    free(base);
  }
}