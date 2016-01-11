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

#ifndef PB_INCLUDE_DESCRIPTOR_H
#define PB_INCLUDE_DESCRIPTOR_H

#include <protobluff/core/common.h>
#include <protobluff/core/descriptor.h>
#include <protobluff/core/string.h>

/* ----------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------- */

#define INT32      PB_TYPE_INT32       /*!< 32-bit integer, var., signed */
#define INT64      PB_TYPE_INT64       /*!< 64-bit integer, var., signed */
#define UINT32     PB_TYPE_UINT32      /*!< 32-bit integer, var., unsigned */
#define UINT64     PB_TYPE_UINT64      /*!< 32-bit integer, var., unsigned */
#define SINT32     PB_TYPE_SINT32      /*!< 32-bit integer, var., signed */
#define SINT64     PB_TYPE_SINT64      /*!< 64-bit integer, var., signed */
#define FIXED32    PB_TYPE_FIXED32     /*!< 32-bit integer, fix., unsigned */
#define FIXED64    PB_TYPE_FIXED64     /*!< 64-bit integer, fix., unsigned */
#define SFIXED32   PB_TYPE_SFIXED32    /*!< 32-bit integer, fix., signed */
#define SFIXED64   PB_TYPE_SFIXED64    /*!< 64-bit integer, fix., signed */
#define BOOL       PB_TYPE_BOOL        /*!< Boolean, var. */
#define ENUM       PB_TYPE_ENUM        /*!< Enumeration, var. */
#define FLOAT      PB_TYPE_FLOAT       /*!< Single-precison float */
#define DOUBLE     PB_TYPE_DOUBLE      /*!< Double-precison float */
#define STRING     PB_TYPE_STRING      /*!< String */
#define BYTES      PB_TYPE_BYTES       /*!< Byte array */
#define MESSAGE    PB_TYPE_MESSAGE     /*!< Message */

/* ------------------------------------------------------------------------- */

#define REQUIRED   PB_LABEL_REQUIRED   /*!< Required field */
#define OPTIONAL   PB_LABEL_OPTIONAL   /*!< Optional field */
#define REPEATED   PB_LABEL_REPEATED   /*!< Repeated field */
#define ONEOF      PB_LABEL_ONEOF      /*!< Oneof member */

/* ------------------------------------------------------------------------- */

#define PACKED     PB_FLAG_PACKED      /*!< Flag: packed field */

#endif /* PB_INCLUDE_DESCRIPTOR_H */