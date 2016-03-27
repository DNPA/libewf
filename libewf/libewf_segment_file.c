/*
 * libewf segment file
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

#include "libewf_includes.h"

#include <errno.h>

#include <libewf/libewf_definitions.h>

#include "libewf_common.h"
#include "libewf_endian.h"
#include "libewf_notify.h"
#include "libewf_section.h"
#include "libewf_segment_file.h"
#include "libewf_segment_table.h"
#include "libewf_string.h"

#include "ewf_definitions.h"
#include "ewf_file_header.h"

/* Detects if a file is an EWF file (check for the EWF file signature)
 * Returns 1 if true, 0 if not, or -1 on error
 */
int libewf_segment_file_check_file_signature( int file_descriptor )
{
	uint8_t signature[ 8 ];

	static char *function = "libewf_segment_file_check_file_signature";
	ssize_t read_count    = 0;

	if( file_descriptor <= -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	read_count = libewf_common_read( file_descriptor, signature, 8 );

	if( read_count <= -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: error reading signature from file.\n",
		 function );

		return( -1 );
	}
	else if( read_count != 8 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to read signature from file.\n",
		 function );

		return( -1 );
	}
	return( ewf_file_header_check_signature( signature ) );
}

/* Reads the file header from a segment file
 * Returns the amount of bytes read if successful, or -1 on errror
 */
ssize_t libewf_segment_file_read_file_header( int file_descriptor, uint16_t *segment_number )
{
	EWF_FILE_HEADER file_header;

	static char *function = "libewf_segment_file_check_file_signature";
	ssize_t read_count    = 0;

	if( file_descriptor <= -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	if( segment_number == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number.\n",
		 function );

		return( -1 );
	}
	read_count = ewf_file_header_read( &file_header, file_descriptor );

	if( read_count <= -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to read file header.\n",
		 function );

		return( -1 );
	}
	if( ewf_file_header_check_signature( file_header.signature ) == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: file signature does not match.\n",
		 function );

		return( -1 );
	}
	if( libewf_endian_convert_16bit( segment_number, file_header.fields_segment ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to convert fields segment number value.\n",
		 function );

		return( -1 );
	}
	return( read_count );
}

#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )

/* Opens EWF segment file(s) for reading
 * Returns 1 if successful, 0 if not, or -1 on error
 */
int libewf_segment_file_read_wide_open( LIBEWF_INTERNAL_HANDLE *internal_handle, wchar_t * const filenames[], uint16_t file_amount, uint8_t flags )
{
	wchar_t *error_string   = NULL;
	static char *function   = "libewf_segment_file_read_wide_open";
	uint32_t iterator       = 0;
	uint16_t segment_number = 0;
	int file_descriptor     = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing segment table.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing filenames.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_descriptor == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file descriptors.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_offset == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file offsets.\n",
		 function );

		return( -1 );
	}
	if( file_amount < 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file amount at least 1 is required.\n",
		 function );

		return( -1 );
	}
	/* TODO check for maximum amount of open file descriptors
	 */
	for( iterator = 0; iterator < file_amount; iterator++ )
	{
		file_descriptor = libewf_common_wide_open( filenames[ iterator ], flags );

		if( file_descriptor == -1 )
		{
			error_string = libewf_common_wide_strerror( errno );

			if( error_string == NULL )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to open file: %ls.\n",
				 function, filenames[ iterator ] );
			}
			else
			{
				LIBEWF_WARNING_PRINT( "%s: unable to open file: %ls with error: %s.\n",
				 function, filenames[ iterator ], error_string );

				libewf_common_free( error_string );
			}
			return( -1 );
		}
		if( libewf_segment_file_read_file_header( file_descriptor, &segment_number ) <= -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to read file header in: %ls.\n",
			 function, filenames[ iterator ] );

			return( -1 );
		}
		if( segment_number == 0 )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid segment number: 0.\n",
			 function );

			return( -1 );
		}
		if( segment_number > internal_handle->segment_table->amount )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid segment number, value out of range.\n",
			 function );

			return( -1 );
		}
		if( libewf_segment_table_set_wide_filename(
		     internal_handle->segment_table,
		     0,
		     filenames[ iterator ],
		     libewf_common_string_length( filenames[ iterator ] ) ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set filename in segment table.\n",
			 function );

			return( -1 );
		}
		internal_handle->segment_table->file_descriptor[ segment_number ] = file_descriptor;
		internal_handle->segment_table->file_offset[ segment_number ]     = (off_t) EWF_FILE_HEADER_SIZE;

		LIBEWF_VERBOSE_PRINT( "%s: added segment file: %ls with file descriptor: %d with segment number: %" PRIu16 ".\n",
		 function, filenames[ iterator ], file_descriptor, segment_number );
	}
	return( 1 );
}
#else

/* Opens EWF segment file(s) for reading
 * Returns 1 if successful, 0 if not, or -1 on error
 */
int libewf_segment_file_read_open( LIBEWF_INTERNAL_HANDLE *internal_handle, char * const filenames[], uint16_t file_amount, uint8_t flags )
{
	char *error_string      = NULL;
	static char *function   = "libewf_segment_file_read_open";
	uint32_t iterator       = 0;
	uint16_t segment_number = 0;
	int file_descriptor     = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing segment table.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing filenames.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_descriptor == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file descriptors.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_offset == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file offsets.\n",
		 function );

		return( -1 );
	}
	if( file_amount < 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file amount at least 1 is required.\n",
		 function );

		return( -1 );
	}
	/* TODO check for maximum amount of open file descriptors
	 */
	for( iterator = 0; iterator < file_amount; iterator++ )
	{
		file_descriptor = libewf_common_open( filenames[ iterator ], flags );

		if( file_descriptor == -1 )
		{
			error_string = libewf_common_strerror( errno );

			if( error_string == NULL )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to open file: %s.\n",
				 function, filenames[ iterator ] );
			}
			else
			{
				LIBEWF_WARNING_PRINT( "%s: unable to open file: %s with error: %s.\n",
				 function, filenames[ iterator ], error_string );

				libewf_common_free( error_string );
			}
			return( -1 );
		}
		if( libewf_segment_file_read_file_header( file_descriptor, &segment_number ) <= -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to read file header in: %s.\n",
			 function, filenames[ iterator ] );

			return( -1 );
		}
		if( segment_number == 0 )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid segment number: 0.\n",
			 function );

			return( -1 );
		}
		if( segment_number > internal_handle->segment_table->amount )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid segment number, value out of range.\n",
			 function );

			return( -1 );
		}
		if( libewf_segment_table_set_filename(
		     internal_handle->segment_table,
		     0,
		     filenames[ iterator ],
		     libewf_common_string_length( filenames[ iterator ] ) ) != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to set filename in segment table.\n",
			 function );

			return( -1 );
		}
		internal_handle->segment_table->file_descriptor[ segment_number ] = file_descriptor;
		internal_handle->segment_table->file_offset[ segment_number ]     = (off_t) EWF_FILE_HEADER_SIZE;

		LIBEWF_VERBOSE_PRINT( "%s: added segment file: %s with file descriptor: %d with segment number: %" PRIu16 ".\n",
		 function, filenames[ iterator ], file_descriptor, segment_number );
	}
	return( 1 );
}
#endif

