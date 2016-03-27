/*
 * Chunk table functions
 *
 * Copyright (c) 2006-2012, Joachim Metz <joachim.metz@gmail.com>
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

#include "libewf_chunk_data.h"
#include "libewf_chunk_table.h"
#include "libewf_definitions.h"
#include "libewf_io_handle.h"
#include "libewf_libbfio.h"
#include "libewf_libcerror.h"
#include "libewf_libcnotify.h"
#include "libewf_libmfcache.h"
#include "libewf_libmfdata.h"
#include "libewf_list_type.h"
#include "libewf_section.h"
#include "libewf_unused.h"

#include "ewf_checksum.h"
#include "ewf_definitions.h"
#include "ewf_table.h"

/* Initialize the chunk table
 * Returns 1 if successful or -1 on error
 */
int libewf_chunk_table_initialize(
     libewf_chunk_table_t **chunk_table,
     libewf_io_handle_t *io_handle,
     libcerror_error_t **error )
{
	static char *function = "libewf_chunk_table_initialize";

	if( chunk_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid chunk table.",
		 function );

		return( -1 );
	}
	if( *chunk_table != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid chunk table value already set.",
		 function );

		return( -1 );
	}
	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	*chunk_table = memory_allocate_structure(
	                libewf_chunk_table_t );

	if( *chunk_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create chunk table.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     *chunk_table,
	     0,
	     sizeof( libewf_chunk_table_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear chunk table.",
		 function );

		goto on_error;
	}
	( *chunk_table )->io_handle = io_handle;

	return( 1 );

on_error:
	if( *chunk_table != NULL )
	{
		memory_free(
		 *chunk_table );

		*chunk_table = NULL;
	}
	return( -1 );
}

/* Frees the chunk table including elements
 * Returns 1 if successful or -1 on error
 */
int libewf_chunk_table_free(
     libewf_chunk_table_t **chunk_table,
     libcerror_error_t **error )
{
	static char *function = "libewf_chunk_table_free";

	if( chunk_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid chunk table.",
		 function );

		return( -1 );
	}
	if( *chunk_table != NULL )
	{
		memory_free(
		 *chunk_table );

		*chunk_table = NULL;
	}
	return( 1 );
}

/* Clones the chunk table
 * Returns 1 if successful or -1 on error
 */
