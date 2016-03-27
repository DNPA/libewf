/*
 * Section reading/writing functions
 *
 * Copyright (c) 2006-2010, Joachim Metz <jbmetz@users.sourceforge.net>
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
#include <byte_stream.h>
#include <memory.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>
#include <libnotify.h>

#include "libewf_chunk_value.h"
#include "libewf_definitions.h"
#include "libewf_compression.h"
#include "libewf_debug.h"
#include "libewf_hash_sections.h"
#include "libewf_header_values.h"
#include "libewf_header_sections.h"
#include "libewf_libbfio.h"
#include "libewf_media_values.h"
#include "libewf_offset_table.h"
#include "libewf_section.h"
#include "libewf_section_list.h"
#include "libewf_sector_list.h"
#include "libewf_segment_file_handle.h"
#include "libewf_single_files.h"
#include "libewf_unused.h"

#include "ewf_data.h"
#include "ewf_definitions.h"
#include "ewf_digest.h"
#include "ewf_error2.h"
#include "ewf_file_header.h"
#include "ewf_hash.h"
#include "ewf_ltree.h"
#include "ewf_session.h"
#include "ewf_volume.h"
#include "ewf_volume_smart.h"
#include "ewfx_delta_chunk.h"

/* Tests if a buffer entirely consists of zero values
 * Returns 1 if zero, 0 if not, or -1 on error
 */
int libewf_section_test_zero(
     uint8_t *buffer,
     size_t size,
     liberror_error_t **error )
{
	static char *function = "libewf_write_test_zero";
	size_t iterator       = 0;

	if( buffer == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid buffer.",
		 function );

		return( -1 );
	}
	if( size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid size value exceeds maximum.",
		 function );

		return( -1 );
	}
	for( iterator = 0;
	     iterator < size;
	     iterator++ )
	{
		if( buffer[ iterator ] != 0 )
		{
			return( 0 );
		}
	}
	return( 1 );
}

/* Reads the section start
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_start_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         ewf_section_t *section,
         uint64_t *section_size,
         uint64_t *section_next,
         liberror_error_t **error )
{
	static char *function        = "libewf_section_start_read";
	ssize_t read_count           = 0;
	uint32_t calculated_checksum = 0;
	uint32_t stored_checksum     = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid section.",
		 function );

		return( -1 );
	}
	if( section_size == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid section size.",
		 function );

		return( -1 );
	}
	if( section_next == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid section next.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              segment_file_handle->file_io_pool_entry,
	              (uint8_t *) section,
	              sizeof( ewf_section_t ),
	              error );

	if( read_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read section start.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
	 	 "%s: section:\n",
		 function );
		libnotify_print_data(
		 (uint8_t *) section,
		 sizeof( ewf_section_t ) );
	}
#endif
	byte_stream_copy_to_uint64_little_endian(
	 section->size,
	 *section_size );

	byte_stream_copy_to_uint64_little_endian(
	 section->next,
	 *section_next );

	byte_stream_copy_to_uint32_little_endian(
	 section->checksum,
	 stored_checksum );

	calculated_checksum = ewf_checksum_calculate(
	                       section,
	                       sizeof( ewf_section_t ) - sizeof( uint32_t ),
	                       1 );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: type\t\t\t\t: %s\n",
		 function,
		 (char *) section->type );

		libnotify_printf(
		 "%s: next\t\t\t\t: %" PRIu64 "\n",
		 function,
		 *section_next );

		libnotify_printf(
		 "%s: size\t\t\t\t: %" PRIu64 "\n",
		 function,
		 *section_size );

		libnotify_printf(
	 	 "%s: padding:\n",
		 function );
		libnotify_print_data(
		 section->padding,
		 40 );

		libnotify_printf(
		 "%s: checksum\t\t\t: 0x%08" PRIx32 " (0x%08" PRIx32 ")\n",
		 function,
		 stored_checksum,
		 calculated_checksum );

		libnotify_printf(
		 "\n" );
	}
#endif
	if( stored_checksum != calculated_checksum )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		*section_size = 0;
		*section_next = 0;

		return( -1 );
	}
	if( *section_size > (uint64_t) INT64_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		*section_size = 0;
		*section_next = 0;

		return( -1 );
	}
	if( *section_next > (uint64_t) INT64_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section next value exceeds maximum.",
		 function );

		*section_size = 0;
		*section_next = 0;

		return( -1 );
	}
	return( read_count );
}

/* Writes a section start to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_start_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         off64_t segment_file_offset,
         uint8_t *section_type,
         size_t section_type_length,
         size64_t section_data_size,
         liberror_error_t **error )
{
	ewf_section_t section;

	static char *function        = "libewf_section_start_write";
	ssize_t write_count          = 0;
	uint64_t section_size        = 0;
	uint64_t section_offset      = 0;
	uint32_t calculated_checksum = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( segment_file_offset < 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
		 "%s: invalid segment file offset value less than zero.",
		 function );

		return( -1 );
	}
	if( section_type == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid section type.",
		 function );

		return( -1 );
	}
	if( ( section_type_length == 0 )
	 || ( section_type_length >= 16 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: section type is out of bounds.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     &section,
	     0,
	     sizeof( ewf_section_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear section.",
		 function );

		return( -1 );
	}
	if( memory_copy(
	     section.type,
	     section_type,
	     section_type_length ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set section type.",
		 function );

		return( -1 );
	}
	section_size   = sizeof( ewf_section_t ) + section_data_size;
	section_offset = (uint64_t) segment_file_offset + section_size;

	if( section_size > (uint64_t) INT64_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( section_offset > (uint64_t) INT64_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section offset value exceeds maximum.",
		 function );

		return( -1 );
	}
	byte_stream_copy_from_uint64_little_endian(
	 section.size,
	 section_size );

	byte_stream_copy_from_uint64_little_endian(
	 section.next,
	 section_offset );

	calculated_checksum = ewf_checksum_calculate(
	                       &section,
	                       sizeof( ewf_section_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_from_uint32_little_endian(
	 section.checksum,
	 calculated_checksum );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: writing section start of type: %s with size: %" PRIu64 " and checksum: 0x%08" PRIx32 ".\n",
		 function,
		 (char *) section_type,
		 section_size,
		 calculated_checksum );
	}
#endif
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) &section,
	               sizeof( ewf_section_t ),
	               error );

	if( write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section to file.",
		 function );

		return( -1 );
	}
	return( write_count );
}

/* Reads a compressed string section and decompresses it
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_compressed_string_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t compressed_string_size,
         uint8_t **uncompressed_string,
         size_t *uncompressed_string_size,
         liberror_error_t **error )
{
	uint8_t *compressed_string = NULL;
	static char *function      = "libewf_section_compressed_string_read";
	void *reallocation         = NULL;
	ssize_t read_count         = 0;
	int result                 = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( uncompressed_string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid uncompressed string.",
		 function );

		return( -1 );
	}
	if( *uncompressed_string != NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid uncompressed string value already set.",
		 function );

		return( -1 );
	}
	if( uncompressed_string_size == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid uncompressed string size.",
		 function );

		return( -1 );
	}
	if( compressed_string_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid compressed string size value exceeds maximum.",
		 function );

		return( -1 );
	}
	compressed_string = (uint8_t *) memory_allocate(
	                                 sizeof( uint8_t ) * compressed_string_size );

	if( compressed_string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create compressed string.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              segment_file_handle->file_io_pool_entry,
	              compressed_string,
	              compressed_string_size,
	              error );

	if( read_count != (ssize_t) compressed_string_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read compressed string.",
		 function );

		memory_free(
		 compressed_string );

		return( -1 );
	}
	/* On average the uncompressed string will be twice as large as the compressed string
	 */
	*uncompressed_string_size = 2 * compressed_string_size;

	*uncompressed_string = (uint8_t *) memory_allocate(
	                                    sizeof( uint8_t ) * *uncompressed_string_size );

	if( *uncompressed_string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create uncompressed string.",
		 function );

		memory_free(
		 compressed_string );

		return( -1 );
	}
	result = libewf_decompress(
	          *uncompressed_string,
	          uncompressed_string_size,
	          compressed_string,
	          compressed_string_size,
	          error );

	while( ( result == -1 )
	    && ( *uncompressed_string_size > 0 ) )
	{
		liberror_error_free(
		 error );

		reallocation = memory_reallocate(
		                *uncompressed_string,
		                sizeof( uint8_t ) * *uncompressed_string_size );

		if( reallocation == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to resize uncompressed string.",
			 function );

			memory_free(
			 compressed_string );
			memory_free(
			 *uncompressed_string );

			*uncompressed_string = NULL;

			return( -1 );
		}
		*uncompressed_string = (uint8_t *) reallocation;

		result = libewf_decompress(
		          *uncompressed_string,
		          uncompressed_string_size,
		          compressed_string,
		          compressed_string_size,
		          error );
	}
	memory_free(
	 compressed_string );

	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_COMPRESSION,
		 LIBERROR_COMPRESSION_ERROR_UNCOMPRESS_FAILED,
		 "%s: unable to decompress string.",
		 function );

		memory_free(
		 *uncompressed_string );

		*uncompressed_string = NULL;

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: uncompressed string:\n",
		 function );
		libnotify_print_data(
		 *uncompressed_string,
		 *uncompressed_string_size );
	}
#endif
	return( read_count );
}

