/*
 * ewfstring
 * String functions for the ewf tools
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
#include <character_string.h>
#include <memory.h>
#include <notify.h>
#include <system_string.h>

#include <errno.h>

#if defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif

#if defined( HAVE_STRING_H )
#include <string.h>
#endif

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include "ewfstring.h"

#if defined( HAVE_WINDOWS_API )
#define ewfcommon_strerror_r( error_number, string, size ) \
	strerror_s( string, size, error_number )

#define LIBEWF_COMMON_STRERROR_R_RETURN_ERROR	0

#elif defined( HAVE_STRERROR_R )
#define ewfcommon_strerror_r( error_number, string, size ) \
	strerror_r( error_number, string, size )

#if defined( STRERROR_R_CHAR_P )
#define LIBEWF_COMMON_STRERROR_R_RETURN_ERROR	NULL
#else
#define LIBEWF_COMMON_STRERROR_R_RETURN_ERROR	0
#endif

#endif

/* Function to wrap strerror()
 * Returns a new instance to a string containing the error string, NULL on error
 */
char *ewfstring_strerror(
       int error_number )
{
	static char *function     = "ewfstring_strerror";
#if !defined( ewfstring_strerror_r ) && defined( HAVE_STRERROR )
	char *static_error_string = NULL;
#endif
#if defined( ewfstring_strerror_r ) || defined( HAVE_STRERROR )
	char *error_string        = NULL;
	size_t error_string_size  = 256;

	error_string = (char *) memory_allocate(
	                         sizeof( char ) * error_string_size );

	if( error_string == NULL )
	{
		notify_warning_printf( "%s: unable to create error string.\n",
		 function );

		return( NULL );
	}
#endif
#if defined( ewfstring_strerror_r )
	if( ewfstring_strerror_r(
	     error_number,
	     error_string,
	     error_string_size ) != LIBEWF_COMMON_STRERROR_R_RETURN_ERROR )
	{
		notify_warning_printf( "%s: unable to set error string.\n",
		 function );

		memory_free(
		 error_string );

		return( NULL );
	}
	return( error_string );

#elif defined( HAVE_STRERROR )
	static_error_string = strerror(
	                       error_number );

	if( static_error_string == NULL )
	{
		notify_warning_printf( "%s: unable to create static error string.\n",
		 function );

		memory_free(
		 error_string );

		return( NULL );
	}
	if( memory_copy(
	     error_string,
	     static_error_string,
	     error_string_size ) == NULL )
	{
		notify_warning_printf( "%s: unable to set error string.\n",
		 function );

		memory_free(
		 error_string );

		return( NULL );
	}
	return( error_string );
#else

	return( NULL );
#endif
}

#if defined( HAVE_WIDE_CHARACTER_TYPE ) && defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )

#if defined( HAVE_WINDOWS_API )
#define ewfstring_wide_strerror_r( error_number, string, size ) \
	_wcserror_s( string, size, error_number )

#define LIBEWF_COMMON_WIDE_STRERROR_R_RETURN_ERROR	0

#else
#error Missing wide character equivalent of strerror()
#endif

/* Function to wrap wide character equivalent of strerror()
 * Returns a new instance to a string containing the error string, NULL on error
 */
wchar_t *ewfstring_wide_strerror(
          int error_number )
{
	static char *function      = "ewfstring_wide_strerror";
#if defined( libewf_wide_common_strerror_r )
	wchar_t *error_string      = NULL;
	uint16_t error_string_size = 256;

	error_string = (wchar_t *) memory_allocate(
	                            sizeof( wchar_t ) * error_string_size );

	if( error_string == NULL )
	{
		notify_warning_printf( "%s: unable to create error string.\n",
		 function );

		return( NULL );
	}
	if( ewfstring_wide_strerror_r(
	     error_number,
	     error_string,
	     error_string_size ) != LIBEWF_COMMON_STRERROR_R_RETURN_ERROR )
	{
		notify_warning_printf( "%s: unable to set error string.\n",
		 function );

		memory_free(
		 error_string );

		return( NULL );
	}
	return( error_string );
#else

	return( NULL );
#endif
}
#endif

/* Copies the source string (of system_character_t) into the destination string for a certain length
 * Terminates the destination string with \0 at ( length - 1 )
 * Returns 1 if successful, -1 on error
 */
int ewfstring_copy_system_string_to_character_string(
     character_t *destination,
     const system_character_t *source,
     size_t length )
{
	static char *function = "ewfstring_copy_system_string_to_character_string";
	ssize_t conversion    = (ssize_t) ( sizeof( character_t ) - sizeof( system_character_t ) );
	size_t iterator       = 0;

	if( source == NULL )
	{
		notify_warning_printf( "%s: invalid source.\n",
		 function );

		return( -1 );
	}
	if( destination == NULL )
	{
		notify_warning_printf( "%s: invalid destination.\n",
		 function );

		return( -1 );
	}
	for( iterator = 0; iterator < length; iterator++ )
	{
		if( conversion == 0 )
		{
			destination[ iterator ] = (character_t) source[ iterator ];
		}
#if defined( HAVE_WIDE_CHARACTER_TYPE )
		else if( conversion > 0 )
		{
			destination[ iterator ] = (character_t) btowc(
			                                           (int) source[ iterator ] );
		}
		else if( conversion < 0 )
		{
			destination[ iterator ] = (character_t) wctob(
			                                           (wint_t) source[ iterator ] );

			/* If character is out of the basic ASCII range use '_' as a place holder
			 */
			if( destination[ iterator ] == EOF )
			{
				destination[ iterator ] = '_';
			}
		}
#endif
		else
		{
			notify_warning_printf( "%s: unsupported conversion.\n",
			 function );

			return( -1 );
		}
	}
	destination[ length - 1 ] = (character_t) '\0';

	return( 1 );
}

/* Copies the source string into the destination string (of system_character_t) for a certain length
 * Terminates the destination string with \0 at ( length - 1 )
 * Returns 1 if successful, -1 on error
 */
int ewfstring_copy_character_string_to_system_string(
     system_character_t *destination,
     const character_t *source,
     size_t length )
{
	static char *function = "ewfstring_copy_character_string_to_system_string";
	ssize_t conversion    = (ssize_t) ( sizeof( character_t ) - sizeof( system_character_t ) );
	size_t iterator       = 0;

	if( source == NULL )
	{
		notify_warning_printf( "%s: invalid source.\n",
		 function );

		return( -1 );
	}
	if( destination == NULL )
	{
		notify_warning_printf( "%s: invalid destination.\n",
		 function );

		return( -1 );
	}
	for( iterator = 0; iterator < length; iterator++ )
	{
		if( conversion == 0 )
		{
			destination[ iterator ] = (system_character_t) source[ iterator ];
		}
#if defined( HAVE_WIDE_CHARACTER_TYPE )
		else if( conversion > 0 )
		{
			destination[ iterator ] = (system_character_t) wctob(
			                                    (wint_t) source[ iterator ] );

			/* If character is out of the basic ASCII range use '_' as a place holder
			 */
			if( destination[ iterator ] == EOF )
			{
				destination[ iterator ] = '_';
			}
		}
		else if( conversion < 0 )
		{
			destination[ iterator ] = (system_character_t) btowc(
			                                    (int) source[ iterator ] );
		}
#endif
		else
		{
			notify_warning_printf( "%s: unsupported conversion.\n",
			 function );

			return( -1 );
		}
	}
	destination[ length - 1 ] = (system_character_t) '\0';

	return( 1 );
}

