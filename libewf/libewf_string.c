/*
 * Character type string functions
 *
 * Copyright (c) 2006-2010, Joachim Metz <forensics@hoffmannbv.nl>,
 * Hoffmann Investigations.
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
#include <types.h>

#include <liberror.h>

#if defined( HAVE_ERRNO_H ) || defined( WINAPI )
#include <errno.h>
#endif

#if defined( HAVE_STDARG_H ) || defined( WINAPI )
#include <stdarg.h>
#elif defined( HAVE_VARARGS_H )
#include <varargs.h>
#else
#error Missing headers stdarg.h and varargs.h
#endif

#include "libewf_string.h"

/* Copies a string to a decimal 64-bit value
 * Return 1 if successful or -1 on error
 */
int libewf_string_copy_to_64bit_decimal(
     libewf_character_t *string,
     size_t string_size,
     uint64_t *value_64bit,
     liberror_error_t **error )
{
	static char *function  = "libewf_string_copy_to_64bit_decimal";
	size_t string_iterator = 0;
	uint8_t byte_value     = 0;

	if( string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid string.",
		 function );

		return( -1 );
	}
	if( string_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid string size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( value_64bit == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid value 64-bit.",
		 function );

		return( -1 );
	}
	/* TODO handle +/- sign ? */
	/* TODO check for end of string character ? */

	if( ( string_size - string_iterator ) > (size_t) 20 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_TOO_LARGE,
		 "%s: invalid string size value too large.",
		 function );

		return( -1 );
	}
	*value_64bit = 0;

	while( string_iterator < ( string_size - 1 ) )
	{
		*value_64bit *= 10;

		if( ( string[ string_iterator ] >= (libewf_character_t) '0' )
		 && ( string[ string_iterator ] <= (libewf_character_t) '9' ) )
		{
			byte_value = (uint8_t) ( string[ string_iterator ] - (libewf_character_t) '0' );
		}
		else
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: unsupported character value: %" PRIc_LIBEWF ".",
			 function,
			 string[ string_iterator ] );

			return( -1 );
		}
		*value_64bit += byte_value;

		string_iterator++;
	}
	return( 1 );
}

/* Copies a string to a hexadecimal 64-bit value
 * Return 1 if successful or -1 on error
 */
int libewf_string_copy_to_64bit_hexadecimal(
     libewf_character_t *string,
     size_t string_size,
     uint64_t *value_64bit,
     liberror_error_t **error )
{
	static char *function  = "libewf_string_copy_to_64bit_hexadecimal";
	size_t string_iterator = 0;
	uint8_t byte_value     = 0;

	if( string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid string.",
		 function );

		return( -1 );
	}
	if( string_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid string size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( value_64bit == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid value 64-bit.",
		 function );

		return( -1 );
	}
	if( ( string_size > 2 )
	 && ( string[ 0 ] == (libewf_character_t) '0' )
	 && ( string[ 1 ] == (libewf_character_t) 'x' ) )
	{
		string_iterator = 2;
	}
	/* TODO check for end of string character ? */

	if( ( string_size - string_iterator ) > (size_t) 20 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_TOO_LARGE,
		 "%s: invalid string size value too large.",
		 function );

		return( -1 );
	}
	*value_64bit = 0;

	while( string_iterator < ( string_size - 1 ) )
	{
		*value_64bit <<= 4;

		if( ( string[ string_iterator ] >= (libewf_character_t) '0' )
		 && ( string[ string_iterator ] <= (libewf_character_t) '9' ) )
		{
			byte_value = (uint8_t) ( string[ string_iterator ] - (libewf_character_t) '0' );
		}
		else if( ( string[ string_iterator ] >= (libewf_character_t) 'A' )
		      && ( string[ string_iterator ] <= (libewf_character_t) 'F' ) )
		{
			byte_value = (uint8_t) ( string[ string_iterator ] - (libewf_character_t) 'A' );
		}
		else if( ( string[ string_iterator ] >= (libewf_character_t) 'a' )
		      && ( string[ string_iterator ] <= (libewf_character_t) 'f' ) )
		{
			byte_value = (uint8_t) ( string[ string_iterator ] - (libewf_character_t) 'a' );
		}
		else
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: unsupported character value: %" PRIc_LIBEWF ".",
			 function,
			 string[ string_iterator ] );

			return( -1 );
		}
		*value_64bit += byte_value;

		string_iterator++;
	}
	return( 1 );
}

#if !defined( narrow_string_snprintf )

#if defined( HAVE_STDARG_H ) || defined( WINAPI )
#define VARARGS( function, type, argument ) \
	function( libewf_character_t *string, size_t string_size, type argument, ... )
#define VASTART( argument_list, type, name ) \
	va_start( argument_list, name )
#define VAEND( argument_list ) \
	va_end( argument_list )

#elif defined( HAVE_VARARGS_H )
#define VARARGS( function, type, argument ) \
	function( libewf_character_t *string, size_t string_size, va_alist ) va_dcl
#define VASTART( argument_list, type, name ) \
	{ type name; va_start( argument_list ); name = va_arg( argument_list, type )
#define VAEND( argument_list ) \
	va_end( argument_list ); }

#endif

/* Print a formatted string on the string
 * Returns the amount of printed characters if successful or -1 on error
 */
int VARARGS(
     libewf_string_snprintf,
     const libewf_character_t *,
     format )
{
	va_list argument_list;

	int print_count = 0;

	if( string == NULL )
	{
		return( -1 );
	}
	if( string_size > (size_t) SSIZE_MAX )
	{
		return( -1 );
	}
	VASTART(
	 argument_list,
	 char *,
	 format );

	print_count = narrow_string_sprintf(
		       string,
		       format,
		       argument_list );

	VAEND(
	 argument_list );

	if( print_count <= -1 )
	{
		return( -1 );
	}
	if( (size_t) print_count >= string_size )
	{
		return( -1 );
	}
	return( print_count );
}

#undef VARARGS
#undef VASTART
#undef VAEND

#endif /* !defined( narrow_string_snprintf ) */

