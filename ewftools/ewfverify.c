/*
 * ewfverify
 * Verifies the integrity of the media data within the EWF file
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

#include "../libewf/libewf_includes.h"

#include <errno.h>
#include <stdio.h>

#if defined( HAVE_UNISTD_H )
#include <unistd.h>
#endif

#if defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf_extern.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_EXPORT
#endif

#include <libewf.h>

#include "../libewf/libewf_common.h"
#include "../libewf/libewf_string.h"

#include "ewfcommon.h"
#include "ewfdigest_context.h"
#include "ewfgetopt.h"
#include "ewfglob.h"
#include "ewfmd5.h"
#include "ewfsignal.h"
#include "ewfsha1.h"
#include "ewfstring.h"

/* Prints the executable usage information
 */
void usage( void )
{
	fprintf( stdout, "Usage: ewfverify [ -d digest_type ] [ -hqsvV ] ewf_files\n\n" );

	fprintf( stdout, "\t-d: calculate additional digest (hash) types besides md5, options: sha1\n" );
	fprintf( stdout, "\t-h: shows this help\n" );
	fprintf( stdout, "\t-q: quiet shows no status information\n" );
	fprintf( stderr, "\t-s: swap byte pairs of the media data (from AB to BA)\n" );
	fprintf( stderr, "\t    (use this for big to little endian conversion and vice versa)\n" );
	fprintf( stdout, "\t-v: verbose output to stderr\n" );
	fprintf( stdout, "\t-V: print version\n" );
}

/* The main program
 */
