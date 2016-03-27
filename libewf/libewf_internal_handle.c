/*
 * libewf handle
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

#include "libewf_includes.h"

#include <time.h>

#include <libewf/libewf_definitions.h>

#include "libewf_common.h"
#include "libewf_header_values.h"
#include "libewf_internal_handle.h"
#include "libewf_notify.h"
#include "libewf_string.h"

#include "ewf_crc.h"
#include "ewf_data.h"
#include "ewf_definitions.h"
#include "ewf_file_header.h"

/* Allocates memory for a new handle struct
 * Returns a pointer to the new instance, NULL on error
 */
libewf_internal_handle_t *libewf_internal_handle_alloc( uint8_t flags )
{
	libewf_internal_handle_t *internal_handle = NULL;
	static char *function                     = "libewf_internal_handle_alloc";

	internal_handle = (libewf_internal_handle_t *) libewf_common_alloc(
	                                                sizeof( libewf_internal_handle_t ) );

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate handle.\n",
		 function );

		return( NULL );
	}
	internal_handle->read                     = NULL;
	internal_handle->write                    = NULL;
	internal_handle->media_values             = NULL;
	internal_handle->segment_table            = NULL;
	internal_handle->delta_segment_table      = NULL;
	internal_handle->offset_table             = NULL;
	internal_handle->secondary_offset_table   = NULL;
	internal_handle->chunk_cache              = NULL;
	internal_handle->header_sections          = NULL;
	internal_handle->hash_sections            = NULL;
	internal_handle->header_values            = NULL;
	internal_handle->hash_values              = NULL;
	internal_handle->acquiry_errors           = NULL;
	internal_handle->current_chunk            = 0;
	internal_handle->current_chunk_offset     = 0;
	internal_handle->compression_level        = EWF_COMPRESSION_UNKNOWN;
	internal_handle->compress_empty_block     = 0;
	internal_handle->format                   = LIBEWF_FORMAT_UNKNOWN;
	internal_handle->ewf_format               = EWF_FORMAT_UNKNOWN;
	internal_handle->error_tollerance         = LIBEWF_ERROR_TOLLERANCE_COMPENSATE;

	/* The segment table is initially filled with a single entry
	 */
	internal_handle->segment_table = libewf_segment_table_alloc( 1 );

	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create segment table.\n",
		 function );

		libewf_common_free( internal_handle );

		return( NULL );
	}
	/* The delta segment table is initially filled with a single entry
	 */
	internal_handle->delta_segment_table = libewf_segment_table_alloc( 1 );

	if( internal_handle->delta_segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create delta segment table.\n",
		 function );

		libewf_segment_table_free( internal_handle->segment_table );
		libewf_common_free( internal_handle );

		return( NULL );
	}
	internal_handle->offset_table = libewf_offset_table_alloc( 0 );

	if( internal_handle->offset_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create offset table.\n",
		 function );

		libewf_segment_table_free( internal_handle->segment_table );
		libewf_segment_table_free( internal_handle->delta_segment_table );
		libewf_common_free( internal_handle );

		return( NULL );
	}
	internal_handle->secondary_offset_table = libewf_offset_table_alloc( 0 );

	if( internal_handle->secondary_offset_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create secondary offset table.\n",
		 function );

		libewf_offset_table_free( internal_handle->offset_table );
		libewf_segment_table_free( internal_handle->delta_segment_table );
		libewf_segment_table_free( internal_handle->segment_table );
		libewf_common_free( internal_handle );

		return( NULL );
	}
	internal_handle->chunk_cache = libewf_chunk_cache_alloc( EWF_MINIMUM_CHUNK_SIZE + sizeof( ewf_crc_t ) );

	if( internal_handle->chunk_cache == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create chunk cache.\n",
		 function );

		libewf_offset_table_free( internal_handle->secondary_offset_table );
		libewf_offset_table_free( internal_handle->offset_table );
		libewf_segment_table_free( internal_handle->delta_segment_table );
		libewf_segment_table_free( internal_handle->segment_table );
		libewf_common_free( internal_handle );

		return( NULL );
	}
	internal_handle->media_values = libewf_media_values_alloc();

	if( internal_handle->media_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create media values.\n",
		 function );

		libewf_chunk_cache_free( internal_handle->chunk_cache );
		libewf_offset_table_free( internal_handle->secondary_offset_table );
		libewf_offset_table_free( internal_handle->offset_table );
		libewf_segment_table_free( internal_handle->delta_segment_table );
		libewf_segment_table_free( internal_handle->segment_table );
		libewf_common_free( internal_handle );

		return( NULL );
	}
	internal_handle->header_sections = libewf_header_sections_alloc();

	if( internal_handle->header_sections == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create header sections.\n",
		 function );

		libewf_media_values_free( internal_handle->media_values );
		libewf_chunk_cache_free( internal_handle->chunk_cache );
		libewf_offset_table_free( internal_handle->secondary_offset_table );
		libewf_offset_table_free( internal_handle->offset_table );
		libewf_segment_table_free( internal_handle->delta_segment_table );
		libewf_segment_table_free( internal_handle->segment_table );
		libewf_common_free( internal_handle );

		return( NULL );
	}
	internal_handle->hash_sections = libewf_hash_sections_alloc();

	if( internal_handle->hash_sections == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create hash sections.\n",
		 function );

		libewf_header_sections_free( internal_handle->header_sections );
		libewf_media_values_free( internal_handle->media_values );
		libewf_chunk_cache_free( internal_handle->chunk_cache );
		libewf_offset_table_free( internal_handle->secondary_offset_table );
		libewf_offset_table_free( internal_handle->offset_table );
		libewf_segment_table_free( internal_handle->delta_segment_table );
		libewf_segment_table_free( internal_handle->segment_table );
		libewf_common_free( internal_handle );

		return( NULL );
	}
	internal_handle->acquiry_errors = libewf_sector_table_alloc( 0 );

	if( internal_handle->acquiry_errors == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create acquiry errors.\n",
		 function );

		libewf_hash_sections_free( internal_handle->hash_sections );
		libewf_header_sections_free( internal_handle->header_sections );
		libewf_media_values_free( internal_handle->media_values );
		libewf_chunk_cache_free( internal_handle->chunk_cache );
		libewf_offset_table_free( internal_handle->secondary_offset_table );
		libewf_offset_table_free( internal_handle->offset_table );
		libewf_segment_table_free( internal_handle->delta_segment_table );
		libewf_segment_table_free( internal_handle->segment_table );
		libewf_common_free( internal_handle );

		return( NULL );
	}
	if( ( flags & LIBEWF_FLAG_READ ) == LIBEWF_FLAG_READ )
	{
		internal_handle->read = libewf_internal_handle_read_alloc();

		if( internal_handle->read == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to create subhandle read.\n",
			 function );

			libewf_sector_table_free( internal_handle->acquiry_errors );
			libewf_hash_sections_free( internal_handle->hash_sections );
			libewf_header_sections_free( internal_handle->header_sections );
			libewf_media_values_free( internal_handle->media_values );
			libewf_chunk_cache_free( internal_handle->chunk_cache );
			libewf_offset_table_free( internal_handle->secondary_offset_table );
			libewf_offset_table_free( internal_handle->offset_table );
			libewf_segment_table_free( internal_handle->delta_segment_table );
			libewf_segment_table_free( internal_handle->segment_table );
			libewf_common_free( internal_handle );

			return( NULL );
		}
	}
	if( ( flags & LIBEWF_FLAG_WRITE ) == LIBEWF_FLAG_WRITE )
	{
		internal_handle->write = libewf_internal_handle_write_alloc();

		if( internal_handle->write == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to create subhandle write.\n",
			 function );

			if( internal_handle->read != NULL )
			{
				libewf_internal_handle_read_free( internal_handle->read );
			}
			libewf_sector_table_free( internal_handle->acquiry_errors );
			libewf_hash_sections_free( internal_handle->hash_sections );
			libewf_header_sections_free( internal_handle->header_sections );
			libewf_media_values_free( internal_handle->media_values );
			libewf_chunk_cache_free( internal_handle->chunk_cache );
			libewf_offset_table_free( internal_handle->secondary_offset_table );
			libewf_offset_table_free( internal_handle->offset_table );
			libewf_segment_table_free( internal_handle->delta_segment_table );
			libewf_segment_table_free( internal_handle->segment_table );
			libewf_common_free( internal_handle );

			return( NULL );
		}
	}
	return( internal_handle );
}

