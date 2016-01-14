## Interface

The interface for the allocator that needs to be implemented is defined in
`allocator.h`:

``` c
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

typedef struct pb_allocator_t {
  struct {
    pb_allocator_alloc_f alloc;        /*!< Allocate a memory block */
    pb_allocator_realloc_f realloc;    /*!< Resize a memory block */
    pb_allocator_free_f free;          /*!< Free a memory block */
  } proc;
  void *data;                          /*!< Internal allocator data */
} pb_allocator_t;
```

Custom allocators can be used with buffers, journals and encoders.

## Implementations

### Default allocator

The default allocator is globally defined through the variable
`allocator_default`.

### Chunk allocator

Starting with protobluff 0.2.2, a chunk allocator implementation is included
in the full runtime. Memory blocks are stored in pre-allocated chunks to
account for later growth by allocating more space than is needed upfront. If
the block grows inside the corresponding chunk's capacity, the same block is
returned. Otherwise, the chunk is doubled in size to make room for more growth:

``` c
pb_allocator_t allocator = pb_allocator_chunk_create();
pb_allocator_t allocator = pb_allocator_chunk_create_with_capacity(1024);
...
pb_allocator_chunk_destroy(&allocator);
```