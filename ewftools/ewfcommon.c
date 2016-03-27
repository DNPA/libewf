/*
 * ewfcommon
 * Common functions for the ewf tools
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

#include "../libewf/libewf_includes.h"

#include <errno.h>

#if defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif

#if defined( HAVE_STRING_H )
#include <string.h>
#endif

#if defined( HAVE_SYS_UTSNAME_H )
#include <sys/utsname.h>
#endif

#if defined( HAVE_ZLIB_H ) && defined( HAVE_LIBZ )
#include <zlib.h>
#endif

#if defined( HAVE_OPENSSL_OPENSSLV_H ) && defined( HAVE_LIBCRYPTO )
#include <openssl/opensslv.h>
#endif

#if defined( HAVE_UUID_UUID_H ) && defined( HAVE_LIBUUID )
#include <uuid/uuid.h>
#endif

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include "../libewf/libewf_common.h"
#include "../libewf/libewf_notify.h"
#include "../libewf/libewf_string.h"

#include "../libewf/ewf_digest_hash.h"

#include "ewfcommon.h"

/* EWFCOMMON_BUFFER_SIZE definition is intended for testing purposes
 */
#if !defined( EWFCOMMON_BUFFER_SIZE )
#define EWFCOMMON_BUFFER_SIZE	chunk_size
#endif

#if !defined( LIBEWF_OPERATING_SYSTEM )
#define LIBEWF_OPERATING_SYSTEM "Unknown"
#endif

/* Swaps the byte order of byte pairs within a buffer of a certain size
 * Returns 1 if successful, -1 on error
 */
int ewfcommon_swap_byte_pairs( uint8_t *buffer, size_t size )
{
	static char *function = "ewfcommon_swap_byte_pairs";
	uint8_t byte          = 0;
	size_t iterator       = 0;

	if( buffer == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid buffer.\n",
		 function );

		return( -1 );
	}
	if( size > (size_t) SSIZE_MAX )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	for( iterator = 0; iterator < size; iterator += 2 )
	{
		byte                   = buffer[ iterator ];
		buffer[ iterator ]     = buffer[ iterator + 1 ];
		buffer[ iterator + 1 ] = byte;
	}
	return( 1 );
}

/* Determines the current platform, or NULL on error
 */
LIBEWF_CHAR *ewfcommon_determine_operating_system( void )
{
	LIBEWF_CHAR *string    = NULL;
	char *operating_system = NULL;
	uint32_t length        = 0;

#if defined( HAVE_SYS_UTSNAME_H )
	struct utsname utsname_buffer;

	/* Determine the operating system
	 */
	if( uname( &utsname_buffer ) == 0 )
	{
		operating_system = utsname_buffer.sysname;
	}
	else
	{
		operating_system = "Undetermined";
	}
#else
	operating_system = LIBEWF_OPERATING_SYSTEM;
#endif
	length = (uint32_t) strlen( operating_system ) + 1;
	string = (LIBEWF_CHAR *) libewf_common_alloc( LIBEWF_CHAR_SIZE * length );

	if( ( string != NULL )
	 && ( ewfstring_copy_libewf_char_from_char_t( string, operating_system, length ) != 1 ) )
	{
		libewf_common_free( string );
	
		return( NULL );
	}
	return( string );
}

/* Determines the GUID
 * Returns 1 if successful, or -1 on error
 */
int8_t ewfcommon_determine_guid( uint8_t *guid, uint8_t libewf_format )
{
	static char *function = "ewfcommon_determine_guid";

	if( guid == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid GUID.\n",
		 function );

		return( -1 );
	}
#if defined( HAVE_UUID_UUID_H ) && defined( HAVE_LIBUUID )
#if defined( HAVE_UUID_GENERATE_RANDOM )
	if( ( libewf_format == LIBEWF_FORMAT_ENCASE5 )
	 || ( libewf_format == LIBEWF_FORMAT_ENCASE6 )
	 || ( libewf_format == LIBEWF_FORMAT_EWFX ) )
	{
		uuid_generate_random( guid );
	}
#endif
#if defined( HAVE_UUID_GENERATE_TIME )
	if( ( libewf_format == LIBEWF_FORMAT_LINEN5 )
	 || ( libewf_format == LIBEWF_FORMAT_LINEN6 ) )
	{
		uuid_generate_time( guid );
	}
#endif
#endif
	return( 1 );
}

/* Reads data from a file descriptor into the chunk cache
 * Returns the amount of bytes read, 0 if at end of input, or -1 on error
 */