#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )

/* Opens EWF segment file(s) for writing
 * Returns 1 if successful, 0 if not, or -1 on error
int libewf_segment_file_write_wide_open( LIBEWF_INTERNAL_HANDLE *internal_handle, wchar_t * const filenames[], uint16_t file_amount )
{
}
 */
#else

/* Opens EWF segment file(s) for writing
 * Returns 1 if successful, 0 if not, or -1 on error
int libewf_segment_file_write_open( LIBEWF_INTERNAL_HANDLE *internal_handle, char * const filenames[], uint16_t file_amount )
{
}
 */
#endif

/* Reads the segment table from all segment files
 * Returns 1 if successful, 0 if not, or -1 on error
 */
int libewf_segment_file_read_segment_table( LIBEWF_INTERNAL_HANDLE *internal_handle )
{
	static char *function   = "libewf_segment_file_read_segment_table";
	uint16_t segment_number = 0;
	int last_segment_file   = 0;
	int result              = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing segment table.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_descriptor == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file descriptors.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->section_list == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing section lists.\n",
		 function );

		return( -1 );
	}
	for( segment_number = 1; segment_number < internal_handle->segment_table->amount; segment_number++ )
	{
		LIBEWF_VERBOSE_PRINT( "%s: reading section list for segment number: %" PRIu16 ".\n",
		 function, segment_number );

		result = libewf_segment_file_read_sections(
		          internal_handle,
		          segment_number,
		          internal_handle->segment_table->file_descriptor[ segment_number ],
		          internal_handle->segment_table->section_list[ segment_number ],
		          &last_segment_file );

		if( result == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to read sections.\n",
			 function );

			return( -1 );
		}
		else if( result == 0 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to missing next or done section.\n",
			 function );

			return( 0 );
		}
	}
	/* Check to see if the done section has been found in the last segment file
	 */
	if( last_segment_file != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to find the last segment file.\n",
		 function );

		return( 0 );
	}
	return( 1 );
}

/* Reads all sections from a segment file into the section list specific
 * for the segment file in the segment table in the handle
 * Returns 1 if successful, 0 if not, or -1 on error
 */
int libewf_segment_file_read_sections( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, int file_descriptor, LIBEWF_SECTION_LIST *section_list, int *last_segment_file )
{
	EWF_SECTION section;

	static char *function = "libewf_segment_file_read_sections";
	off_t previous_offset = 0;
	int result            = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing segment table.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number.\n",
		 function );

		return( -1 );
	}
	if( segment_number > internal_handle->segment_table->amount )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number, value out of range.\n",
		 function );

		return( -1 );
	}
	if( file_descriptor <= -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	if( section_list == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid section list.\n",
		 function );

		return( -1 );
	}
	if( last_segment_file == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid last segment file.\n",
		 function );

		return( -1 );
	}
	*last_segment_file = 0;

	/* The first offset is directly after the file header (13 byte)
	 */
	previous_offset = (off_t) EWF_FILE_HEADER_SIZE;

	while( result != -1 )
	{
		result = libewf_section_read(
		          internal_handle,
		          file_descriptor,
		          &section,
		          section_list,
		          segment_number,
		          &previous_offset );

		if( result != 1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to read section.\n",
			 function );

			return( -1 );
		}
		/* The done and next sections point back at themselves
		 */
		if( ewf_section_is_type_next( &section ) == 1 )
		{
			return( 1 );
		}
		else if( ewf_section_is_type_done( &section ) == 1 )
		{
			*last_segment_file = 1;

			return( 1 );
		}
	}
	return( 0 );
}

#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )

/* Determines a wide character extension for a certain segment file
 * For EWF-E01, EWF-S01 segment file extension naming scheme
 * Returns 1 on success, -1 on error
 */
int libewf_segment_file_create_wide_extension( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, int16_t maximum_amount_of_segments, wchar_t* extension )
{
	static char *function                   = "libewf_segment_file_create_wide_extension";
	wchar_t extension_first_character       = (wchar_t) '\0';
	wchar_t extension_additional_characters = (wchar_t) '\0';

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
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
	if( maximum_amount_of_segments <= -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid maximum amount of segment files.\n",
		 function );

		return( -1 );
	}
	if( segment_number > (uint16_t) maximum_amount_of_segments )
	{
		LIBEWF_WARNING_PRINT( "%s: segment number exceeds the maximum amount of segment files.\n",
		 function );

		return( -1 );
	}
	if( ( internal_handle->format == LIBEWF_FORMAT_EWF )
	 || ( internal_handle->format == LIBEWF_FORMAT_EWFX ) )
	{
		extension_first_character       = (wchar_t) 'e';
		extension_additional_characters = (wchar_t) 'a';
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_S01 )
	{
		extension_first_character       = (wchar_t) 's';
		extension_additional_characters = (wchar_t) 'a';
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_E01 )
	{
		extension_first_character       = (wchar_t) 'E';
		extension_additional_characters = (wchar_t) 'A';
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_L01 )
	{
		extension_first_character       = (wchar_t) 'L';
		extension_additional_characters = (wchar_t) 'A';
	}
	else
	{
		LIBEWF_WARNING_PRINT( "%s: unsupported EWF format.\n",
		 function );

		return( -1 );
	}
	extension[ 0 ] = extension_first_character;

	if( segment_number <= 99 )
	{
		extension[ 2 ] = (wchar_t) '0' + (wchar_t) ( segment_number % 10 );
		extension[ 1 ] = (wchar_t) '0' + (wchar_t) ( segment_number / 10 );
	}
	else if( segment_number >= 100 )
	{
		segment_number -= 100;
		extension[ 2 ]  = extension_additional_characters + (wchar_t) ( segment_number % 26 );
		segment_number /= 26;
		extension[ 1 ]  = extension_additional_characters + (wchar_t) ( segment_number % 26 );
		segment_number /= 26;

		if( segment_number >= 26 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to support for more segment files.\n",
			 function );

			return( -1 );
		}
		extension[ 0 ] = extension_first_character + (wchar_t) segment_number;
	}
	/* Safety check
	 */
	if( ( extension[ 0 ] > (wchar_t) 'z' )
	 || ( ( extension[ 0 ] > (wchar_t) 'Z' )
	  && ( extension[ 0 ] < (wchar_t) 'a' ) ) )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to support for more segment files.\n",
		 function );

		return( -1 );
	}
	extension[ 3 ] = (wchar_t) '\0';

	return( 1 );
}
#else

