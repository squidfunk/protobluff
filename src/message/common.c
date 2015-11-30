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

#include "message/common.h"

/* ----------------------------------------------------------------------------
 * Mappings
 * ------------------------------------------------------------------------- */

/*! Mapping: error ==> error string */
static const char *error_map[] = {
  [PB_ERROR_NONE]       = "None",
  [PB_ERROR_ALLOC]      = "Allocation failed",
  [PB_ERROR_INVALID]    = "Invalid arguments or data",
  [PB_ERROR_DESCRIPTOR] = "Invalid descriptor",
  [PB_ERROR_WIRETYPE]   = "Invalid wiretype",
  [PB_ERROR_VARINT]     = "Invalid varint",
  [PB_ERROR_OFFSET]     = "Invalid offset",
  [PB_ERROR_ABSENT]     = "Absent field or value"
};

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/* LCOV_EXCL_START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*!
 * Retrieve a string representation of an error.
 *
 * \param[in] error Error
 * \return          Error string
 */
extern const char *
pb_error_string(pb_error_t error) {
  return error_map[error];
}

/* LCOV_EXCL_STOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */