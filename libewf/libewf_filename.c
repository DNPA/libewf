/*
 * libewf filename
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

#include <common.h>
#include <memory.h>

#include "libewf_includes.h"

#include <libewf/libewf_definitions.h>

#include "libewf_filename.h"
#include "libewf_notify.h"

#include "ewf_definitions.h"

/* Creates the filename extension for a certain segment file
 * For EWF-E01, EWF-S01 segment file extension naming schema
 * Returns 1 on success, -1 on error
 */
int libewf_filename_set_extension(
     libewf_filename_t *extension,
     uint16_t segment_number,
     uint16_t maximum_amount_of_segments,
     uint8_t segment_file_type,
     uint8_t format,
     uint8_t ewf_format )
{
	static char *function                             = "libewf_filename_set_extension";
	libewf_filename_t extension_first_character       = (libewf_filename_t) '\0';
	libewf_filename_t extension_additional_characters = (libewf_filename_t) '\0';

	if( extension == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid extension.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment 0.\n",
		 function );

		return( -1 );
	}
	if( segment_number > maximum_amount_of_segments )
	{
		LIBEWF_WARNING_PRINT( "%s: segment number: %" PRIu16 " exceeds the maximum amount of segment files: %" PRIu16 ".\n",
		 function, segment_number, maximum_amount_of_segments );

		return( -1 );
	}
	if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF )
	{
		if( ( format == LIBEWF_FORMAT_EWF )
		 || ( format == LIBEWF_FORMAT_EWFX ) )
		{
			extension_first_character       = (libewf_filename_t) 'e';
			extension_additional_characters = (libewf_filename_t) 'a';
		}
		else if( ewf_format == EWF_FORMAT_S01 )
		{
			extension_first_character       = (libewf_filename_t) 's';
			extension_additional_characters = (libewf_filename_t) 'a';
		}
		else
		{
			extension_first_character       = (libewf_filename_t) 'E';
			extension_additional_characters = (libewf_filename_t) 'A';
		}
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_LWF )
	{
		extension_first_character       = (libewf_filename_t) 'L';
		extension_additional_characters = (libewf_filename_t) 'A';
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_DWF )
	{
		extension_first_character       = (libewf_filename_t) 'd';
		extension_additional_characters = (libewf_filename_t) 'a';
	}
	else
	{
		LIBEWF_WARNING_PRINT( "%s: unsupported segment file type.\n",
		 function );

		return( -1 );
	}
	extension[ 0 ] = extension_first_character;

	if( segment_number <= 99 )
	{
		extension[ 2 ] = (libewf_filename_t) '0' + (libewf_filename_t) ( segment_number % 10 );
		extension[ 1 ] = (libewf_filename_t) '0' + (libewf_filename_t) ( segment_number / 10 );
	}
	else if( segment_number >= 100 )
	{
		segment_number -= 100;
		extension[ 2 ]  = extension_additional_characters + (libewf_filename_t) ( segment_number % 26 );
		segment_number /= 26;
		extension[ 1 ]  = extension_additional_characters + (libewf_filename_t) ( segment_number % 26 );
		segment_number /= 26;

		if( segment_number >= 26 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to support for more segment files.\n",
			 function );

			return( -1 );
		}
		extension[ 0 ] = extension_first_character + (libewf_filename_t) segment_number;
	}
	/* Safety check
	 */
	if( ( extension[ 0 ] > (libewf_filename_t) 'z' )
	 || ( ( extension[ 0 ] > (libewf_filename_t) 'Z' )
	  && ( extension[ 0 ] < (libewf_filename_t) 'a' ) ) )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to support for more segment files.\n",
		 function );

		return( -1 );
	}
	extension[ 3 ] = (libewf_filename_t) '\0';

	return( 1 );
}

/* Creates a filename for a certain segment file
 * Returns 1 if successful, or -1 on error
 */
int libewf_filename_create(
     libewf_filename_t **filename,
     size_t *length_filename,
     libewf_filename_t *basename,
     size_t length_basename,
     uint16_t segment_number,
     uint16_t maximum_amount_of_segments,
     uint8_t segment_file_type,
     uint8_t format,
     uint8_t ewf_format )
{
	libewf_filename_t *new_filename = NULL;
	static char *function           = "libewf_filename_create";

	if( filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid filename.\n",
		 function );

		return( -1 );
	}
	if( *filename != NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: filename already set.\n",
		 function );

		return( -1 );
	}
	if( length_filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid length filename.\n",
		 function );

		return( -1 );
	}
	if( basename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid basename.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment 0.\n",
		 function );

		return( -1 );
	}
	/* The actual filename also contains a '.', 3 character extension and a end of string byte
	 */
	new_filename = memory_allocate(
	                sizeof( libewf_filename_t ) * ( length_basename + 5 ) );

	if( new_filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate filename.\n",
		 function );

		return( -1 );
	}
	/* Add one additional character for the end of line
	 */
	if( libewf_filename_copy(
	     new_filename,
	     basename,
	     ( length_basename + 1 ) ) == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to copy basename.\n",
		 function );

		libewf_common_free( new_filename );

		return( -1 );
	}
	new_filename[ length_basename ] = (libewf_filename_t) '.';

	if( libewf_filename_set_extension(
	     &( new_filename[ length_basename + 1 ] ),
	     segment_number,
	     maximum_amount_of_segments,
	     segment_file_type,
	     format,
	     ewf_format ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to set extension.\n",
		 function );

		libewf_common_free( new_filename );

		return( -1 );
	}
	*filename        = new_filename;
	*length_filename = length_basename + 5;

	return( 1 );
}