int libewf_chunk_table_clone(
     intptr_t **destination_chunk_table,
     intptr_t *source_chunk_table,
     libcerror_error_t **error )
{
	static char *function = "libewf_chunk_table_clone";

	if( destination_chunk_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid destination chunk table.",
		 function );

		return( -1 );
	}
	if( *destination_chunk_table != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid destination chunk table value already set.",
		 function );

		return( -1 );
	}
	if( source_chunk_table == NULL )
	{
		*destination_chunk_table = NULL;

		return( 1 );
	}
	*destination_chunk_table = memory_allocate_structure_as_value(
	                            libewf_chunk_table_t );

	if( *destination_chunk_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create destination chunk table.",
		 function );

		goto on_error;
	}
	if( memory_copy(
	     *destination_chunk_table,
	     source_chunk_table,
	     sizeof( libewf_chunk_table_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy source to destination chunk table.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( *destination_chunk_table != NULL )
	{
		memory_free(
		 *destination_chunk_table );

		*destination_chunk_table = NULL;
	}
	return( -1 );
}

/* Reads a chunk
 * Callback function for the chunk table list
 * Returns 1 if successful or -1 on error
 */
int libewf_chunk_table_read_chunk(
     intptr_t *io_handle,
     libbfio_pool_t *file_io_pool,
     libmfdata_list_element_t *list_element,
     libmfcache_cache_t *cache,
     int file_io_pool_entry,
     off64_t element_data_offset,
     size64_t element_data_size,
     uint32_t element_data_flags,
     uint8_t read_flags LIBEWF_ATTRIBUTE_UNUSED,
     libcerror_error_t **error )
{
	libewf_chunk_data_t *chunk_data = NULL;
	static char *function           = "libewf_chunk_table_read_chunk";
	ssize_t read_count              = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	int element_index               = 0;
#endif

	LIBEWF_UNREFERENCED_PARAMETER( read_flags )

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( element_data_size > (size64_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid element data size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( ( element_data_flags & LIBMFDATA_RANGE_FLAG_IS_SPARSE ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported element data flags.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		if( libmfdata_list_element_get_element_index(
		     list_element,
		     &element_index,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve element index from list element.",
			 function );

			goto on_error;
		}
		if( ( element_data_flags & LIBMFDATA_RANGE_FLAG_IS_COMPRESSED ) != 0 )
		{
			libcnotify_printf(
			 "%s: reading compressed chunk: %d from file IO pool entry: %d at offset: %" PRIi64 " (0x%08" PRIx64 ") of size: %" PRIu64 "\n",
			 function,
			 element_index,
			 file_io_pool_entry,
			 element_data_offset,
			 element_data_offset,
			 element_data_size );
		}
		else
		{
			libcnotify_printf(
			 "%s: reading uncompressed chunk: %d from file IO pool entry: %d at offset: %" PRIi64 " (0x%08" PRIx64 ") of size: %" PRIu64 "\n",
			 function,
			 element_index,
			 file_io_pool_entry,
			 element_data_offset,
			 element_data_offset,
			 element_data_size );
		}
	}
#endif
	if( libbfio_pool_seek_offset(
	     file_io_pool,
	     file_io_pool_entry,
	     element_data_offset,
	     SEEK_SET,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek chunk offset: %" PRIi64 " in file IO pool entry: %d.",
		 function,
		 element_data_offset,
		 file_io_pool_entry );

		goto on_error;
	}
	if( libewf_chunk_data_initialize(
	     &chunk_data,
	     (size_t) element_data_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create chunk data.",
		 function );

		goto on_error;
	}
	if( chunk_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: missing chunk data.",
		 function );

		goto on_error;
	}
	read_count = libbfio_pool_read_buffer(
		      file_io_pool,
		      file_io_pool_entry,
		      chunk_data->data,
		      (size_t) element_data_size,
		      error );

	if( read_count != (ssize_t) element_data_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read chunk data.",
		 function );

		goto on_error;
	}
	chunk_data->data_size = (size_t) read_count;

	if( ( element_data_flags & LIBMFDATA_RANGE_FLAG_IS_COMPRESSED ) != 0 )
	{
		chunk_data->has_checksum  = 1;
		chunk_data->is_compressed = 1;
	}
	else if( ( element_data_flags & LIBEWF_RANGE_FLAG_HAS_CHECKSUM ) != 0 )
	{
		chunk_data->has_checksum = 1;
	}
	chunk_data->is_packed = 1;

	if( libmfdata_list_element_set_element_value(
	     list_element,
	     cache,
	     (intptr_t *) chunk_data,
	     (int (*)(intptr_t **, libcerror_error_t **)) &libewf_chunk_data_free,
	     LIBMFDATA_LIST_ELEMENT_VALUE_FLAG_MANAGED,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set chunk data as element value.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( chunk_data != NULL )
	{
		libewf_chunk_data_free(
		 &chunk_data,
		 NULL );
	}
	return( -1 );
}

/* Reads a table section for the chunk offsets
 * Callback function for the chunk table list
 * Returns 1 if successful, 0 if not or -1 on error
 */
int libewf_chunk_table_read_offsets(
     intptr_t *io_handle,
     libbfio_pool_t *file_io_pool,
     libmfdata_list_t *chunk_table_list,
     int element_index,
     int number_of_elements LIBEWF_ATTRIBUTE_UNUSED,
     libmfcache_cache_t *cache LIBEWF_ATTRIBUTE_UNUSED,
     int file_io_pool_entry,
     off64_t element_group_offset,
     size64_t element_group_size,
     uint8_t read_flags,
     libcerror_error_t **error )
{
	uint8_t table_footer_data[ 16 ];

	libewf_chunk_table_t *chunk_table = NULL;
	libewf_section_t *section         = NULL;
	uint8_t *table_entries_data       = NULL;
	static char *function             = "libewf_chunk_table_read_offsets";
	size_t table_entries_data_size    = 0;
	size_t table_footer_data_size     = 0;
	ssize_t read_count                = 0;
	uint64_t base_offset              = 0;
	uint32_t calculated_checksum      = 0;
	uint32_t number_of_entries        = 0;
	uint32_t stored_checksum          = 0;
	uint8_t table_entries_corrupted   = 0;
	int result                        = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	uint8_t *trailing_data            = NULL;
#endif

	LIBEWF_UNREFERENCED_PARAMETER( number_of_elements )
	LIBEWF_UNREFERENCED_PARAMETER( cache )

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	chunk_table = (libewf_chunk_table_t *) io_handle;

	if( chunk_table->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid chunk table - missing IO handle.",
		 function );

		return( -1 );
	}
	if( element_group_size > (size64_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid element group size value exceeds maximum.",
		 function );

		goto on_error;
	}
	if( libewf_section_initialize(
	     &section,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create section.",
		 function );

		goto on_error;
	}
	if( chunk_table->io_handle->major_version == 1 )
	{
		read_count = libewf_section_descriptor_read(
			      section,
			      file_io_pool,
			      file_io_pool_entry,
			      element_group_offset,
			      chunk_table->io_handle->major_version,
			      error );

		if( read_count == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read section descriptor.",
			 function );

			goto on_error;
		}
		if( element_group_size != section->size )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: invalid element group size value out of bounds.",
			 function );

			goto on_error;
		}
		element_group_size -= read_count;
	}
	else if( chunk_table->io_handle->major_version == 2 )
	{
		if( libbfio_pool_seek_offset(
		     file_io_pool,
		     file_io_pool_entry,
		     element_group_offset,
		     SEEK_SET,
		     error ) == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_SEEK_FAILED,
			 "%s: unable to seek chunk table offset: %" PRIi64 " in file IO pool entry: %d.",
			 function,
			 element_group_offset,
			 file_io_pool_entry );

			goto on_error;
		}
/* TODO check bounds */
		section->start_offset = element_group_offset;
		section->data_size    = (uint32_t) element_group_size;
	}
	read_count = libewf_section_table_header_read(
		      section,
		      file_io_pool,
		      file_io_pool_entry,
		      chunk_table->io_handle->major_version,
		      chunk_table->io_handle->format,
		      &number_of_entries,
		      &base_offset,
		      error );
	
	if( read_count < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read table section header.",
		 function );

		goto on_error;
	}
	element_group_size -= read_count;

	if( number_of_entries == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_INPUT,
		 LIBCERROR_INPUT_ERROR_INVALID_DATA,
		 "%s: invalid number of entries.",
		 function );

		goto on_error;
	}
	table_entries_data_size = number_of_entries;

	if( chunk_table->io_handle->major_version == 1 )
	{
		table_entries_data_size *= sizeof( ewf_table_entry_v1_t );
	}
	else if( chunk_table->io_handle->major_version == 2 )
	{
		table_entries_data_size *= sizeof( ewf_table_entry_v2_t );
	}
	if( table_entries_data_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid table entries data size value exceeds maximum.",
		 function );

		goto on_error;
	}
	table_entries_data = (uint8_t *) memory_allocate(
	                                  sizeof( uint8_t ) * table_entries_data_size );

	if( table_entries_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create table entries data.",
		 function );

		goto on_error;
	}
	if( element_group_size < (size64_t) table_entries_data_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: invalid element group size value too small.",
		 function );

		return( -1 );
	}
	read_count = libbfio_pool_read_buffer(
		      file_io_pool,
		      file_io_pool_entry,
		      table_entries_data,
		      table_entries_data_size,
		      error );

	if( read_count != (ssize_t) table_entries_data_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read table offsets data.",
		 function );

		goto on_error;
	}
	element_group_size -= read_count;

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
	 	 "%s: table entries data:\n",
		 function );
		libcnotify_print_data(
		 table_entries_data,
		 table_entries_data_size,
		 0 );
	}
#endif
	/* The original EWF and SMART (EWF-S01) formats do not contain a checksum after the table offsets
	 */
	if( chunk_table->io_handle->ewf_format != EWF_FORMAT_S01 )
	{
		if( chunk_table->io_handle->major_version == 1 )
		{
			table_footer_data_size = 4;
		}
		else if( chunk_table->io_handle->major_version == 2 )
		{
			table_footer_data_size = 16;
		}
		if( element_group_size < (size64_t) table_footer_data_size )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
			 "%s: invalid element group size value too small.",
			 function );

			return( -1 );
		}
		read_count = libbfio_pool_read_buffer(
			      file_io_pool,
			      file_io_pool_entry,
			      table_footer_data,
			      table_footer_data_size,
			      error );

		if( read_count != (ssize_t) table_footer_data_size )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read table footer.",
			 function );

			goto on_error;
		}
		element_group_size -= read_count;

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: table footer data:\n",
			 function );
			libcnotify_print_data(
			 table_footer_data,
			 table_footer_data_size,
			 0 );
		}
