/*
 * Filename functions
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
#include <memory.h>
#include <types.h>

#include "libewf_definitions.h"
#include "libewf_filename.h"
#include "libewf_libcerror.h"
#include "libewf_libcstring.h"

#include "ewf_definitions.h"

/* Sets the extension for a certain segment file
 * Using the format specific naming schema
 * Returns 1 on success or -1 on error
 */
int libewf_filename_set_extension(
     char *filename,
     size_t filename_size,
     size_t *filename_index,
     uint32_t segment_number,
     uint32_t maximum_number_of_segments,
     uint8_t segment_file_type,
     uint8_t format,
     uint8_t ewf_format,
     libcerror_error_t **error )
{
	static char *function      = "libewf_filename_set_extension";
	size_t string_index        = 0;
	char additional_characters = 0;
	char first_character       = 0;
	uint8_t major_version      = 1;

	if( filename == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		return( -1 );
	}
	if( filename_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid filename size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( filename_index == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename index.",
		 function );

		return( -1 );
	}
	if( *filename_index >= filename_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: filename index value is out of bounds.",
		 function );

		return( -1 );
	}
	if( ( segment_number == 0 )
	 || ( segment_number > maximum_number_of_segments ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: segment number: %" PRIu16 " is out of bounds.",
		 function,
		 segment_number );

		return( -1 );
	}
/* TODO check filename size */
	string_index = *filename_index;

	if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF1 )
	{
		if( ( format == LIBEWF_FORMAT_EWF )
		 || ( format == LIBEWF_FORMAT_EWFX ) )
		{
			first_character       = 'e';
			additional_characters = 'a';
		}
		else if( ewf_format == EWF_FORMAT_S01 )
		{
			first_character       = 's';
			additional_characters = 'a';
		}
		else
		{
			first_character       = 'E';
			additional_characters = 'A';
		}
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF1_LOGICAL )
	{
		first_character       = 'L';
		additional_characters = 'A';
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF2 )
	{
		major_version         = 2;
		first_character       = 'E';
		additional_characters = 'A';
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF2_LOGICAL )
	{
		major_version         = 2;
		first_character       = 'L';
		additional_characters = 'A';
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF1_DELTA )
	{
		first_character       = 'd';
		additional_characters = 'a';
	}
	else
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported segment file type.",
		 function );

		return( -1 );
	}
	filename[ string_index++ ] = first_character;

	if( major_version == 2 )
	{
		filename[ string_index++ ] = 'x';
	}
	if( segment_number <= 99 )
	{
		filename[ string_index + 1 ] = '0' + (char) ( segment_number % 10 );
		filename[ string_index ]     = '0' + (char) ( segment_number / 10 );
	}
	else if( segment_number >= 100 )
	{
		segment_number -= 100;

		filename[ string_index + 1 ] = additional_characters + (char) ( segment_number % 26 );
		segment_number              /= 26;

		filename[ string_index ] = additional_characters + (char) ( segment_number % 26 );
		segment_number          /= 26;

		if( segment_number >= 26 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: unable to support for more segment files.",
			 function );

			return( -1 );
		}
		filename[ *filename_index ] += (char) segment_number;
	}
	string_index += 2;

	/* Safety check
	 */
	if( ( filename[ *filename_index ] > 'z' )
	 || ( ( filename[ *filename_index ] > 'Z' )
	  &&  ( filename[ *filename_index ] < 'a' ) ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unable to support for more segment files.",
		 function );

		return( -1 );
	}
	filename[ string_index++ ] = 0;

	*filename_index = string_index;

	return( 1 );
}

#if defined( HAVE_WIDE_CHARACTER_TYPE )

/* Sets the extension for a certain segment file
 * Using the format specific naming schema
 * Returns 1 on success or -1 on error
 */
int libewf_filename_set_extension_wide(
     wchar_t *filename,
     size_t filename_size,
     size_t *filename_index,
     uint32_t segment_number,
     uint32_t maximum_number_of_segments,
     uint8_t segment_file_type,
     uint8_t format,
     uint8_t ewf_format,
     libcerror_error_t **error )
{
	static char *function         = "libewf_filename_set_extension_wide";
	size_t string_index           = 0;
	wchar_t additional_characters = 0;
	wchar_t first_character       = 0;
	uint8_t major_version         = 1;

	if( filename == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		return( -1 );
	}
	if( filename_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid filename size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( filename_index == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename index.",
		 function );

		return( -1 );
	}
	if( *filename_index >= filename_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: filename index value is out of bounds.",
		 function );

		return( -1 );
	}
	if( ( segment_number == 0 )
	 || ( segment_number > maximum_number_of_segments ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: segment number: %" PRIu16 " is out of bounds.",
		 function,
		 segment_number );

		return( -1 );
	}