/* Determines an extension for a certain segment file
 * For EWF-E01, EWF-S01 segment file extension naming scheme
 * Returns 1 on success, -1 on error
 */
int libewf_segment_file_create_extension( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, int16_t maximum_amount_of_segments, char* extension )
{
	static char *function                = "libewf_segment_file_create_extension";
	char extension_first_character       = (char) '\0';
	char extension_additional_characters = (char) '\0';

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
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
	if( maximum_amount_of_segments <= -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid maximum amount of segment files.\n",
		 function );

		return( -1 );
	}
	if( segment_number > (uint16_t) maximum_amount_of_segments )
	{
		LIBEWF_WARNING_PRINT( "%s: segment number exceeds the maximum amount of segment files.\n",
		 function );

		return( -1 );
	}
	if( ( internal_handle->format == LIBEWF_FORMAT_EWF )
	 || ( internal_handle->format == LIBEWF_FORMAT_EWFX ) )
	{
		extension_first_character       = (char) 'e';
		extension_additional_characters = (char) 'a';
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_S01 )
	{
		extension_first_character       = (char) 's';
		extension_additional_characters = (char) 'a';
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_E01 )
	{
		extension_first_character       = (char) 'E';
		extension_additional_characters = (char) 'A';
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_L01 )
	{
		extension_first_character       = (char) 'L';
		extension_additional_characters = (char) 'A';
	}
	else
	{
		LIBEWF_WARNING_PRINT( "%s: unsupported EWF format.\n",
		 function );

		return( -1 );
	}
	extension[ 0 ] = extension_first_character;

	if( segment_number <= 99 )
	{
		extension[ 2 ] = (char) '0' + (char) ( segment_number % 10 );
		extension[ 1 ] = (char) '0' + (char) ( segment_number / 10 );
	}
	else if( segment_number >= 100 )
	{
		segment_number -= 100;
		extension[ 2 ]  = extension_additional_characters + (char) ( segment_number % 26 );
		segment_number /= 26;
		extension[ 1 ]  = extension_additional_characters + (char) ( segment_number % 26 );
		segment_number /= 26;

		if( segment_number >= 26 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to support for more segment files.\n",
			 function );

			return( -1 );
		}
		extension[ 0 ] = extension_first_character + (char) segment_number;
	}
	/* Safety check
	 */
	if( ( extension[ 0 ] > (char) 'z' )
	 || ( ( extension[ 0 ] > (char) 'Z' )
	  && ( extension[ 0 ] < (char) 'a' ) ) )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to support for more segment files.\n",
		 function );

		return( -1 );
	}
	extension[ 3 ] = (char) '\0';

	return( 1 );
}
#endif

#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )

/* Creates a wide character filename for a certain segment file
 * Returns the pointer to the filename, NULL on error
 */
wchar_t *libewf_segment_file_create_wide_filename( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, int16_t maximum_amount_of_segments, wchar_t* basename )
{
	wchar_t *filename     = NULL;
	static char *function = "libewf_segment_file_create_wide_filename";
	size_t length         = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( NULL );
	}
	if( basename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid basename.\n",
		 function );

		return( NULL );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment 0.\n",
		 function );

		return( NULL );
	}
	length = libewf_common_wide_string_length( basename );

	if( length == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: an emtpy basename is not supported.\n",
		 function );

		return( NULL );
	}
	/* The actual filename also contain a . 3 character extension and a end of string byte
	 */
	filename = libewf_common_alloc( ( length + 5 ) * sizeof( wchar_t ) );

	if( filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate filename.\n",
		 function );

		return( NULL );
	}
	/* Add one additional character for the end of line
	 */
	if( libewf_common_wide_string_copy( filename, basename, ( length + 1 ) ) == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to copy basename.\n",
		 function );

		libewf_common_free( filename );

		return( NULL );
	}
	filename[ length ] = '.';

	if( libewf_segment_file_create_wide_extension(
	     internal_handle,
	     segment_number,
	     maximum_amount_of_segments,
	     &filename[ length + 1 ] ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine extension.\n",
		 function );

		libewf_common_free( filename );

		return( NULL );
	}
	return( filename );
}
#else

/* Creates a filename for a certain segment file
 * Returns the pointer to the filename, NULL on error
 */
char *libewf_segment_file_create_filename( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, int16_t maximum_amount_of_segments, char* basename )
{
	char *filename        = NULL;
	static char *function = "libewf_segment_file_create_filename";
	size_t length         = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( NULL );
	}
	if( basename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid basename.\n",
		 function );

		return( NULL );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment 0.\n",
		 function );

		return( NULL );
	}
	length = libewf_common_string_length( basename );

	if( length == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: an emtpy basename is not supported.\n",
		 function );

		return( NULL );
	}
	/* The actual filename also contain a . 3 character extension and a end of string byte
	 */
	filename = libewf_common_alloc( ( length + 5 ) * sizeof( char ) );

	if( filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate filename.\n",
		 function );

		return( NULL );
	}
	/* Add one additional character for the end of line
	 */
	if( libewf_common_string_copy( filename, basename, ( length + 1 ) ) == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to copy basename.\n",
		 function );

		libewf_common_free( filename );

		return( NULL );
	}
	filename[ length ] = '.';

	if( libewf_segment_file_create_extension(
	     internal_handle,
	     segment_number,
	     maximum_amount_of_segments,
	     &filename[ length + 1 ] ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to determine extension.\n",
		 function );

		libewf_common_free( filename );

		return( NULL );
	}
	return( filename );
}
#endif

/* Creates a new segment file entry within the segment table
 * Returns 1 on success, -1 on error
 */
int libewf_segment_file_create_file_entry( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, int16_t maximum_amount_of_segments )
{
	LIBEWF_SEGMENT_TABLE *segment_table = NULL;
#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
	wchar_t *filename                   = NULL;
#else
	char *filename                      = NULL;
#endif
	static char *function               = "libewf_write_create_segment_file_entry";
	size_t length_filename              = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing segment table.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment 0.\n",
		 function );

		return( -1 );
	}

	/* Check if one additional entry in the segment table is needed
	 */
	if( segment_number >= internal_handle->segment_table->amount )
	{
		/* Add one additional entry because the 0 entry is used for the basename
		 */
		segment_table = libewf_segment_table_realloc(
		                 internal_handle->segment_table,
		                 ( segment_number + 1 ) );

		if( segment_table == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to reallocate segment table.\n",
			 function );

			return( -1 );
		}
		internal_handle->segment_table = segment_table;

	}
	/* Check if the entry has already been filled
	 */
	else if( internal_handle->segment_table->filename[ segment_number ] != NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: segment file has already been created.\n",
		 function );

		return( -1 );
	}