/* Frees memory of a handle struct including elements
 */
void libewf_internal_handle_free( libewf_internal_handle_t *internal_handle )
{
	static char *function = "libewf_internal_handle_free";

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return;
	}
	if( internal_handle->media_values != NULL )
	{
		libewf_media_values_free( internal_handle->media_values );
	}
	if( internal_handle->read != NULL )
	{
		libewf_internal_handle_read_free( internal_handle->read );
	}
	if( internal_handle->write != NULL )
	{
		libewf_internal_handle_write_free( internal_handle->write );
	}
	if( internal_handle->segment_table != NULL )
	{
		libewf_segment_table_free( internal_handle->segment_table );
	}
	if( internal_handle->delta_segment_table != NULL )
	{
		libewf_segment_table_free( internal_handle->delta_segment_table );
	}
	if( internal_handle->offset_table != NULL )
	{
		libewf_offset_table_free( internal_handle->offset_table );
	}
	if( internal_handle->secondary_offset_table != NULL )
	{
		libewf_offset_table_free( internal_handle->secondary_offset_table );
	}
	if( internal_handle->acquiry_errors != NULL )
	{
		libewf_sector_table_free( internal_handle->acquiry_errors );
	}
	if( internal_handle->header_sections != NULL )
	{
		libewf_header_sections_free( internal_handle->header_sections );
	}
	if( internal_handle->hash_sections != NULL )
	{
		libewf_hash_sections_free( internal_handle->hash_sections );
	}
	if( internal_handle->header_values != NULL )
	{
		libewf_values_table_free( internal_handle->header_values );
	}
	if( internal_handle->hash_values != NULL )
	{
		libewf_values_table_free( internal_handle->hash_values );
	}
	if( internal_handle->chunk_cache != NULL )
	{
		libewf_chunk_cache_free( internal_handle->chunk_cache );
	}
	libewf_common_free( internal_handle );
}