#endif
		byte_stream_copy_to_uint32_little_endian(
		 table_footer_data,
		 stored_checksum );

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
	 		 "%s: table entries checksum\t\t\t: 0x%" PRIx32 "\n",
			 function,
			 stored_checksum );

			if( chunk_table->io_handle->major_version == 1 )
			{
				libcnotify_printf(
				 "\n" );
			}
			else if( chunk_table->io_handle->major_version == 2 )
			{
				libcnotify_printf(
				 "%s: padding:\n",
				 function );
				libcnotify_print_data(
				 &( table_footer_data[ 4 ] ),
				 12,
				 0 );
			}
		}
#endif
		calculated_checksum = ewf_checksum_calculate(
				       table_entries_data,
				       table_entries_data_size,
				       1 );

		if( stored_checksum != calculated_checksum )
		{
#if defined( HAVE_VERBOSE_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: checksum does not match (stored: 0x%08" PRIx32 " calculated: 0x%08" PRIx32 ").\n",
				 function,
				 stored_checksum,
				 calculated_checksum );
			}
#endif
			/* The table offsets cannot be fully trusted therefore mark them as corrupted
			 */
			table_entries_corrupted = 1;
		}
	}
	if( ( read_flags & LIBMFDATA_READ_FLAG_IS_BACKUP_RANGE ) == 0 )
	{
		if( chunk_table->io_handle->major_version == 1 )
		{
			result = libewf_chunk_table_fill_v1(
			          chunk_table,
			          chunk_table_list,
			          element_index,
			          file_io_pool_entry,
			          section,
			          (off64_t) base_offset,
			          number_of_entries,
			          table_entries_data,
			          table_entries_data_size,
			          table_entries_corrupted,
			          error );
		}
		else if( chunk_table->io_handle->major_version == 2 )
		{
/* TODO pass table_entries_corrupted */
			result = libewf_chunk_table_fill_v2(
			          chunk_table,
			          chunk_table_list,
			          element_index,
			          file_io_pool_entry,
			          table_entries_data,
			          table_entries_data_size,
			          table_entries_corrupted,
			          error );
		}
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to fill chunk table.",
			 function );

			goto on_error;
		}
	}
	else
	{
		if( chunk_table->io_handle->major_version == 1 )
		{
			result = libewf_chunk_table_correct_v1(
				  chunk_table,
				  chunk_table_list,
				  element_index,
				  file_io_pool_entry,
				  section,
				  (off64_t) base_offset,
				  number_of_entries,
				  table_entries_data,
				  table_entries_data_size,
				  table_entries_corrupted,
				  error );
		}
		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to correct chunk table.",
			 function );

			goto on_error;
		}
	}
	memory_free(
	 table_entries_data );

	table_entries_data = NULL;

	if( libewf_section_free(
	     &section,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free section.",
		 function );

		section = NULL;

		goto on_error;
	}
#if defined( HAVE_VERBOSE_OUTPUT ) || defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		if( ( element_group_size > (size64_t) 0 )
		 && ( chunk_table->io_handle->ewf_format != EWF_FORMAT_S01 )
		 && ( chunk_table->io_handle->format != LIBEWF_FORMAT_ENCASE1 ) )
		{
#if defined( HAVE_DEBUG_OUTPUT )
			if( element_group_size > (size64_t) SSIZE_MAX )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
				 "%s: invalid element group size value exceeds maximum.",
				 function );

				goto on_error;
			}
			libcnotify_printf(
		 	 "%s: trailing data:\n",
			 function );

			trailing_data = (uint8_t *) memory_allocate(
			                             sizeof( uint8_t ) * (size_t) element_group_size );

			if( trailing_data == NULL )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_MEMORY,
				 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
				 "%s: unable to create trailing data.",
				 function );

				goto on_error;
			}
			read_count = libbfio_pool_read_buffer(
				      file_io_pool,
				      file_io_pool_entry,
				      trailing_data,
				      (size_t) element_group_size,
				      error );

			if( read_count != (ssize_t) element_group_size )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read trailing data.",
				 function );

				goto on_error;
			}
			libcnotify_printf(
		 	 "%s: trailing data:\n",
			 function );
			libcnotify_print_data(
			 trailing_data,
			 (size_t) element_group_size,
			 0 );

			memory_free(
			 trailing_data );

			trailing_data = NULL;

#elif defined( HAVE_VERBOSE_OUTPUT )
			libcnotify_printf(
		 	 "%s: trailing data after table section offsets.\n",
			 function );
#endif
		}
	}
#endif
	if( table_entries_corrupted != 0 )
	{
		return( 0 );
	}
	return( 1 );

on_error:
#if defined( HAVE_DEBUG_OUTPUT )
	if( trailing_data != NULL )
	{
		memory_free(
		 trailing_data );
	}
#endif
	if( table_entries_data != NULL )
	{
		memory_free(
		 table_entries_data );
	}
	if( section != NULL )
	{
		libewf_section_free(
		 &section,
		 NULL );
	}
	return( -1 );
}

/* Fills the chunk table from the EWF version 1 sector table entries
 * Returns 1 if successful or -1 on error
 */