#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
	filename = libewf_segment_file_create_wide_filename(
	            internal_handle,
	            segment_number,
	            maximum_amount_of_segments,
	            internal_handle->segment_table->filename[ 0 ] );
#else
	filename = libewf_segment_file_create_filename(
	            internal_handle,
	            segment_number,
	            maximum_amount_of_segments,
	            internal_handle->segment_table->filename[ 0 ] );
#endif

	if( filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create filename.\n",
		 function );

		return( -1 );
	}
#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
	length_filename = libewf_common_wide_string_length( filename );
#else
	length_filename = libewf_common_string_length( filename );
#endif

	if( length_filename == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: filename cannot be empty.\n",
		 function );

		return( -1 );
	}
	internal_handle->segment_table->filename[ segment_number ]        = filename;
	internal_handle->segment_table->file_descriptor[ segment_number ] = -1;

#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
	LIBEWF_VERBOSE_PRINT( "%s: segment file created: %" PRIu32 " with name: %ls.\n",
	 function, segment_number, filename );
#else
	LIBEWF_VERBOSE_PRINT( "%s: segment file created: %" PRIu32 " with name: %s.\n",
	 function, segment_number, filename );
#endif
	return( 1 );
}

/* Write the headers to file
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_headers( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, LIBEWF_SECTION_LIST *section_list, off_t start_offset )
{
	static char *function     = "libewf_segment_file_write_headers";
	size_t header_size        = 0;
	size_t header2_size       = 0;
	size_t xheader_size       = 0;
	ssize_t write_count       = 0;
	ssize_t total_write_count = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( section_list == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid section list.\n",
		 function );

		return( -1 );
	}
	if( file_descriptor == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	if( ( internal_handle->header == NULL )
	 || ( internal_handle->header_size == 0 ) )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid header.\n",
		 function );

		return( -1 );
	}
	header_size = internal_handle->header_size - 1;

	if( ( internal_handle->format == LIBEWF_FORMAT_EWF )
	 || ( internal_handle->format == LIBEWF_FORMAT_SMART )
	 || ( internal_handle->format == LIBEWF_FORMAT_ENCASE1 ) )
	{
		/* The header should be written only once
		 * and using the compression used in the file
		 */
		write_count = libewf_section_header_write(
		               internal_handle,
		               file_descriptor,
		               start_offset,
		               internal_handle->header,
		               header_size,
		               internal_handle->compression_level );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write single header section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "header",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append header section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;
	}
	else if( ( internal_handle->format == LIBEWF_FORMAT_ENCASE2 )
	 || ( internal_handle->format == LIBEWF_FORMAT_ENCASE3 )
	 || ( internal_handle->format == LIBEWF_FORMAT_LINEN5 )
	 || ( internal_handle->format == LIBEWF_FORMAT_LINEN6 )
	 || ( internal_handle->format == LIBEWF_FORMAT_FTK ) )
	{
		/* The header should be written twice
		 * the default compression is used
		 */
		write_count = libewf_section_header_write(
		               internal_handle,
		               file_descriptor,
		               start_offset,
		               internal_handle->header,
		               header_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write first header section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "header",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append first header section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;

		write_count = libewf_section_header_write(
		               internal_handle,
		               file_descriptor,
		               start_offset,
		               internal_handle->header,
		               header_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write second header section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "header",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append second header section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;
	}
	else if( ( internal_handle->format == LIBEWF_FORMAT_ENCASE4 )
	 || ( internal_handle->format == LIBEWF_FORMAT_ENCASE5 )
	 || ( internal_handle->format == LIBEWF_FORMAT_ENCASE6 ) )
	{
		if( ( internal_handle->header2 == NULL )
		 && ( internal_handle->header2_size == 0 ) )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid header2.\n",
			 function );

			return( -1 );
		}
		header2_size = internal_handle->header2_size - 2;

		/* The header2 should be written twice
		 * the default compression is used
		 */
		write_count = libewf_section_header2_write(
		               internal_handle,
		               file_descriptor,
		               start_offset,
		               internal_handle->header2,
		               internal_handle->header2_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write first header2 section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "header2",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append first header2 section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;

		write_count = libewf_section_header2_write(
		               internal_handle,
		               file_descriptor,
		               start_offset,
		               internal_handle->header2,
		               internal_handle->header2_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write second header2 section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "header2",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append second header2 section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;

		/* The header should be written once
		 * the default compression is used
		 */
		write_count = libewf_section_header_write(
		               internal_handle,
		               file_descriptor,
		               start_offset,
		               internal_handle->header,
		               header_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write third header section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "header",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append third header section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;
	}
	/* EWFX uses the header and header2 for backwards compatibility
	 */
	else if( internal_handle->format == LIBEWF_FORMAT_EWFX )
	{
		if( ( internal_handle->xheader == NULL )
		 && ( internal_handle->xheader_size == 0 ) )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid xheader.\n",
			 function );

			return( -1 );
		}
		xheader_size = internal_handle->xheader_size;

		if( ( internal_handle->header2 == NULL )
		 && ( internal_handle->header2_size == 0 ) )
		{
			LIBEWF_WARNING_PRINT( "%s: invalid header2.\n",
			 function );

			return( -1 );
		}
		header2_size = internal_handle->header2_size - 2;

		/* The xheader should be written once
		 * the default compression is used
		 */
		write_count = libewf_section_xheader_write(
		               internal_handle,
		               file_descriptor,
		               start_offset,
		               internal_handle->xheader,
		               xheader_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write firts xheader section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "xheader",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append first xheader section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;

		/* The header2 should be written once
		 * the default compression is used
		 */
		write_count = libewf_section_header2_write(
		               internal_handle,
		               file_descriptor,
		               start_offset,
		               internal_handle->header2,
		               internal_handle->header2_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write second header2 section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "header2",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append second header2 section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;

		/* The header should be written once
		 * the default compression is used
		 */
		write_count = libewf_section_header_write(
		               internal_handle,
		               file_descriptor,
		               start_offset,
		               internal_handle->header,
		               header_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write third header section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "header",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append third header section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;
	}
	return( total_write_count );
}

/* Write the necessary sections at the start of the segment file
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_start( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, int file_descriptor, LIBEWF_SECTION_LIST *section_list )
{
	EWF_FILE_HEADER file_header;

	static char *function     = "libewf_segment_file_write_start";
	ssize_t total_write_count = 0;
	ssize_t write_count       = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing segment table.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number.\n",
		 function );

		return( -1 );
	}
	if( segment_number > internal_handle->segment_table->amount )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number, value out of range.\n",
		 function );

		return( -1 );
	}
	if( file_descriptor <= -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	if( section_list == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid section list.\n",
		 function );

		return( -1 );
	}

	if( libewf_common_memcpy( file_header.signature, evf_file_signature, 8 ) == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to set data.\n",
		 function );

		return( -1 );
	}
	if( libewf_endian_revert_16bit( segment_number, file_header.fields_segment ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to revert segment number.\n",
		 function );

		return( -1 );
	}
	file_header.fields_start    = 1;
	file_header.fields_end[ 0 ] = 0;
	file_header.fields_end[ 1 ] = 0;

	write_count = ewf_file_header_write(
	               &file_header,
	               file_descriptor );

	if( write_count == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to write file header to file.\n",
		 function );

		return( -1 );
	}
	total_write_count += write_count;

	if( segment_number == 1 )
	{
		/* Write header section(s)
		 */
		write_count = libewf_segment_file_write_headers(
		               internal_handle,
		               file_descriptor,
		               section_list,
		               (off_t) total_write_count );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write header sections.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		if( internal_handle->ewf_format == EWF_FORMAT_S01 )
		{
			/* Write volume (SMART) section
			 */
			write_count = libewf_section_volume_s01_write(
			               internal_handle,
			               file_descriptor,
			               (off_t) total_write_count );
		}
		else if( internal_handle->ewf_format == EWF_FORMAT_E01 )
		{
			/* Write volume section
			 */
			write_count = libewf_section_volume_e01_write(
			               internal_handle,
			               file_descriptor,
			               (off_t) total_write_count );
		}
		else
		{
			/* Fail safe
			 */
			write_count = -1;
		}
		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write volume section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "volume",
		     (off_t) total_write_count,
		     (off_t) ( total_write_count + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append volume section to section list.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_E01 )
	{
		/* Write data section
		 */
		write_count = libewf_section_data_write(
		               internal_handle,
		               file_descriptor,
		               (off_t) total_write_count );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write data section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "data",
		     (off_t) total_write_count,
		     (off_t) ( total_write_count + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append data section to section list.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;
	}
	return( total_write_count );
}

/* Write the necessary sections at the end of the segment file
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_end( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, LIBEWF_SECTION_LIST *section_list, off_t start_offset, int last_segment_file )
{
	LIBEWF_CHAR *md5_hash_string = NULL;
	static char *function        = "libewf_segment_file_write_end";
	ssize_t total_write_count    = 0;
	ssize_t write_count          = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( section_list == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid section list.\n",
		 function );

		return( -1 );
	}
	if( file_descriptor == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	if( last_segment_file == 0 )
	{
		/* Write next section
		 */
		write_count = libewf_section_last_write(
		               internal_handle,
		               file_descriptor,
		               (EWF_CHAR *) "next",
		               start_offset );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write next section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "next",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append next section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;
	}
	else
	{
		if( internal_handle->calculated_md5_hash != NULL )
		{
			/* Write the hash section
			 */
			write_count = libewf_section_hash_write(
			               internal_handle,
			               file_descriptor,
			               start_offset,
			               internal_handle->calculated_md5_hash );

			if( write_count == -1 )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to write hash section.\n",
				 function );

				return( -1 );
			}
			if( libewf_section_list_append(
			     section_list,
			     (EWF_CHAR *) "hash",
			     start_offset,
			     ( start_offset + write_count ) ) == NULL )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to append hash section to section list.\n",
				 function );

				return( -1 );
			}
			start_offset      += write_count;
			total_write_count += write_count;

			if( libewf_internal_handle_set_stored_md5_hash(
			     internal_handle,
			     internal_handle->calculated_md5_hash ) != 1 )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to set stored MD5 hash in handle.\n",
				 function );

				return( -1 );
			}
			if( internal_handle->format == LIBEWF_FORMAT_EWFX )
			{
				md5_hash_string = (LIBEWF_CHAR *) libewf_common_alloc( LIBEWF_CHAR_SIZE * LIBEWF_STRING_DIGEST_HASH_LENGTH_MD5 );

				if( md5_hash_string == NULL )
				{
					LIBEWF_WARNING_PRINT( "%s: unable to create MD5 hash string.\n",
					 function );

					return( -1 );
				}
				if( libewf_string_copy_from_digest_hash(
				     md5_hash_string,
				     LIBEWF_STRING_DIGEST_HASH_LENGTH_MD5,
				     internal_handle->stored_md5_hash,
				     EWF_DIGEST_HASH_SIZE_MD5 ) != 1 )
				{
					LIBEWF_WARNING_PRINT( "%s: unable to set MD5 hash string.\n",
					 function );

					libewf_common_free( md5_hash_string );

					return( -1 );
				}
				if( libewf_internal_handle_set_hash_value(
				     internal_handle,
				     _S_LIBEWF_CHAR( "MD5" ),
				     md5_hash_string,
				     LIBEWF_STRING_DIGEST_HASH_LENGTH_MD5 ) != 1 )
				{
					LIBEWF_WARNING_PRINT( "%s: unable to set MD5 hash string in hash values.\n",
					 function );

					libewf_common_free( md5_hash_string );

					return( -1 );
				}
				libewf_common_free( md5_hash_string );
			}
		}
		/* Write the xhash section
		 */
		if( internal_handle->format == LIBEWF_FORMAT_EWFX )
		{
			if( internal_handle->xhash != NULL )
			{
				LIBEWF_WARNING_PRINT( "%s: xhash already set - cleaning previous defintion.\n",
				 function );

				libewf_common_free( internal_handle->xhash );
			}
			internal_handle->xhash = libewf_hash_values_generate_xhash_string_ewfx(
			                          internal_handle->hash_values,
			                          &internal_handle->xhash_size );

			if( internal_handle->xhash == NULL )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to generate xhash.\n",
				 function );

				return( -1 );
			}
			write_count = libewf_section_xhash_write(
			               internal_handle,
			               file_descriptor,
			               start_offset,
			               internal_handle->xhash,
			               internal_handle->xhash_size,
			               EWF_COMPRESSION_DEFAULT );

			if( write_count == -1 )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to write xhash section.\n",
				 function );

				return( -1 );
			}
			if( libewf_section_list_append(
			     section_list,
			     (EWF_CHAR *) "xhash",
			     start_offset,
			     ( start_offset + write_count ) ) == NULL )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to append xhash section to section list.\n",
				 function );

				return( -1 );
			}
			start_offset      += write_count;
			total_write_count += write_count;
		}
		/* Write the done section
		 */
		write_count = libewf_section_last_write(
		               internal_handle,
		               file_descriptor,
		               (EWF_CHAR *) "done",
		               start_offset );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write done section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     section_list,
		     (EWF_CHAR *) "done",
		     start_offset,
		     ( start_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append done section to section list.\n",
			 function );

			return( -1 );
		}
		start_offset      += write_count;
		total_write_count += write_count;
	}
	return( total_write_count );
}

