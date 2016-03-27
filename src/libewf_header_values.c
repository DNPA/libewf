/*
 * libewf header values
 *
 * Copyright (c) 2006, Joachim Metz <forensics@hoffmannbv.nl>,
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
 * - All advertising materials mentioning features or use of this software
 *   must acknowledge the contribution by people stated in the acknowledgements.
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

#include <inttypes.h>

#include "ewf_compress.h"
#include "libewf_definitions.h"
#include "libewf_common.h"
#include "libewf_header_values.h"
#include "libewf_notify.h"

/* Allocates memory for a new header values struct
 * Return a pointer to the new instance, NULL on error
 */
LIBEWF_HEADER_VALUES *libewf_header_values_alloc( void )
{
	LIBEWF_HEADER_VALUES *header_values = NULL;

	header_values = (LIBEWF_HEADER_VALUES *) libewf_alloc_cleared( LIBEWF_HEADER_VALUES_SIZE );

	if( header_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_alloc: unable to allocate libewf_header_values.\n" );

		return( NULL );
	}
	header_values->case_number              = NULL;
	header_values->description              = NULL;
	header_values->examiner_name            = NULL;
	header_values->evidence_number          = NULL;
	header_values->notes                    = NULL;
	header_values->acquiry_date             = NULL;
	header_values->system_date              = NULL;
	header_values->acquiry_operating_system = NULL;
	header_values->acquiry_software_version = NULL;
	header_values->password                 = NULL;
	header_values->compression_type         = NULL;
	header_values->unknown_dc               = NULL;

	return( header_values );
}

/* Allocates memory for a header value string
 * Return a pointer to the new instance, NULL on error
 */
char *libewf_header_value_string_alloc( uint32_t amount )
{
	char *header_value_string = NULL;

	header_value_string = (char *) libewf_alloc_cleared( amount * sizeof( char ) );

	if( header_value_string == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_value_string_alloc: unable to allocate header value string.\n" );

		return( NULL );
	}
	return( header_value_string );
}

/* Frees memory of a header values struct including elements
 */
void libewf_header_values_free( LIBEWF_HEADER_VALUES *header_values )
{
	if( header_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_free: invalid header values.\n" );

		return;
	}
	if( header_values->case_number != NULL )
	{
		libewf_free( header_values->case_number );
	}
	if( header_values->description != NULL )
	{
		libewf_free( header_values->description );
	}
	if( header_values->examiner_name != NULL )
	{
		libewf_free( header_values->examiner_name );
	}
	if( header_values->evidence_number != NULL )
	{
		libewf_free( header_values->evidence_number );
	}
	if( header_values->notes != NULL )
	{
		libewf_free( header_values->notes );
	}
	if( header_values->acquiry_date != NULL )
	{
		libewf_free( header_values->acquiry_date );
	}
	if( header_values->system_date != NULL )
	{
		libewf_free( header_values->system_date );
	}
	if( header_values->acquiry_operating_system != NULL )
	{
		libewf_free( header_values->acquiry_operating_system );
	}
	if( header_values->acquiry_software_version != NULL )
	{
		libewf_free( header_values->acquiry_software_version );
	}
	if( header_values->password != NULL )
	{
		libewf_free( header_values->password );
	}
	if( header_values->compression_type != NULL )
	{
		libewf_free( header_values->compression_type );
	}
	if( header_values->unknown_dc != NULL )
	{
		libewf_free( header_values->unknown_dc );
	}
	libewf_free( header_values );
}

/* Split a string into elements using a delimiter character
 * Return a pointer to the new instance, NULL on error
 */
char **libewf_split_string( char *string, int delimiter, uint32_t *amount )
{
	char **lines         = NULL;
	char **data_set      = NULL;
	char *line_start     = NULL;
	char *line_end       = NULL;
	char *string_end     = NULL;
	uint32_t iterator    = 0;
	uint32_t line_size   = 0;
	uint32_t string_size = 0;

	if( string == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_split_string: invalid string.\n" );

		return( NULL );
	}
	if( amount == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_split_string: invalid amount.\n" );

		return( NULL );
	}
	string_size = libewf_strlen( string );
	line_start  = string;
	line_end    = string;
	string_end  = &string[ string_size ];

	while( 1 )
	{
		line_end = libewf_strchr( line_start, delimiter );

		iterator++;

		if( line_end == NULL )
		{
			break;
		}
		if( line_end == line_start )
		{
			line_start += 1;
		}
		else if( line_end != string )
		{
			line_start = line_end + 1;
		}
	}
	*amount = iterator;
	lines   = (char **) libewf_alloc_cleared( *amount * sizeof( char * ) );

	if( lines == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_split_string: unable to allocate dynamic array lines.\n" );

		return( NULL );
	}
	line_start = string;
	line_end   = string;

	for( iterator = 0; iterator < *amount; iterator++ )
	{
		if( line_end != string )
		{
			line_start = line_end + 1;
		}
		line_end = libewf_strchr( line_start, delimiter );

		/* Check for last value
		 */
		if( line_end == NULL )
		{
			line_size = string_end - line_start;
		}
		else
		{
			line_size = line_end - line_start;
		}
		lines[ iterator ] = libewf_header_value_string_alloc( line_size + 1 );

		if( lines[ iterator ] == NULL )
		{
			LIBEWF_WARNING_PRINT( "libewf_split_string: unable to allocate line string.\n" );

			libewf_split_values_free( lines, ( iterator - 1 ) );

			return( NULL );
		}
		if( line_size > 0 )
		{
			data_set = (char **) libewf_memcpy( (char *) lines[ iterator ], (char *) line_start, line_size );

			if( data_set == NULL )
			{
				LIBEWF_WARNING_PRINT( "libewf_split_string: unable to set dynamic array lines.\n" );

				libewf_split_values_free( lines, iterator );

				return( NULL );
			}
		}
		lines[ iterator ][ line_size ] = '\0';

		/* Correct if first value is empty
		 */
		if( line_end == string )
		{
			line_start += 1;
		}
	}
	return( lines );
}