int libewf_chunk_table_fill_v1(
     libewf_chunk_table_t *chunk_table,
     libmfdata_list_t *chunk_table_list,
     int chunk_index,
     int file_io_pool_entry,
     libewf_section_t *table_section,
     off64_t base_offset,
     uint32_t number_of_offsets,
     const uint8_t *table_entries_data,
     size_t table_entries_data_size,
     uint8_t tainted,
     libcerror_error_t **error )
{
	static char *function           = "libewf_chunk_table_fill_v1";
	off64_t last_chunk_offset       = 0;
	off64_t last_chunk_size         = 0;
	off64_t previous_chunk_offset   = 0;
	size64_t previous_chunk_size    = 0;
	uint32_t chunk_size             = 0;
	uint32_t current_offset         = 0;
	uint32_t next_offset            = 0;
	uint32_t previous_range_flags   = 0;
	uint32_t range_flags            = 0;
	uint32_t stored_offset          = 0;
	uint32_t table_entry_index      = 0;
	uint8_t corrupted               = 0;
	uint8_t is_compressed           = 0;
	uint8_t overflow                = 0;
	int previous_file_io_pool_entry = 0;
	int result                      = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	char *chunk_type                = NULL;
	char *remarks                   = NULL;
#endif

	if( chunk_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid chunk table.",
		 function );

		return( -1 );
	}
	if( table_section == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table section.",
		 function );

		return( -1 );
	}
	if( base_offset < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid base offset.",
		 function );

		return( -1 );
	}
	if( table_entries_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table entries data.",
		 function );

		return( -1 );
	}
	if( table_entries_data_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid table entries data size value exceeds maximum.",
		 function );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 ( ( (ewf_table_entry_v1_t *) table_entries_data )[ table_entry_index ] ).chunk_data_offset,
	 stored_offset );

	while( table_entry_index < ( number_of_offsets - 1 ) )
	{
		if( overflow == 0 )
		{
			is_compressed  = (uint8_t) ( stored_offset >> 31 );
			current_offset = stored_offset & 0x7fffffffUL;
		}
		else
		{
			current_offset = stored_offset;
		}
		byte_stream_copy_to_uint32_little_endian(
		 ( ( (ewf_table_entry_v1_t *) table_entries_data )[ table_entry_index + 1 ] ).chunk_data_offset,
		 stored_offset );

		if( overflow == 0 )
		{
			next_offset = stored_offset & 0x7fffffffUL;
		}
		else
		{
			next_offset = stored_offset;
		}
		corrupted = 0;

		/* This is to compensate for the crappy > 2 GiB segment file solution in EnCase 6.7
		 */
		if( next_offset < current_offset )
		{
			if( stored_offset < current_offset )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: chunk offset: %" PRIu32 " larger than stored chunk offset: %" PRIu32 ".\n",
					 function,
					 current_offset,
					 stored_offset );
				}
#endif
				corrupted = 1;
			}
#if defined( HAVE_DEBUG_OUTPUT )
			else if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: chunk offset: %" PRIu32 " larger than next chunk offset: %" PRIu32 ".\n",
				 function,
				 current_offset,
				 next_offset );
			}
#endif
			chunk_size = stored_offset - current_offset;
		}
		else
		{
			chunk_size = next_offset - current_offset;
		}
		if( chunk_size == 0 )
		{
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: invalid chunk size value is zero.\n",
				 function );
			}
			corrupted = 1;
		}
		if( chunk_size > (uint32_t) INT32_MAX )
		{
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: invalid chunk size value exceeds maximum.\n",
				 function );
			}
			corrupted = 1;
		}
		range_flags = LIBEWF_RANGE_FLAG_HAS_CHECKSUM;

		if( is_compressed != 0 )
		{
			range_flags |= LIBMFDATA_RANGE_FLAG_IS_COMPRESSED;
		}
		if( corrupted != 0 )
		{
			range_flags |= LIBMFDATA_RANGE_FLAG_IS_CORRUPTED;
		}
		if( tainted != 0 )
		{
			range_flags |= LIBMFDATA_RANGE_FLAG_IS_TAINTED;
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( is_compressed == 0 )
		{
			chunk_type = "uncompressed";
		}
		else
		{
			chunk_type = "compressed";
		}
		if( corrupted != 0 )
		{
			remarks = " corrupted";
		}
		else if( tainted != 0 )
		{
			remarks = " tainted";
		}
		else
		{
			remarks = "";
		}
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: %s chunk %d read with: base %" PRIi64 ", offset %" PRIu32 " and size %" PRIu32 "%s.\n",
			 function,
			 chunk_type,
			 chunk_index,
			 base_offset,
			 current_offset,
			 chunk_size,
			 remarks );
		}
#endif
		result = libmfdata_list_is_group(
		          chunk_table_list,
		          chunk_index,
		          error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine if chunk: %d is a group.",
			 function,
			 chunk_index );

			return( -1 );
		}
		else if( result != 0 )
		{
			if( libmfdata_list_set_element_by_index(
			     chunk_table_list,
			     chunk_index,
			     file_io_pool_entry,
			     base_offset + current_offset,
			     (size64_t) chunk_size,
			     range_flags,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to set chunk: %d.",
				 function,
				 chunk_index );

				return( -1 );
			}
		}
		else
		{
			if( libmfdata_list_get_data_range_by_index(
			     chunk_table_list,
			     chunk_index,
			     &previous_file_io_pool_entry,
			     &previous_chunk_offset,
			     &previous_chunk_size,
			     &previous_range_flags,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve data range of chunk: %d.",
				 function,
				 chunk_index );

				return( -1 );
			}
			if( ( previous_range_flags & LIBEWF_RANGE_FLAG_IS_DELTA ) == 0 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
				 "%s: unable to set data range of chunk: %d value already set.",
				 function,
				 chunk_index );

				return( -1 );
			}
			/* No need to overwrite the data range of a delta chunk */
		}
		/* This is to compensate for the crappy > 2 GiB segment file solution in EnCase 6.7
		 */
		if( ( overflow == 0 )
		 && ( ( current_offset + chunk_size ) > (uint32_t) INT32_MAX ) )
		{
#if defined( HAVE_DEBUG_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: chunk offset overflow at: %" PRIu32 ".\n",
				 function,
				 current_offset );
			}
#endif
			overflow      = 1;
			is_compressed = 0;
		}
		chunk_index++;

		table_entry_index++;
	}
	byte_stream_copy_to_uint32_little_endian(
	 ( ( (ewf_table_entry_v1_t *) table_entries_data )[ table_entry_index ] ).chunk_data_offset,
	 stored_offset );

	if( overflow == 0 )
	{
		is_compressed  = (uint8_t) ( stored_offset >> 31 );
		current_offset = stored_offset & 0x7fffffffUL;
	}
	else
	{
		current_offset = stored_offset;
	}
	corrupted = 0;

	/* There is no indication how large the last chunk is.
	 * The only thing known is where it starts.
	 * However it can be determined using the offset of the next section.
	 * The size of the last chunk is determined by subtracting the last offset from the offset of the next section.
	 *
	 * The offset of the next section is either table_section->end_offset for original EWF and EWF-S01
	 * or table_section->start_offset for other types of EWF
	 */
	last_chunk_offset = (off64_t) base_offset + current_offset;

	if( last_chunk_offset > (off64_t) INT64_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid last chunk offset value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( last_chunk_offset < table_section->start_offset )
	{
		last_chunk_size = table_section->start_offset - last_chunk_offset;
	}
	else if( last_chunk_offset < table_section->end_offset )
	{
		last_chunk_size = table_section->end_offset - last_chunk_offset;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	else if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: invalid last chunk offset value exceeds table section end offset.\n",
		 function );
	}