/* Allocates memory for a new handle read struct
 * Returns a pointer to the new instance, NULL on error
 */
libewf_internal_handle_read_t *libewf_internal_handle_read_alloc( void )
{
	libewf_internal_handle_read_t *handle_read = NULL;
	static char *function                      = "libewf_internal_handle_read_alloc";

	handle_read = (libewf_internal_handle_read_t *) libewf_common_alloc(
	                                                 sizeof( libewf_internal_handle_read_t ) );

	if( handle_read == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate handle read.\n",
		 function );

		return( NULL );
	}
	handle_read->crc_errors = libewf_sector_table_alloc( 0 );

	if( handle_read->crc_errors == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate crc errors.\n",
		 function );

		libewf_common_free( handle_read );

		return( NULL );
	}
	handle_read->values_initialized = 0;
	handle_read->wipe_on_error      = 1;

	return( handle_read );
}

/* Frees memory of a handle read struct including elements
 */
void libewf_internal_handle_read_free( libewf_internal_handle_read_t *handle_read )
{
	static char *function = "libewf_internal_handle_read_free";

	if( handle_read == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle read.\n",
		 function );

		return;
	}
	if( handle_read->crc_errors != NULL )
	{
		libewf_sector_table_free( handle_read->crc_errors );
	}
	libewf_common_free( handle_read );
}