/* Writes a compressed string section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_write_compressed_string(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         off64_t segment_file_offset,
         uint8_t *section_type,
         size_t section_type_length,
         uint8_t *uncompressed_string,
         size_t uncompressed_string_size,
         int8_t compression_level,
         liberror_error_t **error )
{
	uint8_t *compressed_string    = NULL;
	static char *function         = "libewf_section_write_compressed_string";
	void *reallocation            = NULL;
	size_t compressed_string_size = 0;
	ssize_t section_write_count   = 0;
	ssize_t write_count           = 0;
	int result                    = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( segment_file_offset < 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
		 "%s: invalid segment file offset value less than zero.",
		 function );

		return( -1 );
	}
	if( section_type == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid section type.",
		 function );

		return( -1 );
	}
	if( uncompressed_string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid uncompressed string.",
		 function );

		return( -1 );
	}
	compressed_string_size = uncompressed_string_size;
	compressed_string      = (uint8_t *) memory_allocate(
	                                      sizeof( uint8_t ) * compressed_string_size );

	if( compressed_string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create compress string.",
		 function );

		return( -1 );
	}
	result = libewf_compress(
	          compressed_string,
	          &compressed_string_size,
	          uncompressed_string,
	          uncompressed_string_size,
	          compression_level,
	          error );

	if( ( result == -1 )
	 && ( compressed_string_size > 0 ) )
	{
		liberror_error_free(
		 error );

		reallocation = memory_reallocate(
		                compressed_string,
		                ( sizeof( uint8_t ) * compressed_string_size ) );

		if( reallocation == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to resize compressed string.",
			 function );

			memory_free(
			 compressed_string );

			return( -1 );
		}
		compressed_string = (uint8_t *) reallocation;

		result = libewf_compress(
		          compressed_string,
		          &compressed_string_size,
		          uncompressed_string,
		          uncompressed_string_size,
		          compression_level,
		          error );
	}
	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_COMPRESSION,
		 LIBERROR_COMPRESSION_ERROR_COMPRESS_FAILED,
		 "%s: unable to compress string.",
		 function );

		memory_free(
		 compressed_string );

		return( -1 );
	}
	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       segment_file_offset,
	                       section_type,
	                       section_type_length,
	                       (size64_t) compressed_string_size,
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section to file.",
		 function );

		memory_free(
		 compressed_string );

		return( -1 );
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               compressed_string,
	               compressed_string_size,
	               error );

	memory_free(
	 compressed_string );

	if( write_count != (ssize_t) compressed_string_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write string to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	if( libewf_section_list_append(
	     segment_file_handle->section_list,
	     section_type,
	     section_type_length,
	     segment_file_offset,
	     segment_file_offset + section_write_count,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append: %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a header section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_header_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint8_t **cached_header,
         size_t *cached_header_size,
         liberror_error_t **error )
{
	uint8_t *header       = NULL;
	static char *function = "libewf_section_header_read";
	ssize_t read_count    = 0;
	size_t header_size    = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( cached_header == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid cached header.",
		 function );

		return( -1 );
	}
	if( cached_header_size == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid cached header size.",
		 function );

		return( -1 );
	}
	read_count = libewf_section_compressed_string_read(
	              file_io_pool,
	              segment_file_handle,
	              section_size,
	              &header,
	              &header_size,
	              error );

	if( read_count != (ssize_t) section_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read header.",
		 function );

		return( -1 );
	}
	if( header == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: missing header.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( libewf_debug_byte_stream_print(
		     "Header",
		     header,
		     header_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to debug print header.",
			 function );

			memory_free(
			 header );

			return( -1 );
		}
	}
#endif
	if( *cached_header == NULL )
	{
		*cached_header      = header;
		*cached_header_size = header_size;
	}
	else
	{
		memory_free(
		 header );
	}
	return( read_count );
}

/* Writes a header section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_header_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *header,
         size_t header_size,
         int8_t compression_level,
         liberror_error_t **error )
{
	static char *function       = "libewf_section_header_write";
	off64_t section_offset      = 0;
	ssize_t section_write_count = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( libewf_debug_byte_stream_print(
		     "Header",
		     header,
		     header_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to debug print header.",
			 function );

			return( -1 );
		}
	}
#endif
	section_write_count = libewf_section_write_compressed_string(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       (uint8_t *) "header",
	                       6,
	                       header,
	                       header_size,
	                       compression_level,
	                       error );

	if( section_write_count == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write header to file.",
		 function );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a header2 section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_header2_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint8_t **cached_header2,
         size_t *cached_header2_size,
         liberror_error_t **error )
{
	uint8_t *header2      = NULL;
	static char *function = "libewf_section_header2_read";
	ssize_t read_count    = 0;
	size_t header2_size   = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( cached_header2 == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid cached header2.",
		 function );

		return( -1 );
	}
	if( cached_header2_size == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid cached header2 size.",
		 function );

		return( -1 );
	}
	read_count = libewf_section_compressed_string_read(
	              file_io_pool,
	              segment_file_handle,
	              section_size,
	              &header2,
	              &header2_size,
	              error );

	if( read_count != (ssize_t) section_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read header2.",
		 function );

		return( -1 );
	}
	if( header2 == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: missing header2.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( libewf_debug_utf16_stream_print(
		     "Header2",
		     header2,
		     header2_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to debug print header2.",
			 function );

			memory_free(
			 header2 );

			return( -1 );
		}
	}
#endif
	if( *cached_header2 == NULL )
	{
		*cached_header2      = header2;
		*cached_header2_size = header2_size;
	}
	else
	{
		memory_free(
		 header2 );
	}
	return( read_count );
}

/* Writes a header2 section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_header2_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *header2,
         size_t header2_size,
         int8_t compression_level,
         liberror_error_t **error )
{
	static char *function       = "libewf_section_header2_write";
	off64_t section_offset      = 0;
	ssize_t section_write_count = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( libewf_debug_utf16_stream_print(
		     "Header2",
		     header2,
		     header2_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to debug print header2.",
			 function );

			return( -1 );
		}
	}
#endif
	section_write_count = libewf_section_write_compressed_string(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       (uint8_t *) "header2",
	                       7,
	                       header2,
	                       header2_size,
	                       compression_level,
	                       error );

	if( section_write_count == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write header2 to file.",
		 function );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads an EWF-S01 (SMART) volume section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_volume_s01_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         uint8_t *format,
         liberror_error_t **error )
{
	ewf_volume_smart_t *volume   = NULL;
	static char *function        = "libewf_section_volume_s01_read";
	uint32_t calculated_checksum = 0;
	uint32_t stored_checksum     = 0;
	ssize_t read_count           = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( media_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media values.",
		 function );

		return( -1 );
	}
	if( format == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid format.",
		 function );

		return( -1 );
	}
	volume = memory_allocate_structure(
	          ewf_volume_smart_t );

	if( volume == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create volume.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              segment_file_handle->file_io_pool_entry,
	              (uint8_t *) volume,
	              sizeof( ewf_volume_smart_t ),
	              error );

	if( read_count != (ssize_t) sizeof( ewf_volume_smart_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read volume.",
		 function );

		memory_free(
		 volume );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 volume->checksum,
	 stored_checksum );

	calculated_checksum = ewf_checksum_calculate(
	                       volume,
	                       sizeof( ewf_volume_smart_t ) - sizeof( uint32_t ),
	                       1 );

	if( stored_checksum != calculated_checksum )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: unknown1:\n",
		 function );
		libnotify_print_data(
		 volume->unknown1,
		 4 );
		libnotify_printf(
		 "%s: unknown2:\n",
		 function );
		libnotify_print_data(
		 volume->unknown2,
		 20 );
		libnotify_printf(
		 "%s: unknown3:\n",
		 function );
		libnotify_print_data(
		 volume->unknown3,
		 45 );
	}
#endif

	byte_stream_copy_to_uint32_little_endian(
	 volume->number_of_chunks,
	 media_values->number_of_chunks );

	byte_stream_copy_to_uint32_little_endian(
	 volume->sectors_per_chunk,
	 media_values->sectors_per_chunk );

	byte_stream_copy_to_uint32_little_endian(
	 volume->bytes_per_sector,
	 media_values->bytes_per_sector );

	byte_stream_copy_to_uint32_little_endian(
	 volume->number_of_sectors,
	 media_values->number_of_sectors );

	if( memory_compare(
	     (void *) volume->signature,
	     (void *) "SMART",
	     5 ) == 0 )
	{
		*format = LIBEWF_FORMAT_SMART;
	}
	else
	{
		*format = LIBEWF_FORMAT_EWF;
	}
	memory_free(
	 volume );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: number of chunks\t: %" PRIu32 "\n",
		 function,
		 media_values->number_of_chunks );
		libnotify_printf(
		 "%s: sectors per chunk\t: %" PRIu32 "\n",
		 function,
		 media_values->sectors_per_chunk );
		libnotify_printf(
		 "%s: bytes per sector\t: %" PRIu32 "\n",
		 function,
		 media_values->bytes_per_sector );
		libnotify_printf(
		 "%s: number of sectors\t: %" PRIu32 "\n",
		 function,
		 media_values->number_of_sectors );
		libnotify_printf(
		 "\n" );
	}
#endif
	return( read_count );
}

/* Writes an EWF-S01 (SMART) volume section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_volume_s01_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         uint8_t format,
         uint8_t no_section_append,
         liberror_error_t **error )
{
	uint8_t *section_type        = (uint8_t *) "volume";
	ewf_volume_smart_t *volume   = NULL;
	static char *function        = "libewf_section_volume_s01_write";
	off64_t section_offset       = 0;
	size_t section_type_length   = 6;
	ssize_t section_write_count  = 0;
	ssize_t write_count          = 0;
	uint32_t calculated_checksum = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( media_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media values.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	volume = memory_allocate_structure(
	          ewf_volume_smart_t );

	if( volume == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create volume.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     volume,
	     0,
	     sizeof( ewf_volume_smart_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear volume.",
		 function );

		memory_free(
		 volume );

		return( -1 );
	}
	volume->unknown1[ 0 ] = 1;

	byte_stream_copy_from_uint32_little_endian(
	 volume->number_of_chunks,
	 media_values->number_of_chunks );

	byte_stream_copy_from_uint32_little_endian(
	 volume->sectors_per_chunk,
	 media_values->sectors_per_chunk );

	byte_stream_copy_from_uint32_little_endian(
	 volume->bytes_per_sector,
	 media_values->bytes_per_sector );

	byte_stream_copy_from_uint32_little_endian(
	 volume->number_of_sectors,
	 media_values->number_of_sectors );

	if( format == LIBEWF_FORMAT_SMART )
	{
		volume->signature[ 0 ] = (uint8_t) 'S';
		volume->signature[ 1 ] = (uint8_t) 'M';
		volume->signature[ 2 ] = (uint8_t) 'A';
		volume->signature[ 3 ] = (uint8_t) 'R';
		volume->signature[ 4 ] = (uint8_t) 'T';
	}
	calculated_checksum = ewf_checksum_calculate(
	                       volume,
	                       sizeof( ewf_volume_smart_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_from_uint32_little_endian(
	 volume->checksum,
	 calculated_checksum );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: volume has %" PRIu32 " chunks of %" PRIi32 " bytes (%" PRIi32 " sectors) each.\n",
		 function,
		 media_values->number_of_chunks,
		 media_values->chunk_size,
		 media_values->sectors_per_chunk );

		libnotify_printf(
		 "%s: volume has %" PRIu64 " sectors of %" PRIi32 " bytes each.\n",
		 function,
		 media_values->number_of_sectors,
		 media_values->bytes_per_sector );
	}
#endif
	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       section_type,
	                       section_type_length,
	                       (size64_t) sizeof( ewf_volume_smart_t ),
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section to file.",
		 function );

		memory_free(
		 volume );

		return( -1 );
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) volume,
	               sizeof( ewf_volume_smart_t ),
	               error );

	memory_free(
	 volume );

	if( write_count != (ssize_t) sizeof( ewf_volume_smart_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write volume to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	if( ( no_section_append == 0 )
	 && ( libewf_section_list_append(
	       segment_file_handle->section_list,
	       section_type,
	       section_type_length,
	       section_offset,
	       section_offset + section_write_count,
	       error ) != 1 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append: %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads an EWF-E01 (EnCase) volume section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_volume_e01_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         int8_t *compression_level,
         liberror_error_t **error )
{
	ewf_volume_t *volume         = NULL;
	static char *function        = "libewf_section_volume_e01_read";
	ssize_t read_count           = 0;
	uint32_t calculated_checksum = 0;
	uint32_t stored_checksum     = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( media_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media values.",
		 function );

		return( -1 );
	}
	if( compression_level == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid compression level.",
		 function );

		return( -1 );
	}
	volume = memory_allocate_structure(
	          ewf_volume_t );

	if( volume == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to read volume.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              segment_file_handle->file_io_pool_entry,
	              (uint8_t *) volume,
	              sizeof( ewf_volume_t ),
	              error );

	if( read_count != (ssize_t) sizeof( ewf_volume_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read volume.",
		 function );

		memory_free(
		 volume );

		return( -1 );
	}
	calculated_checksum = ewf_checksum_calculate(
	                       volume,
	                       sizeof( ewf_volume_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_to_uint32_little_endian(
	 volume->checksum,
	 stored_checksum );

	if( stored_checksum != calculated_checksum )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		memory_free(
		 volume );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: unknown1:\n",
		 function );
		libnotify_print_data(
		 volume->unknown1,
		 3 );
		libnotify_printf(
		 "%s: unknown2:\n",
		 function );
		libnotify_print_data(
		 volume->unknown2,
		 3 );
		libnotify_printf(
		 "%s: PALM volume start sector.\n",
		 function );
		libnotify_print_data(
		 volume->palm_volume_start_sector,
		 4 );
		libnotify_printf(
		 "%s: unknown3:\n",
		 function );
		libnotify_print_data(
		 volume->unknown3,
		 4 );
		libnotify_printf(
		 "%s: SMART logs start sector.\n",
		 function );
		libnotify_print_data(
		 volume->smart_logs_start_sector,
		 4 );
		libnotify_printf(
		 "%s: unknown4:\n",
		 function );
		libnotify_print_data(
		 volume->unknown4,
		 3 );
		libnotify_printf(
		 "%s: unknown5:\n",
		 function );
		libnotify_print_data(
		 volume->unknown5,
		 4 );
		libnotify_printf(
		 "%s: unknown6:\n",
		 function );
		libnotify_print_data(
		 volume->unknown6,
		 963 );
		libnotify_printf(
		 "%s: signature:\n",
		 function );
		libnotify_print_data(
		 volume->signature,
		 5 );
	}
#endif

	byte_stream_copy_to_uint32_little_endian(
	 volume->number_of_chunks,
	 media_values->number_of_chunks );

	byte_stream_copy_to_uint32_little_endian(
	 volume->sectors_per_chunk,
	 media_values->sectors_per_chunk );

	byte_stream_copy_to_uint32_little_endian(
	 volume->bytes_per_sector,
	 media_values->bytes_per_sector );

	byte_stream_copy_to_uint64_little_endian(
	 volume->number_of_sectors,
	 media_values->number_of_sectors );

	byte_stream_copy_to_uint32_little_endian(
	 volume->error_granularity,
	 media_values->error_granularity );

	media_values->media_type  = volume->media_type;
	media_values->media_flags = volume->media_flags;
	*compression_level        = (int8_t) volume->compression_level;

	if( memory_copy(
	     media_values->guid,
	     volume->guid,
	     16 ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set GUID.",
		 function );

		memory_free(
		 volume );

		return( -1 );
	}
	memory_free(
	 volume );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: number of chunks\t: %" PRIu32 "\n",
		 function,
		 media_values->number_of_chunks );
		libnotify_printf(
		 "%s: sectors per chunk\t: %" PRIu32 "\n",
		 function,
		 media_values->sectors_per_chunk );
		libnotify_printf(
		 "%s: bytes per sector\t: %" PRIu32 "\n",
		 function,
		 media_values->bytes_per_sector );
		libnotify_printf(
		 "%s: number of sectors\t: %" PRIu32 "\n",
		 function,
		 media_values->number_of_sectors );
		libnotify_printf(
		 "%s: error granularity\t: %" PRIu32 "\n",
		 function,
		 media_values->error_granularity );
		libnotify_printf(
		 "%s: media type\t\t: 0x%02" PRIx8 "\n",
		 function,
		 media_values->media_type );
		libnotify_printf(
		 "%s: media flags\t\t: 0x%02" PRIx8 "\n",
		 function,
		 media_values->media_flags );
		libnotify_printf(
		 "%s: compression level\t: 0x%02" PRIx8 "\n",
		 function,
		 *compression_level);
		libnotify_printf(
		 "\n" );
	}
#endif
	return( read_count );
}

/* Writes an EWF-E01 (EnCase) volume section to file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_volume_e01_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         int8_t compression_level,
         uint8_t format,
         uint8_t no_section_append,
         liberror_error_t **error )
{
	uint8_t *section_type        = (uint8_t *) "volume";
	ewf_volume_t *volume         = NULL;
	static char *function        = "libewf_section_volume_e01_write";
	off64_t section_offset       = 0;
	size_t section_type_length   = 6;
	ssize_t section_write_count  = 0;
	ssize_t write_count          = 0;
	uint32_t calculated_checksum = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( media_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media values.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	volume = memory_allocate_structure(
	          ewf_volume_t );

	if( volume == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create volume.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     volume,
	     0,
	     sizeof( ewf_volume_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear volume.",
		 function );

		memory_free(
		 volume );

		return( -1 );
	}
	if( format == LIBEWF_FORMAT_FTK )
	{
		volume->media_type = 0x01;
	}
	else
	{
		volume->media_type = media_values->media_type;
	}
	volume->media_flags = media_values->media_flags;

	byte_stream_copy_from_uint32_little_endian(
	 volume->number_of_chunks,
	 media_values->number_of_chunks );

	byte_stream_copy_from_uint32_little_endian(
	 volume->sectors_per_chunk,
	 media_values->sectors_per_chunk );

	byte_stream_copy_from_uint32_little_endian(
	 volume->bytes_per_sector,
	 media_values->bytes_per_sector );

	byte_stream_copy_from_uint64_little_endian(
	 volume->number_of_sectors,
	 media_values->number_of_sectors );

	if( ( format == LIBEWF_FORMAT_ENCASE5 )
	 || ( format == LIBEWF_FORMAT_ENCASE6 )
	 || ( format == LIBEWF_FORMAT_LINEN5 )
	 || ( format == LIBEWF_FORMAT_LINEN6 )
	 || ( format == LIBEWF_FORMAT_EWFX ) )
	{
		volume->compression_level = (uint8_t) compression_level;

		if( memory_copy(
		     volume->guid,
		     media_values->guid,
		     16 ) == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_COPY_FAILED,
			 "%s: unable to set GUID.",
			 function );

			memory_free(
			 volume );

			return( -1 );
		}
		byte_stream_copy_from_uint32_little_endian(
		 volume->error_granularity,
		 media_values->error_granularity );
	}
	calculated_checksum = ewf_checksum_calculate(
	                       volume,
	                       sizeof( ewf_volume_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_from_uint32_little_endian(
	 volume->checksum,
	 calculated_checksum );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: volume has %" PRIu32 " chunks of %" PRIi32 " bytes (%" PRIi32 " sectors) each.\n",
		 function,
		 media_values->number_of_chunks,
		 media_values->chunk_size,
		 media_values->sectors_per_chunk );

		libnotify_printf(
		 "%s: volume has %" PRIu64 " sectors of %" PRIi32 " bytes each.\n",
		 function,
		 media_values->number_of_sectors,
		 media_values->bytes_per_sector );
	}
#endif
	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       section_type,
	                       section_type_length,
	                       (size64_t) sizeof( ewf_volume_t ),
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section to file.",
		 function );

		memory_free(
		 volume );

		return( -1 );
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) volume,
	               sizeof( ewf_volume_t ),
	               error );

	if( write_count != (ssize_t) sizeof( ewf_volume_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write volume to file.",
		 function );

		memory_free(
		 volume );

		return( -1 );
	}
	memory_free(
	 volume );

	section_write_count += write_count;

	if( ( no_section_append == 0 )
	 && ( libewf_section_list_append(
	       segment_file_handle->section_list,
	       section_type,
	       section_type_length,
	       section_offset,
	       section_offset + section_write_count,
	       error ) != 1 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append: %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a volume section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_volume_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         libewf_media_values_t *media_values,
         int8_t *compression_level,
         uint8_t *format,
         uint8_t *ewf_format,
         liberror_error_t **error )
{
	static char *function    = "libewf_section_volume_read";
	ssize_t read_count       = 0;
	size64_t bytes_per_chunk = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( media_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media values.",
		 function );

		return( -1 );
	}
	if( format == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid format.",
		 function );

		return( -1 );
	}
	if( ewf_format == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid ewf format.",
		 function );

		return( -1 );
	}
	if( section_size == sizeof( ewf_volume_smart_t ) )
	{
		*ewf_format = EWF_FORMAT_S01;
		read_count  = libewf_section_volume_s01_read(
		               file_io_pool,
		               segment_file_handle,
		               media_values,
		               format,
		               error );
	}
	else if( section_size == sizeof( ewf_volume_t ) )
	{
		*ewf_format = EWF_FORMAT_E01;
		read_count  = libewf_section_volume_e01_read(
		               file_io_pool,
		               segment_file_handle,
		               media_values,
		               compression_level,
		               error );
	}
	else
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported section data size.",
		 function );

		return( -1 );
	}
	if( read_count != (ssize_t) section_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read volume section.",
		 function );

		return( -1 );
	}
	if( media_values->sectors_per_chunk > (uint32_t) INT32_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid sectors per chunk value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( media_values->bytes_per_sector > (uint32_t) INT32_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid bytes per sector value exceeds maximum.",
		 function );

		return( -1 );
	}
	bytes_per_chunk = (size64_t) media_values->sectors_per_chunk
	                * (size64_t) media_values->bytes_per_sector;

	if( bytes_per_chunk > (size64_t) INT32_MAX )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: chunk size value exceeds maximum defaulting to: %d.\n",
			 function,
			 EWF_MINIMUM_CHUNK_SIZE );
		}
#endif
		media_values->chunk_size = EWF_MINIMUM_CHUNK_SIZE;
	}
	else
	{
		media_values->chunk_size = (uint32_t) bytes_per_chunk;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: volume has %" PRIu32 " chunks of %" PRIi32 " bytes (%" PRIi32 " sectors) each.\n",
		 function,
		 media_values->number_of_chunks,
		 media_values->chunk_size,
		 media_values->sectors_per_chunk );

		libnotify_printf(
		 "%s: volume has %" PRIu64 " sectors of %" PRIi32 " bytes each.\n",
		 function,
		 media_values->number_of_sectors,
		 media_values->bytes_per_sector );
	}
#endif
	if( media_values->media_type == LIBEWF_MEDIA_TYPE_SINGLE_FILES )
	{
		*ewf_format = EWF_FORMAT_L01;
	}
	return( read_count );
}

/* Reads a table section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_table_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint32_t media_number_of_chunks,
         libewf_offset_table_t *offset_table,
         uint8_t format LIBEWF_ATTRIBUTE_UNUSED,
         uint8_t ewf_format,
         liberror_error_t **error )
{
	ewf_table_t table;
	uint8_t stored_checksum_buffer[ 4 ];

	ewf_table_offset_t *offsets     = NULL;
	static char *function           = "libewf_section_table_read";
	size_t offsets_size             = 0;
	ssize_t section_read_count      = 0;
	ssize_t read_count              = 0;
	uint64_t base_offset            = 0;
	uint32_t calculated_checksum    = 0;
	uint32_t number_of_chunks       = 0;
	uint32_t number_of_chunk_values = 0;
	uint32_t stored_checksum        = 0;
	uint8_t offsets_tainted         = 0;

	LIBEWF_UNREFERENCED_PARAMETER( format )

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( libewf_offset_table_get_number_of_chunk_values(
	     offset_table,
	     &number_of_chunk_values,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve the number of chunk values in the offset table.",
		 function );

		return( -1 );
	}
	/* Allocate the necessary number of chunk offsets
	 * this reduces the number of reallocations
	 */
	if( number_of_chunk_values < media_number_of_chunks )
	{
		if( libewf_offset_table_resize(
		     offset_table,
		     media_number_of_chunks,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_RESIZE_FAILED,
			 "%s: unable to resize offset table.",
			 function );

			return( -1 );
		}
	}
	section_read_count = libbfio_pool_read(
	                      file_io_pool,
	                      segment_file_handle->file_io_pool_entry,
	                      (uint8_t *) &table,
	                      sizeof( ewf_table_t ),
	                      error );
	
	if( section_read_count != (ssize_t) sizeof( ewf_table_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read table.",
		 function );

		return( -1 );
	}
	/* The table size contains the size of the checksum (4 bytes)
	 */
	calculated_checksum = ewf_checksum_calculate(
	                       &table,
	                       sizeof( ewf_table_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_to_uint32_little_endian(
	 table.checksum,
	 stored_checksum );

	if( stored_checksum != calculated_checksum )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 table.number_of_chunks,
	 number_of_chunks );

	byte_stream_copy_to_uint64_little_endian(
	 table.base_offset,
	 base_offset );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: padding1:\n",
		 function );
		libnotify_print_data(
		 table.padding1,
		 4 );
		libnotify_printf(
		 "%s: padding2:\n",
		 function );
		libnotify_print_data(
		 table.padding2,
		 4 );
	}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: table is of size %" PRIu32 " chunks checksum 0x%08" PRIx32 " (0x%08" PRIx32 ").\n",
		 function,
		 number_of_chunks,
		 stored_checksum,
		 calculated_checksum );
	}