#endif
	if( last_chunk_size <= 0 )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: invalid last chunk size value is zero or less.\n",
			 function );
		}
#endif
		corrupted = 1;
	}
	if( last_chunk_size > (off64_t) INT32_MAX )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: invalid last chunk size value exceeds maximum.\n",
			 function );
		}
#endif
		corrupted = 1;
	}
	/* Used to indicate the chunk has a checksum
	 */
	range_flags = LIBEWF_RANGE_FLAG_HAS_CHECKSUM;

	if( is_compressed != 0 )
	{
		range_flags |= LIBMFDATA_RANGE_FLAG_IS_COMPRESSED;
	}
	if( corrupted != 0 )
	{
		range_flags |= LIBMFDATA_RANGE_FLAG_IS_CORRUPTED;
	}
	if( tainted != 0 )
	{
		range_flags |= LIBMFDATA_RANGE_FLAG_IS_TAINTED;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( is_compressed == 0 )
	{
		chunk_type = "uncompressed";
	}
	else
	{
		chunk_type = "compressed";
	}
	if( corrupted != 0 )
	{
		remarks = " corrupted";
	}
	else if( tainted != 0 )
	{
		remarks = " tainted";
	}
	else
	{
		remarks = "";
	}
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: %s last chunk %d read with: base %" PRIi64 ", offset %" PRIu32 " and calculated size %" PRIi64 "%s.\n",
		 function,
		 chunk_type,
		 chunk_index,
		 base_offset,
		 current_offset,
		 last_chunk_size,
		 remarks );
	}
#endif
	result = libmfdata_list_is_group(
		  chunk_table_list,
		  chunk_index,
		  error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine if chunk: %d is a group.",
		 function,
		 chunk_index );

		return( -1 );
	}
	else if( result != 0 )
	{
		if( libmfdata_list_set_element_by_index(
		     chunk_table_list,
		     chunk_index,
		     file_io_pool_entry,
		     last_chunk_offset,
		     (size64_t) last_chunk_size,
		     range_flags,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set chunk: %d.",
			 function,
			 chunk_index );

			return( -1 );
		}
	}
	else
	{
		if( libmfdata_list_get_data_range_by_index(
		     chunk_table_list,
		     chunk_index,
		     &previous_file_io_pool_entry,
		     &previous_chunk_offset,
		     &previous_chunk_size,
		     &previous_range_flags,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve data range of chunk: %d.",
			 function,
			 chunk_index );

			return( -1 );
		}
		if( ( previous_range_flags & LIBEWF_RANGE_FLAG_IS_DELTA ) == 0 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
			 "%s: unable to set data range of chunk: %d value already set.",
			 function,
			 chunk_index );

			return( -1 );
		}
		/* No need to overwrite the data range of a delta chunk */
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "\n" );
	}
#endif
	return( 1 );
}

/* Fills the chunk table from the EWF version 2 sector table entries
 * Returns 1 if successful or -1 on error
 */
int libewf_chunk_table_fill_v2(
     libewf_chunk_table_t *chunk_table,
     libmfdata_list_t *chunk_table_list,
     int chunk_index,
     int file_io_pool_entry,
     const uint8_t *table_entries_data,
     size_t table_entries_data_size,
     uint8_t tainted,
     libcerror_error_t **error )
{
	static char *function           = "libewf_chunk_table_fill_v2";
	off64_t previous_chunk_offset   = 0;
	size64_t previous_chunk_size    = 0;
	size_t table_entry_index        = 0;
	uint64_t chunk_offset           = 0;
	uint32_t chunk_data_flags       = 0;
	uint32_t chunk_size             = 0;
	uint32_t previous_range_flags   = 0;
	uint32_t range_flags            = 0;
	int previous_file_io_pool_entry = 0;
	int result                      = 0;

	if( chunk_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid chunk table.",
		 function );

		return( -1 );
	}
	if( table_entries_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table entries data.",
		 function );

		return( -1 );
	}
	if( table_entries_data_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid table entries data size value exceeds maximum.",
		 function );

		return( -1 );
	}
	while( table_entries_data_size >= sizeof( ewf_table_entry_v2_t ) )
	{
		byte_stream_copy_to_uint64_little_endian(
		 ( (ewf_table_entry_v2_t *) table_entries_data )->chunk_data_offset,
		 chunk_offset );

		byte_stream_copy_to_uint32_little_endian(
		 ( (ewf_table_entry_v2_t *) table_entries_data )->chunk_data_size,
		 chunk_size );

		byte_stream_copy_to_uint32_little_endian(
		 ( (ewf_table_entry_v2_t *) table_entries_data )->chunk_data_flags,
		 chunk_data_flags );

#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: table entry: %03" PRIzd " chunk data offset:\t\t0x%08" PRIx64 "\n",
			 function,
			 table_entry_index,
			 chunk_offset );

			libcnotify_printf(
			 "%s: table entry: %03" PRIzd " chunk data size:\t\t%" PRIu32 "\n",
			 function,
			 table_entry_index,
			 chunk_size );

			libcnotify_printf(
			 "%s: table entry: %03" PRIzd " chunk data flags:\t\t0x%08" PRIx32 "\n",
			 function,
			 table_entry_index,
			 chunk_data_flags );
		}