/* Allocates memory for a new handle write struct
 * Returns a pointer to the new instance, NULL on error
 */
libewf_internal_handle_write_t *libewf_internal_handle_write_alloc( void )
{
	libewf_internal_handle_write_t *handle_write = NULL;
	static char *function                        = "libewf_internal_handle_write_alloc";

	handle_write = (libewf_internal_handle_write_t *) libewf_common_alloc(
	                                                   sizeof( libewf_internal_handle_write_t ) );

	if( handle_write == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate handle write.\n",
		 function );

		return( NULL );
	}
	handle_write->data_section                     = NULL;
	handle_write->input_write_count                = 0;
	handle_write->write_count                      = 0;
	handle_write->maximum_segment_file_size        = 0;
	handle_write->segment_file_size                = 0;
	handle_write->maximum_amount_of_segments       = 0;
	handle_write->chunks_section_write_count       = 0;
	handle_write->amount_of_chunks                 = 0;
	handle_write->chunks_per_segment               = 0;
	handle_write->chunks_per_chunks_section        = 0;
	handle_write->segment_amount_of_chunks         = 0;
	handle_write->maximum_section_amount_of_chunks = EWF_MAXIMUM_OFFSETS_IN_TABLE;
	handle_write->section_amount_of_chunks         = 0;
	handle_write->chunks_section_offset            = 0;
	handle_write->chunks_section_number            = 0;
	handle_write->unrestrict_offset_amount         = 0;
	handle_write->values_initialized               = 0;
	handle_write->create_chunks_section            = 0;
	handle_write->write_finalized                  = 0;

	return( handle_write );
}

/* Frees memory of a handle write struct including elements
 */
void libewf_internal_handle_write_free( libewf_internal_handle_write_t *handle_write )
{
	static char *function = "libewf_internal_handle_write_free";

	if( handle_write == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle write.\n",
		 function );

		return;
	}
	if( handle_write->data_section != NULL )
	{
		libewf_common_free( handle_write->data_section );
	}
	libewf_common_free( handle_write );
}

/* Returns the maximum amount of supported segment files to write, or -1 on error
 */
int16_t libewf_internal_handle_get_write_maximum_amount_of_segments( libewf_internal_handle_t *internal_handle )
{
	static char *function = "libewf_internal_handle_get_write_maximum_amount_of_segments";

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->ewf_format == EWF_FORMAT_S01 )
	{
		/* = 4831
		 */
		return( (int16_t) ( ( (int) ( 'z' - 's' ) * 26 * 26 ) + 99 ) );
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_E01 )
	{
		/* = 14295
		 */
		return( (int16_t) ( ( (int) ( 'Z' - 'E' ) * 26 * 26 ) + 99 ) );
	}
	else
	{
		LIBEWF_WARNING_PRINT( "%s: unsupported EWF format.\n",
		 function );

		return( -1 );
	}
}

/* Determines the EWF file format based on known characteristics
 * Returns 1 if the format was determined, -1 on errror
 */