ssize32_t ewfcommon_read_input( LIBEWF_HANDLE *handle, int file_descriptor, uint8_t *buffer, size_t buffer_size, size32_t chunk_size, uint32_t bytes_per_sector, ssize64_t total_read_count, size64_t total_input_size, uint8_t read_error_retry, uint32_t sector_error_granularity, uint8_t wipe_block_on_read_error, uint8_t seek_on_error )
{
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
	CHAR_T *error_string              = NULL;
#endif
	static char *function             = "ewfcommon_read_input";
	off64_t current_read_offset       = 0;
	off64_t current_calculated_offset = 0;
	off64_t error2_sector             = 0;
	ssize_t read_count                = 0;
	ssize_t buffer_offset             = 0;
	size_t read_size                  = 0;
	size_t bytes_to_read              = 0;
	size_t read_remaining_bytes       = 0;
	size_t error_remaining_bytes      = 0;
	int32_t read_amount_of_errors     = 0;
	uint32_t chunk_amount             = 0;
	uint32_t read_error_offset        = 0;
	uint32_t error_skip_bytes         = 0;
	uint32_t error_granularity_offset = 0;
	uint32_t error2_amount_of_sectors = 0;
	uint32_t acquiry_amount_of_errors = 0;
	uint32_t byte_error_granularity   = 0;

	if( handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( buffer == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid read buffer.\n",
		 function );

		return( -1 );
	}
	if( chunk_size == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid chunk size.\n",
		 function );

		return( -1 );
	}
	if( buffer_size > (size_t) SSIZE_MAX )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid buffer size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( file_descriptor == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	if( total_read_count <= -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid total read count.\n",
		 function );

		return( -1 );
	}
	if( libewf_get_write_amount_of_chunks( handle, &chunk_amount ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine amount of chunks written.\n",
		 function );

		return( -1 );
	}
	while( buffer_size > 0 )
	{
		/* Determine the amount of bytes to read from the input
		 * Read as much as possible in chunk sizes
		 */
		if( buffer_size < (size_t) chunk_size )
		{
			read_size = buffer_size;
		}
		else
		{
			read_size = chunk_size;
		}
		bytes_to_read = read_size;

		while( read_amount_of_errors <= read_error_retry )
		{
			read_count = libewf_common_read(
			              file_descriptor,
			              &buffer[ buffer_offset + read_error_offset ],
			              bytes_to_read );

			LIBEWF_VERBOSE_PRINT( "%s: read chunk: %" PRIi64 " with size: %zi.\n",
			 function, ( chunk_amount + 1 ), read_count );

			current_calculated_offset = (off64_t) ( total_read_count + buffer_offset + read_error_offset );

			if( read_count <= -1 )
			{
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
				if( ( errno == ESPIPE )
				 || ( errno == EPERM )
				 || ( errno == ENXIO )
				 || ( errno == ENODEV ) )
				{
					error_string = ewfstring_strerror( errno );

					if( error_string != NULL )
					{
						LIBEWF_WARNING_PRINT( "%s: error reading data: %s.\n",
						 function, error_string );

						libewf_common_free( error_string );
					}
					return( -1 );
				}
#else
				if( errno == ESPIPE )
				{
					LIBEWF_WARNING_PRINT( "%s: error reading data: invalid seek.\n",
					 function );

					return( -1 );
				}
				else if( errno == EPERM )
				{
					LIBEWF_WARNING_PRINT( "%s: error reading data: operation not permitted.\n",
					 function );

					return( -1 );
				}
				else if( errno == ENXIO )
				{
					LIBEWF_WARNING_PRINT( "%s: error reading data: no such device or address.\n",
					 function );

					return( -1 );
				}
				else if( errno == ENODEV )
				{
					LIBEWF_WARNING_PRINT( "%s: error reading data: no such device.\n",
					 function );

					return( -1 );
				}
#endif
				if( seek_on_error == 1 )
				{
					current_read_offset = libewf_common_lseek( file_descriptor, 0, SEEK_CUR );

					if( current_read_offset != current_calculated_offset )
					{
						LIBEWF_VERBOSE_PRINT( "%s: correcting offset drift current: %jd, calculated: %jd.\n",
						 function, current_read_offset, current_calculated_offset );

						if( current_read_offset < current_calculated_offset )
						{
							LIBEWF_WARNING_PRINT( "%s: unable to correct offset drift.\n",
							 function );

							return( -1 );
						}
						read_count         = (ssize_t) ( current_read_offset - current_calculated_offset );
						read_error_offset += read_count;
						bytes_to_read     -= read_count;
					}
				}
			}
			else
			{
				/* The last read is OK, correct read_count
				 */
				if( read_count == (int32_t) bytes_to_read )
				{
					read_count = read_error_offset + bytes_to_read;
				}
				/* The entire read is OK
				 */
				if( read_count == (int32_t) read_size )
				{
					break;
				}
				/* If no end of input can be determined
				 */
				if( total_input_size == 0 )
				{
					/* If some bytes were read it is possible that the end of the input reached
					 */
					if( read_count > 0 )
					{
						return( (int32_t) ( buffer_offset + read_count ) );
					}
				}
				else
				{
					/* Check if the end of the input was reached
					 */
					if( ( total_read_count + buffer_offset + read_count ) >= (int64_t) total_input_size )
					{
						break;
					}
				}
				/* No bytes were read
				 */
				if( read_count == 0 )
				{
					return( 0 );
				}
				LIBEWF_VERBOSE_PRINT( "%s: read error at offset %jd after reading %zi bytes.\n",
				 function, current_calculated_offset, read_count );

				/* There was a read error at a certain offset
				 */
				read_error_offset += read_count;
				bytes_to_read     -= read_count;
			}
			read_amount_of_errors++;

			if( read_amount_of_errors > read_error_retry )
			{
				if( seek_on_error == 0 )
				{
					LIBEWF_VERBOSE_PRINT( "%s: unable to handle more input.\n",
					 function );

					return( 0 );
				}
				current_calculated_offset = total_read_count + buffer_offset;

				/* Check if last chunk is smaller than the chunk size and take corrective measures
				 */
				if( ( total_input_size != 0 )
				 && ( ( current_calculated_offset + chunk_size ) > (int64_t) total_input_size ) )
				{
					read_remaining_bytes = (size_t) ( total_input_size - current_calculated_offset );
				}
				else
				{
					read_remaining_bytes = (size_t) chunk_size;
				}
				if( read_remaining_bytes > (size_t) SSIZE_MAX )
				{
					LIBEWF_VERBOSE_PRINT( "%s: invalid remaining bytes value exceeds maximum.\n",
					 function );

					return( -1 );
				}
				byte_error_granularity   = sector_error_granularity * bytes_per_sector;
				error_remaining_bytes    = read_remaining_bytes - read_error_offset;
				error2_sector            = current_calculated_offset;
				error_granularity_offset = ( read_error_offset / byte_error_granularity ) * byte_error_granularity;
				error_skip_bytes         = ( error_granularity_offset + byte_error_granularity ) - read_error_offset;

				if( wipe_block_on_read_error == 1 )
				{
					LIBEWF_VERBOSE_PRINT( "%s: wiping block of %" PRIu32 " bytes at offset %" PRIu32 ".\n",
					 function, byte_error_granularity, error_granularity_offset );

					if( libewf_common_memset( &buffer[ error_granularity_offset ], 0, byte_error_granularity ) == NULL )
					{
						LIBEWF_WARNING_PRINT( "%s: unable to wipe data in chunk on error.\n",
						 function );

						return( -1 );
					}
					error2_sector            += error_granularity_offset;
					error2_amount_of_sectors  = byte_error_granularity;
				}
				else
				{
					error2_sector            += read_error_offset;
					error2_amount_of_sectors  = error_skip_bytes;
				}
				error2_sector            /= bytes_per_sector;
				error2_amount_of_sectors /= bytes_per_sector;

				if( libewf_add_acquiry_error( handle, error2_sector, error2_amount_of_sectors ) != 1 )
				{
					LIBEWF_WARNING_PRINT( "%s: unable to add acquiry read errror sectors.\n",
					 function );

					return( -1 );
				}
				acquiry_amount_of_errors++;

				LIBEWF_VERBOSE_PRINT( "%s: adding error2: %" PRIu32 " sector: %" PRIu64 ", count: %" PRIu32 ".\n",
				 function, acquiry_amount_of_errors, error2_sector, error2_amount_of_sectors );

				LIBEWF_VERBOSE_PRINT( "%s: skipping %" PRIu32 " bytes.\n",
				 function, error_skip_bytes );

				/* At the end of the input
				 */
				if( ( total_input_size != 0 )
				 && ( ( current_calculated_offset + (int64_t) read_remaining_bytes ) >= (int64_t) total_input_size ) )
				{
					LIBEWF_VERBOSE_PRINT( "%s: at end of input no remaining bytes to read from chunk.\n",
					 function );

					read_count = (ssize_t) read_remaining_bytes;

					break;
				}
				if( libewf_common_lseek( file_descriptor, error_skip_bytes, SEEK_CUR ) == -1 )
				{
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
					error_string = ewfstring_strerror( errno );

					if( error_string != NULL )
					{
						LIBEWF_WARNING_PRINT( "%s: unable skip %" PRIu32 " bytes after sector with error - %s.\n",
						 function, error_skip_bytes, error_string );

						libewf_common_free( error_string );
					}
#else
					LIBEWF_WARNING_PRINT( "%s: unable skip %" PRIu32 " bytes after sector with error.\n",
					 function, error_skip_bytes );
#endif
					return( -1 );
				}
				/* If error granularity skip is still within the chunk
				 */
				if( error_remaining_bytes > byte_error_granularity )
				{
					bytes_to_read          = error_remaining_bytes - error_skip_bytes;
					read_error_offset     += error_skip_bytes;
					read_amount_of_errors  = 0;

					LIBEWF_VERBOSE_PRINT( "%s: remaining to read from chunk %" PRIu32 " bytes.\n",
					 function, bytes_to_read );
				}
				else
				{
					read_count = (ssize_t) read_remaining_bytes;

					LIBEWF_VERBOSE_PRINT( "%s: no remaining bytes to read from chunk.\n",
					 function );

					break;
				}
			}
		}
		buffer_size   -= read_count;
		buffer_offset += read_count;

		/* At the end of the input
		 */
		if( ( total_input_size != 0 )
		 && ( ( total_read_count + buffer_offset ) >= (int64_t) total_input_size ) )
		{
			break;
		}
	}
	return( (int32_t) buffer_offset );
}