#endif
	if( number_of_chunks > 0 )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		/* Check if the maximum number of offsets is not exceeded
		 */
		if( number_of_chunks > EWF_MAXIMUM_OFFSETS_IN_TABLE )
		{
			if( libnotify_verbose != 0 )
			{
				libnotify_printf(
				 "%s: table contains more offsets: %" PRIu32 " than the maximum number: %d.\n",
				 function,
				 number_of_chunks,
				 EWF_MAXIMUM_OFFSETS_IN_TABLE );
			}
		}
#endif
		offsets_size = sizeof( ewf_table_offset_t ) * number_of_chunks;

		if( offsets_size > (size_t) SSIZE_MAX )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
			 "%s: invalid offsets size value exceeds maximum.",
			 function );

			return( -1 );
		}
		offsets = (ewf_table_offset_t *) memory_allocate(
		                                  offsets_size );

		if( offsets == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to allocated table offsets.",
			 function );

			return( -1 );
		}
		read_count = libbfio_pool_read(
		              file_io_pool,
		              segment_file_handle->file_io_pool_entry,
		              (uint8_t *) offsets,
		              offsets_size,
		              error );
	
		if( read_count != (ssize_t) offsets_size )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read table offsets.",
			 function );

			memory_free(
			 offsets );

			return( -1 );
		}
		section_read_count += read_count;

		/* The EWF-S01 format does not contain a checksum after the offsets
		 */
		if( ewf_format != EWF_FORMAT_S01 )
		{
			/* Check if the offset table checksum matches
			 */
			calculated_checksum = ewf_checksum_calculate(
			                       offsets,
			                       offsets_size,
			                       1 );

			read_count = libbfio_pool_read(
			              file_io_pool,
			              segment_file_handle->file_io_pool_entry,
			              stored_checksum_buffer,
			              sizeof( uint32_t ),
			              error );

			if( read_count != (ssize_t) sizeof( uint32_t ) )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read checksum from file descriptor.",
				 function );

				memory_free(
				 offsets );

				return( -1 );
			}
			section_read_count += read_count;

			byte_stream_copy_to_uint32_little_endian(
			 stored_checksum_buffer,
			 stored_checksum );

			if( stored_checksum != calculated_checksum )
			{
#if defined( HAVE_VERBOSE_OUTPUT )
				if( libnotify_verbose != 0 )
				{
					libnotify_printf(
					 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").\n",
					 function,
					 stored_checksum,
					 calculated_checksum );
				}
#endif
				/* The offsets cannot be fully trusted therefore mark them as tainted during fill 
				 */
				offsets_tainted = 1;
			}
		}
		if( libewf_offset_table_fill(
		     offset_table,
		     (off64_t) base_offset,
		     offsets,
		     number_of_chunks,
		     segment_file_handle,
		     offsets_tainted,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to fill offset table.",
			 function );

			memory_free(
			 offsets );

			return( -1 );
		}
		memory_free(
		 offsets );

		if( libewf_offset_table_fill_last_offset(
		     offset_table,
		     segment_file_handle->section_list,
		     offsets_tainted,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to fill last offset.",
			 function );

			return( -1 );
		}
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	else if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: table section contains no offset data.\n",
		 function );
	}
	if( section_size < (size_t) section_read_count )
	{
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: section size: %" PRIzd " smaller than section read count: %" PRIzd ".\n",
			 function,
			 section_size,
			 section_read_count );
		}
	}
#endif
	/* Skip the chunk data within the section
	 * for chunks after the table section
	 */
	if( section_size != (size_t) section_read_count )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		if( ( ewf_format != EWF_FORMAT_S01 )
		 && ( format != LIBEWF_FORMAT_ENCASE1 ) )
		{
			if( libnotify_verbose != 0 )
			{
				libnotify_printf(
			 	 "%s: data found after table offsets.\n",
				 function );
			}
		}
#endif
		if( libbfio_pool_seek_offset(
		     file_io_pool,
		     segment_file_handle->file_io_pool_entry,
		     section_size - section_read_count,
		     SEEK_CUR,
		     error ) == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_SEEK_FAILED,
			 "%s: unable to align with next section.",
			 function );

			return( -1 );
		}
		section_read_count = (ssize_t) section_size;
	}
	segment_file_handle->number_of_chunks += number_of_chunks;

	return( section_read_count );
}

/* Reads a table2 section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_table2_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint32_t media_number_of_chunks,
         libewf_offset_table_t *offset_table,
         uint8_t format LIBEWF_ATTRIBUTE_UNUSED,
         uint8_t ewf_format,
         liberror_error_t **error )
{
	ewf_table_t table;
	uint8_t stored_checksum_buffer[ 4 ];

	ewf_table_offset_t *offsets     = NULL;
	static char *function           = "libewf_section_table2_read";
	size_t offsets_size             = 0;
	ssize_t section_read_count      = 0;
	ssize_t read_count              = 0;
	uint64_t base_offset            = 0;
	uint32_t calculated_checksum    = 0;
	uint32_t number_of_chunks       = 0;
	uint32_t number_of_chunk_values = 0;
	uint32_t stored_checksum        = 0;
	uint8_t offsets_tainted         = 0;

	LIBEWF_UNREFERENCED_PARAMETER( format )

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( libewf_offset_table_get_number_of_chunk_values(
	     offset_table,
	     &number_of_chunk_values,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve the number of chunk values in the offset table.",
		 function );

		return( -1 );
	}
	/* Allocate the necessary number of chunk offsets
	 * this reduces the number of reallocations
	 */
	if( number_of_chunk_values < media_number_of_chunks )
	{
		if( libewf_offset_table_resize(
		     offset_table,
		     media_number_of_chunks,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_RESIZE_FAILED,
			 "%s: unable to resize offset table.",
			 function );

			return( -1 );
		}
	}
	section_read_count = libbfio_pool_read(
	                      file_io_pool,
	                      segment_file_handle->file_io_pool_entry,
	                      (uint8_t *) &table,
	                      sizeof( ewf_table_t ),
	                      error );
	
	if( section_read_count != (ssize_t) sizeof( ewf_table_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read table.",
		 function );

		return( -1 );
	}
	/* The table size contains the size of the checksum (4 bytes)
	 */
	calculated_checksum = ewf_checksum_calculate(
	                       &table,
	                       sizeof( ewf_table_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_to_uint32_little_endian(
	 table.checksum,
	 stored_checksum );

	if( stored_checksum != calculated_checksum )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 table.number_of_chunks,
	 number_of_chunks );

	byte_stream_copy_to_uint64_little_endian(
	 table.base_offset,
	 base_offset );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: padding1:\n",
		 function );
		libnotify_print_data(
		 table.padding1,
		 4 );
		libnotify_printf(
		 "%s: padding2:\n",
		 function );
		libnotify_print_data(
		 table.padding2,
		 4 );
	}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: table is of size %" PRIu32 " chunks checksum 0x%08" PRIx32 " (0x%08" PRIx32 ").\n",
		 function,
		 number_of_chunks,
		 stored_checksum,
		 calculated_checksum );
	}
#endif
	if( number_of_chunks > 0 )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		/* Check if the maximum number of offsets is not exceeded
		 */
		if( number_of_chunks > EWF_MAXIMUM_OFFSETS_IN_TABLE )
		{
			if( libnotify_verbose != 0 )
			{
				libnotify_printf(
				 "%s: table contains more offsets: %" PRIu32 " than the maximum number: %d.\n",
				 function,
				 number_of_chunks,
				 EWF_MAXIMUM_OFFSETS_IN_TABLE );
			}
		}
#endif
		offsets_size = sizeof( ewf_table_offset_t ) * number_of_chunks;

		if( offsets_size > (size_t) SSIZE_MAX )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
			 "%s: invalid offsets size value exceeds maximum.",
			 function );

			return( -1 );
		}
		offsets = (ewf_table_offset_t *) memory_allocate(
		                                  offsets_size );

		if( offsets == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to allocated table offsets.",
			 function );

			return( -1 );
		}
		read_count = libbfio_pool_read(
		              file_io_pool,
		              segment_file_handle->file_io_pool_entry,
		              (uint8_t *) offsets,
		              offsets_size,
		              error );
	
		if( read_count != (ssize_t) offsets_size )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read table offsets.",
			 function );

			memory_free(
			 offsets );

			return( -1 );
		}
		section_read_count += read_count;

		/* The EWF-S01 format does not contain a checksum after the offsets
		 */
		if( ewf_format != EWF_FORMAT_S01 )
		{
			/* Check if the offset table checksum matches
			 */
			calculated_checksum = ewf_checksum_calculate(
			                       offsets,
			                       offsets_size,
			                       1 );

			read_count = libbfio_pool_read(
			              file_io_pool,
			              segment_file_handle->file_io_pool_entry,
			              stored_checksum_buffer,
			              sizeof( uint32_t ),
			              error );

			if( read_count != (ssize_t) sizeof( uint32_t ) )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read checksum from file descriptor.",
				 function );

				memory_free(
				 offsets );

				return( -1 );
			}
			section_read_count += read_count;

			byte_stream_copy_to_uint32_little_endian(
			 stored_checksum_buffer,
			 stored_checksum );

			if( stored_checksum != calculated_checksum )
			{
#if defined( HAVE_VERBOSE_OUTPUT )
				if( libnotify_verbose != 0 )
				{
					libnotify_printf(
					 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").\n",
					 function,
					 stored_checksum,
					 calculated_checksum );
				}
#endif
				/* The offsets cannot be trusted therefore do not try to correct corrupted offsets during compare
				 */
				offsets_tainted = 1;
			}
		}
		if( libewf_offset_table_compare(
		     offset_table,
		     (off64_t) base_offset,
		     offsets,
		     number_of_chunks,
		     segment_file_handle,
		     offsets_tainted,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to compare offset table.",
			 function );

			memory_free(
			 offsets );

			return( -1 );
		}
		memory_free(
		 offsets );

		if( libewf_offset_table_compare_last_offset(
		     offset_table,
		     segment_file_handle->section_list,
		     offsets_tainted,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to compare last offset.",
			 function );

			return( -1 );
		}
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	else if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: table section contains no offset data.\n",
		 function );
	}
	if( section_size < (size_t) section_read_count )
	{
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: section size: %" PRIzd " smaller than section read count: %" PRIzd ".\n",
			 function,
			 section_size,
			 section_read_count );
		}
	}
