/*
 * libewf file io pool
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

#if !defined( _LIBEWF_FILE_IO_POOL_H )
#define _LIBEWF_FILE_IO_POOL_H

#include "libewf_includes.h"

#include "libewf_filename.h"
#include "libewf_file_io_handle.h"

#if defined( __cplusplus )
extern "C" {
#endif

#define LIBEWF_FILE_IO_POOL		libewf_file_io_pool_t
#define LIBEWF_FILE_IO_POOL_SIZE	sizeof( LIBEWF_FILE_IO_POOL )

typedef struct libewf_file_io_pool libewf_file_io_pool_t;

struct libewf_file_io_pool
{
	/* The amount of files in the pool
	 */
	size_t amount;

	/* The amount of open file descriptors
	 */
	size_t open_files;

	/* A dynamic array containting the file io handles
	 */
	LIBEWF_FILE_IO_HANDLE *handle;
};

LIBEWF_FILE_IO_POOL *libewf_file_io_pool_alloc( size_t amount );
int libewf_file_io_pool_realloc( LIBEWF_FILE_IO_POOL *file_io_pool, size_t amount );
void libewf_file_io_pool_free( LIBEWF_FILE_IO_POOL *file_io_pool );

int libewf_file_io_pool_open( LIBEWF_FILE_IO_POOL *file_io_pool, LIBEWF_FILENAME *filename, int flags );

ssize_t libewf_file_io_pool_read( LIBEWF_FILE_IO_POOL *file_io_pool, size_t entry, uint8_t *buffer, size_t size );
ssize_t libewf_file_io_pool_write( LIBEWF_FILE_IO_POOL *file_io_pool, size_t entry, uint8_t *buffer, size_t size );
off64_t libewf_file_io_pool_seek( LIBEWF_FILE_IO_POOL *file_io_pool, size_t entry, off64_t offset, int whence );
int libewf_file_io_pool_close( LIBEWF_FILE_IO_POOL *file_io_pool, size_t entry );

#if defined( __cplusplus )
}
#endif

#endif