/* Reads the data to calculate the MD5 and SHA1 integrity hashes
 * Returns the amount of bytes read if successful, or -1 on error
 */
ssize64_t ewfcommon_read_verify( LIBEWF_HANDLE *handle, uint8_t calculate_md5, LIBEWF_CHAR *md5_hash_string, size_t md5_hash_string_length, uint8_t calculate_sha1, LIBEWF_CHAR *sha1_hash_string, size_t sha1_hash_string_length, uint8_t swap_byte_pairs, void (*callback)( size64_t bytes_read, size64_t bytes_total ) )
{
	EWFMD5_CONTEXT md5_context;
	EWFSHA1_CONTEXT sha1_context;

	EWFDIGEST_HASH md5_hash[ EWFDIGEST_HASH_SIZE_MD5 ];
	EWFDIGEST_HASH sha1_hash[ EWFDIGEST_HASH_SIZE_SHA1 ];

	uint8_t *data              = NULL;
	uint8_t *uncompressed_data = NULL;
	static char *function      = "ewfcommon_read_verify";
	off64_t read_offset        = 0;
	size64_t media_size        = 0;
	size32_t chunk_size        = 0;
	size_t buffer_size         = 0;
	size_t read_size           = 0;
	size_t md5_hash_size       = EWFDIGEST_HASH_SIZE_MD5;
	size_t sha1_hash_size      = EWFDIGEST_HASH_SIZE_SHA1;
	ssize64_t total_read_count = 0;
	ssize_t read_count         = 0;
#if defined( HAVE_RAW_ACCESS )
	uint8_t *raw_read_data     = NULL;
	ssize_t raw_read_count     = 0;
	size_t uncompressed_size   = 0;
	uint32_t chunk_crc         = 0;
	int8_t is_compressed       = 0;
	int8_t read_crc            = 0;
#endif

	if( handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( calculate_md5 == 1 )
	{
		if( md5_hash_string == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid MD5 hash string.\n",
			 function );

			return( -1 );
		}
	}
	if( calculate_sha1 == 1 )
	{
		if( sha1_hash_string == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid SHA1 hash string.\n",
			 function );

			return( -1 );
		}
	}
	if( libewf_get_media_size( handle, &media_size ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine media size.\n",
		 function );

		return( -1 );
	}
	if( libewf_get_chunk_size( handle, &chunk_size ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine chunk size.\n",
		 function );

		return( -1 );
	}
	if( chunk_size == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid chunk size.\n",
		 function );

		return( -1 );
	}
	if( chunk_size > (uint32_t) INT32_MAX )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid chunk size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	buffer_size = EWFCOMMON_BUFFER_SIZE;
	data        = (uint8_t *) libewf_common_alloc( buffer_size * sizeof( uint8_t ) );

	if( data == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate data.\n",
		 function );

		return( -1 );
	} 
