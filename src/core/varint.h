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

#ifndef PB_CORE_VARINT_H
#define PB_CORE_VARINT_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/common.h"

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef size_t
(*pb_varint_size_f)(
  const void *value);                  /*!< Pointer holding value */

typedef size_t
(*pb_varint_pack_f)(
  uint8_t data[],                      /*!< Target buffer */
  const void *value);                  /*!< Pointer holding value */

typedef size_t
(*pb_varint_unpack_f)(
  const uint8_t data[],                /*!< Source buffer */
  size_t left,                         /*!< Remaining bytes */
  void *value);                        /*!< Pointer receiving value */

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

extern size_t
pb_varint_size_int32(
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_size_int64(
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_size_uint8(
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_size_uint32(
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_size_uint64(
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_size_sint32(
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_size_sint64(
  const void *value);                  /* Pointer holding value */

/* ------------------------------------------------------------------------- */

extern size_t
pb_varint_pack_int32(
  uint8_t data[],                      /* Target buffer */
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_pack_int64(
  uint8_t data[],                      /* Target buffer */
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_pack_uint8(
  uint8_t data[],                      /* Target buffer */
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_pack_uint32(
  uint8_t data[],                      /* Target buffer */
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_pack_uint64(
  uint8_t data[],                      /* Target buffer */
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_pack_sint32(
  uint8_t data[],                      /* Target buffer */
  const void *value);                  /* Pointer holding value */

extern size_t
pb_varint_pack_sint64(
  uint8_t data[],                      /* Target buffer */
  const void *value);                  /* Pointer holding value */

/* ------------------------------------------------------------------------- */

extern size_t
pb_varint_scan(
  const uint8_t data[],                /* Source buffer */
  size_t left);                        /* Remaining bytes */

/* ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern size_t
pb_varint_unpack_int32(
  const uint8_t data[],                /* Source buffer */
  size_t left,                         /* Remaining bytes */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern size_t
pb_varint_unpack_int64(
  const uint8_t data[],                /* Source buffer */
  size_t left,                         /* Remaining bytes */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern size_t
pb_varint_unpack_uint8(
  const uint8_t data[],                /* Source buffer */
  size_t left,                         /* Remaining bytes */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern size_t
pb_varint_unpack_uint32(
  const uint8_t data[],                /* Source buffer */
  size_t left,                         /* Remaining bytes */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern size_t
pb_varint_unpack_uint64(
  const uint8_t data[],                /* Source buffer */
  size_t left,                         /* Remaining bytes */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern size_t
pb_varint_unpack_sint32(
  const uint8_t data[],                /* Source buffer */
  size_t left,                         /* Remaining bytes */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern size_t
pb_varint_unpack_sint64(
  const uint8_t data[],                /* Source buffer */
  size_t left,                         /* Remaining bytes */
  void *value);                        /* Pointer receiving value */

/* ----------------------------------------------------------------------------
 * Jump tables
 * ------------------------------------------------------------------------- */

/*! Jump table: type ==> size method */
extern const pb_varint_size_f
pb_varint_size_jump[];

/*! Jump table: type ==> pack method */
extern const pb_varint_pack_f
pb_varint_pack_jump[];

/*! Jump table: type ==> unpack method */
extern const pb_varint_unpack_f
pb_varint_unpack_jump[];

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the packed size of a variable-sized integer of given type.
 *
 * \param[in] type  Type
 * \param[in] value Pointer holding value
 * \return          Packed size
 */
PB_INLINE size_t
pb_varint_size(pb_type_t type, const void *value) {
  assert(pb_varint_size_jump[type]);
  return pb_varint_size_jump[type](value);
}

/*!
 * Pack a variable-sized integer of given type.
 *
 * \param[in]  type   Type
 * \param[out] data[] Target buffer
 * \param[in]  value  Pointer holding value
 * \return            Packed size
 */
PB_INLINE size_t
pb_varint_pack(pb_type_t type, uint8_t data[], const void *value) {
  assert(pb_varint_pack_jump[type]);
  return pb_varint_pack_jump[type](data, value);
}

/*!
 * Unpack a variable-sized integer of given type.
 *
 * \param[in]  type   Type
 * \param[in]  data[] Source buffer
 * \param[in]  left   Remaining bytes
 * \param[out] value  Pointer receiving value
 * \return            Bytes read
 */
PB_WARN_UNUSED_RESULT
PB_INLINE size_t
pb_varint_unpack(
    pb_type_t type, const uint8_t data[], size_t left, void *value) {
  assert(pb_varint_unpack_jump[type]);
  return pb_varint_unpack_jump[type](data, left, value);
}

#endif /* PB_CORE_VARINT_H */