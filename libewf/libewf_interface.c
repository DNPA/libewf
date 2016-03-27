/*
 * Interface functions
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

#include "libewf_definitions.h"
#include "libewf_error.h"
#include "libewf_handle.h"
#include "libewf_hash_values.h"
#include "libewf_header_values.h"
#include "libewf_interface.h"
#include "libewf_string.h"
#include "libewf_segment_file_handle.h"

#include "ewf_definitions.h"
#include "ewf_digest_hash.h"

/* Returns the flags for reading
 */
uint8_t libewf_get_flags_read(
         void )
{
	return( (uint8_t) LIBEWF_FLAG_READ );
}

/* Returns the flags for reading and writing
 */
uint8_t libewf_get_flags_read_write(
         void )
{
	return( (uint8_t) ( LIBEWF_FLAG_READ | LIBEWF_FLAG_WRITE ) );
}

/* Returns the flags for writing
 */
uint8_t libewf_get_flags_write(
         void )
{
	return( (uint8_t) LIBEWF_FLAG_WRITE );
}

/* Retrieves the amount of sectors per chunk from the media information
 * Returns 1 if successful or -1 on error
 */
int libewf_get_sectors_per_chunk(
     libewf_handle_t *handle,
     uint32_t *sectors_per_chunk )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_sectors_per_chunk";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( sectors_per_chunk == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid sectors per chunk.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->media_values->sectors_per_chunk > (uint32_t) INT32_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid sectors per chunk value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*sectors_per_chunk = internal_handle->media_values->sectors_per_chunk;

	return( 1 );
}

/* Retrieves the amount of bytes per sector from the media information
 * Returns 1 if successful or -1 on error
 */
int libewf_get_bytes_per_sector(
     libewf_handle_t *handle,
     uint32_t *bytes_per_sector )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_bytes_per_sector";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( bytes_per_sector == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid bytes per sector.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->media_values->bytes_per_sector > (uint32_t) INT32_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid bytes per sector value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*bytes_per_sector = internal_handle->media_values->bytes_per_sector;

	return( 1 );
}

/* Retrieves the amount of sectors from the media information
 * Returns 1 if successful or -1 on error
 */
int libewf_get_amount_of_sectors(
     libewf_handle_t *handle,
     uint32_t *amount_of_sectors )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_amount_of_sectors";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( amount_of_sectors == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid bytes per sector.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->media_values->amount_of_sectors > (uint32_t) INT32_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid amount of sectors value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*amount_of_sectors = internal_handle->media_values->amount_of_sectors;

	return( 1 );
}

/* Retrieves the chunk size from the media information
 * Returns 1 if successful or -1 on error
 */
int libewf_get_chunk_size(
     libewf_handle_t *handle,
     size32_t *chunk_size )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_chunk_size";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( chunk_size == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid chunk size.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->media_values->chunk_size > (size32_t) INT32_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid chunk size value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*chunk_size = internal_handle->media_values->chunk_size;

	return( 1 );
}

/* Retrieves the error granularity from the media information
 * Returns 1 if successful or -1 on error
 */
int libewf_get_error_granularity(
     libewf_handle_t *handle,
     uint32_t *error_granularity )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_error_granularity";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( error_granularity == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid error granularity.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->media_values->error_granularity > (uint32_t) INT32_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid error granularity value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*error_granularity = internal_handle->media_values->error_granularity;

	return( 1 );
}

/* Retrieves the compression values
 * Returns 1 if successful or -1 on error
 */
int libewf_get_compression_values(
     libewf_handle_t *handle,
     int8_t *compression_level,
     uint8_t *compress_empty_block )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_compression_values";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( compression_level == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid compression level.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( compress_empty_block == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid compress empty block.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*compression_level    = internal_handle->compression_level;
	*compress_empty_block = internal_handle->compress_empty_block;

	return( 1 );
}

/* Retrieves the size of the contained media data
 * Returns 1 if successful or -1 on error
 */
int libewf_get_media_size(
     libewf_handle_t *handle,
     size64_t *media_size )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_media_size";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( media_size == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media size.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->media_values->media_size == 0 )
	{
		internal_handle->media_values->media_size = (size64_t) internal_handle->media_values->amount_of_sectors
		                                          * (size64_t) internal_handle->media_values->bytes_per_sector;
	}
	if( internal_handle->media_values->media_size > (size64_t) INT64_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid media size value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*media_size = internal_handle->media_values->media_size;

	return( 1 );
}

/* Retrieves the media type value
 * Returns 1 if successful or -1 on error
 */
int libewf_get_media_type(
     libewf_handle_t *handle,
     uint8_t *media_type )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_media_type";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->media_values->media_type > (uint8_t) INT8_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid media type value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( media_type == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media type.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*media_type = internal_handle->media_values->media_type;

	return( 1 );
}

/* Retrieves the media flags
 * Returns 1 if successful or -1 on error
 */
int libewf_get_media_flags(
     libewf_handle_t *handle,
     uint8_t *media_flags )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_media_flags";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->media_values->media_flags > (uint8_t) INT8_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid media flags value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( media_flags == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid media flags.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*media_flags = internal_handle->media_values->media_flags;

	return( 1 );
}

/* Retrieves the volume type value
 * Returns 1 if successful or -1 on error
 */
int libewf_get_volume_type(
     libewf_handle_t *handle,
     uint8_t *volume_type )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_volume_type";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( volume_type == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume type.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->media_values->media_flags & 0x02 ) == 0 )
	{
		*volume_type = (int8_t) LIBEWF_VOLUME_TYPE_LOGICAL;
	}
	else
	{
		*volume_type = (int8_t) LIBEWF_VOLUME_TYPE_PHYSICAL;
	}
	return( 1 );
}

/* Retrieves the format type value
 * Returns 1 if successful or -1 on error
 */
int libewf_get_format(
     libewf_handle_t *handle,
     uint8_t *format )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_volume_type";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->format > (uint8_t) INT8_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid format value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( format == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid format.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*format = internal_handle->format;

	return( 1 );
}

/* Retrieves the GUID
 * Returns 1 if successful or -1 on error
 */
int libewf_get_guid(
     libewf_handle_t *handle,
     uint8_t *guid,
     size_t size )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_guid";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( guid == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid GUID.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( size < 16 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: GUID too small.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( memory_copy(
	     guid,
	     internal_handle->media_values->guid,
	     16 ) == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_MEMORY,
		 LIBEWF_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set GUID.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the MD5 hash
 * Returns 1 if successful, 0 if value not present or -1 on error
 */
int libewf_get_md5_hash(
     libewf_handle_t *handle,
     uint8_t *md5_hash,
     size_t size )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_md5_hash";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->hash_sections == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing hash sections.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->hash_sections->md5_hash_set == 0 )
	 && ( internal_handle->hash_values != NULL )
	 && ( libewf_hash_values_generate_md5_hash(
	       internal_handle->hash_values,
	       internal_handle->hash_sections->md5_hash,
	       EWF_DIGEST_HASH_SIZE_MD5,
	       &( internal_handle->hash_sections->md5_hash_set ),
	       &error ) != 1 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to parse MD5 hash value for its value.\n",
		 function );

		libewf_error_backtrace_notify(
		 error);
		libewf_error_free(
		 &error );

		return( -1 );
	}
	if( internal_handle->hash_sections->md5_hash_set == 0 )
	{
		return( 0 );
	}
	if( md5_hash == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid MD5 hash.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( size < EWF_DIGEST_HASH_SIZE_MD5 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: MD5 hash too small.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( memory_copy(
	     md5_hash,
	     internal_handle->hash_sections->md5_hash,
	     EWF_DIGEST_HASH_SIZE_MD5 ) == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_MEMORY,
		 LIBEWF_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set MD5 hash.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the segment filename
 * Returns 1 if successful, 0 if value not present or -1 on error
 */
int libewf_get_segment_filename(
     libewf_handle_t *handle,
     char *filename,
     size_t length )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_segment_filename";
	int result                                = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->segment_table == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing segment table.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	result = libewf_segment_table_get_basename(
	          internal_handle->segment_table,
	          filename,
	          length,
	          &error );

	if( result == -1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve segment table basename.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );
	}
	return( result );
}

#if defined( LIBEWF_WIDE_CHARACTER_TYPE )
#endif

/* Retrieves the delta segment filename
 * Returns 1 if successful, 0 if value not present or -1 on error
 */
int libewf_get_delta_segment_filename(
     libewf_handle_t *handle,
     system_character_t *filename,
     size_t length )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_delta_segment_filename";
	int result                                = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->delta_segment_table == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing delta segment table.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	result = libewf_segment_table_get_basename(
	          internal_handle->delta_segment_table,
	          filename,
	          length,
	          &error );

	if( result == -1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve segment table basename.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );
	}
	return( result );
}

/* Retrieves the amount of acquiry errors
 * Returns 1 if successful, 0 if no acquiry errors are present or -1 on error
 */
int libewf_get_amount_of_acquiry_errors(
     libewf_handle_t *handle,
     uint32_t *amount_of_errors )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_amount_of_acquiry_errors";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->acquiry_errors == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing acquiry errors.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( amount_of_errors == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid amount of errors.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*amount_of_errors = internal_handle->acquiry_errors->amount;

	return( 1 );
}

/* Retrieves the information of an acquiry error
 * Returns 1 if successful, 0 if no acquiry error could be found or -1 on error
 */
