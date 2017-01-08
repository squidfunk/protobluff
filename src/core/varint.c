/*
 * Copyright (c) 2013-2017 Martin Donath <martin.donath@squidfunk.com>
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

#ifdef __SSE2__
#include <emmintrin.h>
#endif /* __SSE2__ */

#include "core/common.h"
#include "core/varint.h"

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Wrapper for __builtin_clz to mitigate undefined behaviour for zero values.
 *
 * Zeroes need one byte on the wire, so we return 31 leading zeroes.
 *
 * \param[in] Value
 * \return    Leading zeroes
 */
#define clz(value) \
  ((value) ? __builtin_clz(value) : 31)

/*!
 * Wrapper for __builtin_clzll to mitigate undefined behaviour for zero values.
 *
 * Zeroes need one byte on the wire, so we return 63 leading zeroes.
 *
 * \param[in] Value
 * \return    Leading zeroes
 */
#define clzll(value) \
  ((value) ? __builtin_clzll(value) : 63)

/* ----------------------------------------------------------------------------
 * Jump tables
 * ------------------------------------------------------------------------- */

/*! Jump table: type ==> size method */
const pb_varint_size_f
pb_varint_size_jump[] = {
  [PB_TYPE_INT32]    = pb_varint_size_int32,
  [PB_TYPE_INT64]    = pb_varint_size_int64,
  [PB_TYPE_UINT32]   = pb_varint_size_uint32,
  [PB_TYPE_UINT64]   = pb_varint_size_uint64,
  [PB_TYPE_SINT32]   = pb_varint_size_sint32,
  [PB_TYPE_SINT64]   = pb_varint_size_sint64,
  [PB_TYPE_FIXED32]  = NULL,
  [PB_TYPE_FIXED64]  = NULL,
  [PB_TYPE_SFIXED32] = NULL,
  [PB_TYPE_SFIXED64] = NULL,
  [PB_TYPE_BOOL]     = pb_varint_size_uint8,
  [PB_TYPE_ENUM]     = pb_varint_size_int32,
  [PB_TYPE_FLOAT]    = NULL,
  [PB_TYPE_DOUBLE]   = NULL,
  [PB_TYPE_STRING]   = NULL,
  [PB_TYPE_BYTES]    = NULL,
  [PB_TYPE_MESSAGE]  = NULL
};

/*! Jump table: type ==> pack method */
const pb_varint_pack_f
pb_varint_pack_jump[] = {
  [PB_TYPE_INT32]    = pb_varint_pack_int32,
  [PB_TYPE_INT64]    = pb_varint_pack_int64,
  [PB_TYPE_UINT32]   = pb_varint_pack_uint32,
  [PB_TYPE_UINT64]   = pb_varint_pack_uint64,
  [PB_TYPE_SINT32]   = pb_varint_pack_sint32,
  [PB_TYPE_SINT64]   = pb_varint_pack_sint64,
  [PB_TYPE_FIXED32]  = NULL,
  [PB_TYPE_FIXED64]  = NULL,
  [PB_TYPE_SFIXED32] = NULL,
  [PB_TYPE_SFIXED64] = NULL,
  [PB_TYPE_BOOL]     = pb_varint_pack_uint8,
  [PB_TYPE_ENUM]     = pb_varint_pack_int32,
  [PB_TYPE_FLOAT]    = NULL,
  [PB_TYPE_DOUBLE]   = NULL,
  [PB_TYPE_STRING]   = NULL,
  [PB_TYPE_BYTES]    = NULL,
  [PB_TYPE_MESSAGE]  = NULL
};