#endif
	/* Skip the chunk data within the section
	 * for chunks after the table section
	 */
	else if( section_size != (size_t) section_read_count )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		if( ( ewf_format != EWF_FORMAT_S01 )
		 && ( format != LIBEWF_FORMAT_ENCASE1 ) )
		{
			if( libnotify_verbose != 0 )
			{
				libnotify_printf(
				 "%s: data found after table offsets.\n",
				 function );
			}
		}
#endif
		if( libbfio_pool_seek_offset(
		     file_io_pool,
		     segment_file_handle->file_io_pool_entry,
		     section_size - section_read_count,
		     SEEK_CUR,
		     error ) == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_SEEK_FAILED,
			 "%s: unable to align with next section.",
			 function );

			return( -1 );
		}
		section_read_count = (ssize_t) section_size;
	}
	segment_file_handle->number_of_chunks += number_of_chunks;

	return( section_read_count );
}

/* Writes a table or table2 section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_table_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         off64_t base_offset,
         ewf_table_offset_t *offsets,
         uint32_t number_of_offsets,
         uint8_t *section_type,
         size_t section_type_length,
         size_t additional_size,
         uint8_t format LIBEWF_ATTRIBUTE_UNUSED,
         uint8_t ewf_format,
         uint8_t no_section_append,
         liberror_error_t **error )
{
	ewf_table_t table;
	uint8_t calculated_checksum_buffer[ 4 ];

	static char *function        = "libewf_section_table_write";
	off64_t section_offset       = 0;
	ssize_t section_write_count  = 0;
	ssize_t write_count          = 0;
	size_t section_size          = 0;
	size_t offsets_size          = 0;
	uint32_t calculated_checksum = 0;
	uint8_t write_checksum       = 0;

	LIBEWF_UNREFERENCED_PARAMETER( format )

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file",
		 function );

		return( -1 );
	}
	if( base_offset < 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
		 "%s: invalid base offset value less than zero.",
		 function );

		return( -1 );
	}
	if( offsets == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid offsets.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	offsets_size = sizeof( ewf_table_offset_t ) * number_of_offsets;
	section_size = sizeof( ewf_table_t ) + offsets_size + additional_size;

	if( ewf_format != EWF_FORMAT_S01 )
	{
		write_checksum = 1;
		section_size  += sizeof( uint32_t );
	}
	if( memory_set(
	     &table,
	     0,
	     sizeof( ewf_table_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear table.",
		 function );

		return( -1 );
	}
	byte_stream_copy_from_uint32_little_endian(
	 table.number_of_chunks,
	 number_of_offsets );

	byte_stream_copy_from_uint64_little_endian(
	 table.base_offset,
	 base_offset );

	calculated_checksum = ewf_checksum_calculate(
	                       &table,
	                       sizeof( ewf_table_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_from_uint32_little_endian(
	 table.checksum,
	 calculated_checksum );

	if( write_checksum != 0 )
	{
		calculated_checksum = ewf_checksum_calculate(
		                       offsets,
		                       offsets_size,
		                       1 );
	}
	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       section_type,
	                       section_type_length,
	                       (size64_t) section_size,
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section: %s to file.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) &table,
	               sizeof( ewf_table_t ),
	               error );

	if( write_count != (ssize_t) sizeof( ewf_table_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write table to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) offsets,
	               offsets_size,
	               error );

	if( write_count != (ssize_t) offsets_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write table offsets to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	if( write_checksum != 0 )
	{
		byte_stream_copy_from_uint32_little_endian(
		 calculated_checksum_buffer,
		 calculated_checksum );

		write_count = libbfio_pool_write(
		               file_io_pool,
		               segment_file_handle->file_io_pool_entry,
	        	       calculated_checksum_buffer,
	        	       sizeof( uint32_t ),
		               error );

		if( write_count != (ssize_t) sizeof( uint32_t ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_WRITE_FAILED,
			 "%s: unable to write table offsets checksum to file.",
			 function );

			return( -1 );
		}
		section_write_count += write_count;
	}
	if( ( no_section_append == 0 )
	 && ( libewf_section_list_append(
	       segment_file_handle->section_list,
	       section_type,
	       section_type_length,
	       section_offset,
	       section_offset + section_size,
	       error ) != 1 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append: %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a sectors section from file
 * Returns the number of bytes read or -1 on error
 */
ssize64_t libewf_section_sectors_read(
           libbfio_pool_t *file_io_pool,
           libewf_segment_file_handle_t *segment_file_handle,
           size64_t section_size,
           uint8_t ewf_format LIBEWF_ATTRIBUTE_UNUSED,
           liberror_error_t **error )
{
	static char *function = "libewf_section_sectors_read";

	LIBEWF_UNREFERENCED_PARAMETER( ewf_format )

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section_size > (size64_t) INT64_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	/* In the EWF-E01 format the sectors section holds the actual data chunks
	 */
	if( ewf_format == EWF_FORMAT_S01 )
	{
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: EWF-S01 format should not contain sectors section.\n",
			 function );
		}
	}
#endif
	/* Skip the chunk data within the section
	 */
	if( libbfio_pool_seek_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     section_size,
	     SEEK_CUR,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to align with next section.",
		 function );

		return( -1 );
	}
	return( (ssize64_t) section_size );
}

/* Writes a sectors section to file
 * Does not write the actual data in the sectors section
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_sectors_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size64_t sectors_data_size,
         uint8_t no_section_append,
         liberror_error_t **error )
{
	uint8_t *section_type       = (uint8_t *) "sectors";
	static char *function       = "libewf_section_sectors_write";
	off64_t section_offset      = 0;
	size_t section_type_length  = 7;
	ssize_t section_write_count = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       section_type,
	                       section_type_length,
	                       sectors_data_size,
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section: %s to file.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	if( ( no_section_append == 0 )
	 && ( libewf_section_list_append(
	       segment_file_handle->section_list,
	       section_type,
	       section_type_length,
	       section_offset,
	       section_offset + sectors_data_size,
	       error ) != 1 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append: %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a ltree section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_ltree_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint8_t *ewf_format,
         uint8_t **cached_ltree_data,
         size_t *cached_ltree_data_size,
         liberror_error_t **error )
{
	ewf_ltree_t *ltree         = NULL;
	uint8_t *ltree_data        = NULL;
	static char *function      = "libewf_section_ltree_read";
	size_t ltree_data_size     = 0;
	ssize_t read_count         = 0;
	ssize_t section_read_count = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( ewf_format == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid ewf format.",
		 function );

		return( -1 );
	}
	if( cached_ltree_data == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid cached ltree.",
		 function );

		return( -1 );
	}
	if( cached_ltree_data_size == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid cached ltree data size.",
		 function );

		return( -1 );
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	if( *ewf_format == EWF_FORMAT_S01 )
	{
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: EWF-S01 format should not contain ltree section.\n",
			 function );
		}
	}
#endif
	*ewf_format = EWF_FORMAT_L01;

	ltree = memory_allocate_structure(
	         ewf_ltree_t );

	if( ltree == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create ltree.",
		 function );

		return( -1 );
	}
	section_read_count = libbfio_pool_read(
	                      file_io_pool,
	                      segment_file_handle->file_io_pool_entry,
	                      (uint8_t *) ltree,
	                      sizeof( ewf_ltree_t ),
	                      error );
	
	if( section_read_count != (ssize_t) sizeof( ewf_ltree_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read ltree.",
		 function );

		memory_free(
		 ltree );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: unknown1:\n",
		 function );
		libnotify_print_data(
		 ltree->unknown1,
		 16 );
		libnotify_printf(
		 "%s: tree size:\n",
		 function );
		libnotify_print_data(
		 ltree->tree_size,
		 4 );
		libnotify_printf(
		 "%s: unknown2:\n",
		 function );
		libnotify_print_data(
		 ltree->unknown2,
		 4 );
		libnotify_printf(
		 "%s: unknown3:\n",
		 function );
		libnotify_print_data(
		 ltree->unknown3,
		 4 );
		libnotify_printf(
		 "%s: unknown4:\n",
		 function );
		libnotify_print_data(
		 ltree->unknown4,
		 20 );
	}
#endif

	memory_free(
	 ltree );

	ltree_data_size = section_size - sizeof( ewf_ltree_t );

	ltree_data = (uint8_t *) memory_allocate(
                                  sizeof( uint8_t ) * ltree_data_size );

	if( ltree_data == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create ltree data.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              segment_file_handle->file_io_pool_entry,
	              ltree_data,
	              ltree_data_size,
	              error );

	if( read_count != (ssize_t) ltree_data_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read ltree data.",
		 function );

		memory_free(
		 ltree_data );

		return( -1 );
	}
	section_read_count += read_count;

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( libewf_debug_utf16_stream_print(
		     "ltree data",
		     ltree_data,
		     ltree_data_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to debug print ltree data.",
			 function );

			memory_free(
			 ltree_data );

			return( -1 );
		}
	}
#endif
	if( *cached_ltree_data == NULL )
	{
		*cached_ltree_data      = ltree_data;
		*cached_ltree_data_size = ltree_data_size;
	}
	else
	{
		memory_free(
		 ltree_data );
	}
	return( section_read_count );
}

/* Reads a session section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_session_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         libewf_sector_list_t *sessions,
         size_t section_size,
         uint8_t ewf_format LIBEWF_ATTRIBUTE_UNUSED,
         liberror_error_t **error )
{
	ewf_session_t ewf_session;
	uint8_t stored_checksum_buffer[ 4 ];

	ewf_session_entry_t *ewf_sessions = NULL;
	static char *function             = "libewf_section_session_read";
	ssize_t section_read_count        = 0;
	ssize_t read_count                = 0;
	size_t ewf_sessions_size          = 0;
	uint32_t calculated_checksum      = 0;
	uint32_t ewf_session_index        = 0;
	uint32_t first_sector             = 0;
	uint32_t last_first_sector        = 0;
	uint32_t number_of_ewf_sessions   = 0;
	uint32_t number_of_sectors        = 0;
	uint32_t stored_checksum          = 0;
	int number_of_elements            = 0;

	LIBEWF_UNREFERENCED_PARAMETER( ewf_format )

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( media_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media values.",
		 function );

		return( -1 );
	}
	if( sessions == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid sessions.",
		 function );

		return( -1 );
	}
	if( section_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( ewf_format == EWF_FORMAT_S01 )
		{
			libnotify_printf(
		 	 "%s: EWF-S01 format should not contain session section.\n",
			 function );
		}
	}
#endif
	section_read_count = libbfio_pool_read(
	                      file_io_pool,
	                      segment_file_handle->file_io_pool_entry,
	                      (uint8_t *) &ewf_session,
	                      sizeof( ewf_session_t ),
	                      error );

	if( section_read_count != (ssize_t) sizeof( ewf_session_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read session.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: session data:\n",
		 function );
		libnotify_print_data(
		 (uint8_t *) &ewf_session,
		 sizeof( ewf_session_t ) );
	}
#endif
	byte_stream_copy_to_uint32_little_endian(
	 ewf_session.number_of_sessions,
	 number_of_ewf_sessions );

	byte_stream_copy_to_uint32_little_endian(
	 ewf_session.checksum,
	 stored_checksum );

	calculated_checksum = ewf_checksum_calculate(
	                       &ewf_session,
	                       sizeof( ewf_session_t ) - sizeof( uint32_t ),
	                       1 );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: number of sessions: %" PRIu32 "\n",
		 function,
		 number_of_ewf_sessions );

		libnotify_printf(
		 "%s: unknown1:\n",
		 function );
		libnotify_print_data(
		 ewf_session.unknown1,
		 28 );

		libnotify_printf(
		 "%s: checksum\t\t\t: 0x%08" PRIx32 " (0x%08" PRIx32 ")\n",
		 function,
		 stored_checksum,
		 calculated_checksum );

		libnotify_printf(
		 "\n" );
	}
#endif
	if( stored_checksum != calculated_checksum )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		return( -1 );
	}
	if( number_of_ewf_sessions > 0 )
	{
		ewf_sessions_size = sizeof( ewf_session_entry_t ) * number_of_ewf_sessions;

		ewf_sessions = (ewf_session_entry_t *) memory_allocate(
		                                        ewf_sessions_size );

		if( ewf_sessions == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create session data.",
			 function );

			return( -1 );
		}
		read_count = libbfio_pool_read(
		              file_io_pool,
		              segment_file_handle->file_io_pool_entry,
		              (uint8_t *) ewf_sessions,
		              ewf_sessions_size,
		              error );
	
		if( read_count != (ssize_t) ewf_sessions_size )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read session data.",
			 function );

			memory_free(
			 ewf_sessions );

			return( -1 );
		}
		section_read_count += read_count;

#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: session entries data:\n",
			 function );
			libnotify_print_data(
			 (uint8_t *) ewf_sessions,
			 ewf_sessions_size );
		}
#endif
		calculated_checksum = ewf_checksum_calculate(
		                       ewf_sessions,
		                       ewf_sessions_size,
		                       1 );

		read_count = libbfio_pool_read(
		              file_io_pool,
		              segment_file_handle->file_io_pool_entry,
		              stored_checksum_buffer,
		              sizeof( uint32_t ),
		              error );

		if( read_count != (ssize_t) sizeof( uint32_t ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read checksum from file descriptor.",
			 function );

			memory_free(
			 ewf_sessions );

			return( -1 );
		}
		section_read_count += read_count;

		byte_stream_copy_to_uint32_little_endian(
		 stored_checksum_buffer,
		 stored_checksum );

#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: checksum\t\t\t: 0x%08" PRIx32 " (0x%08" PRIx32 ")\n",
			 function,
			 stored_checksum,
			 calculated_checksum );

			libnotify_printf(
			 "\n" );
		}
#endif
		if( stored_checksum != calculated_checksum )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_INPUT,
			 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
			 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
			 function,
			 stored_checksum,
			 calculated_checksum );

			memory_free(
			 ewf_sessions );

			return( -1 );
		}
		if( libewf_sector_list_get_number_of_elements(
		     sessions,
		     &number_of_elements,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve number of elements from sessions sector list.",
			 function );

			memory_free(
			 ewf_sessions );

			return( -1 );
		}
		if( number_of_elements == 0 )
		{
			byte_stream_copy_to_uint32_little_endian(
			 ( ewf_sessions[ ewf_session_index ] ).first_sector,
			 last_first_sector );

#if defined( HAVE_DEBUG_OUTPUT )
			if( libnotify_verbose != 0 )
			{
				libnotify_printf(
				 "%s: session: %" PRIu32 " unknown1:\n",
				 function,
				 ewf_session_index );
				libnotify_print_data(
				 ( ewf_sessions[ ewf_session_index ] ).unknown1,
				 4 );

				libnotify_printf(
				 "%s: session: %" PRIu32 " first sector\t: 0 (%" PRIu32 ")\n",
				 function,
				 ewf_session_index,
				 last_first_sector );

				libnotify_printf(
				 "%s: session: %" PRIu32 " unknown2:\n",
				 function,
				 ewf_session_index );
				libnotify_print_data(
				 ( ewf_sessions[ ewf_session_index ] ).unknown2,
				 24 );
			}
#endif
			/* Note that EnCase says the first session starts at session 16
			 * This is either some EnCase specific behavior or the value is used for
			 * other purposes.
			 */
			last_first_sector = 0;

			for( ewf_session_index = 1;
			     ewf_session_index < number_of_ewf_sessions;
			     ewf_session_index++ )
			{
				byte_stream_copy_to_uint32_little_endian(
				 ewf_sessions[ ewf_session_index ].first_sector,
				 first_sector );

#if defined( HAVE_DEBUG_OUTPUT )
				if( libnotify_verbose != 0 )
				{
					libnotify_printf(
					 "%s: session: %" PRIu32 " unknown1:\n",
					 function,
					 ewf_session_index );
					libnotify_print_data(
					 ( ewf_sessions[ ewf_session_index ] ).unknown1,
					 4 );

					libnotify_printf(
					 "%s: session: %" PRIu32 " first sector\t: %" PRIu32 "\n",
					 function,
					 ewf_session_index,
					 first_sector );

					libnotify_printf(
					 "%s: session: %" PRIu32 " unknown2:\n",
					 function,
					 ewf_session_index );
					libnotify_print_data(
					 ( ewf_sessions[ ewf_session_index ] ).unknown2,
					 24 );
				}
#endif
				if( first_sector < last_first_sector )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
					 "%s: invalid first sector: %" PRIu32 " value out of bounds.",
					 function,
					 first_sector );

					memory_free(
					 ewf_sessions );

					return( -1 );
				}
				number_of_sectors = first_sector - last_first_sector;

				if( libewf_sector_list_append_sector(
				     sessions,
				     (uint64_t) last_first_sector,
				     (uint64_t) number_of_sectors,
				     0,
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
					 "%s: unable to append session to sector list.",
					 function );

					memory_free(
					 ewf_sessions );

					return( -1 );
				}
				last_first_sector = first_sector;
			}
			if( media_values->number_of_sectors > last_first_sector )
			{
				number_of_sectors = (uint32_t) ( media_values->number_of_sectors - last_first_sector );
			}
			else
			{
				number_of_sectors = 0;
			}
			memory_free(
			 ewf_sessions );

			if( libewf_sector_list_append_sector(
			     sessions,
			     (uint64_t) last_first_sector,
			     (uint64_t) number_of_sectors,
			     0,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to append session to sector list.",
				 function );

				return( -1 );
			}
		}