int libewf_get_acquiry_error(
     libewf_handle_t *handle,
     uint32_t index,
     off64_t *first_sector,
     uint32_t *amount_of_sectors )
{
	libewf_error_t *error = NULL;
	static char *function = "libewf_get_acquiry_error";
	int result            = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	result = libewf_sector_table_get_sector(
	          ( (libewf_internal_handle_t *) handle )->acquiry_errors,
	          index,
	          first_sector,
	          amount_of_sectors,
	          &error );

	if( result == -1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve acquiry error.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );
	}
	return( result );
}

/* Retrieves the amount of CRC errors
 * Returns 1 if successful or -1 on error
 */
int libewf_get_amount_of_crc_errors(
     libewf_handle_t *handle,
     uint32_t *amount_of_errors )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_amount_of_crc_errors";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->read == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing subhandle read.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->read->crc_errors == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - invalid subhandle read - missing crc errors.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( amount_of_errors == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid amount of errors.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*amount_of_errors = internal_handle->read->crc_errors->amount;

	return( 1 );
}

/* Retrieves the information of a CRC error
 * Returns 1 if successful, 0 if no CRC error could be found or -1 on error
 */
int libewf_get_crc_error(
     libewf_handle_t *handle,
     uint32_t index,
     off64_t *first_sector,
     uint32_t *amount_of_sectors )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_crc_error";
	int result                                = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->read == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing subhandle read.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	result = libewf_sector_table_get_sector(
	          internal_handle->read->crc_errors,
	          index,
	          first_sector,
	          amount_of_sectors,
	          &error );

	if( result == -1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve CRC error.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );
	}
	return( result );
}

/* Retrieves the amount of sessions
 * Returns 1 if successful or -1 on error
 */
int libewf_get_amount_of_sessions(
     libewf_handle_t *handle,
     uint32_t *amount_of_sessions )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_amount_of_sessions";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->sessions == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing sessions.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( amount_of_sessions == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid amount of sessions.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*amount_of_sessions = internal_handle->sessions->amount;

	return( 1 );
}

/* Retrieves the information of a session
 * Returns 1 if successful, 0 if no session could be found or -1 on error
 */
int libewf_get_session(
     libewf_handle_t *handle,
     uint32_t index,
     off64_t *first_sector,
     uint32_t *amount_of_sectors )
{
	libewf_error_t *error = NULL;
	static char *function = "libewf_get_session";
	int result            = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	result = libewf_sector_table_get_sector(
	          ( (libewf_internal_handle_t *) handle )->sessions,
	          index,
	          first_sector,
	          amount_of_sectors,
	          &error );

	if( result == -1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve session.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );
	}
	return( result );
}

/* Retrieves the amount of chunks written
 * Returns 1 if successful or -1 on error
 */
int libewf_get_write_amount_of_chunks(
     libewf_handle_t *handle,
     uint32_t *amount_of_chunks )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_write_amount_of_chunks";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->write == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing subhandle write.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( amount_of_chunks == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid amount of chunks.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*amount_of_chunks = internal_handle->write->amount_of_chunks;

	return( 1 );
}

/* Retrieves the header codepage
 * Returns 1 if successful or -1 on error
 */
int libewf_get_header_codepage(
     libewf_handle_t *handle,
     int *header_codepage )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_header_codepage";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->header_sections == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing header sections.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( header_codepage == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid header codepage.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	*header_codepage = internal_handle->header_sections->header_codepage;

	return( 1 );
}

/* Retrieves the amount of header values
 * Returns 1 if successful, 0 if no header values are present or -1 on error
 */
int libewf_get_amount_of_header_values(
     libewf_handle_t *handle,
     uint32_t *amount_of_values )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_amount_of_header_values";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( amount_of_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid amount of values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->header_values == NULL )
	{
		return( 0 );
	}
	*amount_of_values = internal_handle->header_values->amount_of_values;

	return( 1 );
}

/* Retrieves the header value identifier specified by its index
 * Returns 1 if successful, 0 if value not present or -1 on error
 */
int libewf_get_header_value_identifier(
     libewf_handle_t *handle,
     uint32_t index,
     libewf_character_t *value,
     size_t length )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_header_value_identifier";
	int result                                = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->header_values == NULL )
	{
		return( 0 );
	}
	result = libewf_values_table_get_identifier(
	          internal_handle->header_values,
	          index,
	          value,
	          length,
	          &error );

	if( result == -1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve header value identifier.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );
	}
	return( result );
}

/* Retrieves the header value specified by the identifier
 * Returns 1 if successful, 0 if value not present or -1 on error
 */
int libewf_get_header_value(
     libewf_handle_t *handle,
     libewf_character_t *identifier,
     libewf_character_t *value,
     size_t length )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_header_value";
	size_t identifier_length                  = 0;
	int result                                = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( identifier == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid indentifier.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( value == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid value.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->header_values == NULL )
	{
		return( 0 );
	}
	identifier_length = libewf_string_length(
	                     identifier );

	result = libewf_values_table_get_value(
	          internal_handle->header_values,
	          identifier,
	          identifier_length,
	          value,
	          length,
	          &error );

	if( result == -1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve header value.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );
	}
	return( result );
}