/*! Jump table: type ==> unpack method */
const pb_varint_unpack_f
pb_varint_unpack_jump[] = {
  [PB_TYPE_INT32]    = pb_varint_unpack_int32,
  [PB_TYPE_INT64]    = pb_varint_unpack_int64,
  [PB_TYPE_UINT32]   = pb_varint_unpack_uint32,
  [PB_TYPE_UINT64]   = pb_varint_unpack_uint64,
  [PB_TYPE_SINT32]   = pb_varint_unpack_sint32,
  [PB_TYPE_SINT64]   = pb_varint_unpack_sint64,
  [PB_TYPE_FIXED32]  = NULL,
  [PB_TYPE_FIXED64]  = NULL,
  [PB_TYPE_SFIXED32] = NULL,
  [PB_TYPE_SFIXED64] = NULL,
  [PB_TYPE_BOOL]     = pb_varint_unpack_uint8,
  [PB_TYPE_ENUM]     = pb_varint_unpack_int32,
  [PB_TYPE_FLOAT]    = NULL,
  [PB_TYPE_DOUBLE]   = NULL,
  [PB_TYPE_STRING]   = NULL,
  [PB_TYPE_BYTES]    = NULL,
  [PB_TYPE_MESSAGE]  = NULL
};

/* ----------------------------------------------------------------------------
 * Mappings
 * ------------------------------------------------------------------------- */

/*! Mapping: most-significant bit ==> packed size */
static const size_t
size_map[] = {
  1, 1, 1, 1, 1, 1, 1, 2,
  2, 2, 2, 2, 2, 2, 3, 3,
  3, 3, 3, 3, 3, 4, 4, 4,
  4, 4, 4, 4, 5, 5, 5, 5,
  5, 5, 5, 6, 6, 6, 6, 6,
  6, 6, 7, 7, 7, 7, 7, 7,
  7, 8, 8, 8, 8, 8, 8, 8,
  9, 9, 9, 9, 9, 9, 9, 10
};

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the packed size of a signed 32-bit variable-sized integer.
 *
 * \warning Negative 32-bit and 64-bit integers are always encoded as negative
 * 64-bit integers, so the result will always be ten bytes long.
 *
 * \param[in] value Pointer holding value
 * \return          Packed size
 */
extern size_t
pb_varint_size_int32(const void *value) {
  assert(value);
  return *(const int32_t *)value >= 0
    ? pb_varint_size_uint32(value)
    : 10;
}

/*!
 * Retrieve the packed size of a signed 64-bit variable-sized integer.
 *
 * \param[in] value Pointer holding value
 * \return          Packed size
 */
extern size_t
pb_varint_size_int64(const void *value) {
  return pb_varint_size_uint64(value);
}

/*!
 * Retrieve the packed size of an unsigned 8-bit variable-sized integer.
 *
 * \param[in] value Pointer holding value
 * \return          Packed size
 */
extern size_t
pb_varint_size_uint8(const void *value) {
  assert(value);
  return 1 + (*(const uint8_t *)value > 127);
}

/*!
 * Retrieve the packed size of an unsigned 32-bit variable-sized integer.
 *
 * \param[in] value Pointer holding value
 * \return          Packed size
 */
extern size_t
pb_varint_size_uint32(const void *value) {
  assert(value);
  return size_map[31 - clz(*(const uint32_t *)value)];
}

/*!
 * Retrieve the packed size of an unsigned 64-bit variable-sized integer.
 *
 * \param[in] value Pointer holding value
 * \return          Packed size
 */
extern size_t
pb_varint_size_uint64(const void *value) {
  assert(value);
  return size_map[63 - clzll(*(const uint64_t *)value)];
}

/*!
 * Retrieve the packed size of a signed 32-bit variable-sized integer.
 *
 * This function returns the number of bytes in base-128 representation for
 * packing the integer in zig-zag encoding.
 *
 * \param[in] value Pointer holding value
 * \return          Packed size
 */
extern size_t
pb_varint_size_sint32(const void *value) {
  assert(value);
  return size_map[31 - clz(
      ((*(const int32_t *)value) << 1)
    ^ ((*(const int32_t *)value) >> 31))];
}

/*!
 * Retrieve the packed size of a signed 64-bit variable-sized integer.
 *
 * This function returns the number of bytes in base-128 representation for
 * writing the integer in zig-zag encoding.
 *
 * \param[in] value Pointer holding value
 * \return          Packed size
 */