/* Write the necessary sections before the actual data chunks to file
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_chunks_section_start( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, size32_t chunk_size, uint32_t total_chunk_amount, uint32_t segment_chunk_amount )
{
	LIBEWF_OFFSET_TABLE *reallocation = NULL;
	static char *function             = "libewf_segment_file_write_chunks_section_start";
	ssize_t write_count               = 0;
	size_t section_size               = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->offset_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing offset table.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing segment table.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_descriptor == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file descriptors.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_offset == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file offsets.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number.\n",
		 function );

		return( -1 );
	}
	if( segment_number > internal_handle->segment_table->amount )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number, value out of range.\n",
		 function );

		return( -1 );
	}

	/* The segment_chunk_amount contains the estimated amount of chunks for this section
	 */
	if( internal_handle->offset_table->amount < ( total_chunk_amount + segment_chunk_amount ) )
	{
		reallocation = libewf_offset_table_realloc(
		                internal_handle->offset_table,
		                ( total_chunk_amount + segment_chunk_amount ) );

		if( reallocation == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to reallocate offset table.\n",
			 function );

			return( -1 );
		}
		internal_handle->offset_table = reallocation;
	}
	if( ( internal_handle->ewf_format == EWF_FORMAT_S01 )
	 || ( internal_handle->format == LIBEWF_FORMAT_ENCASE1 ) )
	{
		/* Write table section start
		 */
		write_count = libewf_section_table_write(
		               internal_handle,
		               internal_handle->segment_table->file_descriptor[ segment_number ],
		               internal_handle->segment_table->file_offset[ segment_number ],
		               internal_handle->offset_table,
		               total_chunk_amount,
		               segment_chunk_amount,
		               (EWF_CHAR *) "table",
		               0 );

		if( write_count == -1 )
		{
			LIBEWF_VERBOSE_PRINT( "%s: unable to write table section.\n",
			 function );

			return( -1 );
		}
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_E01 )
	{
		section_size = segment_chunk_amount 
		             * ( chunk_size + EWF_CRC_SIZE );

		/* Write sectors section start
		 */
		write_count = libewf_section_start_write(
		               internal_handle,
		               internal_handle->segment_table->file_descriptor[ segment_number ],
		               (EWF_CHAR *) "sectors",
		               section_size,
		               internal_handle->segment_table->file_offset[ segment_number ] );

		if( write_count == -1 )
		{
			LIBEWF_VERBOSE_PRINT( "%s: unable to write sectors section.\n",
			 function );

			return( -1 );
		}
	}
	internal_handle->segment_table->file_offset[ segment_number ] += write_count;

	return( write_count );
}