int libewf_internal_handle_determine_format( libewf_internal_handle_t *internal_handle, libewf_header_sections_t *header_sections )
{
	static char *function = "libewf_internal_handle_determine_format";

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( header_sections == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid header sections.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->ewf_format == EWF_FORMAT_S01 )
	{
		/* The format identifier for the EWF-S01 format was already set
		 * while reading the volume section
		 */
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_E01 )
	{
		if( header_sections->xheader != NULL )
		{
			internal_handle->format = LIBEWF_FORMAT_EWFX;
		}
		/* The header2 in raw format starts with 0xff 0xfe <number>
		 */
		else if( header_sections->header2 != NULL )
		{
			if( header_sections->header2[ 2 ] == (ewf_char_t) '3' )
			{
				/* The EnCase5 header2 contains av on the 6th position (36 ... 38 ...)
				 * the header2 is an UTF16 string
				 */
				if( ( header_sections->header2[ 36 ] == (ewf_char_t) 'a' )
				 && ( header_sections->header2[ 38 ] == (ewf_char_t) 'v' ) )
				{
					internal_handle->format = LIBEWF_FORMAT_ENCASE5;
				}
				else if( ( header_sections->header2[ 36 ] == (ewf_char_t) 'm' )
				 && ( header_sections->header2[ 38 ] == (ewf_char_t) 'd' ) )
				{
					internal_handle->format = LIBEWF_FORMAT_ENCASE6;
				}
				else
				{
					LIBEWF_WARNING_PRINT( "%s: unsupported header2 format: %c%c.\n",
					 function, (char) header_sections->header2[ 36 ],
					 (char) header_sections->header2[ 38 ] );

					return( -1 );
				}
			}
			else if( header_sections->header2[ 2 ] == (ewf_char_t) '1' )
			{
				internal_handle->format = LIBEWF_FORMAT_ENCASE4;
			}
			else
			{
				LIBEWF_WARNING_PRINT( "%s: unsupported header2 version: %c.\n",
				 function, (char) header_sections->header2[ 2 ] );

				return( -1 );
			}
		}
		else if( header_sections->header != NULL )
		{
			if( header_sections->header[ 0 ] == (ewf_char_t) '3' )
			{
				/* The linen5 header2 contains av on the 6th position (17 18)
				 * the header2 is an UTF16 string
				 */
				if( ( header_sections->header[ 17 ] == (ewf_char_t) 'a' )
				 && ( header_sections->header[ 18 ] == (ewf_char_t) 'v' ) )
				{
					internal_handle->format = LIBEWF_FORMAT_LINEN5;
				}
				else if( ( header_sections->header[ 17 ] == (ewf_char_t) 'm' )
				 && ( header_sections->header[ 18 ] == (ewf_char_t) 'd' ) )
				{
					internal_handle->format = LIBEWF_FORMAT_LINEN6;
				}
				else
				{
					LIBEWF_WARNING_PRINT( "%s: unsupported header format: %c%c.\n",
					 function, (char) header_sections->header[ 17 ],
					 (char) header_sections->header[ 18 ] );

					return( -1 );
				}
			}
			else if( header_sections->header[ 0 ] == (ewf_char_t) '1' )
			{
				/* EnCase uses \r\n
				 */
				if( header_sections->header[ 1 ] == (ewf_char_t) '\r' )
				{
					if( header_sections->header[ 25 ] == (ewf_char_t) 'r' )
					{
						internal_handle->format = LIBEWF_FORMAT_ENCASE1;

						if( header_sections->amount_of_header_sections != 1 )
						{
							LIBEWF_VERBOSE_PRINT( "%s: multiple header sections found.\n",
							 function );
						}
					}
					else if( header_sections->header[ 31 ] == (ewf_char_t) 'r' )
					{
						internal_handle->format = LIBEWF_FORMAT_ENCASE2;
					}
					else
					{
						LIBEWF_WARNING_PRINT( "%s: unsupported header version.\n",
						 function );

						return( -1 );
					}
				}
				/* FTK Imager uses \n
				 */
				else if( header_sections->header[ 1 ] == (ewf_char_t) '\n' )
				{
					if( header_sections->header[ 29 ] == (ewf_char_t) 'r' )
					{
						internal_handle->format = LIBEWF_FORMAT_FTK;
					}
					else
					{
						LIBEWF_WARNING_PRINT( "%s: unsupported header version.\n",
						 function );

						return( -1 );
					}
				}
				else
				{
					LIBEWF_WARNING_PRINT( "%s: unsupported header version.\n",
					 function );

					return( -1 );
				}
			}
			else
			{
				LIBEWF_WARNING_PRINT( "%s: unsupported header version.\n",
				 function );

				return( -1 );
			}
		}
		else
		{
			LIBEWF_WARNING_PRINT( "%s: missing header information.\n",
			 function );

			return( -1 );
		}
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_L01 )
	{
		internal_handle->format = LIBEWF_FORMAT_LVF;
	}
	else
	{
		LIBEWF_WARNING_PRINT( "%s: unsupported EWF file format.\n",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Create the default header values
 * Returns 1 on success, -1 on error
 */
int libewf_internal_handle_create_header_values( libewf_internal_handle_t *internal_handle )
{
	libewf_char_t *case_number              = _S_LIBEWF_CHAR( "Case Number" );
	libewf_char_t *description              = _S_LIBEWF_CHAR( "Description" );
	libewf_char_t *evidence_number          = _S_LIBEWF_CHAR( "Evidence Number" );
	libewf_char_t *examiner_name            = _S_LIBEWF_CHAR( "Examiner Name" );
	libewf_char_t *notes                    = _S_LIBEWF_CHAR( "Notes" );
	libewf_char_t *acquiry_operating_system = _S_LIBEWF_CHAR( "Undetermined" );
	libewf_char_t *acquiry_software_version = LIBEWF_VERSION;
	static char *function                   = "libewf_internal_handle_create_header_values";

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->header_values != NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: header values already created - cleaning up previous header values.\n",
		 function );

		libewf_values_table_free( internal_handle->header_values );
	}
	internal_handle->header_values = libewf_values_table_alloc(
	                                  LIBEWF_HEADER_VALUES_DEFAULT_AMOUNT );

	if( internal_handle->header_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create header values.\n",
		 function );

		return( -1 );
	}
	if( libewf_header_values_initialize(
             internal_handle->header_values ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to initialize the header values.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _S_LIBEWF_CHAR( "case_number" ),
	     case_number,
	     libewf_string_length( case_number ) ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to set case number.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _S_LIBEWF_CHAR( "description" ),
	     description,
	     libewf_string_length( description ) ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to set description.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _S_LIBEWF_CHAR( "evidence_number" ),
	     evidence_number,
	     libewf_string_length( evidence_number ) ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to set evidence number.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _S_LIBEWF_CHAR( "examiner_name" ),
	     examiner_name,
	     libewf_string_length( examiner_name ) ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to set examiner name.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _S_LIBEWF_CHAR( "notes" ),
	     notes,
	     libewf_string_length( notes ) ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to set notes.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _S_LIBEWF_CHAR( "acquiry_operating_system" ),
	     acquiry_operating_system,
	     libewf_string_length( acquiry_operating_system ) ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to acquiry operating system.\n",
		 function );

		return( -1 );
	}
	if( libewf_values_table_set_value(
	     internal_handle->header_values,
	     _S_LIBEWF_CHAR( "acquiry_software_version" ),
	     acquiry_software_version,
	     libewf_string_length( acquiry_software_version ) ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to acquiry software version.\n",
		 function );

		return( -1 );
	}
        /* The acquiry date, system date values and compression type
	 * will be generated automatically when set to NULL
         */
	return( 1 );
}