/* Clears a split values array
 */
void libewf_split_values_free( char **split_values, uint32_t amount )
{
	uint32_t iterator = 0;

	if( split_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_split_values_free: invalid split values array.\n" );

		return;
	}
	for( iterator = 0; iterator < amount; iterator++ )
	{
		libewf_free( split_values[ iterator ] );
	}
	libewf_free( split_values );
}

/* Convert date string within a header value
 * Return a pointer to the new instance, NULL on error
 */
char *libewf_convert_date_header_value( char *header_value, uint8_t date_format )
{
	char **date_elements           = NULL;
	char *date_string              = NULL;
	uint32_t date_element_count    = 0;
	uint32_t date_element_iterator = 0;

	if( header_value == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header_value: invalid header value.\n" );

		return( NULL );
	}
	if( ( date_format != LIBEWF_DATEFORMAT_DAYMONTH ) && ( date_format != LIBEWF_DATEFORMAT_MONTHDAY ) )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header_value: unsupported date format.\n" );

		return( NULL );
	}
	date_string = libewf_header_value_string_alloc( 20 );

	if( date_string == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header_value: unable to create date string.\n" );

		return( NULL );
	}
	date_elements = libewf_split_string( header_value, ' ', &date_element_count );

	if( date_elements == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header_value: unable to split date elements in header value.\n" );

		libewf_free( date_string );

		return( NULL );
	}
	if( date_element_count != 6 )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header_value: unsupported amount of date elements in header value.\n" );

		libewf_free( date_string );

		return( NULL );
	}
	if( date_format == LIBEWF_DATEFORMAT_MONTHDAY )
	{
		if( libewf_strlen( date_elements[ 1 ] ) == 1 )
		{
			date_string[ 0 ] = '0';
			date_string[ 1 ] = date_elements[ 1 ][ 0 ];
		}
		else
		{
			date_string[ 0 ] = date_elements[ 1 ][ 0 ];
			date_string[ 1 ] = date_elements[ 1 ][ 1 ];
		}
		date_string[ 2 ] = '/';

		if( libewf_strlen( date_elements[ 2 ] ) == 1 )
		{
			date_string[ 3 ] = '0';
			date_string[ 4 ] = date_elements[ 2 ][ 0 ];
		}
		else
		{
			date_string[ 3 ] = date_elements[ 2 ][ 0 ];
			date_string[ 4 ] = date_elements[ 2 ][ 1 ];
		}
	}
	else if( date_format == LIBEWF_DATEFORMAT_DAYMONTH )
	{
		if( libewf_strlen( date_elements[ 2 ] ) == 1 )
		{
			date_string[ 0 ] = '0';
			date_string[ 1 ] = date_elements[ 2 ][ 0 ];
		}
		else
		{
			date_string[ 0 ] = date_elements[ 2 ][ 0 ];
			date_string[ 1 ] = date_elements[ 2 ][ 1 ];
		}
		date_string[ 2 ] = '/';

		if( libewf_strlen( date_elements[ 1 ] ) == 1 )
		{
			date_string[ 3 ] = '0';
			date_string[ 4 ] = date_elements[ 1 ][ 0 ];
		}
		else
		{
			date_string[ 3 ] = date_elements[ 1 ][ 0 ];
			date_string[ 4 ] = date_elements[ 1 ][ 1 ];
		}
	}
	date_string[ 5  ] = '/';
	date_string[ 6  ] = date_elements[ 0 ][ 0 ];
	date_string[ 7  ] = date_elements[ 0 ][ 1 ];
	date_string[ 8  ] = date_elements[ 0 ][ 2 ];
	date_string[ 9  ] = date_elements[ 0 ][ 3 ];
	date_string[ 10 ] = ' ';

	if( libewf_strlen( date_elements[ 3 ] ) == 1 )
	{
		date_string[ 11 ] = '0';
		date_string[ 12 ] = date_elements[ 3 ][ 0 ];
	}
	else
	{
		date_string[ 11 ] = date_elements[ 3 ][ 0 ];
		date_string[ 12 ] = date_elements[ 3 ][ 1 ];
	}
	date_string[ 13 ] = ':';

	if( libewf_strlen( date_elements[ 4 ] ) == 1 )
	{
		date_string[ 14 ] = '0';
		date_string[ 15 ] = date_elements[ 4 ][ 0 ];
	}
	else
	{
		date_string[ 14 ] = date_elements[ 4 ][ 0 ];
		date_string[ 15 ] = date_elements[ 4 ][ 1 ];
	}
	date_string[ 16 ] = ':';

	if( libewf_strlen( date_elements[ 5 ] ) == 1 )
	{
		date_string[ 17 ] = '0';
		date_string[ 18 ] = date_elements[ 5 ][ 0 ];
	}
	else
	{
		date_string[ 17 ] = date_elements[ 5 ][ 0 ];
		date_string[ 18 ] = date_elements[ 5 ][ 1 ];
	}
	date_string[ 19 ] = '\0';

	libewf_split_values_free( date_elements, date_element_count );

	return( date_string );
}

/* Generate date string within a header value
 * Return a pointer to the new instance, NULL on error
 */
