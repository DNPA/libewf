/*
 * libewf main handle
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
#include <memory.h>
#include <notify.h>
#include <types.h>

#include "libewf_definitions.h"
#include "libewf_handle.h"
#include "libewf_header_values.h"
#include "libewf_string.h"

#include "ewf_crc.h"
#include "ewf_data.h"
#include "ewf_definitions.h"
#include "ewf_file_header.h"

/* Initialize the handle
 * Returns 1 if successful or -1 on error
 */
int libewf_handle_initialize(
     libewf_handle_t **handle,
     uint8_t flags,
     libewf_error_t **error )
{
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_handle_initialize";

	if( handle == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( *handle == NULL )
	{
		internal_handle = (libewf_internal_handle_t *) memory_allocate(
		                                                sizeof( libewf_internal_handle_t ) );

		if( internal_handle == NULL )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_MEMORY,
			 LIBEWF_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create internal handle.\n",
			 function );

			return( -1 );
		}
		if( memory_set(
		     internal_handle,
		     0,
		     sizeof( libewf_internal_handle_t ) ) == NULL )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_MEMORY,
			 LIBEWF_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear internal handle.\n",
			 function );

			memory_free(
			 internal_handle );

			return( -1 );
		}
		internal_handle->compression_level = EWF_COMPRESSION_UNKNOWN;
		internal_handle->format            = LIBEWF_FORMAT_UNKNOWN;
		internal_handle->ewf_format        = EWF_FORMAT_UNKNOWN;
		internal_handle->error_tollerance  = LIBEWF_ERROR_TOLLERANCE_COMPENSATE;

		/* The segment table is initially filled with a single entry
		 * and no limitations on the amount of open files
		 */
		if( libewf_file_io_pool_initialize(
		     &( internal_handle->file_io_pool ),
		     0,
		     LIBEWF_FILE_IO_POOL_UNLIMITED_AMOUNT_OF_OPEN_FILES ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create file io pool.\n",
			 function );

			memory_free(
			 internal_handle );

			return( -1 );
		}
		/* The segment table is initially filled with a single entry
		 */
		if( libewf_segment_table_initialize(
		     &( internal_handle->segment_table ),
		     1,
		     error ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create segment table.\n",
			 function );

			libewf_file_io_pool_free(
			 &( internal_handle->file_io_pool ) );
			memory_free(
			 internal_handle );

			return( -1 );
		}
		/* The delta segment table is initially filled with a single entry
		 */
		if( libewf_segment_table_initialize(
		     &( internal_handle->delta_segment_table ),
		     1,
		     error ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create delta segment table.\n",
			 function );

			libewf_segment_table_free(
			 &( internal_handle->segment_table ),
			 NULL );
			libewf_file_io_pool_free(
			 &( internal_handle->file_io_pool ) );
			memory_free(
			 internal_handle );

			return( -1 );
		}
		if( libewf_offset_table_initialize(
		     &( internal_handle->offset_table ),
		     0 ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create offset table.\n",
			 function );

			libewf_segment_table_free(
			 &( internal_handle->delta_segment_table ),
			 NULL );
			libewf_segment_table_free(
			 &( internal_handle->segment_table ),
			 NULL );
			libewf_file_io_pool_free(
			 &( internal_handle->file_io_pool ) );
			memory_free(
			 internal_handle );

			return( -1 );
		}
		if( libewf_chunk_cache_initialize(
		     &( internal_handle->chunk_cache ),
		     EWF_MINIMUM_CHUNK_SIZE + sizeof( ewf_crc_t ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create chunk cache.\n",
			 function );

			libewf_offset_table_free(
			 &( internal_handle->offset_table ) );
			libewf_segment_table_free(
			 &( internal_handle->delta_segment_table ),
			 NULL );
			libewf_segment_table_free(
			 &( internal_handle->segment_table ),
			 NULL );
			libewf_file_io_pool_free(
			 &( internal_handle->file_io_pool ) );
			memory_free(
			 internal_handle );

			return( -1 );
		}
		if( libewf_media_values_initialize(
		     &( internal_handle->media_values ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create media values.\n",
			 function );

			libewf_chunk_cache_free(
			 &( internal_handle->chunk_cache ) );
			libewf_offset_table_free(
			 &( internal_handle->offset_table ) );
			libewf_segment_table_free(
			 &( internal_handle->delta_segment_table ),
			 NULL );
			libewf_segment_table_free(
			 &( internal_handle->segment_table ),
			 NULL );
			libewf_file_io_pool_free(
			 &( internal_handle->file_io_pool ) );
			memory_free(
			 internal_handle );

			return( -1 );
		}
		if( libewf_header_sections_initialize(
		     &( internal_handle->header_sections ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create header sections.\n",
			 function );

			libewf_media_values_free(
			 &( internal_handle->media_values ) );
			libewf_chunk_cache_free(
			 &( internal_handle->chunk_cache ) );
			libewf_offset_table_free(
			 &( internal_handle->offset_table ) );
			libewf_segment_table_free(
			 &( internal_handle->delta_segment_table ),
			 NULL );
			libewf_segment_table_free(
			 &( internal_handle->segment_table ),
			 NULL );
			libewf_file_io_pool_free(
			 &( internal_handle->file_io_pool ) );
			memory_free(
			 internal_handle );

			return( -1 );
		}
		if( libewf_hash_sections_initialize(
		     &( internal_handle->hash_sections ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create hash sections.\n",
			 function );

			libewf_header_sections_free(
			 &( internal_handle->header_sections ) );
			libewf_media_values_free(
			 &( internal_handle->media_values ) );
			libewf_chunk_cache_free(
			 &( internal_handle->chunk_cache ) );
			libewf_offset_table_free(
			 &( internal_handle->offset_table ) );
			libewf_segment_table_free(
			 &( internal_handle->delta_segment_table ),
			 NULL );
			libewf_segment_table_free(
			 &( internal_handle->segment_table ),
			 NULL );
			libewf_file_io_pool_free(
			 &( internal_handle->file_io_pool ) );
			memory_free(
			 internal_handle );

			return( -1 );
		}
		if( libewf_sector_table_initialize(
		     &( internal_handle->sessions ),
		     0 ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create sessions.\n",
			 function );

			libewf_hash_sections_free(
			 &( internal_handle->hash_sections ) );
			libewf_header_sections_free(
			 &( internal_handle->header_sections ) );
			libewf_media_values_free(
			 &( internal_handle->media_values ) );
			libewf_chunk_cache_free(
			 &( internal_handle->chunk_cache ) );
			libewf_offset_table_free(
			 &( internal_handle->offset_table ) );
			libewf_segment_table_free(
			 &( internal_handle->delta_segment_table ),
			 NULL );
			libewf_segment_table_free(
			 &( internal_handle->segment_table ),
			 NULL );
			libewf_file_io_pool_free(
			 &( internal_handle->file_io_pool ) );
			memory_free(
			 internal_handle );

			return( -1 );
		}
		if( libewf_sector_table_initialize(
		     &( internal_handle->acquiry_errors ),
		     0 ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create acquiry errors.\n",
			 function );

			libewf_sector_table_free(
			 &( internal_handle->sessions ) );
			libewf_hash_sections_free(
			 &( internal_handle->hash_sections ) );
			libewf_header_sections_free(
			 &( internal_handle->header_sections ) );
			libewf_media_values_free(
			 &( internal_handle->media_values ) );
			libewf_chunk_cache_free(
			 &( internal_handle->chunk_cache ) );
			libewf_offset_table_free(
			 &( internal_handle->offset_table ) );
			libewf_segment_table_free(
			 &( internal_handle->delta_segment_table ),
			 NULL );
			libewf_segment_table_free(
			 &( internal_handle->segment_table ),
			 NULL );
			libewf_file_io_pool_free(
			 &( internal_handle->file_io_pool ) );
			memory_free(
			 internal_handle );

			return( -1 );
		}
		if( ( flags & LIBEWF_FLAG_READ ) == LIBEWF_FLAG_READ )
		{
			if( libewf_internal_handle_subhandle_read_initialize(
			     &( internal_handle->read ),
			     error ) != 1 )
			{
				libewf_error_set(
				 error,
				 LIBEWF_ERROR_DOMAIN_RUNTIME,
				 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create subhandle read.\n",
				 function );

				libewf_sector_table_free(
				 &( internal_handle->acquiry_errors ) );
				libewf_sector_table_free(
				 &( internal_handle->sessions ) );
				libewf_hash_sections_free(
				 &( internal_handle->hash_sections ) );
				libewf_header_sections_free(
				 &( internal_handle->header_sections ) );
				libewf_media_values_free(
				 &( internal_handle->media_values ) );
				libewf_chunk_cache_free(
				 &( internal_handle->chunk_cache ) );
				libewf_offset_table_free(
				 &( internal_handle->offset_table ) );
				libewf_segment_table_free(
				 &( internal_handle->delta_segment_table ),
				 NULL );
				libewf_segment_table_free(
				 &( internal_handle->segment_table ),
				 NULL );
				libewf_file_io_pool_free(
				 &( internal_handle->file_io_pool ) );
				memory_free(
				 internal_handle );

				return( -1 );
			}
		}
		if( ( flags & LIBEWF_FLAG_WRITE ) == LIBEWF_FLAG_WRITE )
		{
			if( libewf_internal_handle_subhandle_write_initialize(
			     &( internal_handle->write ),
			      error ) != 1 )
			{
				libewf_error_set(
				 error,
				 LIBEWF_ERROR_DOMAIN_RUNTIME,
				 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create subhandle write.\n",
				 function );

				libewf_internal_handle_subhandle_read_free(
				 &( internal_handle->read ),
				 NULL );
				libewf_sector_table_free(
				 &( internal_handle->acquiry_errors ) );
				libewf_sector_table_free(
				 &( internal_handle->sessions ) );
				libewf_hash_sections_free(
				 &( internal_handle->hash_sections ) );
				libewf_header_sections_free(
				 &( internal_handle->header_sections ) );
				libewf_media_values_free(
				 &( internal_handle->media_values ) );
				libewf_chunk_cache_free(
				 &( internal_handle->chunk_cache ) );
				libewf_offset_table_free(
				 &( internal_handle->offset_table ) );
				libewf_segment_table_free(
				 &( internal_handle->delta_segment_table ),
				 NULL );
				libewf_segment_table_free(
				 &( internal_handle->segment_table ),
				 NULL );
				libewf_file_io_pool_free(
				 &( internal_handle->file_io_pool ) );
				memory_free(
				 internal_handle );

				return( -1 );
			}
		}
		*handle = (libewf_handle_t *) internal_handle;
	}
	return( 1 );
}

/* Frees the handle including elements
 * Returns 1 if successful or -1 on error
 */
int libewf_handle_free(
     libewf_handle_t **handle,
     libewf_error_t **error )
{
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_internal_handle_free";

	if( handle == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( *handle != NULL )
	{
		internal_handle = (libewf_internal_handle_t *) *handle;

		if( libewf_internal_handle_subhandle_read_free(
		     &( internal_handle->read ),
		     error ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free subhandle read.\n",
			 function );
		}
		if( libewf_internal_handle_subhandle_write_free(
		     &( internal_handle->write ),
		     error ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free subhandle write.\n",
			 function );
		}
		if( libewf_file_io_pool_free(
		     &( internal_handle->file_io_pool ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free file io pool.\n",
			 function );
		}
		if( libewf_segment_table_free(
		     &( internal_handle->segment_table ),
		     error ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free segment table.\n",
			 function );
		}
		if( libewf_segment_table_free(
		     &( internal_handle->delta_segment_table ),
		     error ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free delta segment table.\n",
			 function );
		}
		if( libewf_offset_table_free(
		     &( internal_handle->offset_table ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free offset table.\n",
			 function );
		}
		if( libewf_chunk_cache_free(
		     &( internal_handle->chunk_cache ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free chunk cache.\n",
			 function );
		}
		if( libewf_media_values_free(
		     &( internal_handle->media_values ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free media values.\n",
			 function );
		}
		if( libewf_header_sections_free(
		     &( internal_handle->header_sections ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free header sections.\n",
			 function );
		}
		if( libewf_hash_sections_free(
		     &( internal_handle->hash_sections ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free hash sections.\n",
			 function );
		}
		if( libewf_values_table_free(
		     &( internal_handle->header_values ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free header values.\n",
			 function );
		}
		if( libewf_values_table_free(
		     &( internal_handle->hash_values ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free hash values.\n",
			 function );
		}
		if( libewf_sector_table_free(
		     &( internal_handle->sessions ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free sessions.\n",
			 function );
		}
		if( libewf_sector_table_free(
		     &( internal_handle->acquiry_errors ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free acquiry errors.\n",
			 function );
		}
		memory_free(
		 internal_handle );

		*handle = NULL;
	}
	return( 1 );
}

/* Initialize the subhandle read
 * Returns 1 if successful or -1 on error
 */
int libewf_internal_handle_subhandle_read_initialize(
     libewf_internal_handle_read_t **subhandle_read,
     libewf_error_t **error )
{
	static char *function = "libewf_internal_handle_subhandle_read_initialize";

	if( subhandle_read == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid subhandle read.\n",
		 function );

		return( -1 );
	}
	if( *subhandle_read == NULL )
	{
		*subhandle_read = (libewf_internal_handle_read_t *) memory_allocate(
		                                                     sizeof( libewf_internal_handle_read_t ) );

		if( *subhandle_read == NULL )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_MEMORY,
			 LIBEWF_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create subhandle read.\n",
			 function );

			return( -1 );
		}
		if( memory_set(
		     *subhandle_read,
		     0,
		     sizeof( libewf_internal_handle_read_t ) ) == NULL )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_MEMORY,
			 LIBEWF_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear subhandle read.\n",
			 function );

			memory_free(
			 *subhandle_read );

			*subhandle_read = NULL;

			return( -1 );
		}
		if( libewf_sector_table_initialize(
		     &( ( *subhandle_read )->crc_errors ),
		     0 ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create crc errors.\n",
			 function );

			memory_free(
			 *subhandle_read );

			*subhandle_read = NULL;

			return( -1 );
		}
		( *subhandle_read )->wipe_on_error = 1;
	}
	return( 1 );
}

/* Frees the subhandle read including elements
 * Returns 1 if successful or -1 on error
 */
int libewf_internal_handle_subhandle_read_free(
     libewf_internal_handle_read_t **subhandle_read,
     libewf_error_t **error )
{
	static char *function = "libewf_internal_subhandle_read_free";

	if( subhandle_read == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid subhandle read.\n",
		 function );

		return( 1 );
	}
	if( *subhandle_read != NULL )
	{
		if( libewf_sector_table_free(
		     &( ( *subhandle_read )->crc_errors ) ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
			 "%s: unable to free crc errors.\n",
			 function );
		}
		memory_free(
		 *subhandle_read );

		*subhandle_read = NULL;
	}
	return( 1 );
}

/* Initialize the subhandle write
 * Returns 1 if successful or -1 on error
 */
int libewf_internal_handle_subhandle_write_initialize(
     libewf_internal_handle_write_t **subhandle_write,
     libewf_error_t **error )
{
	static char *function = "libewf_internal_handle_subhandle_write_initialize";

	if( subhandle_write == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid subhandle write.\n",
		 function );

		return( -1 );
	}
	if( *subhandle_write == NULL )
	{
		*subhandle_write = (libewf_internal_handle_write_t *) memory_allocate(
		                                                       sizeof( libewf_internal_handle_write_t ) );

		if( subhandle_write == NULL )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_MEMORY,
			 LIBEWF_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create subhandle write.\n",
			 function );

			return( -1 );
		}
		if( memory_set(
		     *subhandle_write,
		     0,
		     sizeof( libewf_internal_handle_write_t ) ) == NULL )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_MEMORY,
			 LIBEWF_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear subhandle write.\n",
			 function );

			memory_free(
			 *subhandle_write );

			*subhandle_write = NULL;

			return( -1 );
		}
		( *subhandle_write )->maximum_segment_file_size        = INT32_MAX;
		( *subhandle_write )->segment_file_size                = LIBEWF_DEFAULT_SEGMENT_FILE_SIZE;
		( *subhandle_write )->remaining_segment_file_size      = LIBEWF_DEFAULT_SEGMENT_FILE_SIZE;
		( *subhandle_write )->delta_segment_file_size          = INT64_MAX;
		( *subhandle_write )->maximum_section_amount_of_chunks = EWF_MAXIMUM_OFFSETS_IN_TABLE;
	}
	return( 1 );
}

/* Frees the subhandle write including elements
 * Returns 1 if successful or -1 on error
 */
int libewf_internal_handle_subhandle_write_free(
     libewf_internal_handle_write_t **subhandle_write,
     libewf_error_t **error )
{
	static char *function = "libewf_internal_handle_subhandle_write_free";

	if( subhandle_write == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid subhandle write.\n",
		 function );

		return( 1 );
	}
	if( *subhandle_write != NULL )
	{
		if( ( *subhandle_write )->data_section != NULL )
		{
			memory_free(
			 ( *subhandle_write )->data_section );
		}
		if( ( *subhandle_write )->table_offsets != NULL )
		{
			memory_free(
			 ( *subhandle_write )->table_offsets );
		}
		memory_free(
		 *subhandle_write );

		*subhandle_write = NULL;
	}
	return( 1 );
}

/* Retrieves the maximum amount of supported segment files to write
 * Returns 1 if successful or -1 on error
 */
int libewf_internal_handle_get_write_maximum_amount_of_segments(
     uint8_t ewf_format,
     uint16_t *maximum_amount_of_segments,
     libewf_error_t **error )
{
	static char *function = "libewf_internal_handle_get_write_maximum_amount_of_segments";

	if( maximum_amount_of_segments == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid maximum amount of segments.\n",
		 function );

		return( -1 );
	}
	if( ewf_format == EWF_FORMAT_S01 )
	{
		/* = 4831
		 */
		*maximum_amount_of_segments = (uint16_t) ( ( (int) ( 'z' - 's' ) * 26 * 26 ) + 99 );
	}
	else if( ewf_format == EWF_FORMAT_E01 )
	{
		/* = 14295
		 */
		*maximum_amount_of_segments = (uint16_t) ( ( (int) ( 'Z' - 'E' ) * 26 * 26 ) + 99 );
	}
	else
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported EWF format.\n",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Inializes the media values
 * Returns 1 if the values were successfully initialized, -1 on errror
 */
int libewf_internal_handle_initialize_media_values(
     libewf_internal_handle_t *internal_handle,
     uint32_t sectors_per_chunk,
     uint32_t bytes_per_sector,
     size64_t media_size,
     libewf_error_t **error )
{
	static char *function            = "libewf_internal_handle_initialize_media_values";
	size32_t chunk_size              = 0;
	size64_t maximum_input_file_size = 0;
	int64_t amount_of_chunks         = 0;
	int64_t amount_of_sectors        = 0;

	if( internal_handle == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		return( -1 );
	}
	if( ( sectors_per_chunk == 0 )
	 || ( sectors_per_chunk > (uint32_t) INT32_MAX ) )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_OUT_OF_RANGE,
		 "%s: invalid sectors per chunk.\n",
		 function );

		return( -1 );
	}
	if( ( bytes_per_sector == 0 )
	 || ( bytes_per_sector > (uint32_t) INT32_MAX ) )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_OUT_OF_RANGE,
		 "%s: invalid bytes per sector.\n",
		 function );

		return( -1 );
	}
	if( media_size > (size64_t) INT64_MAX )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_EXCEEDS_MAXIMUM,
		 "%s: invalid media size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	/* Determine the chunk size
	 */
	chunk_size = sectors_per_chunk * bytes_per_sector;

	if( ( chunk_size == 0 )
	 || ( chunk_size > (size32_t) INT32_MAX ) )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
		 "%s: invalid chunk size.\n",
		 function );

		return( -1 );
	}
	/* Check if the input file size does not exceed the maximum possible input file size
	 * for the chunk size
	 */
	maximum_input_file_size = (size64_t) chunk_size * (size64_t) UINT32_MAX;

	if( media_size > maximum_input_file_size )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
		 "%s: media size cannot be larger than size: %" PRIu64 " with a chunk size of: %" PRIu32 ".\n",
		 function, maximum_input_file_size, chunk_size );

		return( -1 );
	}
	internal_handle->media_values->sectors_per_chunk = sectors_per_chunk;
	internal_handle->media_values->bytes_per_sector  = bytes_per_sector;
	internal_handle->media_values->chunk_size        = chunk_size;
	internal_handle->media_values->media_size        = media_size;

	/* If a media size was provided
	 */
	if( media_size > 0 )
	{
		/* Determine the amount of chunks to write
		 */
		amount_of_chunks = (int64_t) media_size / (int64_t) chunk_size;

		if( ( media_size % chunk_size ) != 0 )
		{
			amount_of_chunks += 1;
		}
		if( amount_of_chunks > (int64_t) UINT32_MAX )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_EXCEEDS_MAXIMUM,
			 "%s: invalid amount of chunks value exceeds maximum.\n",
			 function );

			return( -1 );
		}
		internal_handle->media_values->amount_of_chunks = (uint32_t) amount_of_chunks;

		/* Determine the amount of sectors to write
		 */
		amount_of_sectors = (int64_t) media_size / (int64_t) bytes_per_sector;

		if( amount_of_chunks > (int64_t) UINT32_MAX )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_EXCEEDS_MAXIMUM,
			 "%s: invalid amount of sectors value exceeds maximum.\n",
			 function );

			return( -1 );
		}
		internal_handle->media_values->amount_of_sectors = (uint32_t) amount_of_sectors;
	}
	return( 1 );
}