/* Retrieves the amount of hash values
 * Returns 1 if successful or -1 on error
 */
int libewf_get_amount_of_hash_values(
     libewf_handle_t *handle,
     uint32_t *amount_of_values )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_amount_of_hash_values";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( amount_of_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid amount of values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->hash_values == NULL )
	{
		return( 0 );
	}
	*amount_of_values = internal_handle->hash_values->amount_of_values;

	return( 1 );
}

/* Retrieves the hash value identifier specified by its index
 * Returns 1 if successful, 0 if value not present or -1 on error
 */
int libewf_get_hash_value_identifier(
     libewf_handle_t *handle,
     uint32_t index,
     libewf_character_t *value,
     size_t length )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_hash_value_identifier";
	int result                                = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->hash_values == NULL )
	{
		return( 0 );
	}
	result = libewf_values_table_get_identifier(
	          internal_handle->hash_values,
	          index,
	          value,
	          length,
	          &error );

	if( result == -1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve hash value identifier.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );
	}
	return( result );
}

/* Retrieves the hash value specified by the identifier
 * Returns 1 if successful, 0 if value not present or -1 on error
 */
int libewf_get_hash_value(
     libewf_handle_t *handle,
     libewf_character_t *identifier,
     libewf_character_t *value,
     size_t length )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_get_hash_value";
	size_t identifier_length                  = 0;
	int result                                = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( identifier == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid indentifier.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( value == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid value.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	identifier_length = libewf_string_length(
	                     identifier );

	if( ( internal_handle->hash_values == NULL )
	 && ( internal_handle->hash_sections != NULL )
	 && ( internal_handle->hash_sections->md5_hash_set != 0 )
	 && ( identifier_length == 3 )
	 && ( libewf_string_compare(
	       identifier,
	       _LIBEWF_STRING( "MD5" ),
	       identifier_length ) == 0 ) )
	{
		if( libewf_hash_values_parse_md5_hash(
		     &( internal_handle->hash_values ),
		     internal_handle->hash_sections->md5_hash,
		     EWF_DIGEST_HASH_SIZE_MD5,
		     &error ) != 1 )
		{
			libewf_error_set(
			 &error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to parse MD5 hash for its value.\n",
			 function );

			libewf_error_backtrace_notify(
			 error);
			libewf_error_free(
			 &error );

			return( -1 );
		}
	}
	if( internal_handle->hash_values == NULL )
	{
		return( 0 );
	}
	result = libewf_values_table_get_value(
                  internal_handle->hash_values,
	          identifier,
	          identifier_length,
	          value,
	          length,
	          &error );

	if( result == -1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve hash value identifier.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );
	}
	return( result );
}

/* Sets the amount of sectors per chunk in the media information
 * Returns 1 if successful or -1 on error
 */
int libewf_set_sectors_per_chunk(
     libewf_handle_t *handle,
     uint32_t sectors_per_chunk )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_sectors_per_chunk";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: sectors per chunk cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( libewf_internal_handle_set_media_values(
	     internal_handle,
	     sectors_per_chunk,
	     internal_handle->media_values->bytes_per_sector,
	     internal_handle->media_values->media_size,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set media values.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Sets the amount of bytes per sector in the media information
 * Returns 1 if successful or -1 on error
 */