#if defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
int wmain( int argc, wchar_t * const argv[] )
#else
int main( int argc, char * const argv[] )
#endif
{
	EWFDIGEST_HASH md5_hash[ EWFDIGEST_HASH_SIZE_MD5 ];

#if !defined( HAVE_GLOB_H )
	EWFGLOB *glob                            = NULL;
	int32_t glob_count                       = 0;
#endif
	LIBEWF_HANDLE *handle                    = NULL;
	LIBEWF_CHAR *stored_md5_hash_string      = NULL;
	LIBEWF_CHAR *calculated_md5_hash_string  = NULL;
	LIBEWF_CHAR *stored_sha1_hash_string     = NULL;
	LIBEWF_CHAR *calculated_sha1_hash_string = NULL;
	LIBEWF_CHAR *program                     = _S_LIBEWF_CHAR( "ewfverify" );

	CHAR_T *time_string                      = NULL;
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
        CHAR_T *error_string                     = NULL;
#endif
	void *callback                           = &ewfcommon_process_status_fprint;
	INT_T option                             = 0;
	time_t timestamp_start                   = 0;
	time_t timestamp_end                     = 0;
	int64_t count                            = 0;
	int8_t stored_md5_hash_result            = 0;
	int8_t stored_sha1_hash_result           = 0;
	uint8_t calculate_md5                    = 1;
	uint8_t calculate_sha1                   = 0;
	uint8_t swap_byte_pairs                  = 0;
	uint8_t verbose                          = 0;
	int match_md5_hash                       = 0;
	int match_sha1_hash                      = 0;

	ewfsignal_initialize();

	ewfcommon_version_fprint( stdout, program );

	while( ( option = ewfgetopt( argc, argv, _S_CHAR_T( "d:hsqvV" ) ) ) != (INT_T) -1 )
	{
		switch( option )
		{
			case (INT_T) '?':
			default:
				fprintf( stderr, "Invalid argument: %" PRIs "\n", argv[ optind ] );

				usage();

				return( EXIT_FAILURE );

			case (INT_T) 'd':
				if( CHAR_T_COMPARE( optarg, _S_CHAR_T( "sha1" ), 4 ) == 0 )
				{
					calculate_sha1 = 1;
				}
				else
				{
					fprintf( stderr, "Unsupported digest type.\n" );
				}
				break;

			case (INT_T) 'h':
				usage();

				return( EXIT_SUCCESS );

			case (INT_T) 'q':
				callback = NULL;

				break;

			case (INT_T) 's':
				swap_byte_pairs = 1;

				break;
			case (INT_T) 'v':
				verbose = 1;

				break;

			case (INT_T) 'V':
				ewfcommon_copyright_fprint( stdout );

				return( EXIT_SUCCESS );
		}
	}
	if( optind == argc )
	{
		fprintf( stderr, "Missing EWF image file(s).\n" );

		usage();

		return( EXIT_FAILURE );
	}
	libewf_set_notify_values( stderr, verbose );

#if !defined( HAVE_GLOB_H )
	glob = ewfglob_alloc();

	if( glob == NULL )
	{
		fprintf( stderr, "Unable to create glob.\n" );

		return( EXIT_FAILURE );
	}
	glob_count = ewfglob_resolve( glob, &argv[ optind ], ( argc - optind ) );

	if( glob_count <= 0 )
	{
		fprintf( stderr, "Unable to resolve glob.\n" );

		ewfglob_free( glob );

		return( EXIT_FAILURE );
	}
	handle = libewf_open( glob->results, glob->amount, LIBEWF_OPEN_READ );

	ewfglob_free( glob );
#else
	handle = libewf_open( &argv[ optind ], ( argc - optind ), LIBEWF_OPEN_READ );
#endif

	if( handle == NULL )
	{
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
		if( errno != 0 )
		{
			error_string = ewfstring_strerror( errno );
		}
		if( error_string != NULL )
		{
			fprintf( stderr, "Unable to open EWF file(s) with failure: %" PRIs ".\n",
			 error_string );

			libewf_common_free( error_string );
		}
		else
		{
			fprintf( stderr, "Unable to open EWF image file(s).\n" );
		}
#else
		fprintf( stderr, "Unable to open EWF image file(s).\n" );
#endif

		return( EXIT_FAILURE );
	}
	if( calculate_md5 == 1 )
	{
		stored_md5_hash_string = (LIBEWF_CHAR *) libewf_common_alloc( LIBEWF_CHAR_SIZE * LIBEWF_STRING_DIGEST_HASH_LENGTH_MD5 );

		if( stored_md5_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create stored MD5 hash string.\n" );

			return( EXIT_FAILURE );
		}
		calculated_md5_hash_string = (LIBEWF_CHAR *) libewf_common_alloc( LIBEWF_CHAR_SIZE * LIBEWF_STRING_DIGEST_HASH_LENGTH_MD5 );

		if( calculated_md5_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create calculated MD5 hash string.\n" );

			libewf_common_free( stored_md5_hash_string );

			return( EXIT_FAILURE );
		}
	}
	if( calculate_sha1 == 1 )
	{
		stored_sha1_hash_string = (LIBEWF_CHAR *) libewf_common_alloc( LIBEWF_CHAR_SIZE * LIBEWF_STRING_DIGEST_HASH_LENGTH_SHA1 );

		if( stored_sha1_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create stored SHA1 hash string.\n" );

			if( calculate_md5 == 1 )
			{
				libewf_common_free( stored_md5_hash_string );
				libewf_common_free( calculated_md5_hash_string );
			}
			return( EXIT_FAILURE );
		}
		calculated_sha1_hash_string = (LIBEWF_CHAR *) libewf_common_alloc( LIBEWF_CHAR_SIZE * LIBEWF_STRING_DIGEST_HASH_LENGTH_SHA1 );

		if( calculated_sha1_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create calculated SHA1 hash string.\n" );

			if( calculate_md5 == 1 )
			{
				libewf_common_free( stored_md5_hash_string );
				libewf_common_free( calculated_md5_hash_string );
			}
			libewf_common_free( stored_sha1_hash_string );

			return( EXIT_FAILURE );
		}
	}
	/* Start verifying data
	 */
	timestamp_start = time( NULL );
	time_string     = libewf_common_ctime( &timestamp_start );

	if( time_string != NULL )
	{
		fprintf( stdout, "Verify started at: %" PRIs "\n", time_string );

		libewf_common_free( time_string );
	}
	else
	{
		fprintf( stdout, "Verify started.\n" );
	}
	if( callback != NULL )
	{
		ewfcommon_process_status_initialize(
		 stdout,
		 _S_LIBEWF_CHAR( "verified" ),
		 timestamp_start );
	}
	fprintf( stdout, "This could take a while.\n\n" );

	if( calculate_sha1 == 1 )
	{
		if( libewf_parse_hash_values( handle ) != 1 )
		{
			fprintf( stderr, "Unable to get parse hash values.\n" );
		}
	}
	count = ewfcommon_read_verify(
	         handle,
	         calculate_md5,
	         calculated_md5_hash_string,
	         LIBEWF_STRING_DIGEST_HASH_LENGTH_MD5,
	         calculate_sha1,
	         calculated_sha1_hash_string,
	         LIBEWF_STRING_DIGEST_HASH_LENGTH_SHA1,
	         swap_byte_pairs,
	         callback );

	timestamp_end = time( NULL );
	time_string   = libewf_common_ctime( &timestamp_end );

	if( count <= -1 )
	{
		if( time_string != NULL )
		{
			fprintf( stdout, "Verify failed at: %" PRIs "\n", time_string );

			libewf_common_free( time_string );
		}
		else
		{
			fprintf( stdout, "Verify failed.\n" );
		}
		if( libewf_close( handle ) != 0 )
		{
			fprintf( stderr, "Unable to close EWF file(s).\n" );
		}
		if( calculate_md5 == 1 )
		{
			libewf_common_free( stored_md5_hash_string );
			libewf_common_free( calculated_md5_hash_string );
		}
		if( calculate_sha1 == 1 )
		{
			libewf_common_free( stored_sha1_hash_string );
			libewf_common_free( calculated_sha1_hash_string );
		}
		return( EXIT_FAILURE );
	}
	if( time_string != NULL )
	{
		fprintf( stdout, "Verify completed at: %" PRIs "\n", time_string );

		libewf_common_free( time_string );
	}
	else
	{
		fprintf( stdout, "Verify completed.\n" );
	}
	ewfcommon_process_summary_fprint(
	 stdout,
	 _S_LIBEWF_CHAR( "Read" ),
	 count,
	 timestamp_start,
	 timestamp_end );

	fprintf( stdout, "\n" );

	if( calculate_md5 == 1 )
	{
		stored_md5_hash_result = libewf_get_md5_hash(
					  handle,
					  md5_hash,
					  EWFDIGEST_HASH_SIZE_MD5 );

		if( stored_md5_hash_result == -1 )
		{
			fprintf( stderr, "Unable to get stored MD5 hash.\n" );

			if( libewf_close( handle ) != 0 )
			{
				fprintf( stderr, "Unable to close EWF file(s).\n" );
			}
			libewf_common_free( stored_md5_hash_string );
			libewf_common_free( calculated_md5_hash_string );

			if( calculate_sha1 == 1 )
			{
				libewf_common_free( stored_sha1_hash_string );
				libewf_common_free( calculated_sha1_hash_string );
			}
			return( EXIT_FAILURE );
		}
		if( ewfdigest_copy_to_string(
		     md5_hash,
		     EWFDIGEST_HASH_SIZE_MD5,
		     stored_md5_hash_string,
		     LIBEWF_STRING_DIGEST_HASH_LENGTH_MD5 ) != 1 )
		{
			fprintf( stderr, "Unable to get stored MD5 hash string.\n" );

			if( libewf_close( handle ) != 0 )
			{
				fprintf( stderr, "Unable to close EWF file(s).\n" );
			}
			libewf_common_free( stored_md5_hash_string );
			libewf_common_free( calculated_md5_hash_string );

			if( calculate_sha1 == 1 )
			{
				libewf_common_free( stored_sha1_hash_string );
				libewf_common_free( calculated_sha1_hash_string );
			}
			return( EXIT_FAILURE );
		}
	}
	if( calculate_sha1 == 1 )
	{
		stored_sha1_hash_result = libewf_get_hash_value(
		                           handle,
		                           _S_LIBEWF_CHAR( "SHA1" ),
		                           stored_sha1_hash_string,
		                           LIBEWF_STRING_DIGEST_HASH_LENGTH_SHA1 );

		if( stored_sha1_hash_result == -1 )
		{
			fprintf( stderr, "Unable to get stored SHA1 hash.\n" );

			if( libewf_close( handle ) != 0 )
			{
				fprintf( stderr, "Unable to close EWF file(s).\n" );
			}
			if( calculate_md5 == 1 )
			{
				libewf_common_free( stored_md5_hash_string );
				libewf_common_free( calculated_md5_hash_string );
			}
			libewf_common_free( stored_sha1_hash_string );
			libewf_common_free( calculated_sha1_hash_string );

			return( EXIT_FAILURE );
		}
	}
	ewfcommon_crc_errors_fprint( stdout, handle );

	if( libewf_close( handle ) != 0 )
	{
		fprintf( stderr, "Unable to close EWF file(s).\n" );

		if( calculate_md5 == 1 )
		{
			libewf_common_free( stored_md5_hash_string );
			libewf_common_free( calculated_md5_hash_string );
		}
		if( calculate_sha1 == 1 )
		{
			libewf_common_free( stored_sha1_hash_string );
			libewf_common_free( calculated_sha1_hash_string );
		}
		return( EXIT_FAILURE );
	}
	if( calculate_md5 == 1 )
	{
		if( stored_md5_hash_result == 0 )
		{
			fprintf( stdout, "MD5 hash stored in file:\tN/A\n" );
		}
		else
		{
			fprintf( stdout, "MD5 hash stored in file:\t%" PRIs_EWF "\n", stored_md5_hash_string );
		}
		fprintf( stdout, "MD5 hash calculated over data:\t%" PRIs_EWF "\n", calculated_md5_hash_string );

		match_md5_hash = ( libewf_string_compare(
		                    stored_md5_hash_string,
		                    calculated_md5_hash_string,
		                    LIBEWF_STRING_DIGEST_HASH_LENGTH_MD5 ) == 0 );

		libewf_common_free( stored_md5_hash_string );
		libewf_common_free( calculated_md5_hash_string );
	}
	if( calculate_sha1 == 1 )
	{
		if( stored_sha1_hash_result == 0 )
		{
			fprintf( stdout, "SHA1 hash stored in file:\tN/A\n" );
		}
		else
		{
			fprintf( stdout, "SHA1 hash stored in file:\t%" PRIs_EWF "\n", stored_sha1_hash_string );
		}
		fprintf( stdout, "SHA1 hash calculated over data:\t%" PRIs_EWF "\n", calculated_sha1_hash_string );

		match_sha1_hash = ( libewf_string_compare(
		                     stored_sha1_hash_string,
		                     calculated_sha1_hash_string,
		                     LIBEWF_STRING_DIGEST_HASH_LENGTH_SHA1 ) == 0 );

		libewf_common_free( stored_sha1_hash_string );
		libewf_common_free( calculated_sha1_hash_string );
	}
	if( match_md5_hash
	 && ( ( calculate_sha1 == 0 )
	  || ( match_sha1_hash ) ) )
	{
		fprintf( stdout, "\n%" PRIs_EWF ": SUCCESS\n", program );

		return( EXIT_SUCCESS );
	}
	else
	{
		fprintf( stdout, "\n%" PRIs_EWF ": FAILURE\n", program );

		return( EXIT_FAILURE );
	}
}