#if defined( HAVE_RAW_ACCESS )
	raw_read_data = (uint8_t *) libewf_common_alloc( buffer_size * sizeof( uint8_t ) );

	if( raw_read_data == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate raw read data.\n",
		 function );

		libewf_common_free( data );

		return( -1 );
	}
#endif
	if( calculate_md5 == 1 )
	{
		if( ewfmd5_initialize( &md5_context ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to initialize MD5 digest context.\n",
			 function );

			libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
			libewf_common_free( raw_read_data );
#endif

			return( -1 );
		}
	}
	if( calculate_sha1 == 1 )
	{
		if( ewfsha1_initialize( &sha1_context ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to initialize SHA1 digest context.\n",
			 function );

			libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
			libewf_common_free( raw_read_data );
#endif

			return( -1 );
		}
	}

	while( total_read_count < (ssize64_t) media_size )
	{
		read_size = buffer_size;

		if( ( media_size - total_read_count ) < read_size )
		{
			read_size = (size_t) ( media_size - total_read_count );
		}
#if defined( HAVE_RAW_ACCESS )
		raw_read_count = libewf_raw_read_buffer(
		                  handle,
		                  (void *) raw_read_data,
		                  (size_t) buffer_size,
		                  &is_compressed,
		                  &chunk_crc,
		                  &read_crc );

		if( raw_read_count <= -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to read raw buffer.\n",
			 function );

			libewf_common_free( data );
			libewf_common_free( raw_read_data );

			return( -1 );
		}
		uncompressed_size = buffer_size;

		read_count = libewf_raw_read_prepare_buffer(
		              handle,
		              raw_read_data,
		              (size_t) raw_read_count,
		              data,
		              &uncompressed_size,
		              is_compressed );

		if( read_count <= -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to prepare read raw buffer.\n",
			 function );

			libewf_common_free( data );
			libewf_common_free( raw_read_data );

			return( -1 );
		}
		if( is_compressed == 1 )
		{
			uncompressed_data = data;
		}
		else
		{
			uncompressed_data = raw_read_data;
		}
		if( read_size != uncompressed_size )
		{
			LIBEWF_WARNING_PRINT( "%s: mismatch in read and uncompressed buffer size.\n",
			 function );

			libewf_common_free( data );
			libewf_common_free( raw_read_data );

			return( -1 );
		}