/* Initializes the read values
 * Returns 1 if successful, -1 on error
 */
int libewf_internal_handle_read_initialize(
     libewf_internal_handle_t *internal_handle )
{
	static char *function = "libewf_internal_handle_read_initialize";

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->read == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing subhandle read.\n",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Initializes the write values
 * Returns 1 if successful, -1 on error
 */
int libewf_internal_handle_write_initialize(
     libewf_internal_handle_t *internal_handle )
{
	static char *function               = "libewf_internal_handle_write_initialize";
	int64_t required_amount_of_segments = 0;
	int64_t amount_of_chunks            = 0;
	int64_t amount_of_sectors           = 0;
	uint64_t maximum_input_file_size    = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->chunk_cache == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing chunk cache.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->media_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing media values.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->offset_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing offset table.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->write == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing subhandle write.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->write->values_initialized != 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: write values were initialized and cannot be initialized anymore.\n",
		 function );

		return( -1 );
	}
	/* Determine the chunk size
	 */
	internal_handle->media_values->chunk_size = internal_handle->media_values->sectors_per_chunk
	                                          * internal_handle->media_values->bytes_per_sector;

	if( internal_handle->media_values->chunk_size == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: the media chunk size cannot be zero - using default media values.\n",
		 function );

		internal_handle->media_values->sectors_per_chunk = 64;
		internal_handle->media_values->bytes_per_sector  = 512;
		internal_handle->media_values->chunk_size        = EWF_MINIMUM_CHUNK_SIZE;
	}
	if( internal_handle->media_values->chunk_size > (uint32_t) INT32_MAX )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid media chunk size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->media_values->bytes_per_sector > (uint32_t) INT32_MAX )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid bytes per sector value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( ( internal_handle->compression_level != EWF_COMPRESSION_NONE )
	 && ( internal_handle->compression_level != EWF_COMPRESSION_FAST )
	 && ( internal_handle->compression_level != EWF_COMPRESSION_BEST ) )
	{
		LIBEWF_WARNING_PRINT( "%s: unsupported compression level - using default.\n",
		 function );

		internal_handle->compression_level    = EWF_COMPRESSION_NONE;
		internal_handle->compress_empty_block = 1;
	}
	/* Check if the input file size does not exceed the maximum input file size
	 */
	maximum_input_file_size = (uint64_t) internal_handle->media_values->chunk_size
	                        * (uint64_t) UINT32_MAX;

	if( internal_handle->media_values->media_size > maximum_input_file_size )
	{
		LIBEWF_WARNING_PRINT( "%s: media size cannot be larger than size: %" PRIu64 " with a chunk size of: %" PRIu32 ".\n",
		 function, maximum_input_file_size, internal_handle->media_values->chunk_size );

		return( -1 );
	}
	if( internal_handle->media_values->media_size > (uint64_t) INT64_MAX )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid media size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	/* Determine the EWF file format
	 */
	if( internal_handle->format == LIBEWF_FORMAT_LVF )
	{
		LIBEWF_WARNING_PRINT( "%s: writing format LVF currently not supported.\n",
		 function );

		return( -1 );
	}
	if( ( internal_handle->format != LIBEWF_FORMAT_ENCASE1 )
	 && ( internal_handle->format != LIBEWF_FORMAT_ENCASE2 )
	 && ( internal_handle->format != LIBEWF_FORMAT_ENCASE3 )
	 && ( internal_handle->format != LIBEWF_FORMAT_ENCASE4 )
	 && ( internal_handle->format != LIBEWF_FORMAT_ENCASE5 )
	 && ( internal_handle->format != LIBEWF_FORMAT_ENCASE6 )
	 && ( internal_handle->format != LIBEWF_FORMAT_LINEN5 )
	 && ( internal_handle->format != LIBEWF_FORMAT_LINEN6 )
	 && ( internal_handle->format != LIBEWF_FORMAT_SMART )
	 && ( internal_handle->format != LIBEWF_FORMAT_FTK )
	 && ( internal_handle->format != LIBEWF_FORMAT_LVF )
	 && ( internal_handle->format != LIBEWF_FORMAT_EWF )
	 && ( internal_handle->format != LIBEWF_FORMAT_EWFX ) )
	{
		LIBEWF_WARNING_PRINT( "%s: unsupported format - using default.\n",
		 function );

		internal_handle->format = LIBEWF_FORMAT_ENCASE5;
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
	/* Determine if the segment file size is in allowed ranges
	 */
	if( internal_handle->write->segment_file_size == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: the segment file size cannot be zero - using default value.\n",
		 function );

		internal_handle->write->segment_file_size = LIBEWF_DEFAULT_SEGMENT_FILE_SIZE;
	}
	if( internal_handle->write->segment_file_size > internal_handle->write->maximum_segment_file_size )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment file size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	/* Determine the maximum amount of segments allowed to write
	 */
	internal_handle->write->maximum_amount_of_segments = libewf_internal_handle_get_write_maximum_amount_of_segments( internal_handle );

	if( internal_handle->write->maximum_amount_of_segments == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine the maximum amount of allowed segment files.\n",
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
			LIBEWF_WARNING_PRINT( "%s: EWF file format does not allow for streaming write.\n",
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
			LIBEWF_WARNING_PRINT( "%s: the settings exceed the maximum amount of allowed segment files.\n",
			 function );

			return( -1 );
		}
		/* Determine the amount of chunks to write
		 */
		amount_of_chunks = (int64_t) internal_handle->media_values->media_size
		                 / (int64_t) internal_handle->media_values->chunk_size;

		if( ( internal_handle->media_values->media_size % internal_handle->media_values->chunk_size ) != 0 )
		{
			amount_of_chunks += 1;
		}
		if( amount_of_chunks > (int64_t) UINT32_MAX )
		{
			LIBEWF_WARNING_PRINT( "%s: the settings exceed the maximum amount of allowed chunks.\n",
			 function );

			return( -1 );
		}
		internal_handle->media_values->amount_of_chunks = (uint32_t) amount_of_chunks;

		/* Determine the amount of sectors to write
		 */
		amount_of_sectors = (int64_t) internal_handle->media_values->media_size
		                  / (int64_t) internal_handle->media_values->bytes_per_sector;

		if( amount_of_chunks > (int64_t) UINT32_MAX )
		{
			LIBEWF_WARNING_PRINT( "%s: the settings exceed the maximum amount of allowed sectors.\n",
			 function );

			return( -1 );
		}
		internal_handle->media_values->amount_of_sectors = (uint32_t) amount_of_sectors;
	}
        /* Allocate the necessary amount of chunk offsets
	 * this reduces the amount of reallocations
	 */
	if( internal_handle->offset_table->amount < amount_of_chunks )
	{
		if( libewf_offset_table_realloc(
		     internal_handle->offset_table,
		     (uint32_t) amount_of_chunks ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to reallocate offset table.\n",
			 function );

			return( -1 );
		}
	}
	/* Make sure the chuck cache is large enough
	 */
	if( ( internal_handle->media_values->chunk_size + sizeof( ewf_crc_t ) ) > internal_handle->chunk_cache->allocated_size )
	{
		if( libewf_chunk_cache_realloc(
		     internal_handle->chunk_cache,
		     ( internal_handle->media_values->chunk_size + sizeof( ewf_crc_t ) ) ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to reallocate chunk cache.\n",
			 function );

			return( -1 );
		}
	}
	/* Create the headers if required
	 */
	if( internal_handle->header_sections == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing header sections.\n",
		 function );

		return( -1 );
	}
	if( ( internal_handle->header_sections->header == NULL )
	 && ( internal_handle->header_sections->header2 == NULL )
	 && ( internal_handle->header_sections->xheader == NULL ) )
	{
		if( internal_handle->header_values == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: empty header values - using default.\n",
			 function );

			if( libewf_internal_handle_create_header_values( internal_handle ) != 1 )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to create header values.\n",
				 function );

				return( -1 );
			}
		}
		if( libewf_header_sections_create(
		     internal_handle->header_sections,
		     internal_handle->header_values,
		     internal_handle->compression_level,
		     internal_handle->format ) == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to create header(s).\n",
			 function );

			return( -1 );
		}
	}
	/* Flag that the write values were initialized
	 */
	internal_handle->write->values_initialized = 1;

	return( 1 );
}