#endif
		table_entries_data      += sizeof( ewf_table_entry_v2_t );
		table_entries_data_size -= sizeof( ewf_table_entry_v2_t );

		table_entry_index++;

		if( ( chunk_data_flags & ~( 0x00000007UL ) ) != 0 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
			 "%s: unsupported chunk data flags.",
			 function );

			return( -1 );
		}
		range_flags = 0;

		if( ( chunk_data_flags & LIBEWF_CHUNK_DATA_FLAG_IS_COMPRESSED ) != 0 )
		{
			range_flags |= LIBMFDATA_RANGE_FLAG_IS_COMPRESSED;
		}
		if( ( chunk_data_flags & LIBEWF_CHUNK_DATA_FLAG_HAS_CHECKSUM ) != 0 )
		{
			range_flags |= LIBEWF_RANGE_FLAG_HAS_CHECKSUM;
		}
		if( ( chunk_data_flags & LIBEWF_CHUNK_DATA_FLAG_USES_PATTERN_FILL ) != 0 )
		{
			range_flags |= LIBEWF_RANGE_FLAG_USES_PATTERN_FILL;
		}
		if( tainted != 0 )
		{
			range_flags |= LIBMFDATA_RANGE_FLAG_IS_TAINTED;
		}
		result = libmfdata_list_is_group(
		          chunk_table_list,
		          chunk_index,
		          error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine if chunk: %d is a group.",
			 function,
			 chunk_index );

			return( -1 );
		}
		else if( result != 0 )
		{
			if( libmfdata_list_set_element_by_index(
			     chunk_table_list,
			     chunk_index,
			     file_io_pool_entry,
			     chunk_offset,
			     (size64_t) chunk_size,
			     range_flags,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to set chunk: %d.",
				 function,
				 chunk_index );

				return( -1 );
			}
		}
		else
		{
			if( libmfdata_list_get_data_range_by_index(
			     chunk_table_list,
			     chunk_index,
			     &previous_file_io_pool_entry,
			     &previous_chunk_offset,
			     &previous_chunk_size,
			     &previous_range_flags,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve data range of chunk: %d.",
				 function,
				 chunk_index );

				return( -1 );
			}
			if( ( previous_range_flags & LIBEWF_RANGE_FLAG_IS_DELTA ) == 0 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
				 "%s: unable to set data range of chunk: %d value already set.",
				 function,
				 chunk_index );

				return( -1 );
			}
			/* No need to overwrite the data range of a delta chunk */
		}
		chunk_index++;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "\n" );
	}
#endif
	return( 1 );
}

/* Corrects the chunk table from the offsets
 * Returns 1 if successful or -1 on error
 */
int libewf_chunk_table_correct_v1(
     libewf_chunk_table_t *chunk_table,
     libmfdata_list_t *chunk_table_list,
     int chunk_index,
     int file_io_pool_entry,
     libewf_section_t *table_section,
     off64_t base_offset,
     uint32_t number_of_offsets,
     const uint8_t *table_entries_data,
     size_t table_entries_data_size,
     uint8_t tainted,
     libcerror_error_t **error )
{
	static char *function           = "libewf_chunk_table_correct_v1";
	off64_t last_chunk_offset       = 0;
	off64_t last_chunk_size         = 0;
	off64_t previous_chunk_offset   = 0;
	size64_t previous_chunk_size    = 0;
	uint32_t chunk_size             = 0;
	uint32_t current_offset         = 0;
	uint32_t next_offset            = 0;
	uint32_t previous_range_flags   = 0;
	uint32_t range_flags            = 0;
	uint32_t stored_offset          = 0;
	uint32_t table_entry_index      = 0;
	uint8_t corrupted               = 0;
	uint8_t is_compressed           = 0;
	uint8_t mismatch                = 0;
	uint8_t overflow                = 0;
	uint8_t update_data_range       = 0;
	int previous_file_io_pool_entry = 0;
	int result                      = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	char *chunk_type                = NULL;
	char *remarks                   = NULL;
#endif

	if( chunk_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid chunk table.",
		 function );

		return( -1 );
	}
	if( table_section == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table section.",
		 function );

		return( -1 );
	}
	if( base_offset < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid base offset.",
		 function );

		return( -1 );
	}
	if( table_entries_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table entries data.",
		 function );

		return( -1 );
	}
	if( table_entries_data_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid table entries data size value exceeds maximum.",
		 function );

		return( -1 );
	}
	byte_stream_copy_to_uint32_little_endian(
	 ( ( (ewf_table_entry_v1_t *) table_entries_data )[ table_entry_index ] ).chunk_data_offset,
	 stored_offset );

	while( table_entry_index < ( number_of_offsets - 1 ) )
	{
		if( overflow == 0 )
		{
			is_compressed  = (uint8_t) ( stored_offset >> 31 );
			current_offset = stored_offset & 0x7fffffffUL;
		}
		else
		{
			current_offset = stored_offset;
		}
		byte_stream_copy_to_uint32_little_endian(
		 ( ( (ewf_table_entry_v1_t *) table_entries_data )[ table_entry_index + 1 ] ).chunk_data_offset,
		 stored_offset );

		if( overflow == 0 )
		{
			next_offset = stored_offset & 0x7fffffffUL;
		}
		else
		{
			next_offset = stored_offset;
		}
		corrupted = 0;

		/* This is to compensate for the crappy > 2 GiB segment file solution in EnCase 6.7
		 */
		if( next_offset < current_offset )
		{
			if( stored_offset < current_offset )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: chunk offset: %" PRIu32 " larger than stored chunk offset: %" PRIu32 ".\n",
					 function,
					 current_offset,
					 stored_offset );
				}
#endif
				corrupted = 1;
			}
#if defined( HAVE_DEBUG_OUTPUT )
			else if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: chunk offset: %" PRIu32 " larger than next chunk offset: %" PRIu32 ".\n",
				 function,
				 current_offset,
				 next_offset );
			}
#endif
			chunk_size = stored_offset - current_offset;
		}
		else
		{
			chunk_size = next_offset - current_offset;
		}
		if( chunk_size == 0 )
		{
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: invalid chunk size value is zero.\n",
				 function );
			}
			corrupted = 1;
		}
		if( chunk_size > (uint32_t) INT32_MAX )
		{
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: invalid chunk size value exceeds maximum.\n",
				 function );
			}
			corrupted = 1;
		}
		range_flags = LIBEWF_RANGE_FLAG_HAS_CHECKSUM;

		if( is_compressed != 0 )
		{
			range_flags |= LIBMFDATA_RANGE_FLAG_IS_COMPRESSED;
		}
		if( corrupted != 0 )
		{
			range_flags |= LIBMFDATA_RANGE_FLAG_IS_CORRUPTED;
		}
		if( tainted != 0 )
		{
			range_flags |= LIBMFDATA_RANGE_FLAG_IS_TAINTED;
		}
