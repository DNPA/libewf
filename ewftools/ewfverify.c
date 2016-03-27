/*
 * ewfverify
 * Verifies the integrity of the media data within the EWF file
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
#include <system_string.h>

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

#include "ewfcommon.h"
#include "ewfdigest_context.h"
#include "ewfgetopt.h"
#include "ewfglob.h"
#include "ewfmd5.h"
#include "ewfoutput.h"
#include "ewfsignal.h"
#include "ewfsha1.h"
#include "ewfstring.h"

/* Prints the executable usage information to the stream
 */
void usage_fprint(
      FILE *stream )
{
	if( stream == NULL )
	{
		return;
	}
	fprintf( stream, "Usage: ewfverify [ -d digest_type ] [ -hqsvVw ] ewf_files\n\n" );

	fprintf( stream, "\t-d: calculate additional digest (hash) types besides md5, options: sha1\n" );
	fprintf( stream, "\t-h: shows this help\n" );
	fprintf( stream, "\t-l: logs verification errors and the digest (hash) to the filename\n" );
	fprintf( stream, "\t-q: quiet shows no status information\n" );
	fprintf( stream, "\t-s: swap byte pairs of the media data (from AB to BA)\n" );
	fprintf( stream, "\t    (use this for big to little endian conversion and vice versa)\n" );
	fprintf( stream, "\t-v: verbose output to stderr\n" );
	fprintf( stream, "\t-V: print version\n" );
	fprintf( stream, "\t-w: wipe sectors on CRC error (mimic EnCase like behavior)\n" );
}

/* The main program
 */