char *libewf_generate_date_header_value( time_t timestamp )
{
	char *date_string        = NULL;
	struct tm *time_elements = NULL;

	time_elements = localtime( &timestamp );

	if( time_elements == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_generate_date_header_value: unable to create time elements.\n" );

		return( NULL );
	}
	date_string = libewf_header_value_string_alloc( 20 );

	if( date_string == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_generate_date_header_value: unable to create date string.\n" );

		return( NULL );
	}
	if( snprintf( date_string, 20, "%4d %d %d %d %d %d",
		( time_elements->tm_year + 1900 ), ( time_elements->tm_mon + 1 ), time_elements->tm_mday,
		time_elements->tm_hour, time_elements->tm_min, time_elements->tm_sec ) <= -1 )
	{
		LIBEWF_WARNING_PRINT( "libewf_generate_date_header_value: unable to fill date string.\n" );

		libewf_free( date_string );

		return( NULL );
	}
	/* Make sure the string is terminated correctly
	 */
	date_string[ 19 ] = '\0';

	return( date_string );
}

/* Convert date string within a header2 value
 * Return a pointer to the new instance, NULL on error
 */
char *libewf_convert_date_header2_value( char *header_value, uint8_t date_format )
{
	time_t timestamp;

	struct tm *time_elements = NULL;
	char *date_string        = NULL;
	uint32_t result          = 0;

	if( header_value == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header2_value: invalid header value.\n" );

		return( NULL );
	}
	if( ( date_format != LIBEWF_DATEFORMAT_DAYMONTH ) && ( date_format != LIBEWF_DATEFORMAT_MONTHDAY ) )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header2_value: unsupported date format.\n" );

		return( NULL );
	}
	timestamp     = atoll( header_value );
	time_elements = localtime( &timestamp );

	if( time_elements == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header2_value: unable to create time elements.\n" );

		return( NULL );
	}
	date_string = libewf_header_value_string_alloc( 20 );

	if( date_string == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header2_value: unable to create date string.\n" );

		return( NULL );
	}
	if( date_format == LIBEWF_DATEFORMAT_MONTHDAY )
	{
		result = strftime( date_string, 20, "%m/%d/%Y %H:%M:%S", time_elements );
	}
	else if( date_format == LIBEWF_DATEFORMAT_DAYMONTH )
	{
		result = strftime( date_string, 20, "%d/%m/%Y %H:%M:%S", time_elements );
	}
	if( result == 0 )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header2_value: unable to fill date string.\n" );

		libewf_free( date_string );

		return( NULL );
	}
	return( date_string );
}

/* Generate date string within a header2 value
 * Return a pointer to the new instance, NULL on error
 */
char *libewf_generate_date_header2_value( time_t timestamp )
{
	char *date_string = NULL;

	date_string = libewf_header_value_string_alloc( 11 );

	if( date_string == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_generate_date_header2_value: unable to create date string.\n" );

		return( NULL );
	}
	if( snprintf( date_string, 11, "%" PRIu32, (uint32_t) timestamp ) <= -1 )
	{
		LIBEWF_WARNING_PRINT( "libewf_generate_date_header2_value: unable to create date string.\n" );

		libewf_free( date_string );

		return( NULL );
	}
	/* Make sure the string is terminated correctly
	 */
	date_string[ 10 ] = '\0';

	return( date_string );
}

/* Set a header value
 * Return a pointer to the new instance, NULL on error
 */
char *libewf_header_values_set_value( char* header_value, char *value )
{
	char *data_set       = NULL;
	uint32_t string_size = 0;

	/* Clear the buffer of the previous header value
	 */
	if( header_value != NULL )
	{
		libewf_free( header_value );
	}
	string_size = libewf_strlen( value );

	/* Don't bother with empty values
	 */
	if( string_size <= 0 )
	{
		return( NULL );
	}
	/* Add 1 additional byte required for the end of string character
	 */
	header_value = libewf_header_value_string_alloc( string_size + 1 );

	if( header_value == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_set_value: unable to create header value.\n" );

		return( NULL );
	}
	data_set = (char *) libewf_memcpy( (void *) header_value, (void *) value, string_size );

	if( data_set == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_set_value: unable to set header value.\n" );

		libewf_free( header_value );

		return( NULL );
	}
	/* Make sure the string is terminated correctly
	 */
	header_value[ string_size ] = '\0';

	return( header_value );
}

/* Parse an EWF header for the values
 * Return a pointer to the new instance, NULL on error
 */