#else
		read_count = libewf_read_random( handle, (void *) data, read_size, read_offset );

		if( read_count <= -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: error reading data.\n",
			 function );

			libewf_common_free( data );

			return( -1 );
		}
		uncompressed_data = data;
#endif
		if( read_count == 0 )
		{
			LIBEWF_WARNING_PRINT( "%s: unexpected end of data.\n",
			 function );

			libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
			libewf_common_free( raw_read_data );
#endif

			return( -1 );
		}
		if( read_count > (ssize_t) read_size )
		{
			LIBEWF_WARNING_PRINT( "%s: more bytes read than requested.\n",
			 function );

			libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
			libewf_common_free( raw_read_data );
#endif

			return( -1 );
		}
		/* Swap byte pairs
		 */
		if( ( swap_byte_pairs == 1 )
		 && ( ewfcommon_swap_byte_pairs( uncompressed_data, read_count ) != 1 ) )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to swap byte pairs.\n",
			 function );

			libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
			libewf_common_free( raw_read_data );
#endif
			return( -1 );
		}
		if( calculate_md5 == 1 )
		{
			ewfmd5_update( &md5_context, uncompressed_data, read_count );
		}
		if( calculate_sha1 == 1 )
		{
			ewfsha1_update( &sha1_context, uncompressed_data, read_count );
		}
		read_offset      += (off64_t) read_size;
		total_read_count += (ssize64_t) read_count;

		if( callback != NULL )
		{
			callback( (size64_t) total_read_count, media_size );
		}
  	}
	libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
	libewf_common_free( raw_read_data );
#endif

	if( calculate_md5 == 1 )
	{
		if( ewfmd5_finalize( &md5_context, md5_hash, &md5_hash_size ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set MD5 hash.\n",
			 function );

			return( -1 );
		}
		if( ewfdigest_copy_to_string(
		     md5_hash,
		     md5_hash_size,
		     md5_hash_string,
		     md5_hash_string_length ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set MD5 hash string.\n",
			 function );

			return( -1 );
		}
	}
	if( calculate_sha1 == 1 )
	{
		if( ewfsha1_finalize( &sha1_context, sha1_hash, &sha1_hash_size ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set SHA1 hash.\n",
			 function );

			return( -1 );
		}
		if( ewfdigest_copy_to_string(
		     sha1_hash,
		     sha1_hash_size,
		     sha1_hash_string,
		     sha1_hash_string_length ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set SHA1 hash string.\n",
			 function );

			return( -1 );
		}
	}
	return( total_read_count );
}

/* Writes data in EWF format from a file descriptor
 * Returns the amount of bytes written, or -1 on error
 */
ssize64_t ewfcommon_write_from_file_descriptor( LIBEWF_HANDLE *handle, int input_file_descriptor, size64_t write_size, off64_t write_offset, uint8_t read_error_retry, uint32_t sector_error_granularity, uint8_t wipe_block_on_read_error, uint8_t seek_on_error, uint8_t calculate_md5, LIBEWF_CHAR *md5_hash_string, size_t md5_hash_string_length, uint8_t calculate_sha1, LIBEWF_CHAR *sha1_hash_string, size_t sha1_hash_string_length, uint8_t swap_byte_pairs, void (*callback)( uint64_t bytes_read, uint64_t bytes_total ) )
{
	EWFMD5_CONTEXT md5_context;
	EWFSHA1_CONTEXT sha1_context;

	EWFDIGEST_HASH md5_hash[ EWFDIGEST_HASH_SIZE_MD5 ];
	EWFDIGEST_HASH sha1_hash[ EWFDIGEST_HASH_SIZE_SHA1 ];

	uint8_t *data               = NULL;
	static char *function       = "ewfcommon_write_from_file_descriptor";
	size32_t chunk_size         = 0;
	size_t buffer_size          = 0;
	size_t md5_hash_size        = EWFDIGEST_HASH_SIZE_MD5;
	size_t sha1_hash_size       = EWFDIGEST_HASH_SIZE_SHA1;
	ssize64_t total_write_count = 0;
	ssize64_t write_count       = 0;
	ssize32_t read_count        = 0;
	uint32_t bytes_per_sector   = 0;
#if defined( HAVE_RAW_ACCESS )
	uint8_t *compressed_data    = NULL;
	uint8_t *raw_write_data     = NULL;
	size_t compressed_size      = 0;
	ssize_t raw_write_count     = 0;
	uint32_t chunk_crc          = 0;
	int8_t is_compressed        = 0;
	int8_t write_crc            = 0;
#endif

	if( handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( calculate_md5 == 1 )
	{
		if( md5_hash_string == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid MD5 hash string.\n",
			 function );

			return( -1 );
		}
	}
	if( calculate_sha1 == 1 )
	{
		if( sha1_hash_string == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid SHA1 hash string.\n",
			 function );

			return( -1 );
		}
	}
	if( input_file_descriptor == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	if( libewf_get_chunk_size( handle, &chunk_size ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine chunk size.\n",
		 function );

		return( -1 );
	}
	if( chunk_size == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid chunk size.\n",
		 function );

		return( -1 );
	}
	if( libewf_get_bytes_per_sector( handle, &bytes_per_sector ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to get bytes per sector.\n",
		 function );

		return( -1 );
	}
	if( bytes_per_sector == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid amount of bytes per sector.\n",
		 function );

		return( -1 );
	}
	if( write_size > 0 )
	{
		if( libewf_set_write_input_size( handle, write_size ) == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set input write size in handle.\n",
			 function );

			return( -1 );
		}
		if( write_offset > 0 )
		{
			if( write_offset >= (off64_t) write_size )
			{
				LIBEWF_WARNING_PRINT( "%s: invalid offset to write.\n",
				 function );

				return( -1 );
			}
			if( libewf_common_lseek(
			     input_file_descriptor,
			     write_offset,
			     SEEK_SET ) != (off64_t) write_offset )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to find write offset.\n",
				 function );

				return( -1 );
			}
		}
	}
	else if( write_offset > 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: ignoring write offset in a stream mode.\n",
		 function );
	}
	buffer_size = EWFCOMMON_BUFFER_SIZE;
	data        = (uint8_t *) libewf_common_alloc( buffer_size * sizeof( uint8_t ) );

	if( data == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate data.\n",
		 function );

		return( -1 );
	}
