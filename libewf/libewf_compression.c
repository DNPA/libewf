/*
 * Compression handling functions
 *
 * Copyright (c) 2006-2008, Joachim Metz <forensics@hoffmannbv.nl>,
 * Hoffmann Investigations. All rights reserved.
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <notify.h>

#if defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif

#if defined( HAVE_ZLIB_H )
#include <zlib.h>
#endif

#include "libewf_compression.h"

#include "ewf_definitions.h"

/* Compresses data, wraps zlib uncompress function
 * Returns 1 on success, -1 on error
 */
int libewf_compress(
     uint8_t *compressed_data,
     size_t *compressed_size,
     uint8_t *uncompressed_data,
     size_t uncompressed_size,
     int8_t compression_level )
{
	static char *function        = "libewf_compress";
	uLongf safe_compressed_size  = 0;
	int zlib_compression_level   = 0;
	int result                   = 0;

	if( compressed_data == NULL )
	{
		notify_warning_printf( "%s: invalid compressed data buffer.\n",
		 function );

		return( -1 );
	}
	if( uncompressed_data == NULL )
	{
		notify_warning_printf( "%s: invalid uncompressed data buffer.\n",
		 function );

		return( -1 );
	}
	if( compressed_data == uncompressed_data )
	{
		notify_warning_printf( "%s: invalid uncompressed data buffer equals compressed data buffer.\n",
		 function );

		return( -1 );
	}
	if( compressed_size == NULL )
	{
		notify_warning_printf( "%s: invalid compressed size.\n",
		 function );

		return( -1 );
	}
	if( compression_level == EWF_COMPRESSION_DEFAULT )
	{
		zlib_compression_level = Z_BEST_SPEED;
	}
	else if( compression_level == EWF_COMPRESSION_FAST )
	{
		zlib_compression_level = Z_BEST_SPEED;
	}
	else if( compression_level == EWF_COMPRESSION_BEST )
	{
		zlib_compression_level = Z_BEST_COMPRESSION;
	}
	else if( compression_level == EWF_COMPRESSION_NONE )
	{
		zlib_compression_level = Z_NO_COMPRESSION;
	}
	else
	{
		notify_warning_printf( "%s: unsupported compression level.\n",
		 function );

		return( -1 );
	}
	safe_compressed_size = (uLongf) *compressed_size;

	result = compress2(
	          (Bytef *) compressed_data,
	          &safe_compressed_size,
	          (Bytef *) uncompressed_data,
	          (uLong) uncompressed_size,
	          zlib_compression_level );

	if( result == Z_OK )
	{
		*compressed_size = (size_t) safe_compressed_size;

		return( 1 );
	}
	else if( result == Z_BUF_ERROR )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		notify_verbose_printf( "%s: unable to write compressed data: target buffer too small.\n",
		 function );
#endif

#if defined( HAVE_COMPRESS_BOUND )
		/* Use compressBound to determine the size of the uncompressed buffer
		 */
		safe_compressed_size = compressBound( (uLong) uncompressed_size );
		*compressed_size     = (size_t) safe_compressed_size;
#else
		/* Estimate that a factor 2 enlargement should suffice
		 */
		*compressed_size *= 2;
#endif
	}
	else if( result == Z_MEM_ERROR )
	{
		notify_warning_printf( "%s: unable to write compressed data: insufficient memory.\n",
		 function );

		*compressed_size = 0;
	}
	else
	{
		notify_warning_printf( "%s: zlib returned undefined error: %d.\n",
		 function, result );

		*compressed_size = 0;
	}
	return( -1 );
}

/* Uncompresses data, wraps zlib uncompress function
 * Returns 1 on success, 0 on failure, -1 on error
 */
int libewf_uncompress(
     uint8_t *uncompressed_data,
     size_t *uncompressed_size,
     uint8_t *compressed_data,
     size_t compressed_size )
{
	static char *function         = "libewf_uncompress";
	uLongf safe_uncompressed_size = 0;
	int result                    = 0;

	if( uncompressed_data == NULL )
	{
		notify_warning_printf( "%s: invalid uncompressed data buffer.\n",
		 function );

		return( -1 );
	}
	if( compressed_data == NULL )
	{
		notify_warning_printf( "%s: invalid compressed data buffer.\n",
		 function );

		return( -1 );
	}
	if( uncompressed_data == compressed_data )
	{
		notify_warning_printf( "%s: invalid compressed data buffer equals uncompressed data buffer.\n",
		 function );

		return( -1 );
	}
	if( uncompressed_size == NULL )
	{
		notify_warning_printf( "%s: invalid uncompressed size.\n",
		 function );

		return( -1 );
	}
	safe_uncompressed_size = (uLongf) *uncompressed_size;

	result = uncompress(
	          (Bytef *) uncompressed_data,
	          &safe_uncompressed_size,
	          (Bytef *) compressed_data,
	          (uLong) compressed_size );

	if( result == Z_OK )
	{
		*uncompressed_size = (size_t) safe_uncompressed_size;

		return( 1 );
	}
	else if( result == Z_DATA_ERROR )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		notify_verbose_printf( "%s: unable to read compressed data: data error.\n",
		 function );
#endif

		*uncompressed_size = 0;

		return( 0 );
	}
	else if( result == Z_BUF_ERROR )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		notify_verbose_printf( "%s: unable to read compressed data: target buffer too small.\n",
		 function );
#endif

		/* Estimate that a factor 2 enlargement should suffice
		 */
		*uncompressed_size *= 2;
	}
	else if( result == Z_MEM_ERROR )
	{
		notify_warning_printf( "%s: unable to read compressed data: insufficient memory.\n",
		 function );

		*uncompressed_size = 0;
	}
	else
	{
		notify_warning_printf( "%s: zlib returned undefined error: %d.\n",
		 function, result );

		*uncompressed_size = 0;
	}
	return( -1 );
}

