/*
 * ewfglob
 * Globbing functions for the libewf tools
 *
 * Copyright (c) 2006-2008, Joachim Metz <forensics@hoffmannbv.nl>,
 * Hoffmann Investigations. All rights reserved.
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the creator, related organisations, nor the names of
 *   its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER, COMPANY AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if !defined( _EWFGLOB_H )
#define _EWFGLOB_H

#include <common.h>

#include <libewf/types.h>

#include "ewfcommon.h"

#if defined( __cplusplus )
extern "C" {
#endif

#if !defined( HAVE_GLOB_H )

typedef struct ewfglob ewfglob_t;

struct ewfglob
{
	/* The amount of globs resolved
	 */
	uint16_t amount;

	/* The resolved globs
	 */
	CHAR_T **results;
};

ewfglob_t *ewfglob_alloc(
            void );

ewfglob_t *ewfglob_realloc(
            ewfglob_t *glob,
            uint16_t new_amount );

void ewfglob_free(
      ewfglob_t *glob );

int32_t ewfglob_resolve(
         ewfglob_t *glob,
         CHAR_T * const patterns[],
         uint32_t amount_of_patterns );

#endif

#if defined( __cplusplus )
}
#endif

#endif