/* Write a chunk of data to a segment file and update the offset table
 * Set write_crc to a non 0 value if the CRC is not provided within the chunk data
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_chunks_data( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, uint32_t chunk, EWF_CHUNK *chunk_data, size_t size, int8_t is_compressed, EWF_CRC *chunk_crc, int8_t write_crc, uint32_t amount_of_chunks )
{
	LIBEWF_OFFSET_TABLE *reallocation = NULL;
#if defined( HAVE_VERBOSE_OUTPUT )
	char *chunk_type                  = NULL;
#endif
	static char *function             = "libewf_segment_file_write_chunks_data";
	ssize_t write_count               = 0;
	ssize_t total_write_count         = 0;
	size_t chunk_size                 = size;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->offset_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing offset table.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing segment table.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_descriptor == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file descriptors.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_offset == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file offsets.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number.\n",
		 function );

		return( -1 );
	}
	if( segment_number > internal_handle->segment_table->amount )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number, value out of range.\n",
		 function );

		return( -1 );
	}
	if( chunk_data == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid chunk data.\n",
		 function );

		return( -1 );
	}
	if( chunk_crc == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid chunk CRC.\n",
		 function );

		return( -1 );
	}
	if( size > (size_t) SSIZE_MAX )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( write_crc != 0 )
	{
		chunk_size += EWF_CRC_SIZE;
	}
	/* Make sure the chunk is available in the offset table
	 */
	if( internal_handle->offset_table->amount < ( chunk + 1 ) )
	{
		reallocation = libewf_offset_table_realloc(
		                internal_handle->offset_table,
		                ( chunk + 1 ) );

		if( reallocation == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to reallocate offset table.\n",
			 function );

			return( -1 );
		}
		internal_handle->offset_table = reallocation;
	}
	/* Set the values in the offset table
	 */
	if( libewf_offset_table_set_values(
	     internal_handle->offset_table,
	     amount_of_chunks,
	     internal_handle->segment_table->file_descriptor[ segment_number ],
	     is_compressed,
	     internal_handle->segment_table->file_offset[ segment_number ],
	     chunk_size,
	     segment_number ) == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to set offset value in offset table.\n",
		 function );

		return( -1 );
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	/* Print a verbose notification
	 */
	if( is_compressed == 0 )
	{
		chunk_type = "UNCOMPRESSED";
	}
	else
	{
		chunk_type = "COMPRESSED";
	}
	LIBEWF_VERBOSE_PRINT( "%s: writing %s chunk: %" PRIu32 " at offset: %jd with size: %zu, with CRC: %" PRIu32 ".\n",
	 function, chunk_type, ( chunk + 1 ), internal_handle->segment_table->file_offset[ segment_number ],
	 chunk_size, *chunk_crc );
#endif

	/* Write the chunk data to the segment file
	 */
	write_count = ewf_chunk_write(
	               chunk_data,
	               internal_handle->segment_table->file_descriptor[ segment_number ],
	               size );

	if( write_count != (ssize_t) size )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to write data.\n",
		 function );

		return( -1 );
	}
	internal_handle->segment_table->file_offset[ segment_number ] += write_count;
	total_write_count                                             += write_count;

	/* Write the CRC if necessary
	 */
	if( write_crc != 0 )
	{
		write_count = ewf_crc_write(
		               chunk_crc,
		               internal_handle->segment_table->file_descriptor[ segment_number ] );

		if( write_count != (ssize_t) EWF_CRC_SIZE )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to write CRC.\n",
			 function );

			return( -1 );
		}
		internal_handle->segment_table->file_offset[ segment_number ] += write_count;
		total_write_count                                             += write_count;
	}
	return( total_write_count );
}