#if defined( HAVE_RAW_ACCESS )
	compressed_data = (uint8_t *) libewf_common_alloc( 2 * buffer_size * sizeof( uint8_t ) );

	if( compressed_data == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate compressed data.\n",
		 function );

		libewf_common_free( data );

		return( -1 );
	}
#endif
	if( calculate_md5 == 1 )
	{
		if( ewfmd5_initialize( &md5_context ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to initialize MD5 digest context.\n",
			 function );

			libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
			libewf_common_free( compressed_data );
#endif

			return( -1 );
		}
	}
	if( calculate_sha1 == 1 )
	{
		if( ewfsha1_initialize( &sha1_context ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to initialize SHA1 digest context.\n",
			 function );

			libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
			libewf_common_free( compressed_data );
#endif

			return( -1 );
		}
	}
	while( ( write_size == 0 )
	 || ( total_write_count < (int64_t) write_size ) )
	{
		/* Read a chunk from the file descriptor
		 */
		read_count = ewfcommon_read_input(
		              handle,
		              input_file_descriptor,
		              data,
		              buffer_size,
		              chunk_size,
		              bytes_per_sector,
		              total_write_count,
		              write_size,
		              read_error_retry,
		              sector_error_granularity,
		              wipe_block_on_read_error,
		              seek_on_error );

		if( read_count <= -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to read chunk from file.\n",
			 function );

			libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
			libewf_common_free( compressed_data );
#endif

			return( -1 );
		}
		if( read_count == 0 )
		{
			if( write_size != 0 )
			{
				LIBEWF_WARNING_PRINT( "%s: unexpected end of input.\n",
				 function );

				libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
				libewf_common_free( compressed_data );
#endif
				return( -1 );
			}
			break;
		}
		/* Swap byte pairs
		 */
		if( ( swap_byte_pairs == 1 )
		 && ( ewfcommon_swap_byte_pairs( data, read_count ) != 1 ) )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to swap byte pairs.\n",
			 function );

			return( -1 );
		}
		if( calculate_md5 == 1 )
		{
/* MSVS C++ does not allow pre compiler macro in macro defintions
 */
			ewfmd5_update(
			 &md5_context,
			 data,
			 read_count );
		}
		if( calculate_sha1 == 1 )
		{
/* MSVS C++ does not allow pre compiler macro in macro defintions
 */
			ewfsha1_update(
			 &sha1_context,
			 data,
			 read_count );
		}
#if defined( HAVE_RAW_ACCESS )
		compressed_size = 2 * buffer_size;

		raw_write_count = libewf_raw_write_prepare_buffer(
		                   handle,
		                   data,
		                   (size_t) read_count,
		                   compressed_data,
		                   &compressed_size,
		                   &is_compressed,
		                   &chunk_crc,
		                   &write_crc );

		if( raw_write_count <= -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to prepare buffer for write raw.\n",
			 function );

			libewf_common_free( data );
			libewf_common_free( compressed_data );

			return( -1 );
		}
		if( is_compressed == 1 )
		{
			raw_write_data = compressed_data;
		}
		else
		{
			raw_write_data = data;
		}
		write_count = libewf_raw_write_buffer(
		               handle,
		               (void *) raw_write_data,
		               (size_t) raw_write_count,
		               (size_t) read_count,
		               is_compressed,
		               chunk_crc,
		               write_crc );
#else
		write_count = libewf_write_buffer(
		               handle,
		               (void *) data,
		               read_count );
#endif

		if( write_count != read_count )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write chunk to file.\n",
			 function );

			libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
			libewf_common_free( compressed_data );
#endif

			return( -1 );
		}
		total_write_count += read_count;

		/* Callback for status update
		 */
		if( callback != NULL )
		{
			callback( (size64_t) total_write_count, write_size );
		}
	}
	libewf_common_free( data );