#if defined( HAVE_VERBOSE_OUTPUT )
		else if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: sessions already set.\n",
			 function );
		}
#endif
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	else if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: session section contains no session data.\n",
		 function );
	}
#endif
	return( section_read_count );
}

/* Writes a session section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_session_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_sector_list_t *sessions,
         liberror_error_t **error )
{
	ewf_session_t ewf_session;
	uint8_t calculated_checksum_buffer[ 4 ];

	ewf_session_entry_t *ewf_sessions = NULL;
	uint8_t *section_type             = (uint8_t *) "session";
	static char *function             = "libewf_section_session_write";
	off64_t section_offset            = 0;
	ssize_t section_write_count       = 0;
	ssize_t write_count               = 0;
	size_t section_type_length        = 7;
	size_t section_size               = 0;
	size_t ewf_sessions_size          = 0;
	uint64_t first_sector             = 0;
	uint64_t number_of_sectors        = 0;
	uint32_t calculated_checksum      = 0;
	int number_of_sessions            = 0;
	int session_index                 = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	if( libewf_sector_list_get_number_of_elements(
	     sessions,
	     &number_of_sessions,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of elements from sessions sector list.",
		 function );

		return( -1 );
	}
	if( number_of_sessions <= 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid number of sessions value out of bounds.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     &ewf_session,
	     0,
	     sizeof( ewf_session_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear session.",
		 function );

		return( -1 );
	}
	byte_stream_copy_from_uint32_little_endian(
	 ewf_session.number_of_sessions,
	 number_of_sessions );

	calculated_checksum = ewf_checksum_calculate(
	                       &ewf_session,
	                       sizeof( ewf_session_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_from_uint32_little_endian(
	 ewf_session.checksum,
	 calculated_checksum );

	ewf_sessions_size = sizeof( ewf_session_entry_t ) * number_of_sessions;

	ewf_sessions = (ewf_session_entry_t *) memory_allocate(
						ewf_sessions_size );

	if( ewf_sessions == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create session entries.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     ewf_sessions,
	     0,
	     ewf_sessions_size ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear sessions entries.",
		 function );

		memory_free(
		 ewf_sessions );

		return( -1 );
	}
	for( session_index = 0;
	     session_index < number_of_sessions;
	     session_index++ )
	{
		if( libewf_sector_list_get_sector(
		     sessions,
		     session_index,
		     &first_sector,
		     &number_of_sectors,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve session: %d from sector list.",
			 function,
			 session_index );

			memory_free(
			 ewf_sessions );

			return( -1 );
		}
		/* Note that EnCase says the first session starts at session 16
		 * This is either some EnCase specific behavior or the value is used for
		 * other purposes.
		 */
		if( ( session_index == 0 )
		 && ( first_sector == 0 ) )
		{
			first_sector = 16;
		}
		byte_stream_copy_from_uint32_little_endian(
		 ewf_sessions[ session_index ].first_sector,
		 (uint32_t) first_sector );
	}
	calculated_checksum = ewf_checksum_calculate(
			       ewf_sessions,
			       ewf_sessions_size,
			       1 );

	section_size = sizeof( ewf_session_t ) + ewf_sessions_size + sizeof( uint32_t );

	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       section_type,
	                       section_type_length,
	                       (size64_t) section_size,
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section: %s to file.",
		 function,
		 (char *) section_type );

		memory_free(
		 ewf_sessions );

		return( -1 );
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) &ewf_session,
	               sizeof( ewf_session_t ),
	               error );

	if( write_count != (ssize_t) sizeof( ewf_session_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write session to file.",
		 function );

		memory_free(
		 ewf_sessions );

		return( -1 );
	}
	section_write_count += write_count;

	write_count = libbfio_pool_write(
		       file_io_pool,
		       segment_file_handle->file_io_pool_entry,
		       (uint8_t *) ewf_sessions,
		       ewf_sessions_size,
		       error );

	if( write_count != (ssize_t) ewf_sessions_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write session entries to file.",
		 function );

		memory_free(
		 ewf_sessions );

		return( -1 );
	}
	section_write_count += write_count;

	memory_free(
	 ewf_sessions );

	byte_stream_copy_from_uint32_little_endian(
	 calculated_checksum_buffer,
	 calculated_checksum );

	write_count = libbfio_pool_write(
		       file_io_pool,
		       segment_file_handle->file_io_pool_entry,
		       calculated_checksum_buffer,
		       sizeof( uint32_t ),
		       error );

	if( write_count != (ssize_t) sizeof( uint32_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write session entries checksum to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	if( libewf_section_list_append(
	     segment_file_handle->section_list,
	     section_type,
	     section_type_length,
	     section_offset,
	     section_offset + section_write_count,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a data section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_data_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         libewf_media_values_t *media_values,
         uint8_t ewf_format LIBEWF_ATTRIBUTE_UNUSED,
         liberror_error_t **error )
{
	ewf_data_t *data             = NULL;
	static char *function        = "libewf_section_data_read";
	ssize_t read_count           = 0;
	uint64_t number_of_sectors   = 0;
	uint32_t bytes_per_sector    = 0;
	uint32_t calculated_checksum = 0;
	uint32_t error_granularity   = 0;
	uint32_t number_of_chunks    = 0;
	uint32_t sectors_per_chunk   = 0;
	uint32_t stored_checksum     = 0;

	LIBEWF_UNREFERENCED_PARAMETER( ewf_format )

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( media_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media values.",
		 function );

		return( -1 );
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	if( ewf_format == EWF_FORMAT_S01 )
	{
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: EWF-S01 format should not contain data section.\n",
			 function );
		}
	}
#endif
	if( section_size != sizeof( ewf_data_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: mismatch in section data size.",
		 function );

		return( -1 );
	}
	data = memory_allocate_structure(
	        ewf_data_t );

	if( data == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create data.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              segment_file_handle->file_io_pool_entry,
	              (uint8_t *) data,
	              sizeof( ewf_data_t ),
	              error );
	
	if( read_count != (ssize_t) sizeof( ewf_data_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read data.",
		 function );

		memory_free(
		 data );

		return( -1 );
	}
	calculated_checksum = ewf_checksum_calculate(
	                       data,
	                       sizeof( ewf_data_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_to_uint32_little_endian(
	 data->checksum,
	 stored_checksum );

	if( stored_checksum != calculated_checksum )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: unknown1:\n",
		 function );
		libnotify_print_data(
		 data->unknown1,
		 3 );
		libnotify_printf(
		 "%s: unknown2:\n",
		 function );
		libnotify_print_data(
		 data->unknown2,
		 3 );
		libnotify_printf(
		 "%s: PALM volume start sector.\n",
		 function );
		libnotify_print_data(
		 data->palm_volume_start_sector,
		 4 );
		libnotify_printf(
		 "%s: unknown3:\n",
		 function );
		libnotify_print_data(
		 data->unknown3,
		 4 );
		libnotify_printf(
		 "%s: SMART logs start sector.\n",
		 function );
		libnotify_print_data(
		 data->smart_logs_start_sector,
		 4 );
		libnotify_printf(
		 "%s: unknown4:\n",
		 function );
		libnotify_print_data(
		 data->unknown4,
		 3 );
		libnotify_printf(
		 "%s: unknown5:\n",
		 function );
		libnotify_print_data(
		 data->unknown5,
		 4 );
		libnotify_printf(
		 "%s: unknown6:\n",
		 function );
		libnotify_print_data(
		 data->unknown6,
		 963 );
		libnotify_printf(
		 "%s: signature:\n",
		 function );
		libnotify_print_data(
		 data->signature,
		 5 );
	}
#endif
	if( ( data->media_type != 0 )
	 && ( data->media_type != media_values->media_type ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_VALUE_MISMATCH,
		 "%s: media type does not match in data section.",
		 function );

		memory_free(
		 data );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 data->number_of_chunks,
	 number_of_chunks );

	if( ( number_of_chunks != 0 )
	 && ( number_of_chunks != media_values->number_of_chunks ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_VALUE_MISMATCH,
		 "%s: number of chunks: %" PRIu32 " does not match number in data section: %" PRIu32 ".",
		 function,
		 media_values->number_of_chunks,
		 number_of_chunks );

		memory_free(
		 data );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 data->sectors_per_chunk,
	 sectors_per_chunk );

	if( ( sectors_per_chunk != 0 )
	 && ( sectors_per_chunk != media_values->sectors_per_chunk ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_VALUE_MISMATCH,
		 "%s: sectors per chunk does not match in data section.",
		 function );

		memory_free(
		 data );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 data->bytes_per_sector,
	 bytes_per_sector );

	if( ( bytes_per_sector != 0 )
	 && ( bytes_per_sector != media_values->bytes_per_sector ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_VALUE_MISMATCH,
		 "%s: bytes per sector does not match in data section.",
		 function );

		memory_free(
		 data );

		return( -1 );
	}
	byte_stream_copy_to_uint64_little_endian(
	 data->number_of_sectors,
	 number_of_sectors );

	if( ( number_of_sectors != 0 )
	 && ( number_of_sectors != media_values->number_of_sectors ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_VALUE_MISMATCH,
		 "%s: number of sectors does not match in data section.",
		 function );

		memory_free(
		 data );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 data->error_granularity,
	 error_granularity );

	if( ( error_granularity != 0 )
	 && ( error_granularity != media_values->error_granularity ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_VALUE_MISMATCH,
		 "%s: error granularity does not match in data section.",
		 function );

		memory_free(
		 data );

		return( -1 );
	}
	if( ( data->media_flags != 0 )
	 && ( data->media_flags != media_values->media_flags ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_VALUE_MISMATCH,
		 "%s: media flags do not match in data section.",
		 function );

		memory_free(
		 data );

		return( -1 );
	}
	if( ( data->guid[ 0 ] != 0 )
	 || ( data->guid[ 1 ] != 0 )
	 || ( data->guid[ 2 ] != 0 )
	 || ( data->guid[ 3 ] != 0 )
	 || ( data->guid[ 4 ] != 0 )
	 || ( data->guid[ 5 ] != 0 )
	 || ( data->guid[ 6 ] != 0 )
	 || ( data->guid[ 7 ] != 0 )
	 || ( data->guid[ 8 ] != 0 )
	 || ( data->guid[ 9 ] != 0 )
	 || ( data->guid[ 10 ] != 0 )
	 || ( data->guid[ 11 ] != 0 )
	 || ( data->guid[ 12 ] != 0 )
	 || ( data->guid[ 13 ] != 0 )
	 || ( data->guid[ 14 ] != 0 )
	 || ( data->guid[ 15 ] != 0 ) )
	{
		if( memory_compare(
		     media_values->guid,
		     data->guid,
		     16 ) != 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_INPUT,
			 LIBERROR_INPUT_ERROR_VALUE_MISMATCH,
			 "%s: GUID does not match in data section.",
			 function );

			memory_free(
			 data );

			return( -1 );
		}
	}
	memory_free(
	 data );

	return( read_count );
}