LIBEWF_HEADER_VALUES *libewf_header_values_parse_header( EWF_HEADER *header, uint8_t date_format )
{
	LIBEWF_HEADER_VALUES *header_values = NULL;
	char **lines                        = NULL;
	char **types                        = NULL;
	char **values                       = NULL;
	char *date_string                   = NULL;
	uint32_t line_count                 = 0;
	uint32_t type_count                 = 0;
	uint32_t value_count                = 0;
	uint32_t iterator                   = 0;
	uint32_t hash_length                = 0;

	if( header == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_parse_header: invalid header value.\n" );

		return( NULL );
	}
	lines = libewf_split_string( (char *) header, '\n', &line_count );

	if( lines == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header_value: unable to split lines in header.\n" );

		return( NULL );
	}
	types = libewf_split_string( lines[ 2 ], '\t', &type_count );

	if( types == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header_value: unable to split types in header line.\n" );

		libewf_split_values_free( lines, line_count );

		return( NULL );
	}
	values = libewf_split_string( lines[ 3 ], '\t', &value_count );

	if( values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_convert_date_header_value: unable to split values in header line.\n" );

		libewf_split_values_free( lines, line_count );
		libewf_split_values_free( types, type_count );

		return( NULL );
	}
	libewf_split_values_free( lines, line_count );

	header_values = libewf_header_values_alloc();

	if( header_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_parse_header: unable to create header values.\n" );

		libewf_split_values_free( types, type_count );
		libewf_split_values_free( values, value_count );

		return( NULL );
	}
	for( iterator = 0; iterator < type_count; iterator++ )
	{
		if( libewf_strncmp( types[ iterator ], "av", 2 ) == 0 )
		{
			header_values->acquiry_software_version = libewf_header_values_set_value( header_values->acquiry_software_version, values[ iterator ] );
		}
		else if( libewf_strncmp( types[ iterator ], "ov", 2 ) == 0 )
		{
			header_values->acquiry_operating_system = libewf_header_values_set_value( header_values->acquiry_operating_system, values[ iterator ] );
		}
		else if( libewf_strncmp( types[ iterator ], "dc", 2 ) == 0 )
		{
			header_values->unknown_dc = libewf_header_values_set_value( header_values->unknown_dc, values[ iterator ] );
		}
		else if( ( libewf_strncmp( types[ iterator ], "m", 1 ) == 0 ) || ( libewf_strncmp( types[ iterator ], "u", 1 ) == 0 ) )
		{
			/* If the date string contains spaces it's in the old header
			 * format otherwise is in new header2 format
			 */
			if( libewf_strchr( values[ iterator ] , ' ' ) != NULL )
			{
				date_string = libewf_convert_date_header_value( values[ iterator ], date_format );
			}
			else
			{
				date_string = libewf_convert_date_header2_value( values[ iterator ], date_format );
			}
			if( date_string == NULL )
			{
				LIBEWF_WARNING_PRINT( "libewf_header_values_parse_header: unable to create date string.\n" );
			}
			else
			{
				if( libewf_strncmp( types[ iterator ], "m", 1 ) == 0 )
				{
					header_values->acquiry_date = libewf_header_values_set_value( header_values->acquiry_date, date_string );
				}
				else if( libewf_strncmp( types[ iterator ], "u", 1 ) == 0 )
				{
					header_values->system_date = libewf_header_values_set_value( header_values->system_date, date_string );
				}
				libewf_free( date_string );
			}
		}
		else if( libewf_strncmp( types[ iterator ], "p", 1 ) == 0 )
		{
			hash_length = libewf_strlen( values[ iterator ] );

			if( hash_length <= 0 )
			{
				/* Empty hash do nothing
				 */
			}
			else if( ( hash_length == 1 ) && ( values[ iterator ][ 0 ] == '0' ) )
			{
				/* Empty hash do nothing
				 */
			}
			else
			{
				header_values->password = libewf_header_values_set_value( header_values->password, values[ iterator ] );
			}
		}
		else if( libewf_strncmp( types[ iterator ], "a", 1 ) == 0 )
		{
			header_values->description = libewf_header_values_set_value( header_values->description, values[ iterator ] );
		}
		else if( libewf_strncmp( types[ iterator ], "c", 1 ) == 0 )
		{
			header_values->case_number = libewf_header_values_set_value( header_values->case_number, values[ iterator ] );
		}
		else if( libewf_strncmp( types[ iterator ], "n", 1 ) == 0 )
		{
			header_values->evidence_number = libewf_header_values_set_value( header_values->evidence_number, values[ iterator ] );
		}
		else if( libewf_strncmp( types[ iterator ], "e", 1 ) == 0 )
		{
			header_values->examiner_name = libewf_header_values_set_value( header_values->examiner_name, values[ iterator ] );
		}
		else if( libewf_strncmp( types[ iterator ], "t", 1 ) == 0 )
		{
			header_values->notes = libewf_header_values_set_value( header_values->notes, values[ iterator ] );
		}
		else if( libewf_strncmp( types[ iterator ], "r", 1 ) == 0 )
		{
			header_values->compression_type = libewf_header_values_set_value( header_values->compression_type, values[ iterator ] );
		}
		else
		{
			LIBEWF_WARNING_PRINT( "libewf_header_values_parse_header: unsupported type: %d with value: %s.\n", types[ iterator ], values[ iterator ] );
		}
	}
	libewf_split_values_free( types, type_count );
	libewf_split_values_free( values, value_count );

	return( header_values );
}

/* Print the header values to a stream
 */
void libewf_header_values_fprint( FILE *stream, LIBEWF_HEADER_VALUES *header_values )
{
	if( stream == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_fprint: invalid stream.\n" );

		return;
	}
	if( header_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_fprint: invalid header values.\n" );

		return;
	}
	if( header_values->case_number != NULL )
	{
		fprintf( stream, "\tCase number:\t\t%s\n", header_values->case_number );
	}
	if( header_values->description != NULL )
	{
		fprintf( stream, "\tDescription:\t\t%s\n", header_values->description );
	}
	if( header_values->examiner_name != NULL )
	{
		fprintf( stream, "\tExaminer name:\t\t%s\n", header_values->examiner_name );
	}
	if( header_values->evidence_number != NULL )
	{
		fprintf( stream, "\tEvidence number:\t%s\n", header_values->evidence_number );
	}
	if( header_values->notes != NULL )
	{
		fprintf( stream, "\tNotes:\t\t\t%s\n", header_values->notes );
	}
	if( header_values->acquiry_date != NULL )
	{
		fprintf( stream, "\tAcquiry date:\t\t%s\n", header_values->acquiry_date );
	}
	if( header_values->system_date != NULL )
	{
		fprintf( stream, "\tSystem date:\t\t%s\n", header_values->system_date );
	}
	if( header_values->acquiry_operating_system != NULL )
	{
		fprintf( stream, "\tOperating system used:\t%s\n", header_values->acquiry_operating_system );
	}
	if( header_values->acquiry_software_version != NULL )
	{
		fprintf( stream, "\tSoftware used:\t\t%s\n", header_values->acquiry_software_version );
	}
	if( header_values->password != NULL )
	{
		fprintf( stream, "\tPassword:\t\t(hash: %s)\n", header_values->password );
	}
	else
	{
		fprintf( stream, "\tPassword:\t\tN/A\n" );
	}
	if( header_values->compression_type != NULL )
	{
		if( libewf_strncmp( header_values->compression_type, LIBEWF_COMPRESSIONTYPE_NONE, 1 ) == 0 )
		{
			fprintf( stream, "\tCompression type:\tno compression\n" );
		}
		else if( libewf_strncmp( header_values->compression_type, LIBEWF_COMPRESSIONTYPE_FAST, 1 ) == 0 )
		{
			fprintf( stream, "\tCompression type:\tgood (fast) compression\n" );
		}
		else if( libewf_strncmp( header_values->compression_type, LIBEWF_COMPRESSIONTYPE_BEST, 1 ) == 0 )
		{
			fprintf( stream, "\tCompression type:\tbest compression\n" );
		}
		else
		{
			fprintf( stream, "\tCompression type:\tunknown compression\n" );
		}
	}
	if( header_values->unknown_dc != NULL )
	{
		fprintf( stream, "\tUnknown value dc:\t%s\n", header_values->unknown_dc );
	}
}