#if defined( HAVE_RAW_ACCESS )
	libewf_common_free( compressed_data );
#endif

	if( calculate_md5 == 1 )
	{
		if( ewfmd5_finalize( &md5_context, md5_hash, &md5_hash_size ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set MD5 hash.\n",
			 function );

			return( -1 );
		}
		if( ewfdigest_copy_to_string(
		     md5_hash,
		     md5_hash_size,
		     md5_hash_string,
		     md5_hash_string_length ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set MD5 hash string.\n",
			 function );

			return( -1 );
		}
		/* The MD5 hash must be set before write finalized is used
		 */
		if( libewf_set_md5_hash(
		     handle,
		     md5_hash,
		     md5_hash_size ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set MD5 hash in handle.\n",
			 function );

			return( -1 );
		}
		/* The MD5 hash string must be set before write finalized is used
		 */
		if( libewf_set_hash_value_md5(
		     handle,
		     md5_hash_string,
		     md5_hash_string_length ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set MD5 hash string in handle.\n",
			 function );

			return( -1 );
		}
	}
	if( calculate_sha1 == 1 )
	{
		if( ewfsha1_finalize( &sha1_context, sha1_hash, &sha1_hash_size ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set SHA1 hash.\n",
			 function );

			return( -1 );
		}
		if( ewfdigest_copy_to_string(
		     sha1_hash,
		     sha1_hash_size,
		     sha1_hash_string,
		     sha1_hash_string_length ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set SHA1 hash string.\n",
			 function );

			return( -1 );
		}
		/* The SHA1 hash string must be set before write finalized is used
		 */
		if( libewf_set_hash_value_sha1(
		     handle,
		     sha1_hash_string,
		     sha1_hash_string_length ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set SHA1 hash string in handle.\n",
			 function );

			return( -1 );
		}
	}
	write_count = libewf_write_finalize( handle );

	if( write_count == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to finalize EWF file(s).\n",
		 function );

		return( -1 );
	}
	total_write_count += write_count;

	return( total_write_count );
}

/* Reads the media data and exports it in raw format
 * Returns a -1 on error, the amount of bytes read on success
 */
ssize64_t ewfcommon_export_raw( LIBEWF_HANDLE *handle, CHAR_T *target_filename, size64_t maximum_file_size, size64_t read_size, off64_t read_offset, uint8_t swap_byte_pairs, void (*callback)( size64_t bytes_read, size64_t bytes_total ) )
{
	uint8_t *data              = NULL;
	static char *function      = "ewfcommon_export_raw";
	size64_t media_size        = 0;
	size32_t chunk_size        = 0;
	size_t size                = 0;
	size_t buffer_size         = 0;
	ssize64_t total_read_count = 0;
	ssize_t read_count         = 0;
	ssize_t write_count        = 0;
	uint8_t read_all           = 0;
	int file_descriptor        = -1;

	if( handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( target_filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid target filename.\n",
		 function );

		return( -1 );
	}
	if( CHAR_T_COMPARE( target_filename, _S_CHAR_T( "-" ), 1 ) == 0 )
	{
		file_descriptor = 1;
	}
	else
	{
		file_descriptor = libewf_common_open( target_filename, LIBEWF_OPEN_WRITE );

		if( file_descriptor == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to open filename.\n",
			 function );

			return( -1 );
		}
	}
	if( libewf_get_media_size( handle, &media_size ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine media size.\n",
		 function );

		return( -1 );
	}
	if( libewf_get_chunk_size( handle, &chunk_size ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine chunk size.\n",
		 function );

		return( -1 );
	}
	if( chunk_size == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid chunk size.\n",
		 function );

		return( -1 );
	}
	if( ( read_size == 0 )
	 || ( read_size > media_size )
	 || ( read_size > (ssize64_t) INT64_MAX ) )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid size.\n",
		 function );

		return( -1 );
	}
	if( read_offset >= (off64_t) media_size )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid offset.\n",
		 function );

		return( -1 );
	}
	if( ( read_size + read_offset ) > media_size )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to export beyond size of media.\n",
		 function );

		return( -1 );
	}
	read_all    = (uint8_t) ( ( read_size == media_size ) && ( read_offset == 0 ) );
	buffer_size = EWFCOMMON_BUFFER_SIZE;
	data        = (uint8_t *) libewf_common_alloc( buffer_size * sizeof( uint8_t ) );

	if( data == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate data.\n",
		 function );

		return( -1 );
	}
	while( total_read_count < (int64_t) read_size )
	{
		size = buffer_size;

		if( ( media_size - total_read_count ) < size )
		{
			size = (size_t) ( media_size - total_read_count );
		}
		read_count = libewf_read_random( handle, (void *) data, size, read_offset );

		if( read_count <= -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: error reading data.\n",
			 function );

			libewf_common_free( data );

			return( -1 );
		}
		if( read_count == 0 )
		{
			LIBEWF_WARNING_PRINT( "%s: unexpected end of data.\n",
			 function );

			libewf_common_free( data );

			return( -1 );
		}
		if( read_count > (ssize_t) size )
		{
			LIBEWF_WARNING_PRINT( "%s: more bytes read than requested.\n",
			 function );

			libewf_common_free( data );

			return( -1 );
		}
		read_offset += size;

		/* Swap byte pairs
		 */
		if( ( swap_byte_pairs == 1 )
		 && ( ewfcommon_swap_byte_pairs( data, read_count ) != 1 ) )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to swap byte pairs.\n",
			 function );

			return( -1 );
		}
		write_count = libewf_common_write( file_descriptor, data, (size_t) read_count );

		if( write_count < read_count )
		{
			LIBEWF_WARNING_PRINT( "%s: error writing data.\n",
			 function );

			libewf_common_free( data );

			return( -1 );
		}
		total_read_count += read_count;

		if( callback != NULL )
		{
			callback( total_read_count, read_size );
		}
  	}
	libewf_common_free( data );

	return( total_read_count );
}