/* Writes a data section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_data_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         int8_t compression_level,
         uint8_t format,
         ewf_data_t **cached_data_section,
         uint8_t no_section_append,
         liberror_error_t **error )
{
	uint8_t *section_type        = (uint8_t *) "data";
	static char *function        = "libewf_section_data_write";
	off64_t section_offset       = 0;
	size_t section_type_length   = 4;
	ssize_t section_write_count  = 0;
	ssize_t write_count          = 0;
	uint32_t calculated_checksum = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( media_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media values.",
		 function );

		return( -1 );
	}
	if( cached_data_section == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid caches data section.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	/* Check if the data section was already created
	 */
	if( *cached_data_section == NULL )
	{
		*cached_data_section = memory_allocate_structure(
		                        ewf_data_t );

		if( *cached_data_section == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create data.",
			 function );

			return( -1 );
		}
		if( memory_set(
		     *cached_data_section,
		     0,
		     sizeof( ewf_data_t ) ) == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear data.",
			 function );

			memory_free(
			 *cached_data_section );

			*cached_data_section = NULL;

			return( -1 );
		}
		if( format == LIBEWF_FORMAT_FTK )
		{
			( *cached_data_section )->media_type = 0x01;
		}
		else
		{
			( *cached_data_section )->media_type = media_values->media_type;
		}
		( *cached_data_section )->media_flags = media_values->media_flags;

		byte_stream_copy_from_uint32_little_endian(
		 ( *cached_data_section )->number_of_chunks,
		 media_values->number_of_chunks );

		byte_stream_copy_from_uint32_little_endian(
		 ( *cached_data_section )->sectors_per_chunk,
		 media_values->sectors_per_chunk );

		byte_stream_copy_from_uint32_little_endian(
		 ( *cached_data_section )->bytes_per_sector,
		 media_values->bytes_per_sector );

		byte_stream_copy_from_uint64_little_endian(
		 ( *cached_data_section )->number_of_sectors,
		 media_values->number_of_sectors );

		if( ( format == LIBEWF_FORMAT_ENCASE5 )
		 || ( format == LIBEWF_FORMAT_ENCASE6 )
		 || ( format == LIBEWF_FORMAT_LINEN5 )
		 || ( format == LIBEWF_FORMAT_LINEN6 )
		 || ( format == LIBEWF_FORMAT_EWFX ) )
		{
			byte_stream_copy_from_uint32_little_endian(
			 ( *cached_data_section )->error_granularity,
			 media_values->error_granularity );

			( *cached_data_section )->compression_level = (uint8_t) compression_level;

			if( memory_copy(
			     ( *cached_data_section )->guid,
			     media_values->guid,
			     16 ) == NULL )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_MEMORY,
				 LIBERROR_MEMORY_ERROR_COPY_FAILED,
				 "%s: unable to set GUID.",
				 function );

				return( -1 );
			}
		}
		calculated_checksum = ewf_checksum_calculate(
		                       *cached_data_section,
		                       sizeof( ewf_data_t ) - sizeof( uint32_t ),
		                       1 );

		byte_stream_copy_from_uint32_little_endian(
		 ( *cached_data_section )->checksum,
		 calculated_checksum );
	}
	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       section_type,
	                       section_type_length,
	                       (size64_t) sizeof( ewf_data_t ),
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section: %s to file.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) *cached_data_section,
	               sizeof( ewf_data_t ),
	               error );

	if( write_count != (ssize_t) sizeof( ewf_data_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write data to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	if( ( no_section_append == 0 )
	 && ( libewf_section_list_append(
	       segment_file_handle->section_list,
	       section_type,
	       section_type_length,
	       section_offset,
	       section_offset + section_write_count,
	       error ) != 1 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append: %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a error2 section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_error2_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_sector_list_t *acquiry_errors,
         size_t section_size,
         uint8_t ewf_format LIBEWF_ATTRIBUTE_UNUSED,
         liberror_error_t **error )
{
	ewf_error2_t error2;
	uint8_t stored_checksum_buffer[ 4 ];

	ewf_error2_sector_t *error2_sectors = NULL;
	static char *function               = "libewf_section_error2_read";
	size_t error2_sectors_size          = 0;
	ssize_t section_read_count          = 0;
	ssize_t read_count                  = 0;
	uint32_t calculated_checksum        = 0;
	uint32_t error_index                = 0;
	uint32_t first_sector               = 0;
	uint32_t number_of_errors           = 0;
	uint32_t number_of_sectors          = 0;
	uint32_t stored_checksum            = 0;
	int number_of_elements              = 0;

	LIBEWF_UNREFERENCED_PARAMETER( ewf_format )

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( acquiry_errors == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid acquiry errors.",
		 function );

		return( -1 );
	}
	if( section_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	if( ewf_format == EWF_FORMAT_S01 )
	{
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: EWF-S01 format should not contain error2 section.\n",
			 function );
		}
	}
#endif
	section_read_count = libbfio_pool_read(
	                      file_io_pool,
	                      segment_file_handle->file_io_pool_entry,
	                      (uint8_t *) &error2,
	                      sizeof( ewf_error2_t ),
	                      error );
	
	if( section_read_count != (ssize_t) sizeof( ewf_error2_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read error2.",
		 function );

		return( -1 );
	}
	calculated_checksum = ewf_checksum_calculate(
	                       &error2,
	                       sizeof( ewf_error2_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_to_uint32_little_endian(
	 error2.checksum,
	 stored_checksum);

	byte_stream_copy_to_uint32_little_endian(
	 error2.number_of_errors,
	 number_of_errors );

	if( stored_checksum!= calculated_checksum)
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: unknown:\n",
		 function );
		libnotify_print_data(
		 error2.unknown,
		 200 );
	}
#endif

	if( number_of_errors > 0 )
	{
		error2_sectors_size = sizeof( ewf_error2_sector_t ) * number_of_errors;

		error2_sectors = (ewf_error2_sector_t *) memory_allocate(
		                                          error2_sectors_size );

		if( error2_sectors == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create error2 sectors.",
			 function );

			return( -1 );
		}
		read_count = libbfio_pool_read(
		              file_io_pool,
		              segment_file_handle->file_io_pool_entry,
		              (uint8_t *) error2_sectors,
		              error2_sectors_size,
	                      error );
	
		if( read_count != (ssize_t) error2_sectors_size )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read error2 sectors.",
			 function );

			memory_free(
			 error2_sectors );

			return( -1 );
		}
		section_read_count += read_count;

		calculated_checksum = ewf_checksum_calculate(
		                       error2_sectors,
		                       error2_sectors_size,
		                       1 );

		read_count = libbfio_pool_read(
		              file_io_pool,
		              segment_file_handle->file_io_pool_entry,
		              stored_checksum_buffer,
		              sizeof( uint32_t ),
	                      error );

		if( read_count != (ssize_t) sizeof( uint32_t ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read checksum from file descriptor.",
			 function );

			memory_free(
			 error2_sectors );

			return( -1 );
		}
		section_read_count += read_count;

		byte_stream_copy_to_uint32_little_endian(
		 stored_checksum_buffer,
		 stored_checksum );

		if( stored_checksum != calculated_checksum )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_INPUT,
			 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
			 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
			 function,
			 stored_checksum,
			 calculated_checksum );

			memory_free(
			 error2_sectors );

			return( -1 );
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: error2 sectors:\n",
			 function );
			libnotify_print_data(
			 (uint8_t *) error2_sectors,
			 error2_sectors_size );
		}
#endif
		if( libewf_sector_list_get_number_of_elements(
		     acquiry_errors,
		     &number_of_elements,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve number of elements from acquiry errors sector list.",
			 function );

			memory_free(
			 error2_sectors );

			return( -1 );
		}
		if( number_of_elements == 0 )
		{
			for( error_index = 0;
			     error_index < number_of_errors;
			     error_index++ )
			{
				byte_stream_copy_to_uint32_little_endian(
				 error2_sectors[ error_index ].first_sector,
				 first_sector );

				byte_stream_copy_to_uint32_little_endian(
				 error2_sectors[ error_index ].number_of_sectors,
				 number_of_sectors );

#if defined( HAVE_DEBUG_OUTPUT )
				if( libnotify_verbose != 0 )
				{
					libnotify_printf(
					 "%s: error2: %" PRIu32 " first sector\t: %" PRIu32 "\n",
					 function,
					 error_index,
					 first_sector );

					libnotify_printf(
					 "%s: error2: %" PRIu32 " number of sectors\t: %" PRIu32 "\n",
					 function,
					 error_index,
					 number_of_sectors );
				}
#endif
				if( libewf_sector_list_append_sector(
				     acquiry_errors,
				     (uint64_t) first_sector,
				     (uint64_t) number_of_sectors,
				     0,
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
					 "%s: unable to append acquiry error to sector list.",
					 function );

					memory_free(
					 error2_sectors );

					return( -1 );
				}
			}
			memory_free(
			 error2_sectors );
		}
#if defined( HAVE_VERBOSE_OUTPUT )
		else if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: acquiry errors already set.\n",
			 function );
		}
#endif
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	else if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: error2 section contains no error data.\n",
		 function );
	}
#endif
	return( section_read_count );
}