/* TODO check filename size */
	string_index = *filename_index;

	if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF1 )
	{
		if( ( format == LIBEWF_FORMAT_EWF )
		 || ( format == LIBEWF_FORMAT_EWFX ) )
		{
			first_character       = (wchar_t) 'e';
			additional_characters = (wchar_t) 'a';
		}
		else if( ewf_format == EWF_FORMAT_S01 )
		{
			first_character       = (wchar_t) 's';
			additional_characters = (wchar_t) 'a';
		}
		else
		{
			first_character       = (wchar_t) 'E';
			additional_characters = (wchar_t) 'A';
		}
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF1_LOGICAL )
	{
		first_character       = (wchar_t) 'L';
		additional_characters = (wchar_t) 'A';
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF2 )
	{
		major_version         = 2;
		first_character       = 'E';
		additional_characters = 'A';
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF2_LOGICAL )
	{
		major_version         = 2;
		first_character       = 'L';
		additional_characters = 'A';
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF1_DELTA )
	{
		first_character       = (wchar_t) 'd';
		additional_characters = (wchar_t) 'a';
	}
	else
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported segment file type.",
		 function );

		return( -1 );
	}
	filename[ string_index++ ] = first_character;

	if( major_version == 2 )
	{
		filename[ string_index++ ] = 'x';
	}
	if( segment_number <= 99 )
	{
		filename[ string_index + 1 ] = (wchar_t) '0' + (wchar_t) ( segment_number % 10 );
		filename[ string_index ]     = (wchar_t) '0' + (wchar_t) ( segment_number / 10 );
	}
	else if( segment_number >= 100 )
	{
		segment_number -= 100;

		filename[ string_index + 1 ] = additional_characters + (wchar_t) ( segment_number % 26 );
		segment_number              /= 26;

		filename[ string_index ] = additional_characters + (wchar_t) ( segment_number % 26 );
		segment_number          /= 26;

		if( segment_number >= 26 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: unable to support for more segment files.",
			 function );

			return( -1 );
		}
		filename[ *filename_index ] += (wchar_t) segment_number;
	}
	string_index += 2;

	/* Safety check
	 */
	if( ( filename[ *filename_index ] > (wchar_t) 'z' )
	 || ( ( filename[ *filename_index ] > (wchar_t) 'Z' )
	  &&  ( filename[ *filename_index ] < (wchar_t) 'a' ) ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unable to support for more segment files.",
		 function );

		return( -1 );
	}
	filename[ string_index++ ] = 0;

	*filename_index = string_index;

	return( 1 );
}

#endif

/* Creates a filename for a certain segment file
 * Returns 1 if successful or -1 on error
 */
int libewf_filename_create(
     libcstring_system_character_t **filename,
     size_t *filename_size,
     libcstring_system_character_t *basename,
     size_t basename_length,
     uint16_t segment_number,
     uint16_t maximum_number_of_segments,
     uint8_t segment_file_type,
     uint8_t format,
     uint8_t ewf_format,
     libcerror_error_t **error )
{
	static char *function = "libewf_filename_create";

	if( filename == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename.",
		 function );

		return( -1 );
	}
	if( *filename != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: filename already set.",
		 function );

		return( -1 );
	}
	if( filename_size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filename size.",
		 function );

		return( -1 );
	}
	if( basename == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid basename.",
		 function );

		return( -1 );
	}
	/* The actual filename also contains a '.', 3 character extension and a end of string byte
	 */
	*filename_size = basename_length + 5;

	*filename = libcstring_system_string_allocate(
	             *filename_size );

	if( *filename == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create filename.",
		 function );

		goto on_error;
	}
	if( libcstring_system_string_copy(
	     *filename,
	     basename,
	     basename_length ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set basename.",
		 function );

		goto on_error;
	}
	( *filename )[ basename_length++ ] = (libcstring_system_character_t) '.';

#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	if( libewf_filename_set_extension_wide(
	     *filename,
	     *filename_size,
	     &basename_length,
	     segment_number,
	     maximum_number_of_segments,
	     segment_file_type,
	     format,
	     ewf_format,
	     error ) != 1 )
#else
	if( libewf_filename_set_extension(
	     *filename,
	     *filename_size,
	     &basename_length,
	     segment_number,
	     maximum_number_of_segments,
	     segment_file_type,
	     format,
	     ewf_format,
	     error ) != 1 )
#endif
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set extension.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( *filename != NULL )
	{
		memory_free(
		 *filename );

		*filename = NULL;
	}
	*filename_size = 0;

	return( -1 );
}