/* Generate an Encase3 header
 * Return a pointer to the new instance, NULL on error
 */
EWF_HEADER *libewf_header_values_generate_header_string_encase3( LIBEWF_HEADER_VALUES *header_values, time_t timestamp, uint8_t compression_level )
{
	EWF_HEADER *header             = NULL;
	char *header_string_head       = "1\r\nmain\r\nc\tn\ta\te\tt\tav\tov\tm\tu\tp\tr\r\n";
	char *header_string_tail       = "\r\n\r\n";
	char *case_number              = "";
	char *description              = "";
	char *examiner_name            = "";
	char *evidence_number          = "";
	char *notes                    = "";
	char *system_date              = "";
	char *acquiry_date             = "";
	char *acquiry_operating_system = "";
	char *acquiry_software_version = "";
	char *password_hash            = "";
	char *compression_type         = "";
	uint32_t size                  = 0;

	if( header_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_encase3: invalid header values.\n" );

		return( NULL );
	}
	if( ( compression_level != EWF_COMPRESSION_NONE ) && ( compression_level != EWF_COMPRESSION_FAST ) && ( compression_level != EWF_COMPRESSION_BEST ) )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_encase3: compression level not supported.\n" );

		return( NULL );
	}
	size = libewf_strlen( header_string_head );

	if( header_values->case_number != NULL )
	{
		size       += libewf_strlen( header_values->case_number );
		case_number = header_values->case_number;
	}
	if( header_values->description != NULL )
	{
		size       += libewf_strlen( header_values->description );
		description = header_values->description;
	}
	if( header_values->examiner_name != NULL )
	{
		size         += libewf_strlen( header_values->examiner_name );
		examiner_name = header_values->examiner_name;
	}
	if( header_values->evidence_number != NULL )
	{
		size           += libewf_strlen( header_values->evidence_number );
		evidence_number = header_values->evidence_number;
	}
	if( header_values->notes != NULL )
	{
		size += libewf_strlen( header_values->notes );
		notes = header_values->notes;
	}
	if( header_values->acquiry_date != NULL )
	{
		size        += libewf_strlen( header_values->acquiry_date );
		acquiry_date = header_values->acquiry_date;
	}
	else
	{
		acquiry_date = libewf_generate_date_header_value( timestamp );
		size        += libewf_strlen( acquiry_date );
	}
	if( header_values->system_date != NULL )
	{
		size       += libewf_strlen( header_values->system_date );
		system_date = header_values->system_date;
	}
	else
	{
		system_date = libewf_generate_date_header_value( timestamp );
		size       += libewf_strlen( system_date );
	}
	if( header_values->acquiry_operating_system != NULL )
	{
		size                    += libewf_strlen( header_values->acquiry_operating_system );
		acquiry_operating_system = header_values->acquiry_operating_system;
	}
	if( header_values->acquiry_software_version != NULL )
	{
		size                    += libewf_strlen( header_values->acquiry_software_version );
		acquiry_software_version = header_values->acquiry_software_version;
	}
	if( header_values->password != NULL )
	{
		size         += libewf_strlen( header_values->password );
		password_hash = header_values->password;
	}
	else
	{
		size         += 1;
		password_hash = "0";
	}
	if( header_values->compression_type != NULL )
	{
		size            += libewf_strlen( header_values->compression_type );
		compression_type = header_values->compression_type;
	}
	else
	{
		if( compression_level == EWF_COMPRESSION_NONE )
		{
			compression_type = LIBEWF_COMPRESSIONTYPE_NONE;
		}
		else if( compression_level == EWF_COMPRESSION_FAST )
		{
			compression_type = LIBEWF_COMPRESSIONTYPE_FAST;
		}
		else if( compression_level == EWF_COMPRESSION_BEST )
		{
			compression_type = LIBEWF_COMPRESSIONTYPE_BEST;
		}
		size += libewf_strlen( compression_type );
	}
	size += libewf_strlen( header_string_tail );

	/* allow for 10x \t and 1x \0
	 */
	size += 11;

	header = ewf_header_alloc( size );

	if( header == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_encase3: unable to create header.\n" );

		return( NULL );
	}
	if( snprintf( (char *) header, size, "%s%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s%s",
		header_string_head, case_number, evidence_number, description, examiner_name,
		notes, acquiry_software_version, acquiry_operating_system, acquiry_date,
		system_date, password_hash, compression_type, header_string_tail ) <= -1 )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_encase3: unable to set header.\n" );

		ewf_header_free( header );

		if( header_values->acquiry_date == NULL )
		{
			libewf_free( acquiry_date );
		}
		if( header_values->system_date == NULL )
		{
			libewf_free( system_date );
		}
		return( NULL );
	}
	/* Make sure the string is terminated
	 */
	header[ size - 1 ] = '\0';

	if( header_values->acquiry_date == NULL )
	{
		libewf_free( acquiry_date );
	}
	if( header_values->system_date == NULL )
	{
		libewf_free( system_date );
	}
	return( header );
}

/* Generate an Encase4 header
 * Return a pointer to the new instance, NULL on error
 */
