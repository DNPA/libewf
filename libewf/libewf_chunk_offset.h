/*
 * libewf chunk offset
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

#if !defined( _LIBEWF_CHUNK_OFFSET_H )
#define _LIBEWF_CHUNK_OFFSET_H

#include "libewf_includes.h"

#include "libewf_segment_file_handle.h"

#include "ewf_table.h"

#if defined( __cplusplus )
extern "C" {
#endif

#define LIBEWF_CHUNK_OFFSET libewf_chunk_offset_t
#define LIBEWF_CHUNK_OFFSET_SIZE sizeof( LIBEWF_CHUNK_OFFSET )

typedef struct libewf_chunk_offset libewf_chunk_offset_t;

struct libewf_chunk_offset
{
	/* A reference to the segment file handle
	 */
	LIBEWF_SEGMENT_FILE_HANDLE *segment_file_handle;

	/* The file offset of the chunk in the segment file
	 */
	off64_t file_offset;

	/* The size of the chunk
	 */
	size_t size;

	/* Value to indicate if the chunk is compressed
	 */
	uint8_t compressed;

	/* Value to indicate if the chunk is stored in
	 * a delta segment file
	 */
	uint8_t dirty;
};

#if defined( __cplusplus )
}
#endif

#endif