int libewf_set_bytes_per_sector(
     libewf_handle_t *handle,
     uint32_t bytes_per_sector )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_bytes_per_sector";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->read != NULL )
	 || ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: bytes per sector cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( libewf_internal_handle_set_media_values(
	     internal_handle,
	     internal_handle->media_values->sectors_per_chunk,
	     bytes_per_sector,
	     internal_handle->media_values->media_size,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set media values.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Sets the error granularity
 * Returns 1 if successful or -1 on error
 */
int libewf_set_error_granularity(
     libewf_handle_t *handle,
     uint32_t error_granularity )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_error_granularity";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: error granularity cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle->media_values->error_granularity = error_granularity;

	return( 1 );
}

/* Sets the compression values
 * Returns 1 if successful or -1 on error
 */
int libewf_set_compression_values(
     libewf_handle_t *handle,
     int8_t compression_level,
     uint8_t compress_empty_block )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_compression_values";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: compression values cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( compression_level != EWF_COMPRESSION_NONE )
	 && ( compression_level != EWF_COMPRESSION_FAST )
	 && ( compression_level != EWF_COMPRESSION_BEST ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported compression level.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle->compression_level    = compression_level;
	internal_handle->compress_empty_block = 0;

	/* Compress empty block is only useful when no compression is used
	 */
	if( compression_level == EWF_COMPRESSION_NONE )
	{
		internal_handle->compress_empty_block = compress_empty_block;
	}
	return( 1 );
}

/* Sets the media size
 * Returns 1 if successful or -1 on error
 */
int libewf_set_media_size(
     libewf_handle_t *handle,
     size64_t media_size )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_media_size";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->read != NULL )
	 || ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: media size cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( libewf_internal_handle_set_media_values(
	     internal_handle,
	     internal_handle->media_values->sectors_per_chunk,
	     internal_handle->media_values->bytes_per_sector,
	     media_size,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set media values.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Sets the segment file size
 * Returns 1 if successful or -1 on error
 */
int libewf_set_segment_file_size(
     libewf_handle_t *handle,
     size64_t segment_file_size )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_segment_file_size";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( ( internal_handle->read != NULL )
	 || ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: segment file size cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( segment_file_size > (size64_t) INT64_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid segment file size value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( segment_file_size == 0 )
	 || ( segment_file_size > internal_handle->write->maximum_segment_file_size ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_VALUE_OUT_OF_RANGE,
		 "%s: invalid segment file size value out of range.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle->write->segment_file_size = segment_file_size;

	return( 1 );
}

/* Sets the delta segment file size
 * Returns 1 if successful or -1 on error
 */
int libewf_set_delta_segment_file_size(
     libewf_handle_t *handle,
     size64_t delta_segment_file_size )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_delta_segment_file_size";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: delta segment file size cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( delta_segment_file_size > (size64_t) INT64_MAX )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid delta segment file size value exceeds maximum.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( delta_segment_file_size == 0 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_VALUE_OUT_OF_RANGE,
		 "%s: invalid delta segment file size value out of range.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle->write->delta_segment_file_size = delta_segment_file_size;

	return( 1 );
}

/* Sets the media type
 * Returns 1 if successful or -1 on error
 */
int libewf_set_media_type(
     libewf_handle_t *handle,
     uint8_t media_type )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_media_type";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->read != NULL )
	 || ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: media type cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle->media_values->media_type = media_type;

	return( 1 );
}

/* Sets the volume type
 * Returns 1 if successful or -1 on error
 */
int libewf_set_volume_type(
     libewf_handle_t *handle,
     uint8_t volume_type )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_volume_type";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->read != NULL )
	 || ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: volume type cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( volume_type == LIBEWF_VOLUME_TYPE_LOGICAL )
	{
		/* Uses 1-complement of EWF_MEDIA_FLAGS_IS_PHYSICAL
		 */
		internal_handle->media_values->media_flags &= ~EWF_MEDIA_FLAGS_IS_PHYSICAL;
	}
	else if( volume_type == LIBEWF_VOLUME_TYPE_PHYSICAL )
	{
		internal_handle->media_values->media_flags |= EWF_MEDIA_FLAGS_IS_PHYSICAL;
	}
	else
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported volume type.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	return( 1 );
}

/* Sets the output format
 * Returns 1 if successful or -1 on error
 */
int libewf_set_format(
     libewf_handle_t *handle,
     uint8_t format )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_format";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( ( internal_handle->read != NULL )
	 || ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: format cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( libewf_internal_handle_set_format(
	     internal_handle,
	     format,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set format.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Sets the GUID
 * Returns 1 if successful or -1 on error
 */
int libewf_set_guid(
     libewf_handle_t *handle,
     uint8_t *guid,
     size_t size )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_guid";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( guid == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid GUID.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( size < 16 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: GUID too small.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->read != NULL )
	 || ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: GUID cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( memory_copy(
	     internal_handle->media_values->guid,
	     guid,
	     16 ) == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_MEMORY,
		 LIBEWF_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set GUID.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	return( 1 );
}

/* Sets the MD5 hash
 * Returns 1 if successful or -1 on error
 */
int libewf_set_md5_hash(
     libewf_handle_t *handle,
     uint8_t *md5_hash,
     size_t size )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_md5_hash";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->hash_sections == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing hash sections.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->read != NULL )
	 || ( internal_handle->hash_sections->md5_hash_set ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: md5 hash cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( md5_hash == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid MD5 hash.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( size < EWF_DIGEST_HASH_SIZE_MD5 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: MD5 hash too small.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( memory_copy(
	     internal_handle->hash_sections->md5_hash,
	     md5_hash,
	     EWF_DIGEST_HASH_SIZE_MD5 ) == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_MEMORY,
		 LIBEWF_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set MD5 hash.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( libewf_hash_values_parse_md5_hash(
	     &( internal_handle->hash_values ),
	     md5_hash,
	     EWF_DIGEST_HASH_SIZE_MD5,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to parse MD5 hash for its value.\n",
		 function );

		libewf_error_backtrace_notify(
		 error);
		libewf_error_free(
		 &error );

		return( -1 );
	}
	internal_handle->hash_sections->md5_hash_set = 1;

	return( 1 );
}

/* Sets the segment file
 * Returns 1 if successful or -1 on error
 */
int libewf_set_segment_filename(
     libewf_handle_t *handle,
     system_character_t *filename,
     size_t length )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_segment_filename";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->write == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: segment filename cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing segment table.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( libewf_segment_table_set_basename(
	     internal_handle->segment_table,
	     filename,
	     length,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set segment table basename.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Sets the delta segment file
 * Returns 1 if successful or -1 on error
 */
int libewf_set_delta_segment_filename(
     libewf_handle_t *handle,
     system_character_t *filename,
     size_t length )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_delta_segment_filename";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->write == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: delta segment filename cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->delta_segment_table == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing delta segment table.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( libewf_segment_table_set_basename(
	     internal_handle->delta_segment_table,
	     filename,
	     length,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set segment table basename.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Sets the read wipe chunk on error
 * The chunk is not wiped if read raw is used
 * Returns 1 if successful or -1 on error
 */
int libewf_set_read_wipe_chunk_on_error(
     libewf_handle_t *handle,
     uint8_t wipe_on_error )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_read_wipe_chunk_on_error";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->read == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing subhandle read.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle->read->wipe_on_error = wipe_on_error;

	return( 1 );
}

/* Sets the header codepage
 * Returns 1 if successful or -1 on error
 */
int libewf_set_header_codepage(
     libewf_handle_t *handle,
     int header_codepage )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_header_codepage";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->header_sections == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing header sections.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( header_codepage != LIBEWF_CODEPAGE_ASCII )
	 || ( header_codepage != LIBEWF_CODEPAGE_WINDOWS_1250 )
	 || ( header_codepage != LIBEWF_CODEPAGE_WINDOWS_1251 )
	 || ( header_codepage != LIBEWF_CODEPAGE_WINDOWS_1252 )
	 || ( header_codepage != LIBEWF_CODEPAGE_WINDOWS_1253 )
	 || ( header_codepage != LIBEWF_CODEPAGE_WINDOWS_1254 )
	 || ( header_codepage != LIBEWF_CODEPAGE_WINDOWS_1256 )
	 || ( header_codepage != LIBEWF_CODEPAGE_WINDOWS_1257 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported header codepage.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle->header_sections->header_codepage = header_codepage;

	return( 1 );
}

/* Sets the header value specified by the identifier
 * Returns 1 if successful or -1 on error
 */
int libewf_set_header_value(
     libewf_handle_t *handle,
     libewf_character_t *identifier,
     libewf_character_t *value,
     size_t length )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_header_value";
	size_t identifier_length                  = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( ( internal_handle->read != NULL )
	 || ( internal_handle->write == NULL )
	 || ( internal_handle->write->values_initialized != 0 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: header value cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( identifier == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid identifier.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->header_values == NULL )
	{
		if( libewf_values_table_initialize(
		     &( internal_handle->header_values ),
		     LIBEWF_HEADER_VALUES_DEFAULT_AMOUNT,
		     &error ) != 1 )
		{
			libewf_error_set(
			 &error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create header values.\n",
			 function );

			libewf_error_backtrace_notify(
			 error );
			libewf_error_free(
			 &error );

			return( -1 );
		}
		if( libewf_header_values_initialize(
		     internal_handle->header_values,
		     &error ) != 1 )
		{
			libewf_error_set(
			 &error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to initialize header values.\n",
			 function );

			libewf_error_backtrace_notify(
			 error );
			libewf_error_free(
			 &error );

			return( -1 );
		}
	}
	identifier_length = libewf_string_length(
	                     identifier );

	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     identifier,
	     identifier_length,
	     value,
	     length,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set header value.\n",
		 function );

		libewf_error_backtrace_notify(
		 error );
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Sets the hash value specified by the identifier
 * Returns 1 if successful or -1 on error
 */
int libewf_set_hash_value(
     libewf_handle_t *handle,
     libewf_character_t *identifier,
     libewf_character_t *value,
     size_t length )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_set_hash_value";
	size_t identifier_length                  = 0;

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->read != NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: hash value cannot be changed.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( identifier == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid identifier.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->hash_values == NULL )
	{
		if( libewf_values_table_initialize(
		     &( internal_handle->hash_values ),
		     LIBEWF_HASH_VALUES_DEFAULT_AMOUNT,
		     &error ) != 1 )
		{
			libewf_error_set(
			 &error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create hash values.\n",
			 function );

			libewf_error_backtrace_notify(
			 error);
			libewf_error_free(
			 &error );

			return( -1 );
		}
		if( libewf_hash_values_initialize(
		     internal_handle->hash_values,
		     &error ) != 1 )
		{
			libewf_error_set(
			 &error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to initialize hash values.\n",
			 function );

			libewf_error_backtrace_notify(
			 error);
			libewf_error_free(
			 &error );

			return( -1 );
		}
	}
	identifier_length = libewf_string_length(
	                     identifier );

	if( libewf_values_table_set_value(
	     internal_handle->hash_values,
	     identifier,
	     identifier_length,
	     value,
	     length,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set hash value.\n",
		 function );

		libewf_error_backtrace_notify(
		 error);
		libewf_error_free(
		 &error );

		return( -1 );
	}
	if( ( internal_handle->hash_sections != NULL )
	 && ( identifier_length == 3 )
	 && ( libewf_string_compare(
	       identifier,
	       _LIBEWF_STRING( "MD5" ),
	       identifier_length ) == 0 ) )
	{
		if( libewf_hash_values_generate_md5_hash(
		     internal_handle->hash_values,
		     internal_handle->hash_sections->md5_hash,
		     EWF_DIGEST_HASH_SIZE_MD5,
		     &( internal_handle->hash_sections->md5_hash_set ),
		     &error ) != 1 )
		{
			libewf_error_set(
			 &error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to parse MD5 hash value for its value.\n",
			 function );

			libewf_error_backtrace_notify(
			 error);
			libewf_error_free(
			 &error );

			return( -1 );
		}
	}
	return( 1 );
}

/* Parses the header values from the xheader, header2 or header section
 * Will parse the first available header in order mentioned above
 * Returns 1 if successful, 0 if already parsed or -1 on error
 */
int libewf_parse_header_values(
     libewf_handle_t *handle,
     uint8_t date_format )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_parse_header_values";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->header_sections == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing header sections.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->header_values != NULL )
	{
		return( 0 );
	}
	if( ( internal_handle->header_sections->xheader != NULL )
	 && ( libewf_header_values_parse_xheader(
	       &( internal_handle->header_values ),
	       internal_handle->header_sections->xheader,
	       internal_handle->header_sections->xheader_size,
	       date_format,
	       &error ) != 1 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to parse xheader.\n",
		 function );

		/* TODO error_tollerance */

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->header_values == NULL )
	 && ( internal_handle->header_sections->header2 != NULL )
	 && ( libewf_header_values_parse_header2(
	       &( internal_handle->header_values ),
	       internal_handle->header_sections->header2,
	       internal_handle->header_sections->header2_size,
	       date_format,
	       &error ) != 1 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to parse header2.\n",
		 function );

		/* TODO error_tollerance */

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( ( internal_handle->header_values == NULL )
	 && ( internal_handle->header_sections->header != NULL )
	 && ( libewf_header_values_parse_header(
	       &( internal_handle->header_values ),
	       internal_handle->header_sections->header,
	       internal_handle->header_sections->header_size,
	       internal_handle->header_sections->header_codepage,
	       date_format,
	       &error ) != 1 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to parse header.\n",
		 function );

		/* TODO error_tollerance */

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->header_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to parse header(s) for values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	/* The EnCase2 and EnCase3 format are the same
	 * only the acquiry software version provides insight in which version of EnCase was used
	 */
	if( ( internal_handle->format == LIBEWF_FORMAT_ENCASE2 )
	 && ( internal_handle->header_values->amount_of_values > LIBEWF_HEADER_VALUES_DEFAULT_AMOUNT )
	 && ( internal_handle->header_values->value != NULL )
	 && ( internal_handle->header_values->value[ LIBEWF_HEADER_VALUES_INDEX_ACQUIRY_SOFTWARE_VERSION ] != NULL )
	 && ( internal_handle->header_values->value[ LIBEWF_HEADER_VALUES_INDEX_ACQUIRY_SOFTWARE_VERSION ][ 0 ] == '3' ) )
 	{
		internal_handle->format = LIBEWF_FORMAT_ENCASE3;
	}
	return( 1 );
}

/* Parses the hash values from the xhash section
 * Returns 1 if successful, 0 if already parsed or -1 on error
 */
int libewf_parse_hash_values(
     libewf_handle_t *handle )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_parse_hash_values";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->hash_sections == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing hash sections.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_handle->hash_values != NULL )
	{
		return( 0 );
	}
	if( ( internal_handle->hash_sections->xhash != NULL )
	 && ( libewf_hash_values_parse_xhash(
	       &( internal_handle->hash_values ),
	       internal_handle->hash_sections->xhash,
	       internal_handle->hash_sections->xhash_size,
	       &error ) != 1 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to parse xhash for values.\n",
		 function );

		/* TODO error_tollerance */

		libewf_error_backtrace_notify(
		 error);
		libewf_error_free(
		 &error );

		return( -1 );
	}
	else if( ( internal_handle->hash_sections->md5_hash_set != 0 )
	      && ( libewf_hash_values_parse_md5_hash(
	            &( internal_handle->hash_values ),
	            internal_handle->hash_sections->md5_hash,
	            EWF_DIGEST_HASH_SIZE_MD5,
	            &error ) != 1 ) )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to parse MD5 hash for its value.\n",
		 function );

		/* TODO error_tollerance */

		libewf_error_backtrace_notify(
		 error);
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Add an acquiry error
 * Returns 1 if successful or -1 on error
 */
int libewf_add_acquiry_error(
     libewf_handle_t *handle,
     off64_t first_sector,
     uint32_t amount_of_sectors )
{
	libewf_error_t *error = NULL;
	static char *function = "libewf_add_acquiry_error";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( libewf_sector_table_add_sector(
	     ( (libewf_internal_handle_t *) handle )->acquiry_errors,
	     first_sector,
	     amount_of_sectors,
	     1,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to add acquiry error.\n",
		 function );

		libewf_error_backtrace_notify(
		 error);
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Add a CRC error
 * Returns 1 if successful or -1 on error
 */
int libewf_add_crc_error(
     libewf_handle_t *handle,
     off64_t first_sector,
     uint32_t amount_of_sectors )
{
	libewf_error_t *error                     = NULL;
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_add_crc_error";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_handle = (libewf_internal_handle_t *) handle;

	if( internal_handle->read == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing subhandle read.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( libewf_sector_table_add_sector(
	     internal_handle->read->crc_errors,
	     first_sector,
	     amount_of_sectors,
	     1,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to add CRC error.\n",
		 function );

		libewf_error_backtrace_notify(
		 error);
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Add a session
 * Returns 1 if successful or -1 on error
 */
int libewf_add_session(
     libewf_handle_t *handle,
     off64_t first_sector,
     uint32_t amount_of_sectors )
{
	libewf_error_t *error = NULL;
	static char *function = "libewf_add_session";

	if( handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( libewf_sector_table_add_sector(
	     ( (libewf_internal_handle_t *) handle )->sessions,
	     first_sector,
	     amount_of_sectors,
	     0,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to add session.\n",
		 function );

		libewf_error_backtrace_notify(
		 error);
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Copies the header values from the source to the destination handle
 * Returns 1 if successful or -1 on error
 */
int libewf_copy_header_values(
     libewf_handle_t *destination_handle,
     libewf_handle_t *source_handle )
{
	libewf_error_t *error                                 = NULL;
	libewf_internal_handle_t *internal_destination_handle = NULL;
	libewf_internal_handle_t *internal_source_handle      = NULL;
	static char *function                                 = "libewf_copy_header_values";

	if( destination_handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid destination handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( source_handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid source handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_destination_handle = (libewf_internal_handle_t *) destination_handle;
	internal_source_handle      = (libewf_internal_handle_t *) source_handle;

	if( internal_source_handle->header_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid source handle - missing header values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_destination_handle->header_values == NULL )
	{
		if( libewf_values_table_initialize(
		     &( internal_destination_handle->header_values ),
		     LIBEWF_HEADER_VALUES_DEFAULT_AMOUNT,
		     &error ) != 1 )
		{
			libewf_error_set(
			 &error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create header values in destination handle.\n",
			 function );

			libewf_error_backtrace_notify(
			 error);
			libewf_error_free(
			 &error );

			return( -1 );
		}
		if( libewf_header_values_initialize(
		     internal_destination_handle->header_values,
		     &error ) != 1 )
		{
			libewf_error_set(
			 &error,
			 LIBEWF_ERROR_DOMAIN_RUNTIME,
			 LIBEWF_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to initialize header values.\n",
			 function );

			libewf_error_backtrace_notify(
			 error);
			libewf_error_free(
			 &error );

			return( -1 );
		}
	}
	if( libewf_header_values_copy(
	     internal_destination_handle->header_values,
	     internal_source_handle->header_values,
	     &error ) != 1 )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy header values.\n",
		 function );

		libewf_error_backtrace_notify(
		 error);
		libewf_error_free(
		 &error );

		return( -1 );
	}
	return( 1 );
}

/* Copies the media values from the source to the destination handle
 * Returns 1 if successful or -1 on error
 */
int libewf_copy_media_values(
     libewf_handle_t *destination_handle,
     libewf_handle_t *source_handle )
{
	libewf_error_t *error                                 = NULL;
	libewf_internal_handle_t *internal_destination_handle = NULL;
	libewf_internal_handle_t *internal_source_handle      = NULL;
	static char *function                                 = "libewf_copy_media_values";

	if( destination_handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid destination handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( source_handle == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_ARGUMENTS,
		 LIBEWF_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid source handle.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	internal_destination_handle = (libewf_internal_handle_t *) destination_handle;
	internal_source_handle      = (libewf_internal_handle_t *) source_handle;

	if( internal_source_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid source handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( internal_destination_handle->media_values == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_RUNTIME,
		 LIBEWF_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid destination handle - missing media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	if( memory_copy(
	     internal_destination_handle->media_values,
	     internal_source_handle->media_values,
	     sizeof( libewf_media_values_t ) ) == NULL )
	{
		libewf_error_set(
		 &error,
		 LIBEWF_ERROR_DOMAIN_MEMORY,
		 LIBEWF_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to copy media values.\n",
		 function );

		libewf_error_backtrace_notify(
		error );
		libewf_error_free(
		&error );

		return( -1 );
	}
	return( 1 );
}