/* Correct the sections before the actual data chunks
 * Also write the necessary sections after the actual data chunks to file (like table and table2 sections for EWF-E01 format)
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_chunks_correction( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, off_t chunks_section_offset, size_t chunks_section_size, uint32_t amount_of_chunks, uint32_t section_amount_of_chunks )
{
	EWF_CHAR *table_section_string = NULL;
	static char *function          = "libewf_segment_file_write_chunks_correction";
	ssize_t total_write_count      = 0;
	ssize_t write_count            = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing segment table.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_descriptor == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file descriptors.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_offset == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file offsets.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->section_list == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing section lists.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number.\n",
		 function );

		return( -1 );
	}
	if( segment_number > internal_handle->segment_table->amount )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number, value out of range.\n",
		 function );

		return( -1 );
	}

	/* Seek the start of the data chunks
	*/
	LIBEWF_VERBOSE_PRINT( "%s: setting file descriptor to start of chunks section offset: %" PRIu32 ".\n",
	 function, chunks_section_offset );

	if( libewf_common_lseek(
	     internal_handle->segment_table->file_descriptor[ segment_number ],
	     chunks_section_offset,
	     SEEK_SET ) == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to find offset to correct sectors size.\n",
		 function );

		return( -1 );
	}
	if( ( internal_handle->ewf_format == EWF_FORMAT_S01 )
	 || ( internal_handle->format == LIBEWF_FORMAT_ENCASE1 ) )
	{
		LIBEWF_VERBOSE_PRINT( "%s: correcting table section size: %zu offset: %jd.\n",
		 function, chunks_section_size, chunks_section_offset );

		/* Rewrite table section start
		 */
		write_count = libewf_section_table_write(
		               internal_handle,
		               internal_handle->segment_table->file_descriptor[ segment_number ],
		               chunks_section_offset,
		               internal_handle->offset_table,
		               ( amount_of_chunks - section_amount_of_chunks ),
		               section_amount_of_chunks,
		               (EWF_CHAR *) "table",
		               chunks_section_size );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to rewrite %" PRIs_EWF " section.\n",
			 function, table_section_string );

			return( -1 );
		}
		if( libewf_section_list_append(
		     internal_handle->segment_table->section_list[ segment_number ],
		     (EWF_CHAR *) "table",
		     chunks_section_offset,
		     ( chunks_section_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append table section to section list.\n",
			 function );

			return( -1 );
		}
	}
	else if( internal_handle->ewf_format == EWF_FORMAT_E01 )
	{
		LIBEWF_VERBOSE_PRINT( "%s: correcting sectors section size: %zu offset: %jd.\n",
		 function, chunks_section_size, chunks_section_offset );

		/* Rewrite sectors section start
		 */
		write_count = libewf_section_start_write(
		               internal_handle,
		               internal_handle->segment_table->file_descriptor[ segment_number ],
		               (EWF_CHAR *) "sectors",
		               chunks_section_size,
		               chunks_section_offset );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to rewrite sectors section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     internal_handle->segment_table->section_list[ segment_number ],
		     (EWF_CHAR *) "sectors",
		     chunks_section_offset,
		     ( chunks_section_offset + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append sectors section to section list.\n",
			 function );

			return( -1 );
		}
	}
	/* Seek the end of the chunks section
	*/
	LIBEWF_VERBOSE_PRINT( "%s: setting file descriptor back to end of data at offset: %" PRIu32 ".\n",
	 function, internal_handle->segment_table->file_offset[ segment_number ] );

	if( libewf_common_lseek(
	     internal_handle->segment_table->file_descriptor[ segment_number ],
	     internal_handle->segment_table->file_offset[ segment_number ],
	     SEEK_SET ) == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to find offset to continue.\n",
		 function );

		return( -1 );
	}
	if( ( internal_handle->ewf_format == EWF_FORMAT_E01 )
	 && ( internal_handle->format != LIBEWF_FORMAT_ENCASE1 ) )
	{
		/* Write table section start
		 */
		write_count = libewf_section_table_write(
		               internal_handle,
		               internal_handle->segment_table->file_descriptor[ segment_number ],
		               internal_handle->segment_table->file_offset[ segment_number ],
		               internal_handle->offset_table,
		               ( amount_of_chunks - section_amount_of_chunks ),
		               section_amount_of_chunks,
		               (EWF_CHAR *) "table",
		               0 );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to rewrite table section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     internal_handle->segment_table->section_list[ segment_number ],
		     (EWF_CHAR *) "table",
		     internal_handle->segment_table->file_offset[ segment_number ],
		     ( internal_handle->segment_table->file_offset[ segment_number ] + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append table section to section list.\n",
			 function );

			return( -1 );
		}
		internal_handle->segment_table->file_offset[ segment_number ] += write_count;
		total_write_count                                             += write_count;

		/* Write table2 section start
		 */
		write_count = libewf_section_table_write(
		               internal_handle,
		               internal_handle->segment_table->file_descriptor[ segment_number ],
		               internal_handle->segment_table->file_offset[ segment_number ],
		               internal_handle->offset_table,
		               ( amount_of_chunks - section_amount_of_chunks ),
		               section_amount_of_chunks,
		               (EWF_CHAR *) "table2",
		               0 );

		if( write_count == -1 )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to rewrite table2 section.\n",
			 function );

			return( -1 );
		}
		if( libewf_section_list_append(
		     internal_handle->segment_table->section_list[ segment_number ],
		     (EWF_CHAR *) "table2",
		     internal_handle->segment_table->file_offset[ segment_number ],
		     ( internal_handle->segment_table->file_offset[ segment_number ] + write_count ) ) == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to append table2 section to section list.\n",
			 function );

			return( -1 );
		}
		internal_handle->segment_table->file_offset[ segment_number ] += write_count;
		total_write_count                                             += write_count;
	}
	return( total_write_count );
}

/* Creates and opens a new segment file and writes the necessary sections at the start of the segment file
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_open( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, int16_t maximum_amount_of_segments )
{
#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
	wchar_t *filename     = NULL;
	wchar_t *error_string = NULL;
#else
	char *filename        = NULL;
	char *error_string    = NULL;
#endif
	static char *function = "libewf_segment_file_write_open";
	ssize_t write_count   = 0;
	int file_descriptor   = -1;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - missing segment table.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing filenames.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_offset == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file offsets.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number: 0.\n",
		 function );

		return( -1 );
	}
	if( segment_number > internal_handle->segment_table->amount )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number, value out of range.\n",
		 function );

		return( -1 );
	}

	/* Create a new segment file
	 */
	if( libewf_segment_file_create_file_entry(
	     internal_handle,
	     segment_number,
	     maximum_amount_of_segments ) == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to create entry for segment file: %" PRIu32 ".\n",
		 function, segment_number );

		return( -1 );
	}
	filename = internal_handle->segment_table->filename[ segment_number ];

	if( filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid filename for segment file: %" PRIu32 ".\n",
		 function, segment_number );

		return( -1 );
	}
