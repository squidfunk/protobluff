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

#ifndef PB_INCLUDE_COMMON_H
#define PB_INCLUDE_COMMON_H

#include <stddef.h>
#include <stdint.h>

/* ----------------------------------------------------------------------------
 * Application binary interface
 * ------------------------------------------------------------------------- */

/*
 * Agnostic C-linkage classifier for extern functions when compiling from C++
 * or other languages for ABI-compatibility.
 */
#ifdef __cplusplus
  #define _C_ "C"
#else
  #define _C_
#endif

/*
 * Only expose exported symbols in the shared library for maximum optimization
 * capability, unless the compiler doesn't support it.
 */
#if defined(__GNUC__) && __GNUC__ >= 4
  #define PB_EXPORT extern _C_ __attribute__((visibility("default")))
#else
  #define PB_EXPORT
#endif

/*
 * Declare a function to be automatically run upon application start up.
 * Heavily used for Protocol Buffers extensions.
 */
#if defined(__GNUC__) && __GNUC__ >= 4
  #define PB_CONSTRUCTOR __attribute__((constructor))
#else
  #define PB_CONSTRUCTOR
  #warning "Automatic initialization of extensions not supported"
#endif

/* ----------------------------------------------------------------------------
 * Compiler attributes
 * ------------------------------------------------------------------------- */

/*
 * If the compiler supports it, check that all return values are propagated
 * correctly. Otherwise just declare the annotation void.
 */
#if defined(__GNUC__) && __GNUC__ >= 4
  #define PB_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
  #define PB_WARN_UNUSED_RESULT
#endif

/* ----------------------------------------------------------------------------
 * Enumerations
 * ------------------------------------------------------------------------- */

typedef enum pb_error_t {
  PB_ERROR_NONE,                       /*!< None (default) */
  PB_ERROR_ALLOC,                      /*!< Allocation failed */
  PB_ERROR_INVALID,                    /*!< Invalid arguments or data */
  PB_ERROR_DESCRIPTOR,                 /*!< Invalid descriptor */
  PB_ERROR_WIRETYPE,                   /*!< Invalid wiretype */
  PB_ERROR_OVERFLOW,                   /*!< Read more bytes than expected */
  PB_ERROR_UNDERRUN,                   /*!< Read less bytes than expected */
  PB_ERROR_OFFSET,                     /*!< Invalid offset */
  PB_ERROR_ABSENT                      /*!< Absent field or value */
} pb_error_t;

/* ------------------------------------------------------------------------- */

typedef enum pb_wiretype_t {
  PB_WIRETYPE_VARINT = 0,              /*!< Variable-sized integer */
  PB_WIRETYPE_64BIT  = 1,              /*!< Fixed 64-bit value */
  PB_WIRETYPE_LENGTH = 2,              /*!< Length-prefixed */
  PB_WIRETYPE_32BIT  = 5               /*!< Fixed 32-bit value */
} pb_wiretype_t;

typedef enum pb_type_t {
  PB_TYPE_UNDEFINED,                   /*!< Undefined (default) */
  PB_TYPE_INT32,                       /*!< 32-bit integer, var., signed */
  PB_TYPE_INT64,                       /*!< 64-bit integer, var., signed */
  PB_TYPE_UINT32,                      /*!< 32-bit integer, var., unsigned */
  PB_TYPE_UINT64,                      /*!< 32-bit integer, var., unsigned */
  PB_TYPE_SINT32,                      /*!< 32-bit integer, var., signed */
  PB_TYPE_SINT64,                      /*!< 64-bit integer, var., signed */
  PB_TYPE_FIXED32,                     /*!< 32-bit integer, fix., unsigned */
  PB_TYPE_FIXED64,                     /*!< 64-bit integer, fix., unsigned */
  PB_TYPE_SFIXED32,                    /*!< 32-bit integer, fix., signed */
  PB_TYPE_SFIXED64,                    /*!< 64-bit integer, fix., signed */
  PB_TYPE_BOOL,                        /*!< Boolean, var. */
  PB_TYPE_ENUM,                        /*!< Enumeration, var. */
  PB_TYPE_FLOAT,                       /*!< Single-precison float */
  PB_TYPE_DOUBLE,                      /*!< Double-precison float */
  PB_TYPE_STRING,                      /*!< String */
  PB_TYPE_BYTES,                       /*!< Byte array */
  PB_TYPE_MESSAGE                      /*!< Message */
} pb_type_t;

typedef enum pb_label_t {
  PB_LABEL_UNDEFINED,                  /*!< Undefined (default) */
  PB_LABEL_REQUIRED,                   /*!< Required field */
  PB_LABEL_OPTIONAL,                   /*!< Optional field */
  PB_LABEL_REPEATED                    /*!< Repeated field */
} pb_label_t;

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef uint32_t pb_tag_t;             /*!< Tag type */
typedef int32_t  pb_enum_t;            /*!< Enum type */
typedef size_t   pb_version_t;         /*!< Version */

/* ------------------------------------------------------------------------- */

typedef struct pb_offset_t {
  size_t start;                        /*!< Start offset */
  size_t end;                          /*!< End offset */
  struct {
    ptrdiff_t origin;                  /*!< Relative offset to origin */
    ptrdiff_t tag;                     /*!< Relative offset to tag */
    ptrdiff_t length;                  /*!< Relative offset to length */
  } diff;
} pb_offset_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT const char *
pb_error_string(
  pb_error_t error);                   /* Error */

#endif /* PB_INCLUDE_COMMON_H */