extern size_t
pb_varint_size_sint64(const void *value) {
  assert(value);
  return size_map[63 - clzll(
      ((*(const int64_t *)value) << 1)
    ^ ((*(const int64_t *)value) >> 63))];
}

/* ------------------------------------------------------------------------- */

/*!
 * Pack a signed 32-bit variable-sized integer.
 *
 * \warning Negative 32-bit and 64-bit integers are always encoded as negative
 * 64-bit integers, so the result will always be ten bytes long.
 *
 * \warning The caller has to ensure that the buffer is appropriately sized for
 * the base-128 representation of the value.
 *
 * \param[out] data[] Target buffer
 * \param[in]  value  Pointer holding value
 * \return            Packed size
 */
extern size_t
pb_varint_pack_int32(uint8_t data[], const void *value) {
  assert(data && value);
  int32_t temp = *(const int32_t *)value;
  if (temp < 0) {
    data[0] = temp | 0x80;
    data[1] = (temp >>  7) | 0x80;
    data[2] = (temp >> 14) | 0x80;
    data[3] = (temp >> 21) | 0x80;
    data[4] = (temp >> 28) | 0x80;
    data[5] = data[6] = data[7] = data[8] = 0xFF;
    data[9] = 0x01;
    return 10;
  }
  return pb_varint_pack_uint32(data, value);
}

/*!
 * Pack a signed 64-bit variable-sized integer.
 *
 * \warning Negative 32-bit and 64-bit integers are always encoded as negative
 * 64-bit integers, so the result will always be ten bytes long.
 *
 * \warning The caller has to ensure that the buffer is appropriately sized for
 * the base-128 representation of the value.
 *
 * \param[out] data[] Target buffer
 * \param[in]  value  Pointer holding value
 * \return            Packed size
 */
extern size_t
pb_varint_pack_int64(uint8_t data[], const void *value) {
  return pb_varint_pack_uint64(data, value);
}

/*!
 * Pack an unsigned 8-bit variable-sized integer.
 *
 * \warning The caller has to ensure that the buffer is appropriately sized for
 * the base-128 representation of the value.
 *
 * \param[out] data[] Target buffer
 * \param[in]  value  Pointer holding value
 * \return            Packed size
 */
extern size_t
pb_varint_pack_uint8(uint8_t data[], const void *value) {
  assert(data && value);
  size_t size = 0; uint8_t temp = *(const uint8_t *)value;
  if (temp & 0x80) {
    data[size++] = temp | 0x80;
    temp >>= 7;
  }
  data[size++] = temp;
  return size;
}

/*!
 * Pack an unsigned 32-bit variable-sized integer.
 *
 * The number checked with the bitwise AND remains the same for reasons of
 * efficiency, saving a lot of unnecessary MOV instructions. The loop is fully
 * unrolled, resulting in a speed up of roughly 25% in code compiled with -O3.
 *
 * \warning The caller has to ensure that the buffer is appropriately sized for
 * the base-128 representation of the value.
 *
 * \param[out] data[] Target buffer
 * \param[in]  value  Pointer holding value
 * \return            Packed size
 */
extern size_t
pb_varint_pack_uint32(uint8_t data[], const void *value) {
  assert(data && value);
  size_t size = 0; uint32_t temp = *(const uint32_t *)value;
  if (temp & 0xFFFFFF80U) {
    data[size++] = temp | 0x80;
    temp >>= 7;
    if (temp & 0xFFFFFF80U) {
      data[size++] = temp | 0x80;
      temp >>= 7;
      if (temp & 0xFFFFFF80U) {
        data[size++] = temp | 0x80;
        temp >>= 7;
        if (temp & 0xFFFFFF80U) {
          data[size++] = temp | 0x80;
          temp >>= 7;
        }
      }
    }
  }
  data[size++] = temp;
  return size;
}

/*!
 * Pack an unsigned 64-bit variable-sized integer.
 *
 * The number checked with the bitwise AND remains the same for reasons of
 * efficiency, saving a lot of unnecessary MOV instructions. The loop is fully
 * unrolled, resulting in a speed up of roughly 25% in code compiled with -O3.
 *
 * \warning The caller has to ensure that the buffer is appropriately sized for
 * the base-128 representation of the value.
 *
 * \param[out] data[] Target buffer
 * \param[in]  value  Pointer holding value
 * \return            Packed size
 */