/* Inializes internal values based on the EWF file format
 * Returns 1 if the values were successfully initialized, -1 on errror
 */
int libewf_internal_handle_initialize_format(
     libewf_internal_handle_t *internal_handle,
     libewf_error_t **error )
{
	static char *function = "libewf_internal_handle_initialize_format";

	if( internal_handle == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( ( internal_handle->format == LIBEWF_FORMAT_EWF )
	 || ( internal_handle->format == LIBEWF_FORMAT_SMART ) )
	{
		internal_handle->ewf_format = EWF_FORMAT_S01;
	}
	else if( internal_handle->format == LIBEWF_FORMAT_LVF )
	{
		internal_handle->ewf_format = EWF_FORMAT_L01;
	}
	else
	{
		internal_handle->ewf_format = EWF_FORMAT_E01;
	}
	if( internal_handle->write != NULL )
	{
		if( internal_handle->format == LIBEWF_FORMAT_ENCASE6 )
		{
			internal_handle->write->maximum_segment_file_size        = INT64_MAX;
			internal_handle->write->maximum_section_amount_of_chunks = EWF_MAXIMUM_OFFSETS_IN_TABLE_ENCASE6;
		}
		else if( internal_handle->format == LIBEWF_FORMAT_EWFX )
		{
			internal_handle->write->unrestrict_offset_amount         = 1;
			internal_handle->write->maximum_segment_file_size        = INT32_MAX;
			internal_handle->write->maximum_section_amount_of_chunks = INT32_MAX;
		}
		else
		{
			internal_handle->write->maximum_segment_file_size        = INT32_MAX;
			internal_handle->write->maximum_section_amount_of_chunks = EWF_MAXIMUM_OFFSETS_IN_TABLE;
		}
		/* Determine the maximum amount of segments allowed to write
		 */
		if( libewf_internal_handle_get_write_maximum_amount_of_segments(
		     internal_handle->ewf_format,
		     &( internal_handle->write->maximum_amount_of_segments ),
		     error ) != 1 )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine the maximum amount of allowed segment files.\n",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

/* Create the default header values
 * Returns 1 on success or -1 on error
 */
int libewf_internal_handle_create_header_values(
     libewf_internal_handle_t *internal_handle,
     libewf_error_t **error )
{
	static char *function = "libewf_internal_handle_create_header_values";

	if( internal_handle == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( internal_handle->header_values != NULL )
	{
		notify_verbose(
		 "%s: header values already created - cleaning up previous header values.\n",
		 function );
	}
#endif
	if( libewf_values_table_free(
	     &( internal_handle->header_values ) ) != 1 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_FREE_FAILED,
		 "%s: unable to free header values.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_initialize(
	     &( internal_handle->header_values ),
	     LIBEWF_HEADER_VALUES_DEFAULT_AMOUNT ) != 1 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create header values.\n",
		 function );

		return( -1 );
	}
	if( libewf_header_values_initialize(
	     internal_handle->header_values ) != 1 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to initialize the header values.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _LIBEWF_CHARACTER_T_STRING( "case_number" ),
	     11,
	     _LIBEWF_CHARACTER_T_STRING( "Case Number" ),
	     11 ) != 1 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set case number.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _LIBEWF_CHARACTER_T_STRING( "description" ),
	     11,
	     _LIBEWF_CHARACTER_T_STRING( "Description" ),
	     11 ) != 1 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set description.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _LIBEWF_CHARACTER_T_STRING( "evidence_number" ),
	     15,
	     _LIBEWF_CHARACTER_T_STRING( "Evidence Number" ),
	     15 ) != 1 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set evidence number.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _LIBEWF_CHARACTER_T_STRING( "examiner_name" ),
	     13,
	     _LIBEWF_CHARACTER_T_STRING( "Examiner Name" ),
	     13 ) != 1 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set examiner name.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _LIBEWF_CHARACTER_T_STRING( "notes" ),
	     5,
	     _LIBEWF_CHARACTER_T_STRING( "Notes" ),
	     5 ) != 1 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set notes.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _LIBEWF_CHARACTER_T_STRING( "acquiry_operating_system" ),
	     24,
	     _LIBEWF_CHARACTER_T_STRING( "Undetermined" ),
	     12 ) != 1 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set acquiry operating system.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _LIBEWF_CHARACTER_T_STRING( "acquiry_software_version" ),
	     24,
	     LIBEWF_VERSION_STRING,
	     libewf_string_length(
	      LIBEWF_VERSION_STRING ) ) != 1 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set acquiry software version.\n",
		 function );

		return( -1 );
	}
        /* The acquiry date, system date values and compression type
	 * will be generated automatically when set to NULL
         */
	return( 1 );
}