#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
	file_descriptor = libewf_common_wide_open( filename, LIBEWF_OPEN_WRITE );

	if( file_descriptor == -1 )
	{
		error_string = libewf_common_wide_strerror( errno );

		if( error_string == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to open file: %ls.\n",
			 function, filename );
		}
		else
		{
			LIBEWF_WARNING_PRINT( "%s: unable to open file: %ls with error: %ls.\n",
			 function, filename, error_string );

			libewf_common_free( error_string );
		}
		return( -1 );
	}
#else
	file_descriptor = libewf_common_open( filename, LIBEWF_OPEN_WRITE );

	if( file_descriptor == -1 )
	{
		error_string = libewf_common_strerror( errno );

		if( error_string == NULL )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to open file: %s.\n",
			 function, filename );
		}
		else
		{
			LIBEWF_WARNING_PRINT( "%s: unable to open file: %s with error: %s.\n",
			 function, filename, error_string );

			libewf_common_free( error_string );
		}
		return( -1 );
	}
#endif
	internal_handle->segment_table->file_descriptor[ segment_number ] = file_descriptor;

	/* Write the start of the segment file
	 * like the file header, the header, volume and/or data section, etc.
	 */
	write_count = libewf_segment_file_write_start(
	               internal_handle,
	               segment_number,
	               file_descriptor,
	               internal_handle->segment_table->section_list[ segment_number ] );

	if( write_count == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to write segment file start.\n",
		 function );

		return( -1 );
	}
	internal_handle->segment_table->file_offset[ segment_number ] += write_count;

	return( write_count );
}

/* Closes the segment file, necessary sections at the end of the segment file will be written
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_close( LIBEWF_INTERNAL_HANDLE *internal_handle, uint16_t segment_number, uint32_t segment_amount_of_chunks, int last_segment_file )
{
	static char *function     = "libewf_segment_file_write_close";
	size_t md5_hash_size      = 0;
	ssize_t total_write_count = 0;
	ssize_t write_count       = 0;

	if( internal_handle == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->filename == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing filenames.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->file_offset == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing file offsets.\n",
		 function );

		return( -1 );
	}
	if( internal_handle->segment_table->section_list == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid handle - invalid segment table - missing section lists.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number: 0.\n",
		 function );

		return( -1 );
	}
	if( segment_number > internal_handle->segment_table->amount )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid segment number, value out of range.\n",
		 function );

		return( -1 );
	}

	if( last_segment_file == 1 )
	{
		md5_hash_size = EWF_DIGEST_HASH_SIZE_MD5;

		if( internal_handle->calculated_md5_hash == NULL )
		{
			internal_handle->calculated_md5_hash = (EWF_DIGEST_HASH *) libewf_common_alloc( md5_hash_size );

			if( internal_handle->calculated_md5_hash == NULL )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to create MD5 hash.\n",
				 function );

				return( -1 );
			}
		}
		/* Complete the MD5 hash calculation
		 */
		if( ( libewf_md5_finalize( &internal_handle->md5_context, internal_handle->calculated_md5_hash, &md5_hash_size ) != 1 )
		 || ( md5_hash_size != EWF_DIGEST_HASH_SIZE_MD5 ) )
		{
			LIBEWF_WARNING_PRINT( "%s: unable to finalize MD5 context.\n",
			 function );

			return( -1 );
		}
		/* Write the data section for a single segment file only for EWF-E01
		 */
		if( ( internal_handle->ewf_format == EWF_FORMAT_E01 ) && ( segment_number == 1 ) )
		{
			write_count = libewf_section_data_write(
			               internal_handle,
			               internal_handle->segment_table->file_descriptor[ segment_number ],
			               internal_handle->segment_table->file_offset[ segment_number ] );

			if( write_count == -1 )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to write data section.\n",
				 function );

				return( -1 );
			}
			if( libewf_section_list_append(
			     internal_handle->segment_table->section_list[ segment_number ],
			     (EWF_CHAR *) "data",
			     internal_handle->segment_table->file_offset[ segment_number ],
			     ( internal_handle->segment_table->file_offset[ segment_number ] + write_count ) ) == NULL )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to append data section to section list.\n",
				 function );

				return( -1 );
			}
			internal_handle->segment_table->file_offset[ segment_number ] += write_count;
			total_write_count                                             += write_count;
		}
		/* Write the error2 section if required 
		 */
		if( ( internal_handle->acquiry_amount_of_errors > 0 )
		 && ( ( internal_handle->format == LIBEWF_FORMAT_ENCASE3 )
		  || ( internal_handle->format == LIBEWF_FORMAT_ENCASE4 )
		  || ( internal_handle->format == LIBEWF_FORMAT_ENCASE5 )
		  || ( internal_handle->format == LIBEWF_FORMAT_ENCASE6 )
		  || ( internal_handle->format == LIBEWF_FORMAT_LINEN5 )
		  || ( internal_handle->format == LIBEWF_FORMAT_LINEN6 )
		  || ( internal_handle->format == LIBEWF_FORMAT_EWFX ) ) )
		{
			write_count = libewf_section_error2_write(
			               internal_handle,
			               internal_handle->segment_table->file_descriptor[ segment_number ],
			               internal_handle->segment_table->file_offset[ segment_number ],
			               internal_handle->acquiry_error_sectors,
			               internal_handle->acquiry_amount_of_errors );

			if( write_count == -1 )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to write error2 section.\n",
				 function );

				return( -1 );
			}
			if( libewf_section_list_append(
			     internal_handle->segment_table->section_list[ segment_number ],
			     (EWF_CHAR *) "error2",
			     internal_handle->segment_table->file_offset[ segment_number ],
			     ( internal_handle->segment_table->file_offset[ segment_number ] + write_count ) ) == NULL )
			{
				LIBEWF_WARNING_PRINT( "%s: unable to append error2 section to section list.\n",
				 function );

				return( -1 );
			}
			internal_handle->segment_table->file_offset[ segment_number ] += write_count;
			total_write_count                                             += write_count;
		}
	}
	/* Write the end of the segment file
	 * like the next, done and/or hash section
	 */
	write_count = libewf_segment_file_write_end(
	               internal_handle,
	               internal_handle->segment_table->file_descriptor[ segment_number ],
	               internal_handle->segment_table->section_list[ segment_number ],
	               internal_handle->segment_table->file_offset[ segment_number ],
	               last_segment_file );

	if( write_count == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to write end of segment file.\n",
		 function );

		return( -1 );
	}
	internal_handle->segment_table->file_offset[ segment_number ] += write_count;
	total_write_count                                             += write_count;

	if( libewf_common_close( internal_handle->segment_table->file_descriptor[ segment_number ] ) != 0 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to close segment file.\n",
		 function );

		return( -1 );
	}
	internal_handle->segment_table->file_descriptor[ segment_number ]  = -1;
	internal_handle->segment_table->amount_of_chunks[ segment_number ] = segment_amount_of_chunks;

	return( total_write_count );
}