extern size_t
pb_varint_pack_uint64(uint8_t data[], const void *value) {
  assert(data && value);
  size_t size = 0; uint64_t temp = *(const uint64_t *)value;
  if (temp & 0xFFFFFFFFFFFFFF80ULL) {
    data[size++] = temp | 0x80;
    temp >>= 7;
    if (temp & 0xFFFFFFFFFFFFFF80ULL) {
      data[size++] = temp | 0x80;
      temp >>= 7;
      if (temp & 0xFFFFFFFFFFFFFF80ULL) {
        data[size++] = temp | 0x80;
        temp >>= 7;
        if (temp & 0xFFFFFFFFFFFFFF80ULL) {
          data[size++] = temp | 0x80;
          temp >>= 7;
          if (temp & 0xFFFFFFFFFFFFFF80ULL) {
            data[size++] = temp | 0x80;
            temp >>= 7;
            if (temp & 0xFFFFFFFFFFFFFF80ULL) {
              data[size++] = temp | 0x80;
              temp >>= 7;
              if (temp & 0xFFFFFFFFFFFFFF80ULL) {
                data[size++] = temp | 0x80;
                temp >>= 7;
                if (temp & 0xFFFFFFFFFFFFFF80ULL) {
                  data[size++] = temp | 0x80;
                  temp >>= 7;
                  if (temp & 0xFFFFFFFFFFFFFF80ULL) {
                    data[size++] = temp | 0x80;
                    temp >>= 7;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  data[size++] = temp;
  return size;
}

/*!
 * Pack a signed 32-bit variable-sized integer in zig-zag encoding.
 *
 * \warning The caller has to ensure that the buffer is appropriately sized for
 * the base-128 representation of the value.
 *
 * \param[out] data[] Target buffer
 * \param[in]  value  Pointer holding value
 * \return            Packed size
 */
extern size_t
pb_varint_pack_sint32(uint8_t data[], const void *value) {
  assert(data && value);
  uint32_t temp = ((*(const int32_t *)value) << 1)
                ^ ((*(const int32_t *)value) >> 31);
  return pb_varint_pack_uint32(data, &temp);
}

/*!
 * Pack a signed 64-bit variable-sized integer in zig-zag encoding.
 *
 * \warning The caller has to ensure that the buffer is appropriately sized for
 * the base-128 representation of the value.
 *
 * \param[out] data[] Target buffer
 * \param[in]  value  Pointer holding value
 * \return            Packed size
 */
extern size_t
pb_varint_pack_sint64(uint8_t data[], const void *value) {
  assert(data && value);
  uint64_t temp = ((*(const int64_t *)value) << 1)
                ^ ((*(const int64_t *)value) >> 63);
  return pb_varint_pack_uint64(data, &temp);
}

/* ------------------------------------------------------------------------- */

/*!
 * Scan a buffer for a valid variable-sized integer.
 *
 * This function checks if an underrun might happen reading a variable-sized
 * integer from a buffer. Only underruns can be checked using this method,
 * overflows may still happen, but are properly reported by the unpack
 * functions. SSE2 intrinsics are used if the compiler supports it.
 *
 * The SSE2 version works in constant time and independent of input length.
 *
 * \warning The SSE2 version is implemented for up to 10 bytes of length, so if
 * an encoded integer with more than 10 high-bits is encountered, an underrun
 * is falsely reported. This could be omitted by introducing a further branch
 * checking if the buffer is larger than 10 bytes and falling back to the
 * non-SSE version, but as it's an entirely internal case that is properly
 * handled by all invoking functions there is no necessity.
 *
 * \warning Using valgrind for tracking down invalid memory accesses, memcheck
 * will complain when it sees loads on data that extend to uninitialized memory
 * regions. This, however, is a false-positive, as uninitialized values are
 * singled out using the mapped bitmask.
 *
 * \param[in] data[] Source buffer
 * \param[in] left   Remaining bytes
 * \return           Bytes read
 */
extern size_t
pb_varint_scan(const uint8_t data[], size_t left) {
  assert(data && left);
  left = left > 10 ? 10 : left;

#ifdef __SSE2__

  /* Mapping: remaining bytes ==> bitmask */
  static const int mask_map[] = {
    0x0000, 0x0001, 0x0003, 0x0007,
    0x000F, 0x001F, 0x003F, 0x007F,
    0x00FF, 0x01FF, 0x03FF
  };

  /* Load buffer into 128-bit integer and create high-bit mask */
  __m128i temp = _mm_loadu_si128((const __m128i *)data);
  __m128i high = _mm_set1_epi8(0x80);

  /* Intersect and extract mask with high-bits set */
  int mask = _mm_movemask_epi8(_mm_and_si128(temp, high));
  mask = (mask & mask_map[left]) ^ mask_map[left];

  /* Count trailing zeroes */
  return mask ? __builtin_ctz(mask) + 1 : 0;

#else

  /* Linear scan */
  size_t size = 0;
  while (data[size++] & 0x80)
    if (!--left)
      return 0;
  return size;

#endif /* __SSE2__ */

}

/* ------------------------------------------------------------------------- */

/*!
 * Unpack a signed 32-bit variable-sized integer.
 *
 * If the unpacking function returns with zero, an overflow happened, which
 * indicates that a negative integer was encountered.
 *
 * \warning Negative 32-bit and 64-bit integers are always encoded as negative
 * 64-bit integers, so the result will always be ten bytes long.
 *
 * \warning The caller has to ensure that the buffer may not underrun and the
 * space pointed to by the value pointer is appropriately sized.
 *
 * \param[in]  data[] Source buffer
 * \param[in]  left   Remaining bytes
 * \param[out] value  Pointer receiving value
 * \return            Bytes read
 */
extern size_t
pb_varint_unpack_int32(const uint8_t data[], size_t left, void *value) {
  size_t size = pb_varint_unpack_uint32(data, left, value);
  return size ? size :
    data[4] & 0x80 && data[5] == 0xFF ? 10 : 0;
}

/*!
 * Unpack a signed 64-bit variable-sized integer.
 *
 * \warning The caller has to ensure that the buffer may not underrun and the
 * space pointed to by the value pointer is appropriately sized.
 *
 * \param[in]  data[] Source buffer
 * \param[in]  left   Remaining bytes
 * \param[out] value  Pointer receiving value
 * \return            Bytes read
 */
extern size_t
pb_varint_unpack_int64(const uint8_t data[], size_t left, void *value) {
  return pb_varint_unpack_uint64(data, left, value);
}

/*!
 * Unpack an unsigned 8-bit variable-sized integer.
 *
 * \warning The caller has to ensure that the buffer may not underrun and the
 * space pointed to by the value pointer is appropriately sized.
 *
 * \param[in]  data[] Source buffer
 * \param[in]  left   Remaining bytes
 * \param[out] value  Pointer receiving value
 * \return            Bytes read
 */
extern size_t
pb_varint_unpack_uint8(const uint8_t data[], size_t left, void *value) {
  assert(data && left && value);
  size_t size = 0; uint8_t temp = (data[size] & 0x7F);
  if (data[size++] & 0x80 && --left)
    temp |= (data[size++] & 0x7F) << 7;
  *(uint8_t *)value = temp;
  return !left || data[size - 1] & 0x80 ? 0 : size;
}

/*!
 * Unpack an unsigned 32-bit variable-sized integer.
 *
 * \warning The caller has to ensure that the buffer may not underrun and the
 * space pointed to by the value pointer is appropriately sized.
 *
 * \param[in]  data[] Source buffer
 * \param[in]  left   Remaining bytes
 * \param[out] value  Pointer receiving value
 * \return            Bytes read
 */
extern size_t
pb_varint_unpack_uint32(const uint8_t data[], size_t left, void *value) {
  assert(data && left && value);
  size_t size = 0; uint32_t temp = (uint32_t)(data[size] & 0x7F);
  if (data[size++] & 0x80 && --left) {
    temp |= (uint32_t)(data[size] & 0x7F) << 7;
    if (data[size++] & 0x80 && --left) {
      temp |= (uint32_t)(data[size] & 0x7F) << 14;
      if (data[size++] & 0x80 && --left) {
        temp |= (uint32_t)(data[size] & 0x7F) << 21;
        if (data[size++] & 0x80 && --left) {
          temp |= (uint32_t)(data[size++]) << 28;
        }
      }
    }
  }
  *(uint32_t *)value = temp;
  return !left || data[size - 1] & 0x80 ? 0 : size;
}

/*!
 * Unpack an unsigned 64-bit variable-sized integer.
 *
 * \warning The caller has to ensure that the buffer may not underrun and the
 * space pointed to by the value pointer is appropriately sized.
 *
 * \param[in]  data[] Source buffer
 * \param[in]  left   Remaining bytes
 * \param[out] value  Pointer receiving value
 * \return            Bytes read
 */
extern size_t
pb_varint_unpack_uint64(const uint8_t data[], size_t left, void *value) {
  assert(data && left && value);
  size_t size = 0; uint64_t temp = (uint64_t)(data[size] & 0x7F);
  if (data[size++] & 0x80 && --left) {
    temp |= (uint64_t)(data[size] & 0x7F) << 7;
    if (data[size++] & 0x80 && --left) {
      temp |= (uint64_t)(data[size] & 0x7F) << 14;
      if (data[size++] & 0x80 && --left) {
        temp |= (uint64_t)(data[size] & 0x7F) << 21;
        if (data[size++] & 0x80 && --left) {
          temp |= (uint64_t)(data[size] & 0x7F) << 28;
          if (data[size++] & 0x80 && --left) {
            temp |= (uint64_t)(data[size] & 0x7F) << 35;
            if (data[size++] & 0x80 && --left) {
              temp |= (uint64_t)(data[size] & 0x7F) << 42;
              if (data[size++] & 0x80 && --left) {
                temp |= (uint64_t)(data[size] & 0x7F) << 49;
                if (data[size++] & 0x80 && --left) {
                  temp |= (uint64_t)(data[size] & 0x7F) << 56;
                  if (data[size++] & 0x80 && --left) {
                    temp |= (uint64_t)(data[size++]) << 63;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  *(uint64_t *)value = temp;
  return !left || data[size - 1] & 0x80 ? 0 : size;
}

/*!
 * Unpack a signed 32-bit variable-sized integer in zig-zag encoding.
 *
 * \warning The caller has to ensure that the buffer may not underrun and the
 * space pointed to by the value pointer is appropriately sized.
 *
 * \param[in]  data[] Source buffer
 * \param[in]  left   Remaining bytes
 * \param[out] value  Pointer receiving value
 * \return            Bytes read
 */
extern size_t
pb_varint_unpack_sint32(const uint8_t data[], size_t left, void *value) {
  assert(data && left && value);
  uint32_t temp; size_t size = pb_varint_unpack_uint32(data, left, &temp);
  *(int32_t *)value = (int32_t)((temp >> 1) ^ -(temp & 1));
  return size;
}

/*!
 * Unpack a signed 64-bit variable-sized integer in zig-zag encoding.
 *
 * \warning The caller has to ensure that the buffer may not underrun and the
 * space pointed to by the value pointer is appropriately sized.
 *
 * \param[in]  data[] Source buffer
 * \param[in]  left   Remaining bytes
 * \param[out] value  Pointer receiving value
 * \return            Bytes read
 */
extern size_t
pb_varint_unpack_sint64(const uint8_t data[], size_t left, void *value) {
  assert(data && left && value);
  uint64_t temp; size_t size = pb_varint_unpack_uint64(data, left, &temp);
  *(int64_t *)value = (int64_t)((temp >> 1) ^ -(temp & 1));
  return size;
}