#if defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
int wmain( int argc, wchar_t * const argv[] )
#else
int main( int argc, char * const argv[] )
#endif
{
	ewfdigest_hash_t md5_hash[ EWFDIGEST_HASH_SIZE_MD5 ];
	system_character_t time_string[ 32 ];

#if !defined( HAVE_GLOB_H )
	ewfglob_t *glob                          = NULL;
	int32_t glob_count                       = 0;
#endif
	character_t *stored_md5_hash_string      = NULL;
	character_t *calculated_md5_hash_string  = NULL;
	character_t *stored_sha1_hash_string     = NULL;
	character_t *calculated_sha1_hash_string = NULL;
	character_t *program                     = _CHARACTER_T_STRING( "ewfverify" );

	system_character_t *log_filename         = NULL;
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
        system_character_t *error_string         = NULL;
#endif

	FILE *log_file_stream                    = NULL;
	void *callback                           = &ewfoutput_process_status_fprint;

	system_integer_t option                  = 0;
	time_t timestamp_start                   = 0;
	time_t timestamp_end                     = 0;
	int64_t count                            = 0;
	uint32_t amount_of_crc_errors            = 0;
	int8_t stored_md5_hash_result            = 0;
	int8_t stored_sha1_hash_result           = 0;
	uint8_t calculate_md5                    = 1;
	uint8_t calculate_sha1                   = 0;
	uint8_t swap_byte_pairs                  = 0;
	uint8_t wipe_chunk_on_error              = 0;
	uint8_t verbose                          = 0;
	int match_md5_hash                       = 0;
	int match_sha1_hash                      = 0;
	int result                               = 0;

	ewfoutput_version_fprint(
	 stdout,
	 program );

	while( ( option = ewfgetopt(
	                   argc,
	                   argv,
	                   _SYSTEM_CHARACTER_T_STRING( "d:hl:sqvVw" ) ) ) != (system_integer_t) -1 )
	{
		switch( option )
		{
			case (system_integer_t) '?':
			default:
				fprintf( stderr, "Invalid argument: %" PRIs_SYSTEM "\n",
				 argv[ optind ] );

				usage_fprint(
				 stdout );

				return( EXIT_FAILURE );

			case (system_integer_t) 'd':
				if( system_string_compare(
				     optarg,
				     _SYSTEM_CHARACTER_T_STRING( "sha1" ),
				     4 ) == 0 )
				{
					calculate_sha1 = 1;
				}
				else
				{
					fprintf( stderr, "Unsupported digest type.\n" );
				}
				break;

			case (system_integer_t) 'h':
				usage_fprint(
				 stdout );

				return( EXIT_SUCCESS );

			case (system_integer_t) 'l':
				log_filename = optarg;

				break;

			case (system_integer_t) 'q':
				callback = NULL;

				break;

			case (system_integer_t) 's':
				swap_byte_pairs = 1;

				break;
			case (system_integer_t) 'v':
				verbose = 1;

				break;

			case (system_integer_t) 'V':
				ewfoutput_copyright_fprint(
				 stdout );

				return( EXIT_SUCCESS );

			case (system_integer_t) 'w':
				wipe_chunk_on_error = 1;

				break;
		}
	}
	if( optind == argc )
	{
		fprintf( stderr, "Missing EWF image file(s).\n" );

		usage_fprint(
		 stdout );

		return( EXIT_FAILURE );
	}
	libewf_set_notify_values(
	 stderr,
	 verbose );

	if( ewfsignal_attach(
	     ewfcommon_signal_handler ) != 1 )
	{
		fprintf( stderr, "Unable to attach signal handler.\n" );
	}
#if !defined( HAVE_GLOB_H )
	glob = ewfglob_alloc();

	if( glob == NULL )
	{
		fprintf( stderr, "Unable to create glob.\n" );

		return( EXIT_FAILURE );
	}
	glob_count = ewfglob_resolve(
	              glob,
	              &argv[ optind ],
	              ( argc - optind ) );

	if( glob_count <= 0 )
	{
		fprintf( stderr, "Unable to resolve glob.\n" );

		ewfglob_free(
		 glob );

		return( EXIT_FAILURE );
	}
	ewfcommon_libewf_handle = libewf_open(
	                           glob->results,
	                           glob->amount,
	                           LIBEWF_OPEN_READ );

	ewfglob_free(
	 glob );
#else
	ewfcommon_libewf_handle = libewf_open(
	                           &argv[ optind ],
	                           ( argc - optind ),
	                           LIBEWF_OPEN_READ );
#endif

	if( ( ewfcommon_abort == 0 )
	 && ( ewfcommon_libewf_handle == NULL ) )
	{
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
		if( errno != 0 )
		{
			error_string = ewfcommon_strerror(
			                errno );
		}
		if( error_string != NULL )
		{
			fprintf( stderr, "Unable to open EWF file(s) with failure: %" PRIs_SYSTEM ".\n",
			 error_string );

			memory_free(
			 error_string );
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
		stored_md5_hash_string = (character_t *) memory_allocate(
		                                          sizeof( character_t ) * EWFSTRING_DIGEST_HASH_LENGTH_MD5 );

		if( stored_md5_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create stored MD5 hash string.\n" );

			return( EXIT_FAILURE );
		}
		calculated_md5_hash_string = (character_t *) memory_allocate( 
		                                              sizeof( character_t )* EWFSTRING_DIGEST_HASH_LENGTH_MD5 );

		if( calculated_md5_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create calculated MD5 hash string.\n" );

			memory_free(
			 stored_md5_hash_string );

			return( EXIT_FAILURE );
		}
	}
	if( calculate_sha1 == 1 )
	{
		stored_sha1_hash_string = (character_t *) memory_allocate( 
		                                           sizeof( character_t )* EWFSTRING_DIGEST_HASH_LENGTH_SHA1 );

		if( stored_sha1_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create stored SHA1 hash string.\n" );

			if( calculate_md5 == 1 )
			{
				memory_free(
				 stored_md5_hash_string );
				memory_free(
				 calculated_md5_hash_string );
			}
			return( EXIT_FAILURE );
		}
		calculated_sha1_hash_string = (character_t *) memory_allocate( 
		                                               sizeof( character_t )* EWFSTRING_DIGEST_HASH_LENGTH_SHA1 );

		if( calculated_sha1_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create calculated SHA1 hash string.\n" );

			if( calculate_md5 == 1 )
			{
				memory_free(
				 stored_md5_hash_string );
				memory_free(
				 calculated_md5_hash_string );
			}
			memory_free(
			 stored_sha1_hash_string );

			return( EXIT_FAILURE );
		}
	}
	if( ewfcommon_abort == 0 )
	{
		/* Start verifying data
		 */
		timestamp_start = time( NULL );

		if( ewfcommon_ctime(
		     &timestamp_start,
		     time_string,
		     32 ) == NULL )
		{
			fprintf( stdout, "Verify started.\n" );
		}
		else
		{
			fprintf( stdout, "Verify started at: %" PRIs_SYSTEM "\n",
			 time_string );
		}
		if( callback != NULL )
		{
			ewfoutput_process_status_initialize(
			 stdout,
			 _CHARACTER_T_STRING( "verified" ),
			 timestamp_start );
		}
		fprintf( stdout, "This could take a while.\n\n" );

		if( calculate_sha1 == 1 )
		{
			if( libewf_parse_hash_values(
			     ewfcommon_libewf_handle ) != 1 )
			{
				fprintf( stderr, "Unable to get parse hash values.\n" );
			}
		}
		count = ewfcommon_read_verify(
			 ewfcommon_libewf_handle,
			 calculate_md5,
			 calculated_md5_hash_string,
			 EWFSTRING_DIGEST_HASH_LENGTH_MD5,
			 calculate_sha1,
			 calculated_sha1_hash_string,
			 EWFSTRING_DIGEST_HASH_LENGTH_SHA1,
			 swap_byte_pairs,
			 wipe_chunk_on_error,
			 callback );
	}
	if( ewfcommon_abort == 0 )
	{
		timestamp_end = time( NULL );

		if( count <= -1 )
		{
			if( ewfcommon_ctime(
			     &timestamp_end,
			     time_string,
			     32 ) == NULL )
			{
				fprintf( stdout, "Verify failed.\n" );
			}
			else
			{
				fprintf( stdout, "Verify failed at: %" PRIs_SYSTEM "\n",
				 time_string );
			}
			if( libewf_close(
			     ewfcommon_libewf_handle ) != 0 )
			{
				fprintf( stderr, "Unable to close EWF file(s).\n" );
			}
			if( calculate_md5 == 1 )
			{
				memory_free(
				 stored_md5_hash_string );
				memory_free(
				 calculated_md5_hash_string );
			}
			if( calculate_sha1 == 1 )
			{
				memory_free(
				 stored_sha1_hash_string );
				memory_free(
				 calculated_sha1_hash_string );
			}
			return( EXIT_FAILURE );
		}
		if( ewfcommon_ctime(
		     &timestamp_end,
		     time_string,
		     32 ) == NULL )
		{
			fprintf( stdout, "Verify completed.\n" );
		}
		else
		{
			fprintf( stdout, "Verify completed at: %" PRIs_SYSTEM "\n",
			 time_string );
		}
		ewfoutput_process_summary_fprint(
		 stdout,
		 _CHARACTER_T_STRING( "Read" ),
		 count,
		 timestamp_start,
		 timestamp_end );

		fprintf( stdout, "\n" );

		if( calculate_md5 == 1 )
		{
			stored_md5_hash_result = libewf_get_md5_hash(
						  ewfcommon_libewf_handle,
						  md5_hash,
						  EWFDIGEST_HASH_SIZE_MD5 );

			if( stored_md5_hash_result == -1 )
			{
				fprintf( stderr, "Unable to get stored MD5 hash.\n" );

				if( libewf_close(
				     ewfcommon_libewf_handle ) != 0 )
				{
					fprintf( stderr, "Unable to close EWF file(s).\n" );
				}
				memory_free(
				 stored_md5_hash_string );
				memory_free(
				 calculated_md5_hash_string );

				if( calculate_sha1 == 1 )
				{
					memory_free(
					 stored_sha1_hash_string );
					memory_free(
					 calculated_sha1_hash_string );
				}
				return( EXIT_FAILURE );
			}
			if( ewfdigest_copy_to_string(
			     md5_hash,
			     EWFDIGEST_HASH_SIZE_MD5,
			     stored_md5_hash_string,
			     EWFSTRING_DIGEST_HASH_LENGTH_MD5 ) != 1 )
			{
				fprintf( stderr, "Unable to get stored MD5 hash string.\n" );

				if( libewf_close(
				     ewfcommon_libewf_handle ) != 0 )
				{
					fprintf( stderr, "Unable to close EWF file(s).\n" );
				}
				memory_free(
				 stored_md5_hash_string );
				memory_free(
				 calculated_md5_hash_string );

				if( calculate_sha1 == 1 )
				{
					memory_free(
					 stored_sha1_hash_string );
					memory_free(
					 calculated_sha1_hash_string );
				}
				return( EXIT_FAILURE );
			}
		}
		if( calculate_sha1 == 1 )
		{
			stored_sha1_hash_result = libewf_get_hash_value(
						   ewfcommon_libewf_handle,
						   _CHARACTER_T_STRING( "SHA1" ),
						   stored_sha1_hash_string,
						   EWFSTRING_DIGEST_HASH_LENGTH_SHA1 );

			if( stored_sha1_hash_result == -1 )
			{
				fprintf( stderr, "Unable to get stored SHA1 hash.\n" );

				if( libewf_close(
				     ewfcommon_libewf_handle ) != 0 )
				{
					fprintf( stderr, "Unable to close EWF file(s).\n" );
				}
				if( calculate_md5 == 1 )
				{
					memory_free(
					 stored_md5_hash_string );
					memory_free(
					 calculated_md5_hash_string );
				}
				memory_free(
				 stored_sha1_hash_string );
				memory_free(
				 calculated_sha1_hash_string );

				return( EXIT_FAILURE );
			}
		}
		if( log_filename != NULL )
		{
			log_file_stream = ewfcommon_fopen(
					   log_filename,
					   _SYSTEM_CHARACTER_T_STRING( "a" ) );

			if( log_file_stream == NULL )
			{
				fprintf( stderr, "Unable to open log file: %s.\n",
				 log_filename );
			}
		}
		ewfoutput_crc_errors_fprint(
		 stdout,
		 ewfcommon_libewf_handle,
		 &amount_of_crc_errors );

		if( log_file_stream != NULL )
		{
			ewfoutput_crc_errors_fprint(
			 log_file_stream,
			 ewfcommon_libewf_handle,
			 &amount_of_crc_errors );
		}
	}
	if( libewf_close(
	     ewfcommon_libewf_handle ) != 0 )
	{
		fprintf( stderr, "Unable to close EWF file(s).\n" );

		if( calculate_md5 == 1 )
		{
			memory_free(
			 stored_md5_hash_string );
			memory_free(
			 calculated_md5_hash_string );
		}
		if( calculate_sha1 == 1 )
		{
			memory_free(
			 stored_sha1_hash_string );
			memory_free(
			 calculated_sha1_hash_string );
		}
		if( log_file_stream != NULL )
		{
			file_io_fclose(
			 log_file_stream );
		}
		return( EXIT_FAILURE );
	}
	if( ewfsignal_detach() != 1 )
	{
		fprintf( stderr, "Unable to detach signal handler.\n" );
	}
	if( ewfcommon_abort != 0 )
	{
		fprintf( stdout, "%" PRIs ": ABORTED\n",
		 program );

		if( calculate_md5 == 1 )
		{
			memory_free(
			 stored_md5_hash_string );
			memory_free(
			 calculated_md5_hash_string );
		}
		if( calculate_sha1 == 1 )
		{
			memory_free(
			 stored_sha1_hash_string );
			memory_free(
			 calculated_sha1_hash_string );
		}
		if( log_file_stream != NULL )
		{
			file_io_fclose(
			 log_file_stream );
		}
		return( EXIT_FAILURE );
	}
	if( calculate_md5 == 1 )
	{
		if( stored_md5_hash_result == 0 )
		{
			fprintf( stdout, "MD5 hash stored in file:\tN/A\n" );

			if( log_file_stream != NULL )
			{
				fprintf( log_file_stream, "MD5 hash stored in file:\tN/A\n" );
			}
		}
		else
		{
			fprintf( stdout, "MD5 hash stored in file:\t%" PRIs "\n",
			 stored_md5_hash_string );

			if( log_file_stream != NULL )
			{
				fprintf( log_file_stream, "MD5 hash stored in file:\t%" PRIs "\n",
				 stored_md5_hash_string );
			}
		}
		fprintf( stdout, "MD5 hash calculated over data:\t%" PRIs "\n",
		 calculated_md5_hash_string );

		if( log_file_stream != NULL )
		{
			fprintf( log_file_stream, "MD5 hash calculated over data:\t%" PRIs "\n",
			 calculated_md5_hash_string );
		}
		match_md5_hash = ( string_compare(
		                    stored_md5_hash_string,
		                    calculated_md5_hash_string,
		                    EWFSTRING_DIGEST_HASH_LENGTH_MD5 ) == 0 );

		memory_free(
		 stored_md5_hash_string );
		memory_free(
		 calculated_md5_hash_string );
	}
	if( calculate_sha1 == 1 )
	{
		if( stored_sha1_hash_result == 0 )
		{
			fprintf( stdout, "SHA1 hash stored in file:\tN/A\n" );

			if( log_file_stream != NULL )
			{
				fprintf( log_file_stream, "SHA1 hash stored in file:\tN/A\n" );
			}
		}
		else
		{
			fprintf( stdout, "SHA1 hash stored in file:\t%" PRIs "\n",
			 stored_sha1_hash_string );

			if( log_file_stream != NULL )
			{
				fprintf( log_file_stream, "SHA1 hash stored in file:\t%" PRIs "\n",
				 stored_sha1_hash_string );
			}
		}
		fprintf( stdout, "SHA1 hash calculated over data:\t%" PRIs "\n",
		 calculated_sha1_hash_string );

		if( log_file_stream != NULL )
		{
			fprintf( log_file_stream, "SHA1 hash calculated over data:\t%" PRIs "\n",
			 calculated_sha1_hash_string );
		}
		match_sha1_hash = ( string_compare(
		                     stored_sha1_hash_string,
		                     calculated_sha1_hash_string,
		                     EWFSTRING_DIGEST_HASH_LENGTH_SHA1 ) == 0 );

		memory_free(
		 stored_sha1_hash_string );
		memory_free(
		 calculated_sha1_hash_string );
	}
	if( log_file_stream != NULL )
	{
		if( file_io_fclose(
		     log_file_stream ) != 0 )
		{
			fprintf( stderr, "Unable to close log file: %s.\n",
			 log_filename );
		}
	}
	/* The EWF file can be verified without an integrity hash
	 */
	if( ( amount_of_crc_errors == 0 )
	 && ( ( calculate_md5 == 0 )
	  || ( stored_md5_hash_result == 0 )
	  || match_md5_hash )
	 && ( ( calculate_sha1 == 0 )
	  || ( stored_sha1_hash_result == 0 )
	  || match_sha1_hash ) )
	{
		fprintf( stdout, "\n%" PRIs ": SUCCESS\n",
		 program );

		result = EXIT_SUCCESS;
	}
	else
	{
		fprintf( stdout, "\n%" PRIs ": FAILURE\n",
		 program );

		result = EXIT_FAILURE;
	}
	return( result );
}