/* Initializes the write values
 * Returns 1 if successful or -1 on error
 */
int libewf_internal_handle_write_initialize(
     libewf_internal_handle_t *internal_handle,
     libewf_error_t **error )
{
	static char *function               = "libewf_internal_handle_write_initialize";
	int64_t required_amount_of_segments = 0;

	if( internal_handle == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID,
		 "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->write == NULL )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing subhandle write.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->write->values_initialized != 0 )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: write values were initialized and cannot be initialized anymore.\n",
		 function );

		return( -1 );
	}
	/* Determine the EWF file format
	 */
	if( internal_handle->format == LIBEWF_FORMAT_LVF )
	{
		libewf_error_set(
		 error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: writing format LVF currently not supported.\n",
		 function );

		return( -1 );
	}
	/* If no input write size was provided check if EWF file format allows for streaming
	 */
	if( internal_handle->media_values->media_size == 0 )
	{
		if( ( internal_handle->format != LIBEWF_FORMAT_ENCASE2 )
		 && ( internal_handle->format != LIBEWF_FORMAT_ENCASE3 )
		 && ( internal_handle->format != LIBEWF_FORMAT_ENCASE4 )
		 && ( internal_handle->format != LIBEWF_FORMAT_ENCASE5 )
		 && ( internal_handle->format != LIBEWF_FORMAT_ENCASE6 )
		 && ( internal_handle->format != LIBEWF_FORMAT_LINEN5 )
		 && ( internal_handle->format != LIBEWF_FORMAT_LINEN6 )
		 && ( internal_handle->format != LIBEWF_FORMAT_FTK )
		 && ( internal_handle->format != LIBEWF_FORMAT_EWFX ) )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: EWF file format does not allow for streaming write.\n",
			 function );

			return( -1 );
		}
	}
	/* If an input write size was provided
	 */
	else if( internal_handle->media_values->media_size > 0 )
	{
		/* Determine the required amount of segments allowed to write
		 */
		required_amount_of_segments = (int64_t) internal_handle->media_values->media_size
		                            / (int64_t) internal_handle->write->segment_file_size;

		if( required_amount_of_segments > (int64_t) internal_handle->write->maximum_amount_of_segments )
		{
			libewf_error_set(
			 error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
			 "%s: the settings exceed the maximum amount of allowed segment files.\n",
			 function );

			return( -1 );
		}
	}
	/* Flag that the write values were initialized
	 */
	internal_handle->write->values_initialized = 1;

	return( 1 );
}