#if defined( HAVE_DEBUG_OUTPUT )
		if( is_compressed == 0 )
		{
			chunk_type = "uncompressed";
		}
		else
		{
			chunk_type = "compressed";
		}
		if( corrupted != 0 )
		{
			remarks = " corrupted";
		}
		else if( tainted != 0 )
		{
			remarks = " tainted";
		}
		else
		{
			remarks = "";
		}
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: %s chunk %d read with: base %" PRIi64 ", offset %" PRIu32 " and size %" PRIu32 "%s.\n",
			 function,
			 chunk_type,
			 chunk_index,
			 base_offset,
			 current_offset,
			 chunk_size,
			 remarks );
		}
#endif
		result = libmfdata_list_is_group(
		          chunk_table_list,
		          chunk_index,
		          error );

		if( result == -1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine if chunk: %d is a group.",
			 function,
			 chunk_index );

			return( -1 );
		}
		else if( result != 0 )
		{
			if( libmfdata_list_set_element_by_index(
			     chunk_table_list,
			     chunk_index,
			     file_io_pool_entry,
			     base_offset + current_offset,
			     (size64_t) chunk_size,
			     range_flags,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to set chunk: %d.",
				 function,
				 chunk_index );

				return( -1 );
			}
		}
		else
		{
			if( libmfdata_list_get_data_range_by_index(
			     chunk_table_list,
			     chunk_index,
			     &previous_file_io_pool_entry,
			     &previous_chunk_offset,
			     &previous_chunk_size,
			     &previous_range_flags,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve data range of chunk: %d.",
				 function,
				 chunk_index );

				return( -1 );
			}
			if( ( previous_range_flags & LIBEWF_RANGE_FLAG_IS_DELTA ) == 0 )
			{
				if( (off64_t) ( base_offset + current_offset ) != previous_chunk_offset )
				{
#if defined( HAVE_DEBUG_OUTPUT )
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
						 "%s: chunk: %d offset mismatch.\n",
						 function,
						 chunk_index );
					}
#endif
					mismatch = 1;
				}
				else if( (size64_t) chunk_size != previous_chunk_size )
				{
#if defined( HAVE_DEBUG_OUTPUT )
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
						 "%s: chunk: %d size mismatch.\n",
						 function,
						 chunk_index );
					}
#endif
					mismatch = 1;
				}
				else if( ( range_flags & LIBMFDATA_RANGE_FLAG_IS_COMPRESSED )
				      != ( previous_range_flags & LIBMFDATA_RANGE_FLAG_IS_COMPRESSED ) )
				{
#if defined( HAVE_DEBUG_OUTPUT )
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
						 "%s: chunk: %d compression flag mismatch.\n",
						 function,
						 chunk_index );
					}
#endif
					mismatch = 1;
				}
				else
				{
					mismatch = 0;
				}
				update_data_range = 0;

				if( mismatch != 0 )
				{
					if( ( corrupted == 0 )
					 && ( tainted == 0 ) )
					{
						update_data_range = 1;
					}
					else if( ( ( previous_range_flags & LIBMFDATA_RANGE_FLAG_IS_CORRUPTED ) != 0 )
					      && ( corrupted == 0 ) )
					{
						update_data_range = 1;
					}
				}
				else if( ( previous_range_flags & LIBMFDATA_RANGE_FLAG_IS_TAINTED ) != 0 )
				{
					update_data_range = 1;
				}
				if( update_data_range != 0 )
				{
					if( libmfdata_list_set_data_range_by_index(
					     chunk_table_list,
					     chunk_index,
					     file_io_pool_entry,
					     base_offset + current_offset,
					     (size64_t) chunk_size,
					     range_flags,
					     error ) != 1 )
					{
						libcerror_error_set(
						 error,
						 LIBCERROR_ERROR_DOMAIN_RUNTIME,
						 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
						 "%s: unable to set data range of chunk: %d.",
						 function,
						 chunk_index );

						return( -1 );
					}
				}
			}
		}
		/* This is to compensate for the crappy > 2 GiB segment file solution in EnCase 6.7
		 */
		if( ( overflow == 0 )
		 && ( ( current_offset + chunk_size ) > (uint32_t) INT32_MAX ) )
		{
#if defined( HAVE_DEBUG_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: chunk offset overflow at: %" PRIu32 ".\n",
				 function,
				 current_offset );
			}
#endif
			overflow      = 1;
			is_compressed = 0;
		}
		chunk_index++;

		table_entry_index++;
	}
	byte_stream_copy_to_uint32_little_endian(
	 ( ( (ewf_table_entry_v1_t *) table_entries_data )[ table_entry_index ] ).chunk_data_offset,
	 stored_offset );

	if( overflow == 0 )
	{
		is_compressed  = (uint8_t) ( stored_offset >> 31 );
		current_offset = stored_offset & 0x7fffffffUL;
	}
	else
	{
		current_offset = stored_offset;
	}
	corrupted = 0;

	/* There is no indication how large the last chunk is.
	 * The only thing known is where it starts.
	 * However it can be determined using the offset of the next section.
	 * The size of the last chunk is determined by subtracting the last offset from the offset of the next section.
	 *
	 * The offset of the next section is either table_section->end_offset for original EWF and EWF-S01
	 * or table_section->start_offset for other types of EWF
	 */
	last_chunk_offset = (off64_t) base_offset + current_offset;

	if( last_chunk_offset > (off64_t) INT64_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid last chunk offset value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( last_chunk_offset < table_section->start_offset )
	{
		last_chunk_size = table_section->start_offset - last_chunk_offset;
	}
	else if( last_chunk_offset < table_section->end_offset )
	{
		last_chunk_size = table_section->end_offset - last_chunk_offset;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	else if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: invalid last chunk offset value exceeds table section end offset.\n",
		 function );
	}
#endif
	last_chunk_size -= table_section->size;

	if( last_chunk_size <= 0 )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: invalid last chunk size value is zero or less.\n",
			 function );
		}
#endif
		corrupted = 1;
	}
	if( last_chunk_size > (off64_t) INT32_MAX )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: invalid last chunk size value exceeds maximum.\n",
			 function );
		}