/* Writes a error2 section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_error2_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_sector_list_t *acquiry_errors,
         liberror_error_t **error )
{
	ewf_error2_t error2;
	uint8_t calculated_checksum_buffer[ 4 ];

	ewf_error2_sector_t *error2_sectors = NULL;
	uint8_t *section_type               = (uint8_t *) "error2";
	static char *function               = "libewf_section_error2_write";
	off64_t section_offset              = 0;
	ssize_t section_write_count         = 0;
	ssize_t write_count                 = 0;
	size_t section_type_length          = 6;
	size_t section_size                 = 0;
	size_t sectors_size                 = 0;
	uint64_t first_sector               = 0;
	uint64_t number_of_sectors          = 0;
	uint32_t calculated_checksum        = 0;
	int error_index                     = 0;
	int number_of_errors                = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	if( libewf_sector_list_get_number_of_elements(
	     acquiry_errors,
	     &number_of_errors,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of elements from acquiry error sector list.",
		 function );

		return( -1 );
	}
	if( number_of_errors <= 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid number of errors value out of bounds.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     &error2,
	     0,
	     sizeof( ewf_error2_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear error2.",
		 function );

		return( -1 );
	}
	byte_stream_copy_from_uint32_little_endian(
	 error2.number_of_errors,
	 number_of_errors );

	calculated_checksum = ewf_checksum_calculate(
	                       &error2,
	                       sizeof( ewf_error2_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_from_uint32_little_endian(
	 error2.checksum,
	 calculated_checksum );

	sectors_size = sizeof( ewf_error2_sector_t ) * number_of_errors;

	/* TODO EnCase compatible way to handle > 32-bit sector values
	 */
	error2_sectors = (ewf_error2_sector_t *) memory_allocate(
						  sectors_size );

	if( error2_sectors == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create error2 sectors.",
		 function );

		return( -1 );
	}
	for( error_index = 0;
	     error_index < number_of_errors;
	     error_index++ )
	{
		if( libewf_sector_list_get_sector(
		     acquiry_errors,
		     error_index,
		     &first_sector,
		     &number_of_sectors,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve acquiry error: %d from sector list.",
			 function,
			 error_index );

			memory_free(
			 error2_sectors );

			return( -1 );
		}
		byte_stream_copy_from_uint32_little_endian(
		 error2_sectors[ error_index ].first_sector,
		 (uint32_t) first_sector );

		byte_stream_copy_from_uint32_little_endian(
		 error2_sectors[ error_index ].number_of_sectors,
		 (uint32_t) number_of_sectors );
	}
	calculated_checksum = ewf_checksum_calculate(
			       error2_sectors,
			       sectors_size,
			       1 );

	section_size = sizeof( ewf_error2_t ) + sectors_size + sizeof( uint32_t );

	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       section_type,
	                       section_type_length,
	                       (size64_t) section_size,
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section: %s to file.",
		 function,
		 (char *) section_type );

		memory_free(
		 error2_sectors );

		return( -1 );
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) &error2,
	               sizeof( ewf_error2_t ),
	               error );

	if( write_count != (ssize_t) sizeof( ewf_error2_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write error2 to file.",
		 function );

		memory_free(
		 error2_sectors );

		return( -1 );
	}
	section_write_count += write_count;

	write_count = libbfio_pool_write(
		       file_io_pool,
		       segment_file_handle->file_io_pool_entry,
		       (uint8_t *) error2_sectors,
		       sectors_size,
		       error );

	if( write_count != (ssize_t) sectors_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write error2 sectors to file.",
		 function );

		memory_free(
		 error2_sectors );

		return( -1 );
	}
	section_write_count += write_count;

	memory_free(
	 error2_sectors );

	byte_stream_copy_from_uint32_little_endian(
	 calculated_checksum_buffer,
	 calculated_checksum );

	write_count = libbfio_pool_write(
		       file_io_pool,
		       segment_file_handle->file_io_pool_entry,
		       calculated_checksum_buffer,
		       sizeof( uint32_t ),
		       error );

	if( write_count != (ssize_t) sizeof( uint32_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write error2 sectors checksum to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	if( libewf_section_list_append(
	     segment_file_handle->section_list,
	     section_type,
	     section_type_length,
	     section_offset,
	     section_offset + section_write_count,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a digest section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_digest_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *md5_hash,
         uint8_t *sha1_hash,
         liberror_error_t **error )
{
	ewf_digest_t digest;

	static char *function        = "libewf_section_digest_read";
	ssize_t read_count           = 0;
	uint32_t calculated_checksum = 0;
	uint32_t stored_checksum     = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( md5_hash == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid md5 hash.",
		 function );

		return( -1 );
	}
	if( sha1_hash == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid sha1 hash.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              segment_file_handle->file_io_pool_entry,
	              (uint8_t *) &digest,
	              sizeof( ewf_digest_t ),
	              error );

	if( read_count != (ssize_t) sizeof( ewf_digest_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read digest.",
		 function );

		return( -1 );
	}
	calculated_checksum = ewf_checksum_calculate(
	                       &digest,
	                       sizeof( ewf_digest_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_to_uint32_little_endian(
	 digest.checksum,
	 stored_checksum );

	if( stored_checksum != calculated_checksum )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: MD5 hash:\n",
		 function );
		libnotify_print_data(
		 digest.md5_hash,
		 16 );
		libnotify_printf(
		 "%s: SHA1 hash:\n",
		 function );
		libnotify_print_data(
		 digest.sha1_hash,
		 20 );
		libnotify_printf(
		 "%s: padding:\n",
		 function );
		libnotify_print_data(
		 digest.padding1,
		 40 );
	}
#endif
	if( memory_copy(
	     md5_hash,
	     digest.md5_hash,
	     16 ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set MD5 hash in handle.",
		 function );

		return( -1 );
	}
	if( memory_copy(
	     sha1_hash,
	     digest.sha1_hash,
	     20 ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set SHA1 hash in handle.",
		 function );

		return( -1 );
	}
	return( read_count );
}

/* Writes a digest section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_digest_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *md5_hash,
         uint8_t *sha1_hash,
         liberror_error_t **error )
{
	ewf_digest_t digest;

	uint8_t *section_type        = (uint8_t *) "digest";
	static char *function        = "libewf_section_digest_write";
	off64_t section_offset       = 0;
	size_t section_type_length   = 6;
	ssize_t section_write_count  = 0;
	ssize_t write_count          = 0;
	uint32_t calculated_checksum = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( md5_hash == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid md5 hash.",
		 function );

		return( -1 );
	}
	if( sha1_hash == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid sha1 hash.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     &digest,
	     0,
	     sizeof( ewf_digest_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear digest.",
		 function );

		return( -1 );
	}
	if( memory_copy(
	     digest.md5_hash,
	     md5_hash,
	     16 ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set MD5 hash.",
		 function );

		return( -1 );
	}
	if( memory_copy(
	     digest.sha1_hash,
	     sha1_hash,
	     20 ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set MD5 hash.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: MD5 hash:\n",
		 function );
		libnotify_print_data(
		 digest.md5_hash,
		 16 );
		libnotify_printf(
		 "%s: SHA1 hash:\n",
		 function );
		libnotify_print_data(
		 digest.sha1_hash,
		 20 );
	}
#endif
	calculated_checksum = ewf_checksum_calculate(
	                       &digest,
	                       sizeof( ewf_digest_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_from_uint32_little_endian(
	 digest.checksum,
	 calculated_checksum );

	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       section_type,
	                       section_type_length,
	                       (size64_t) sizeof( ewf_digest_t ),
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section: %s to file.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) &digest,
	               sizeof( ewf_digest_t ),
	               error );

	if( write_count != (ssize_t) sizeof( ewf_digest_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write digest to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	if( libewf_section_list_append(
	     segment_file_handle->section_list,
	     section_type,
	     section_type_length,
	     section_offset,
	     section_offset + section_write_count,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}


/* Reads a hash section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_hash_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *md5_hash,
         liberror_error_t **error )
{
	ewf_hash_t hash;

	static char *function        = "libewf_section_hash_read";
	ssize_t read_count           = 0;
	uint32_t calculated_checksum = 0;
	uint32_t stored_checksum     = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( md5_hash == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid md5 hash.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              segment_file_handle->file_io_pool_entry,
	              (uint8_t *) &hash,
	              sizeof( ewf_hash_t ),
	              error );

	if( read_count != (ssize_t) sizeof( ewf_hash_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read hash.",
		 function );

		return( -1 );
	}
	calculated_checksum = ewf_checksum_calculate(
	                       &hash,
	                       sizeof( ewf_hash_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_to_uint32_little_endian(
	 hash.checksum,
	 stored_checksum );

	if( stored_checksum != calculated_checksum )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: MD5 hash:\n",
		 function );
		libnotify_print_data(
		 hash.md5_hash,
		 16 );
		libnotify_printf(
		 "%s: unknown1:\n",
		 function );
		libnotify_print_data(
		 hash.unknown1,
		 16 );
	}
#endif
	if( memory_copy(
	     md5_hash,
	     hash.md5_hash,
	     16 ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set MD5 hash in handle.",
		 function );

		return( -1 );
	}
	return( read_count );
}

/* Writes a hash section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_hash_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *md5_hash,
         liberror_error_t **error )
{
	ewf_hash_t hash;

	uint8_t *section_type        = (uint8_t *) "hash";
	static char *function        = "libewf_section_hash_write";
	off64_t section_offset       = 0;
	size_t section_type_length   = 4;
	ssize_t section_write_count  = 0;
	ssize_t write_count          = 0;
	uint32_t calculated_checksum = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( md5_hash == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid md5 hash.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     &hash,
	     0,
	     sizeof( ewf_hash_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear hash.",
		 function );

		return( -1 );
	}
	if( memory_copy(
	     hash.md5_hash,
	     md5_hash,
	     16 ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set MD5 hash.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: MD5 hash:\n",
		 function );
		libnotify_print_data(
		 hash.md5_hash,
		 16 );
	}
#endif
	calculated_checksum = ewf_checksum_calculate(
	                       &hash,
	                       sizeof( ewf_hash_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_from_uint32_little_endian(
	 hash.checksum,
	 calculated_checksum );

	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       section_type,
	                       section_type_length,
	                       (size64_t) sizeof( ewf_hash_t ),
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section: %s to file.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) &hash,
	               sizeof( ewf_hash_t ),
	               error );

	if( write_count != (ssize_t) sizeof( ewf_hash_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write hash to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	if( libewf_section_list_append(
	     segment_file_handle->section_list,
	     section_type,
	     section_type_length,
	     section_offset,
	     section_offset + section_write_count,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

/* Writes the last section start to file
 * This is used for the next and done sections,
 * these sections point back towards themselves
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_last_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *section_type,
         size_t section_type_length,
         uint8_t format,
         uint8_t ewf_format,
         liberror_error_t **error )
{
	ewf_section_t section;

	static char *function        = "libewf_section_last_write";
	ssize_t section_write_count  = 0;
	uint64_t section_size        = 0;
	uint64_t section_offset      = 0;
	uint32_t calculated_checksum = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file",
		 function );

		return( -1 );
	}
	if( section_type == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid section type.",
		 function );

		return( -1 );
	}
	if( ( section_type_length == 0 )
	 || ( section_type_length >= 16 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: section type length value out of bounds.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     &section,
	     0,
	     sizeof( ewf_section_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear section.",
		 function );

		return( -1 );
	}
	/* The EnCase (EWF-E01) format leaves the size of this section empty
	 */
	if( ( ewf_format == EWF_FORMAT_S01 )
	 || ( format == LIBEWF_FORMAT_FTK ) )
	{
		section_size = (uint64_t) sizeof( ewf_section_t );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     (off64_t *) &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	if( memory_copy(
	     section.type,
	     section_type,
	     section_type_length ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set section type.",
		 function );

		return( -1 );
	}
	byte_stream_copy_from_uint64_little_endian(
	 section.size,
	 section_size );

	byte_stream_copy_from_uint64_little_endian(
	 section.next,
	 section_offset );

	calculated_checksum = ewf_checksum_calculate(
	                       &section,
	                       sizeof( ewf_section_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_from_uint32_little_endian(	
	 section.checksum,
	 calculated_checksum );

	section_write_count = libbfio_pool_write(
	                       file_io_pool,
	                       segment_file_handle->file_io_pool_entry,
	                       (uint8_t *) &section,
	                       sizeof( ewf_section_t ),
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section: %s to file.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	if( libewf_section_list_append(
	     segment_file_handle->section_list,
	     section_type,
	     section_type_length,
	     section_offset,
	     section_offset + section_write_count,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a xheader section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_xheader_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint8_t **cached_xheader,
         size_t *cached_xheader_size,
         liberror_error_t **error )
{
	uint8_t *xheader      = NULL;
	static char *function = "libewf_section_xheader_read";
	ssize_t read_count    = 0;
	size_t xheader_size   = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( cached_xheader == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid cached xheader.",
		 function );

		return( -1 );
	}
	if( cached_xheader_size == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid cached xheader size.",
		 function );

		return( -1 );
	}
	read_count = libewf_section_compressed_string_read(
	              file_io_pool,
	              segment_file_handle,
	              section_size,
	              &xheader,
	              &xheader_size,
	              error );

	if( read_count != (ssize_t) section_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read xheader.",
		 function );

		return( -1 );
	}
	if( xheader == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: missing xheader.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( libewf_debug_utf8_stream_print(
		     "XHeader",
		     xheader,
		     xheader_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to debug print xheader.",
			 function );

			memory_free(
			 xheader );

			return( -1 );
		}
	}
#endif
	if( *cached_xheader == NULL )
	{
		*cached_xheader      = xheader;
		*cached_xheader_size = xheader_size;
	}
	else
	{
		memory_free(
		 xheader );
	}
	return( read_count );
}

/* Writes a xheader section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_xheader_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *xheader,
         size_t xheader_size,
         int8_t compression_level,
         liberror_error_t **error )
{
	static char *function       = "libewf_section_xheader_write";
	off64_t section_offset      = 0;
	ssize_t section_write_count = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( libewf_debug_utf8_stream_print(
		     "XHeader",
		     xheader,
		     xheader_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to debug print xheader.",
			 function );

			return( -1 );
		}
	}
#endif
	section_write_count = libewf_section_write_compressed_string(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       (uint8_t *) "xheader",
	                       7,
	                       xheader,
	                       xheader_size,
	                       compression_level,
	                       error );

	if( section_write_count == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write xheader to file.",
		 function );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a xhash section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_xhash_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint8_t **cached_xhash,
         size_t *cached_xhash_size,
         liberror_error_t **error )
{
	uint8_t *xhash        = NULL;
	static char *function = "libewf_section_xhash_read";
	ssize_t read_count    = 0;
	size_t xhash_size     = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( cached_xhash == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid cached xhash.",
		 function );

		return( -1 );
	}
	if( cached_xhash_size == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid cached xhash size.",
		 function );

		return( -1 );
	}
	read_count = libewf_section_compressed_string_read(
	              file_io_pool,
	              segment_file_handle,
	              section_size,
	              &xhash,
	              &xhash_size,
	              error );

	if( read_count != (ssize_t) section_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read xheader.",
		 function );

		return( -1 );
	}
	if( xhash == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: missing xhash.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		 if( libewf_debug_utf8_stream_print(
		      "XHash",
		      xhash,
		      xhash_size,
		      error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to debug print xhash.",
			 function );

			memory_free(
			 xhash );

			return( -1 );
		}
	}
#endif
	if( *cached_xhash == NULL )
	{
		*cached_xhash      = xhash;
		*cached_xhash_size = xhash_size;
	}
	else
	{
		memory_free(
		 xhash );
	}
	return( read_count );
}

/* Writes a xhash section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_xhash_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *xhash,
         size_t xhash_size,
         int8_t compression_level,
         liberror_error_t **error )
{
	static char *function       = "libewf_section_xhash_write";
	off64_t section_offset      = 0;
	ssize_t section_write_count = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( libewf_debug_utf8_stream_print(
		     "XHash",
		     xhash,
		     xhash_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
			 "%s: unable to debug print xhash.",
			 function );

			return( -1 );
		}
	}
#endif
	section_write_count = libewf_section_write_compressed_string(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       (uint8_t *) "xhash",
	                       5,
	                       xhash,
	                       xhash_size,
	                       compression_level,
	                       error );

	if( section_write_count == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write xhash to file.",
		 function );

		return( -1 );
	}
	return( section_write_count );
}

/* Reads a delta chunk section from file
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_delta_chunk_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         libewf_offset_table_t *offset_table,
         liberror_error_t **error )
{
	ewfx_delta_chunk_header_t delta_chunk_header;

	libewf_chunk_value_t *chunk_value = NULL;
	static char *function             = "libewf_section_delta_chunk_read";
	ssize_t read_count                = 0;
	uint32_t calculated_checksum      = 0;
	uint32_t chunk                    = 0;
	uint32_t chunk_size               = 0;
	uint32_t number_of_chunk_values   = 0;
	uint32_t stored_checksum          = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section_size > (size_t) INT32_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              segment_file_handle->file_io_pool_entry,
	              (uint8_t *) &delta_chunk_header,
	              sizeof( ewfx_delta_chunk_header_t ),
	              error );

	if( read_count == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read delta chunk header.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
	 	 "%s: delta chunk header data:\n",
		 function );
		libnotify_print_data(
		 (uint8_t *) &delta_chunk_header,
		 sizeof( ewfx_delta_chunk_header_t ) );
	}
#endif
	byte_stream_copy_to_uint32_little_endian(
	 delta_chunk_header.chunk,
	 chunk );

	byte_stream_copy_to_uint32_little_endian(
	 delta_chunk_header.chunk_size,
	 chunk_size );

	byte_stream_copy_to_uint32_little_endian(
	 delta_chunk_header.checksum,
	 stored_checksum );

#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
	 	 "%s: chunk\t\t\t: %" PRIu32 "\n",
		 function,
		 chunk );

		libnotify_printf(
	 	 "%s: chunk size\t\t: %" PRIu32 "\n",
		 function,
		 chunk_size );

		libnotify_printf(
	 	 "%s: padding:\n",
		 function );
		libnotify_print_data(
		 delta_chunk_header.padding,
		 6 );

		libnotify_printf(
	 	 "%s: checksum\t\t: 0x%08" PRIx32 "\n",
		 function,
		 stored_checksum );

		libnotify_printf(
	 	 "\n" );
	}
#endif
	calculated_checksum = ewf_checksum_calculate(
	                       &delta_chunk_header,
	                       sizeof( ewfx_delta_chunk_header_t ) - sizeof( uint32_t ),
	                       1 );

	if( stored_checksum != calculated_checksum )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_INPUT,
		 LIBERROR_INPUT_ERROR_CHECKSUM_MISMATCH,
		 "%s: checksum does not match (in file: %" PRIu32 " calculated: %" PRIu32 ").",
		 function,
		 stored_checksum,
		 calculated_checksum );

		return( -1 );
	}
	/* The chunk value is stored + 1 count in the file
	 */
	chunk -= 1;

	if( libewf_offset_table_get_number_of_chunk_values(
	     offset_table,
	     &number_of_chunk_values,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve the number of chunk values in the offset table.",
		 function );

		return( -1 );
	}
	if( chunk >= number_of_chunk_values )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid delta chunk: %" PRIu32 " value out of bounds.",
		 function,
		 chunk );

		return( -1 );
	}
	if( chunk_size != ( section_size - sizeof( ewfx_delta_chunk_header_t ) ) )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: chunk size: %" PRIu32 " does not match size of data in section correcting in: %" PRIzd ".\n",
			 function,
			 chunk_size,
			 section_size - sizeof( ewfx_delta_chunk_header_t ) );
		}
#endif
		chunk_size = (uint32_t) ( section_size - sizeof( ewfx_delta_chunk_header_t ) );
	}
	/* Update the chunk data in the offset table
	 */
	if( libewf_offset_table_get_chunk_value(
	     offset_table,
	     chunk,
	     &chunk_value,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve chunk value: %" PRIu32 ".",
		 function,
		 chunk );

		return( -1 );
	}
	if( chunk_value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: missing chunk value: %" PRIu32 ".",
		 function,
		 chunk );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &( chunk_value->file_offset ),
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	chunk_value->segment_file_handle = segment_file_handle;
	chunk_value->size                = chunk_size;
	chunk_value->flags               = LIBEWF_CHUNK_VALUE_FLAG_DELTA_CHUNK;

	/* Skip the chunk data within the section
	 */
	if( libbfio_pool_seek_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     section_size - read_count,
	     SEEK_CUR,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to align with next section.",
		 function );

		return( -1 );
	}
	return( (ssize_t) section_size );
}

/* Writes a delta chunk section to file
 * Returns the number of bytes written or -1 on error
 */