EWF_HEADER *libewf_header_values_generate_header_string_encase4( LIBEWF_HEADER_VALUES *header_values, time_t timestamp )
{
	EWF_HEADER *header             = NULL;
	char *header_string_head       = "1\r\nmain\r\nc\tn\ta\te\tt\tav\tov\tm\tu\tp\r\n";
	char *header_string_tail       = "\r\n\r\n";
	char *case_number              = "";
	char *description              = "";
	char *examiner_name            = "";
	char *evidence_number          = "";
	char *notes                    = "";
	char *system_date              = "";
	char *acquiry_date             = "";
	char *acquiry_operating_system = "";
	char *acquiry_software_version = "";
	char *password_hash            = "";
	uint32_t size                  = 0;

	if( header_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_encase4: invalid header values.\n" );

		return( NULL );
	}
	size = libewf_strlen( header_string_head );

	if( header_values->case_number != NULL )
	{
		size       += libewf_strlen( header_values->case_number );
		case_number = header_values->case_number;
	}
	if( header_values->description != NULL )
	{
		size       += libewf_strlen( header_values->description );
		description = header_values->description;
	}
	if( header_values->examiner_name != NULL )
	{
		size         += libewf_strlen( header_values->examiner_name );
		examiner_name = header_values->examiner_name;
	}
	if( header_values->evidence_number != NULL )
	{
		size           += libewf_strlen( header_values->evidence_number );
		evidence_number = header_values->evidence_number;
	}
	if( header_values->notes != NULL )
	{
		size += libewf_strlen( header_values->notes );
		notes = header_values->notes;
	}
	if( header_values->acquiry_date != NULL )
	{
		size        += libewf_strlen( header_values->acquiry_date );
		acquiry_date = header_values->acquiry_date;
	}
	else
	{
		acquiry_date = libewf_generate_date_header_value( timestamp );
		size        += libewf_strlen( acquiry_date );
	}
	if( header_values->system_date != NULL )
	{
		size       += libewf_strlen( header_values->system_date );
		system_date = header_values->system_date;
	}
	else
	{
		system_date = libewf_generate_date_header_value( timestamp );
		size       += libewf_strlen( system_date );
	}
	if( header_values->acquiry_operating_system != NULL )
	{
		size                    += libewf_strlen( header_values->acquiry_operating_system );
		acquiry_operating_system = header_values->acquiry_operating_system;
	}
	if( header_values->acquiry_software_version != NULL )
	{
		size                    += libewf_strlen( header_values->acquiry_software_version );
		acquiry_software_version = header_values->acquiry_software_version;
	}
	if( header_values->password != NULL )
	{
		size         += libewf_strlen( header_values->password );
		password_hash = header_values->password;
	}
	else
	{
		size         += 1;
		password_hash = "0";
	}
	size += libewf_strlen( header_string_tail );

	/* allow for 9x \t and 1x \0
	 */
	size += 10;

	header = ewf_header_alloc( size );

	if( header == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_encase4: unable to create header.\n" );

		return( NULL );
	}
	if( snprintf( (char *) header, size, "%s%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s%s",
		header_string_head, case_number, evidence_number, description, examiner_name,
		notes, acquiry_software_version, acquiry_operating_system, acquiry_date,
		system_date, password_hash, header_string_tail ) <= -1 )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_encase4: unable to set header.\n" );

		ewf_header_free( header );

		if( header_values->acquiry_date == NULL )
		{
			libewf_free( acquiry_date );
		}
		if( header_values->system_date == NULL )
		{
			libewf_free( system_date );
		}
		return( NULL );
	}
	/* Make sure the string is terminated
	 */
	header[ size - 1 ] = '\0';

	if( header_values->acquiry_date == NULL )
	{
		libewf_free( acquiry_date );
	}
	if( header_values->system_date == NULL )
	{
		libewf_free( system_date );
	}
	return( header );
}

/* Generate an Encase4 header2
 * Return a pointer to the new instance, NULL on error
 */
EWF_HEADER *libewf_header_values_generate_header2_string_encase4( LIBEWF_HEADER_VALUES *header_values, time_t timestamp )
{
	EWF_HEADER *header             = NULL;
	char *header_string_head       = "1\nmain\na\tc\tn\te\tt\tav\tov\tm\tu\tp\n";
	char *header_string_tail       = "\n\n";
	char *case_number              = "";
	char *description              = "";
	char *examiner_name            = "";
	char *evidence_number          = "";
	char *notes                    = "";
	char *system_date              = "";
	char *acquiry_date             = "";
	char *acquiry_operating_system = "";
	char *acquiry_software_version = "";
	char *password_hash            = "";
	uint32_t size                  = 0;

	if( header_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header2_string_encase4: invalid header values.\n" );

		return( NULL );
	}
	size = libewf_strlen( header_string_head );

	if( header_values->case_number != NULL )
	{
		size       += libewf_strlen( header_values->case_number );
		case_number = header_values->case_number;
	}
	if( header_values->description != NULL )
	{
		size       += libewf_strlen( header_values->description );
		description = header_values->description;
	}
	if( header_values->examiner_name != NULL )
	{
		size         += libewf_strlen( header_values->examiner_name );
		examiner_name = header_values->examiner_name;
	}
	if( header_values->evidence_number != NULL )
	{
		size           += libewf_strlen( header_values->evidence_number );
		evidence_number = header_values->evidence_number;
	}
	if( header_values->notes != NULL )
	{
		size += libewf_strlen( header_values->notes );
		notes = header_values->notes;
	}
	if( header_values->acquiry_date != NULL )
	{
		size        += libewf_strlen( header_values->acquiry_date );
		acquiry_date = header_values->acquiry_date;
	}
	else
	{
		acquiry_date = libewf_generate_date_header2_value( timestamp );
		size        += libewf_strlen( acquiry_date );
	}
	if( header_values->system_date != NULL )
	{
		size       += libewf_strlen( header_values->system_date );
		system_date = header_values->system_date;
	}
	else
	{
		system_date = libewf_generate_date_header2_value( timestamp );
		size       += libewf_strlen( system_date );
	}
	if( header_values->acquiry_operating_system != NULL )
	{
		size                    += libewf_strlen( header_values->acquiry_operating_system );
		acquiry_operating_system = header_values->acquiry_operating_system;
	}
	if( header_values->acquiry_software_version != NULL )
	{
		size                    += libewf_strlen( header_values->acquiry_software_version );
		acquiry_software_version = header_values->acquiry_software_version;
	}
	if( header_values->password != NULL )
	{
		size         += libewf_strlen( header_values->password );
		password_hash = header_values->password;
	}
	size += libewf_strlen( header_string_tail );

	/* allow for 9x \t and 1x \0
	 */
	size += 10;

	header = ewf_header_alloc( size );

	if( header == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header2_string_encase4: unable to create header.\n" );

		return( NULL );
	}
	if( snprintf( (char *) header, size, "%s%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s%s",
		header_string_head, description, case_number, evidence_number, examiner_name,
		notes, acquiry_software_version, acquiry_operating_system, acquiry_date,
		system_date, password_hash, header_string_tail ) <= -1 )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header2_string_encase4: unable to set header.\n" );

		ewf_header_free( header );

		if( header_values->acquiry_date == NULL )
		{
			libewf_free( acquiry_date );
		}
		if( header_values->system_date == NULL )
		{
			libewf_free( system_date );
		}
		return( NULL );
	}
	/* Make sure the string is terminated
	 */
	header[ size - 1 ] = '\0';

	if( header_values->acquiry_date == NULL )
	{
		libewf_free( acquiry_date );
	}
	if( header_values->system_date == NULL )
	{
		libewf_free( system_date );
	}
	return( header );
}