#endif
		corrupted = 1;
	}
	range_flags = LIBEWF_RANGE_FLAG_HAS_CHECKSUM;

	if( is_compressed != 0 )
	{
		range_flags |= LIBMFDATA_RANGE_FLAG_IS_COMPRESSED;
	}
	if( corrupted != 0 )
	{
		range_flags |= LIBMFDATA_RANGE_FLAG_IS_CORRUPTED;
	}
	if( tainted != 0 )
	{
		range_flags |= LIBMFDATA_RANGE_FLAG_IS_TAINTED;
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( is_compressed == 0 )
	{
		chunk_type = "uncompressed";
	}
	else
	{
		chunk_type = "compressed";
	}
	if( corrupted != 0 )
	{
		remarks = " corrupted";
	}
	else if( tainted != 0 )
	{
		remarks = " tainted";
	}
	else
	{
		remarks = "";
	}
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: %s last chunk %d read with: base %" PRIi64 ", offset %" PRIu32 " and calculated size %" PRIi64 "%s.\n",
		 function,
		 chunk_type,
		 chunk_index,
		 base_offset,
		 current_offset,
		 last_chunk_size,
		 remarks );
	}
#endif
	result = libmfdata_list_is_group(
		  chunk_table_list,
		  chunk_index,
		  error );

	if( result == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine if chunk: %d is a group.",
		 function,
		 chunk_index );

		return( -1 );
	}
	else if( result != 0 )
	{
		if( libmfdata_list_set_element_by_index(
		     chunk_table_list,
		     chunk_index,
		     file_io_pool_entry,
		     last_chunk_offset,
		     (size64_t) last_chunk_size,
		     range_flags,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set chunk: %d.",
			 function,
			 chunk_index );

			return( -1 );
		}
	}
	else
	{
		if( libmfdata_list_get_data_range_by_index(
		     chunk_table_list,
		     chunk_index,
		     &previous_file_io_pool_entry,
		     &previous_chunk_offset,
		     &previous_chunk_size,
		     &previous_range_flags,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve data range of chunk: %d.",
			 function,
			 chunk_index );

			return( -1 );
		}
		if( ( previous_range_flags & LIBEWF_RANGE_FLAG_IS_DELTA ) == 0 )
		{
				if( (off64_t) ( base_offset + current_offset ) != previous_chunk_offset )
				{
#if defined( HAVE_DEBUG_OUTPUT )
					if( libcnotify_verbose != 0 )
					{
						libcnotify_printf(
						 "%s: chunk: %d offset mismatch.\n",
						 function,
						 chunk_index );
					}
#endif
					mismatch = 1;
			}
			else if( (size64_t) chunk_size != previous_chunk_size )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: chunk: %d size mismatch.\n",
					 function,
					 chunk_index );
				}
#endif
				mismatch = 1;
			}
			else if( ( range_flags & LIBMFDATA_RANGE_FLAG_IS_COMPRESSED )
			      != ( previous_range_flags & LIBMFDATA_RANGE_FLAG_IS_COMPRESSED ) )
			{
#if defined( HAVE_DEBUG_OUTPUT )
				if( libcnotify_verbose != 0 )
				{
					libcnotify_printf(
					 "%s: chunk: %d compression flag mismatch.\n",
					 function,
					 chunk_index );
				}
#endif
				mismatch = 1;
			}
			else
			{
				mismatch = 0;
			}
			update_data_range = 0;

			if( mismatch != 0 )
			{
				if( ( corrupted == 0 )
				 && ( tainted == 0 ) )
				{
					update_data_range = 1;
				}
				else if( ( ( previous_range_flags & LIBMFDATA_RANGE_FLAG_IS_CORRUPTED ) != 0 )
				      && ( corrupted == 0 ) )
				{
					update_data_range = 1;
				}
			}
			else if( ( previous_range_flags & LIBMFDATA_RANGE_FLAG_IS_TAINTED ) != 0 )
			{
				update_data_range = 1;
			}
			if( update_data_range != 0 )
			{
				if( libmfdata_list_set_data_range_by_index(
				     chunk_table_list,
				     chunk_index,
				     file_io_pool_entry,
				     base_offset + current_offset,
				     (size64_t) chunk_size,
				     range_flags,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
					 "%s: unable to set data range of chunk: %d.",
					 function,
					 chunk_index );

					return( -1 );
				}
			}
		}
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "\n" );
	}
#endif
	return( 1 );
}

/* Fills the offsets from the chunk table
 * Returns 1 if successful or -1 on error
 */
int libewf_chunk_table_fill_offsets(
     libmfdata_list_t *chunk_table_list,
     int chunk_index,
     off64_t base_offset,
     ewf_table_entry_v1_t *table_offsets,
     uint32_t number_of_offsets,
     libcerror_error_t **error )
{
	static char *function       = "libewf_chunk_table_fill_offsets";
	off64_t chunk_offset        = 0;
	size64_t chunk_size         = 0;
	uint32_t range_flags        = 0;
	uint32_t table_offset       = 0;
	uint32_t table_offset_index = 0;
	int file_io_pool_entry      = 0;

	if( chunk_table_list == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid chunk table list.",
		 function );

		return( -1 );
	}
	if( base_offset < 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid base offset.",
		 function );

		return( -1 );
	}
	if( table_offsets == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table offsets.",
		 function );

		return( -1 );
	}
	for( table_offset_index = 0;
	     table_offset_index < number_of_offsets;
	     table_offset_index++ )
	{
		if( libmfdata_list_get_data_range_by_index(
		     chunk_table_list,
		     chunk_index,
		     &file_io_pool_entry,
		     &chunk_offset,
		     &chunk_size,
		     &range_flags,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve data range of chunk: %d.",
			 function,
			 chunk_index );

			return( -1 );
		}
		chunk_offset -= base_offset;

		if( ( chunk_offset < 0 )
		 || ( chunk_offset > (off64_t) INT32_MAX ) )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: invalid chunk: %d offset value out of bounds.",
			 function,
			 chunk_index );

			return( -1 );
		}
		table_offset = (uint32_t) chunk_offset;

		if( ( range_flags & LIBMFDATA_RANGE_FLAG_IS_COMPRESSED ) != 0 )
		{
			table_offset |= 0x80000000UL;
		}
		byte_stream_copy_from_uint32_little_endian(
		 table_offsets[ table_offset_index ].chunk_data_offset,
		 table_offset );

		chunk_index++;
	}
	return( 1 );
}