/* Reads the media data and exports it in EWF format
 * Returns a -1 on error, the amount of bytes read on success
 */
ssize64_t ewfcommon_export_ewf( LIBEWF_HANDLE *handle, LIBEWF_HANDLE *export_handle, size64_t read_size, off64_t read_offset, uint8_t swap_byte_pairs, void (*callback)( size64_t bytes_read, size64_t bytes_total ) )
{
	LIBEWF_CHAR header_value[ 128 ];

	uint8_t *data              = NULL;
	static char *function      = "ewfcommon_export_ewf";
	size64_t media_size        = 0;
	size32_t chunk_size        = 0;
	size_t size                = 0;
	size_t buffer_size         = 0;
	ssize64_t total_read_count = 0;
	size_t header_value_length = 128;
	ssize_t read_count         = 0;
	ssize_t write_count        = 0;
	uint8_t read_all           = 0;

	if( handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( export_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid export handle.\n",
		 function );

		return( -1 );
	}
	if( libewf_get_media_size( handle, &media_size ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine media size.\n",
		 function );

		return( -1 );
	}
	if( libewf_get_chunk_size( handle, &chunk_size ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine chunk size.\n",
		 function );

		return( -1 );
	}
	if( chunk_size == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid chunk size.\n",
		 function );

		return( -1 );
	}
	if( ( read_size == 0 )
	 || ( read_size > media_size )
	 || ( read_size > (ssize64_t) INT64_MAX ) )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid size.\n",
		 function );

		return( -1 );
	}
	if( read_offset >= (off64_t) media_size )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid offset.\n",
		 function );

		return( -1 );
	}
	if( ( read_size + read_offset ) > media_size )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to export beyond size of media.\n",
		 function );

		return( -1 );
	}
	if( libewf_set_write_input_size( export_handle, read_size ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to set write size in export handle.\n",
		 function );

		return( -1 );
	}
	if( libewf_parse_header_values( handle, LIBEWF_DATE_FORMAT_ISO8601 ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to parse header values in handle.\n",
		 function );

		return( -1 );
	}
	if( libewf_copy_header_values( export_handle, handle ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to set copy header values to export handle.\n",
		 function );

		return( -1 );
	}
	read_all    = (uint8_t) ( ( read_size == media_size ) && ( read_offset == 0 ) );
	buffer_size = EWFCOMMON_BUFFER_SIZE;
	data        = (uint8_t *) libewf_common_alloc( buffer_size * sizeof( uint8_t ) );

	if( data == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate data.\n",
		 function );

		return( -1 );
	}
	while( total_read_count < (int64_t) read_size )
	{
		size = buffer_size;

		if( ( media_size - total_read_count ) < size )
		{
			size = (size_t) ( media_size - total_read_count );
		}
		read_count = libewf_read_random( handle, (void *) data, size, read_offset );

		if( read_count <= -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: error reading data.\n",
			 function );

			libewf_common_free( data );

			return( -1 );
		}
		if( read_count == 0 )
		{
			LIBEWF_WARNING_PRINT( "%s: unexpected end of data.\n",
			 function );

			libewf_common_free( data );

			return( -1 );
		}
		if( read_count > (ssize_t) size )
		{
			LIBEWF_WARNING_PRINT( "%s: more bytes read than requested.\n",
			 function );

			libewf_common_free( data );

			return( -1 );
		}
		read_offset += size;

		/* Swap byte pairs
		 */
		if( ( swap_byte_pairs == 1 )
		 && ( ewfcommon_swap_byte_pairs( data, read_count ) != 1 ) )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to swap byte pairs.\n",
			 function );

			return( -1 );
		}
		write_count = libewf_write_buffer( export_handle, data, (size_t) read_count );

		if( write_count < read_count )
		{
			LIBEWF_WARNING_PRINT( "%s: error writing data.\n",
			 function );

			libewf_common_free( data );

			return( -1 );
		}
		total_read_count += read_count;

		if( callback != NULL )
		{
			callback( total_read_count, read_size );
		}
  	}
	libewf_common_free( data );

	return( total_read_count );
}