/* Generate an Encase5 header2
 * Return a pointer to the new instance, NULL on error
 */
EWF_HEADER *libewf_header_values_generate_header2_string_encase5( LIBEWF_HEADER_VALUES *header_values, time_t timestamp )
{
	EWF_HEADER *header             = NULL;
	char *header_string_main       = "3\nmain\na\tc\tn\te\tt\tav\tov\tm\tu\tp\tdc\n";
	char *header_string_tail       = "\n\n";
	char *header_string_srce       = "srce\n0\t1\np\tn\tid\tev\ttb\tlo\tpo\tah\tgu\taq\n0\t0\t\t\t\t\t\t\t\t\n\t\t\t\t\t-1\t-1\t\t\t\n\n";
	char *header_string_sub        = "sub\n0\t1\np\tn\tid\tnu\tco\tgu\n0\t0\t\t\t\t\n\t\t\t\t1\t\n\n";
	char *case_number              = "";
	char *description              = "";
	char *examiner_name            = "";
	char *evidence_number          = "";
	char *notes                    = "";
	char *system_date              = "";
	char *acquiry_date             = "";
	char *acquiry_operating_system = "";
	char *acquiry_software_version = "";
	char *password_hash            = "";
	char *unknown_dc               = "";
	uint32_t size                  = 0;

	if( header_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_encase5: invalid header values.\n" );

		return( NULL );
	}
	size  = libewf_strlen( header_string_main );
	size += libewf_strlen( header_string_srce );
	size += libewf_strlen( header_string_sub );

	if( header_values->case_number != NULL )
	{
		size       += libewf_strlen( header_values->case_number );
		case_number = header_values->case_number;
	}
	if( header_values->description != NULL )
	{
		size       += libewf_strlen( header_values->description );
		description = header_values->description;
	}
	if( header_values->examiner_name != NULL )
	{
		size         += libewf_strlen( header_values->examiner_name );
		examiner_name = header_values->examiner_name;
	}
	if( header_values->evidence_number != NULL )
	{
		size           += libewf_strlen( header_values->evidence_number );
		evidence_number = header_values->evidence_number;
	}
	if( header_values->notes != NULL )
	{
		size += libewf_strlen( header_values->notes );
		notes = header_values->notes;
	}
	if( header_values->acquiry_date != NULL )
	{
		size        += libewf_strlen( header_values->acquiry_date );
		acquiry_date = header_values->acquiry_date;
	}
	else
	{
		acquiry_date = libewf_generate_date_header2_value( timestamp );
		size        += libewf_strlen( acquiry_date );
	}
	if( header_values->system_date != NULL )
	{
		size       += libewf_strlen( header_values->system_date );
		system_date = header_values->system_date;
	}
	else
	{
		system_date = libewf_generate_date_header2_value( timestamp );
		size       += libewf_strlen( system_date );
	}
	if( header_values->acquiry_operating_system != NULL )
	{
		size                    += libewf_strlen( header_values->acquiry_operating_system );
		acquiry_operating_system = header_values->acquiry_operating_system;
	}
	if( header_values->acquiry_software_version != NULL )
	{
		size                    += libewf_strlen( header_values->acquiry_software_version );
		acquiry_software_version = header_values->acquiry_software_version;
	}
	if( header_values->password != NULL )
	{
		size         += libewf_strlen( header_values->password );
		password_hash = header_values->password;
	}
	if( header_values->unknown_dc != NULL )
	{
		size      += libewf_strlen( header_values->unknown_dc );
		unknown_dc = header_values->unknown_dc;
	}
	size += libewf_strlen( header_string_tail );

	/* allow for 10x \t and 1x \0
	 */
	size += 11;

	header = ewf_header_alloc( size );

	if( header == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header2_string_encase5: unable to create header.\n" );

		return( NULL );
	}
	if( snprintf( (char *) header, size, "%s%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s%s%s%s",
		header_string_main, description, case_number, evidence_number, examiner_name,
		notes, acquiry_software_version, acquiry_operating_system, acquiry_date,
		system_date, password_hash, unknown_dc, header_string_tail,
		header_string_srce, header_string_sub ) <= -1 )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header2_string_encase5: unable to set header.\n" );

		ewf_header_free( header );

		if( header_values->acquiry_date == NULL )
		{
			libewf_free( acquiry_date );
		}
		if( header_values->system_date == NULL )
		{
			libewf_free( system_date );
		}
		return( NULL );
	}
	/* Make sure the string is terminated
	 */
	header[ size - 1 ] = '\0';

	if( header_values->acquiry_date == NULL )
	{
		libewf_free( acquiry_date );
	}
	if( header_values->system_date == NULL )
	{
		libewf_free( system_date );
	}
	return( header );
}

