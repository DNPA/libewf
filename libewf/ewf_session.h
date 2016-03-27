/*
 * EWF session section (EWF-E01)
 *
 * Copyright (c) 2006-2007, Joachim Metz <forensics@hoffmannbv.nl>,
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

#if !defined( _EWF_SESSION_H )
#define _EWF_SESSION_H

#include "libewf_includes.h"

#if defined( __cplusplus )
extern "C" {
#endif

#define EWF_SESSION ewf_session_t
#define EWF_SESSION_SIZE sizeof( EWF_SESSION )

typedef struct ewf_session ewf_session_t;

struct ewf_session
{
	/* Amount of sessions
	 * consists of 4 bytes
	 */
	uint8_t amount_of_sessions[ 4 ];

	/* Unknown
	 * consists of 28 bytes
	 */
	uint8_t unknown[ 28 ];

	/* The entries with session data
	 */

	/* The section crc of all (previous) session data
	 * consists of 4 bytes (32 bits)
	 * starts with offset 76
	 */
};

#define EWF_SESSION_DATA ewf_session_data_t
#define EWF_SESSION_DATA_SIZE sizeof( EWF_SESSION_DATA )

typedef struct ewf_session_data ewf_session_data_t;

struct ewf_session_data
{
	/* Unknown
	 * consists of 36 bytes
	 */
	uint8_t unknown[ 36 ];
};

#if defined( __cplusplus )
}
#endif

#endif