ssize_t libewf_section_delta_chunk_write(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         uint32_t chunk,
         uint8_t *chunk_buffer,
         size_t chunk_size,
         uint8_t *checksum_buffer,
         uint32_t *chunk_checksum,
         uint8_t write_checksum,
         uint8_t no_section_append,
         liberror_error_t **error )
{
	ewfx_delta_chunk_header_t delta_chunk_header;

	uint8_t *section_type        = (uint8_t *) "delta_chunk";
	static char *function        = "libewf_section_delta_chunk_write";
	off64_t section_offset       = 0;
	ssize_t section_write_count  = 0;
	ssize_t write_count          = 0;
	size_t section_type_length   = 11;
	size_t section_size          = 0;
	size_t write_size            = 0;
	uint32_t calculated_checksum = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( chunk_buffer == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid chunk buffer.",
		 function );

		return( -1 );
	}
	if( chunk_size > (size_t) INT32_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_offset(
	     file_io_pool,
	     segment_file_handle->file_io_pool_entry,
	     &section_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve current offset in segment file.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     &delta_chunk_header,
	     0,
	     sizeof( ewfx_delta_chunk_header_t ) ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear delta chunk header.",
		 function );

		return( -1 );
	}
	write_size = chunk_size;

	if( write_checksum != 0 )
	{
		write_size += sizeof( uint32_t );
	}
	/* The chunk value is stored + 1 count in the file
	 */
	byte_stream_copy_from_uint32_little_endian(
	 delta_chunk_header.chunk,
	 ( chunk + 1 ) );

	byte_stream_copy_from_uint32_little_endian(
	 delta_chunk_header.chunk_size,
	 (uint32_t) write_size );

	delta_chunk_header.padding[ 0 ] = (uint8_t) 'D';
	delta_chunk_header.padding[ 1 ] = (uint8_t) 'E';
	delta_chunk_header.padding[ 2 ] = (uint8_t) 'L';
	delta_chunk_header.padding[ 3 ] = (uint8_t) 'T';
	delta_chunk_header.padding[ 4 ] = (uint8_t) 'A';

	calculated_checksum = ewf_checksum_calculate(
	                       &delta_chunk_header,
	                       sizeof( ewfx_delta_chunk_header_t ) - sizeof( uint32_t ),
	                       1 );

	byte_stream_copy_from_uint32_little_endian(
	 delta_chunk_header.checksum,
	 calculated_checksum );

	section_size = sizeof( ewfx_delta_chunk_header_t ) + write_size;

	section_write_count = libewf_section_start_write(
	                       file_io_pool,
	                       segment_file_handle,
	                       section_offset,
	                       section_type,
	                       section_type_length,
	                       (size64_t) section_size,
	                       error );

	if( section_write_count != (ssize_t) sizeof( ewf_section_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write section: %s to file.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               (uint8_t *) &delta_chunk_header,
	               sizeof( ewfx_delta_chunk_header_t ),
	               error );

	if( write_count != (ssize_t) sizeof( ewfx_delta_chunk_header_t ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write chunk value to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	write_size = chunk_size;

	if( write_checksum != 0 )
	{
		if( checksum_buffer == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
			 "%s: invalid checksum buffer.",
			 function );

			return( -1 );
		}
		byte_stream_copy_from_uint32_little_endian(
		 checksum_buffer,
		 *chunk_checksum );

		if( &( chunk_buffer[ chunk_size ] ) == checksum_buffer )
		{
			write_size += sizeof( uint32_t );
		}
	}
	write_count = libbfio_pool_write(
	               file_io_pool,
	               segment_file_handle->file_io_pool_entry,
	               chunk_buffer,
	               write_size,
	               error );

	if( write_count != (ssize_t) write_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to write chunk data to file.",
		 function );

		return( -1 );
	}
	section_write_count += write_count;

	if( ( write_checksum != 0 )
	 && ( &( chunk_buffer[ chunk_size ] ) != checksum_buffer ) )
	{
		write_count = libbfio_pool_write(
		               file_io_pool,
		               segment_file_handle->file_io_pool_entry,
			       checksum_buffer,
			       sizeof( uint32_t ),
		               error );

		if( write_count != (ssize_t) sizeof( uint32_t ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_WRITE_FAILED,
			 "%s: unable to write checksum to file.",
			 function );

			return( -1 );
		}
		section_write_count += write_count;
	}
	if( ( no_section_append == 0 )
	 && ( libewf_section_list_append(
	       segment_file_handle->section_list,
	       section_type,
	       section_type_length,
	       section_offset,
	       section_offset + section_write_count,
	       error ) != 1 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append: %s section to section list.",
		 function,
		 (char *) section_type );

		return( -1 );
	}
	return( section_write_count );
}

#if defined( HAVE_DEBUG_OUTPUT )

/* Reads a section from file for debugging purposes
 * Returns the number of bytes read or -1 on error
 */
ssize_t libewf_section_debug_read(
         libbfio_pool_t *file_io_pool,
         libewf_segment_file_handle_t *segment_file_handle,
         size64_t section_size,
         liberror_error_t **error )
{
	uint8_t *data              = NULL;
	uint8_t *uncompressed_data = NULL;
	static char *function      = "libewf_section_debug_read";
	ssize_t read_count         = 0;
	size_t uncompressed_size   = 0;
	int result                 = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( section_size > (size64_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}
	uncompressed_size = (size_t) ( section_size * 2 );

	if( uncompressed_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: uncompressed size value exceeds maximum.",
		 function );

		return( -1 );
	}
	data = (uint8_t *) memory_allocate(
	                    (size_t) section_size );

	if( data == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create data.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              segment_file_handle->file_io_pool_entry,
	              data,
	              (size_t) section_size,
	              error );

	if( read_count != (ssize_t) section_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read section data.",
		 function );

		memory_free(
		 data );

		return( -1 );
	}
	uncompressed_data = (uint8_t *) memory_allocate(
	                                 uncompressed_size );

	if( uncompressed_data == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create uncompressed data.",
		 function );

		memory_free(
		 data );

		return( -1 );
	}
	result = libewf_decompress(
	          uncompressed_data,
	          &uncompressed_size,
	          data,
	          (size_t) section_size,
	          error );

	if( result == 0 )
	{
		result = libewf_debug_dump_data(
		          "UNCOMPRESSED data",
		          data,
		          (size_t) section_size,
	                  error );
	}
	else if( result == 1 )
	{
		result = libewf_debug_dump_data(
		          "COMPRESSED data",
		          uncompressed_data,
		          uncompressed_size,
	                  error );
	}
	memory_free(
	 data );
	memory_free(
	 uncompressed_data );

	if( result == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_PRINT_FAILED,
		 "%s: unable to debug print data.",
		 function );

		return( -1 );
	}
	return( read_count );
}

#endif

/* Reads and processes a section
 * The section start offset will be updated
 * Returns 1 if successful or -1 on error
 */
int libewf_section_read(
     libbfio_pool_t *file_io_pool,
     libewf_segment_file_handle_t *segment_file_handle,
     libewf_header_sections_t *header_sections,
     libewf_hash_sections_t *hash_sections,
     libewf_media_values_t *media_values,
     libewf_offset_table_t *offset_table,
     libewf_sector_list_t *sessions,
     libewf_sector_list_t *acquiry_errors,
     libewf_single_files_t *single_files,
     int8_t *compression_level,
     uint8_t *format,
     uint8_t *ewf_format,
     ewf_section_t *section,
     off64_t *section_start_offset,
     liberror_error_t **error )
{
	static char *function      = "libewf_section_read";
	off64_t section_end_offset = 0;
	ssize64_t read_count       = 0;
	uint64_t section_size      = 0;
	uint64_t section_next      = 0;
	size_t section_type_length = 0;
	int result                 = 0;

	if( segment_file_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid segment file.",
		 function );

		return( -1 );
	}
	if( header_sections == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid header sections.",
		 function );

		return( -1 );
	}
	if( hash_sections == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid hash sections.",
		 function );

		return( -1 );
	}
	if( media_values == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media values.",
		 function );

		return( -1 );
	}
	if( acquiry_errors == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid acquiry errors.",
		 function );

		return( -1 );
	}
	if( section == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid section start.",
		 function );

		return( -1 );
	}
	if( section_start_offset == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid section start offset.",
		 function );

		return( -1 );
	}
	if( compression_level == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid compression level.",
		 function );

		return( -1 );
	}
	if( format == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid format.",
		 function );

		return( -1 );
	}
	if( ewf_format == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid ewf format.",
		 function );

		return( -1 );
	}
	if( *section_start_offset > (off64_t) INT64_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section start offset value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( libewf_section_start_read(
	     file_io_pool,
	     segment_file_handle,
	     section,
	     &section_size,
	     &section_next,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read section start.",
		 function );

		return( -1 );
	}
	section_end_offset = *section_start_offset + (off64_t) section_size;

	if( section_end_offset > (off64_t) INT64_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section end offset value exceeds maximum.",
		 function );

		return( -1 );
	}
	section_type_length = libcstring_narrow_string_length(
	                       (char *) section->type );

	if( libewf_section_list_append(
	     segment_file_handle->section_list,
	     section->type,
	     section_type_length,
	     *section_start_offset,
	     section_end_offset,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to append value to section list.",
		 function );

		return( -1 );
	}
	*section_start_offset += sizeof( ewf_section_t );

	/* No need to correct empty sections like done and next
	 */
	if( section_size > 0 )
	{
		section_size -= sizeof( ewf_section_t );
	}
	if( section_size > (uint64_t) INT64_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid section size value exceeds maximum.",
		 function );

		return( -1 );
	}

	/* Nothing to do for the next and done section
	 * The \0 byte is included in the compare
	 */
	if( ( memory_compare(
	       section->type,
	       "next",
	       5 ) == 0 )
	 || ( memory_compare(
	       section->type,
	       "done",
	       5 ) == 0 ) )
	{
	}
	/* Read the header2 section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "header2",
	          8 ) == 0 )
	{
		read_count = libewf_section_header2_read(
		              file_io_pool,
		              segment_file_handle,
		              (size_t) section_size,
		              &( header_sections->header2 ),
		              &( header_sections->header2_size ),
		              error );

		header_sections->number_of_header_sections++;
	}
	/* Read the header section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "header",
	          7 ) == 0 )
	{
		read_count = libewf_section_header_read(
		              file_io_pool,
		              segment_file_handle,
		              (size_t) section_size,
		              &( header_sections->header ),
		              &( header_sections->header_size ),
		              error );

		header_sections->number_of_header_sections++;
	}
	/* Read the xheader section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "xheader",
	          8 ) == 0 )
	{
		read_count = libewf_section_xheader_read(
		              file_io_pool,
		              segment_file_handle,
		              (size_t) section_size,
		              &( header_sections->xheader ),
		              &( header_sections->xheader_size ),
		              error );

		header_sections->number_of_header_sections++;
	}
	/* Read the volume or disk section
	 * The \0 byte is included in the compare
	 */
	else if( ( memory_compare(
	            (void *) section->type,
	            (void *) "volume",
	            7 ) == 0 )
	      || ( memory_compare(
	            (void *) section->type,
	            (void *) "disk",
	            5 ) == 0 ) )
	{
		read_count = libewf_section_volume_read(
		              file_io_pool,
		              segment_file_handle,
		              (size_t) section_size,
		              media_values,
		              compression_level,
		              format,
		              ewf_format,
		              error );

		/* Check if the EWF file format is that of EnCase1
		 * this allows the table read function to reduce verbose
		 * output of additional data in table section
		 */
		if( ( *ewf_format == EWF_FORMAT_E01 )
		 && ( header_sections->number_of_header_sections == 1 ) )
		{
			*format = LIBEWF_FORMAT_ENCASE1;
		}
	}
	/* Read the table2 section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "table2",
	          7 ) == 0 )
	{
		read_count = libewf_section_table2_read(
		              file_io_pool,
		              segment_file_handle,
		              (size_t) section_size,
		              media_values->number_of_chunks,
		              offset_table,
		              *format,
		              *ewf_format,
		              error );
	}
	/* Read the table section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "table",
	          6 ) == 0 )
	{
		read_count = libewf_section_table_read(
		              file_io_pool,
		              segment_file_handle,
		              (size_t) section_size,
		              media_values->number_of_chunks,
		              offset_table,
		              *format,
		              *ewf_format,
		              error );
	}
	/* Read the sectors section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "sectors",
	          8 ) == 0 )
	{
		read_count = libewf_section_sectors_read(
		              file_io_pool,
		              segment_file_handle,
 		              (size64_t) section_size,
 		              *ewf_format,
 		              error );
	}
	/* Read the delta_chunk section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "delta_chunk",
	          12 ) == 0 )
	{
		read_count = libewf_section_delta_chunk_read(
 		              file_io_pool,
 		              segment_file_handle,
 		              (size_t) section_size,
 		              offset_table,
 		              error );
	}
	/* Read the ltree section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "ltree",
	          6 ) == 0 )
	{
		read_count = libewf_section_ltree_read(
		              file_io_pool,
		              segment_file_handle,
 		              (size_t) section_size,
		              ewf_format,
		              &( single_files->ltree_data ),
		              &( single_files->ltree_data_size ),
 		              error );
	}
	/* Read the session section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "session",
	          8 ) == 0 )
	{
		read_count = libewf_section_session_read(
		              file_io_pool,
		              segment_file_handle,
		              media_values,
		              sessions,
		              (size_t) section_size,
		              *ewf_format,
 		              error );
	}
	/* Read the data section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "data",
	          5 ) == 0 )
	{
		read_count = libewf_section_data_read(
		              file_io_pool,
		              segment_file_handle,
		              (size_t) section_size,
		              media_values,
		              *ewf_format,
 		              error );
	}
	/* Read the digest section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "digest",
	          7 ) == 0 )
	{
		read_count = libewf_section_digest_read(
		              file_io_pool,
		              segment_file_handle,
		              hash_sections->md5_digest,
		              hash_sections->sha1_digest,
 		              error );

		result = libewf_section_test_zero(
		          hash_sections->md5_digest,
		          16,
		          error );

		if( result == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine if MD5 hash is empty.",
			 function );

			return( -1 );
		}
		else if( result == 0 )
		{
			hash_sections->md5_digest_set = 1;
		}
		else
		{
			hash_sections->md5_digest_set = 0;
		}
		result = libewf_section_test_zero(
		          hash_sections->sha1_digest,
		          20,
		          error );

		if( result == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine if SHA1 hash is empty.",
			 function );

			return( -1 );
		}
		else if( result == 0 )
		{
			hash_sections->sha1_digest_set = 1;
		}
		else
		{
			hash_sections->sha1_digest_set = 0;
		}
	}
	/* Read the hash section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "hash",
	          5 ) == 0 )
	{
		read_count = libewf_section_hash_read(
		              file_io_pool,
		              segment_file_handle,
		              hash_sections->md5_hash,
 		              error );

		hash_sections->md5_hash_set = 1;
	}
	/* Read the xhash section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "xhash",
	          6 ) == 0 )
	{
		read_count = libewf_section_xhash_read(
		              file_io_pool,
		              segment_file_handle,
		              (size_t) section_size,
		              &( hash_sections->xhash ),
		              &( hash_sections->xhash_size ),
		              error );
	}
	/* Read the error2 section
	 * The \0 byte is included in the compare
	 */
	else if( memory_compare(
	          (void *) section->type,
	          (void *) "error2",
	          7 ) == 0 )
	{
		read_count = libewf_section_error2_read(
		              file_io_pool,
		              segment_file_handle,
		              acquiry_errors,
		              (size_t) section_size,
		              *ewf_format,
 		              error );
	}
	else
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		if( libnotify_verbose != 0 )
		{
			libnotify_printf(
			 "%s: unsupported section type: %s.\n",
			 function,
			 (char *) section->type );
		}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
		if( section_size > SSIZE_MAX )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
			 "%s: section size value exceeds maximum.",
			 function );

			return( -1 );
		}
		read_count = libewf_section_debug_read(
		              file_io_pool,
		              segment_file_handle,
		              (size_t) section_size,
		              error );
#else
		/* Skip the data within the section
		 */
		if( libbfio_pool_seek_offset(
		     file_io_pool,
		     segment_file_handle->file_io_pool_entry,
		     section_end_offset,
		     SEEK_SET,
		     error ) == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_SEEK_FAILED,
			 "%s: unable to align with next section.",
			 function );

			return( -1 );
		}
		read_count = (ssize64_t) section_size;
#endif
	}
	if( read_count != (ssize64_t) section_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read section: %s.",
		 function,
		 (char *) section->type );

		return( -1 );
	}
	*section_start_offset += (off64_t) read_count;

	return( 1 );
}