/* Generate an FTK Imager header
 * Return a pointer to the new instance, NULL on error
 */
EWF_HEADER *libewf_header_values_generate_header_string_ftk( LIBEWF_HEADER_VALUES *header_values, time_t timestamp, uint8_t compression_level )
{
	EWF_HEADER *header             = NULL;
	char *header_string_head       = "1\nmain\nc\tn\ta\te\tt\tav\tov\tm\tu\tp\tr\n";
	char *header_string_tail       = "\n\n";
	char *case_number              = "";
	char *description              = "";
	char *examiner_name            = "";
	char *evidence_number          = "";
	char *notes                    = "";
	char *system_date              = "";
	char *acquiry_date             = "";
	char *acquiry_operating_system = "";
	char *acquiry_software_version = "";
	char *password_hash            = "";
	char *compression_type         = "";
	uint32_t size                  = 0;

	if( header_values == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_ftk: invalid header values.\n" );

		return( NULL );
	}
	if( ( compression_level != EWF_COMPRESSION_NONE ) && ( compression_level != EWF_COMPRESSION_FAST ) && ( compression_level != EWF_COMPRESSION_BEST ) )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_ftk: compression level not supported.\n" );

		return( NULL );
	}
	size = libewf_strlen( header_string_head );

	if( header_values->case_number != NULL )
	{
		size       += libewf_strlen( header_values->case_number );
		case_number = header_values->case_number;
	}
	if( header_values->description != NULL )
	{
		size       += libewf_strlen( header_values->description );
		description = header_values->description;
	}
	if( header_values->examiner_name != NULL )
	{
		size         += libewf_strlen( header_values->examiner_name );
		examiner_name = header_values->examiner_name;
	}
	if( header_values->evidence_number != NULL )
	{
		size           += libewf_strlen( header_values->evidence_number );
		evidence_number = header_values->evidence_number;
	}
	if( header_values->notes != NULL )
	{
		size += libewf_strlen( header_values->notes );
		notes = header_values->notes;
	}
	if( header_values->acquiry_date != NULL )
	{
		size        += libewf_strlen( header_values->acquiry_date );
		acquiry_date = header_values->acquiry_date;
	}
	else
	{
		acquiry_date = libewf_generate_date_header_value( timestamp );
		size        += libewf_strlen( acquiry_date );
	}
	if( header_values->system_date != NULL )
	{
		size       += libewf_strlen( header_values->system_date );
		system_date = header_values->system_date;
	}
	else
	{
		system_date = libewf_generate_date_header_value( timestamp );
		size       += libewf_strlen( system_date );
	}
	if( header_values->acquiry_operating_system != NULL )
	{
		size                    += libewf_strlen( header_values->acquiry_operating_system );
		acquiry_operating_system = header_values->acquiry_operating_system;
	}
	if( header_values->acquiry_software_version != NULL )
	{
		size                    += libewf_strlen( header_values->acquiry_software_version );
		acquiry_software_version = header_values->acquiry_software_version;
	}
	if( header_values->password != NULL )
	{
		size         += libewf_strlen( header_values->password );
		password_hash = header_values->password;
	}
	else
	{
		size         += 1;
		password_hash = "0";
	}
	if( header_values->compression_type != NULL )
	{
		size            += libewf_strlen( header_values->compression_type );
		compression_type = header_values->compression_type;
	}
	else
	{
		if( compression_level == EWF_COMPRESSION_NONE )
		{
			compression_type = LIBEWF_COMPRESSIONTYPE_NONE;
		}
		else if( compression_level == EWF_COMPRESSION_FAST )
		{
			compression_type = LIBEWF_COMPRESSIONTYPE_FAST;
		}
		else if( compression_level == EWF_COMPRESSION_BEST )
		{
			compression_type = LIBEWF_COMPRESSIONTYPE_BEST;
		}
		size += libewf_strlen( compression_type );
	}
	size += libewf_strlen( header_string_tail );

	/* allow for 10x \t and 1x \0
	 */
	size += 11;

	header = ewf_header_alloc( size );

	if( header == NULL )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_ftk: unable to create header.\n" );

		return( NULL );
	}
	if( snprintf( (char *) header, size, "%s%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s%s",
		header_string_head, case_number, evidence_number, description, examiner_name,
		notes, acquiry_software_version, acquiry_operating_system, acquiry_date,
		system_date, password_hash, compression_type, header_string_tail ) <= -1 )
	{
		LIBEWF_WARNING_PRINT( "libewf_header_values_generate_header_string_ftk: unable to set header.\n" );

		ewf_header_free( header );

		if( header_values->acquiry_date == NULL )
		{
			libewf_free( acquiry_date );
		}
		if( header_values->system_date == NULL )
		{
			libewf_free( system_date );
		}
		return( NULL );
	}
	/* Make sure the string is terminated
	 */
	header[ size - 1 ] = '\0';

	if( header_values->acquiry_date == NULL )
	{
		libewf_free( acquiry_date );
	}
	if( header_values->system_date == NULL )
	{
		libewf_free( system_date );
	}
	return( header );
}

