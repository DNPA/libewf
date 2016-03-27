/*
 * ewfverify
 * Verifies the integrity of the media data within the EWF file
 *
 * Copyright (c) 2006-2009, Joachim Metz <forensics@hoffmannbv.nl>,
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
#include <types.h>

#include <liberror.h>

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

#include "character_string.h"
#include "byte_size_string.h"
#include "digest_context.h"
#include "ewfcommon.h"
#include "ewfgetopt.h"
#include "ewflibewf.h"
#include "ewfoutput.h"
#include "ewfsignal.h"
#include "ewfstring.h"
#include "file_stream_io.h"
#include "glob.h"
#include "md5.h"
#include "notify.h"
#include "process_status.h"
#include "sha1.h"
#include "storage_media_buffer.h"
#include "system_string.h"
#include "verification_handle.h"

/* Prints the executable usage information to the stream
 */
void usage_fprint(
      FILE *stream )
{
	if( stream == NULL )
	{
		return;
	}
	fprintf( stream, "Usage: ewfverify [ -d digest_type ] [ -l log_filename ]\n"
	                 "                 [ -p process_buffer_size ] [ -hqvVw ] ewf_files\n\n" );

	fprintf( stream, "\tewf_files: the first or the entire set of EWF segment files\n\n" );

	fprintf( stream, "\t-d:        calculate additional digest (hash) types besides md5, options: sha1\n" );
	fprintf( stream, "\t-h:        shows this help\n" );
	fprintf( stream, "\t-l:        logs verification errors and the digest (hash) to the log_filename\n" );
	fprintf( stream, "\t-p:        specify the process buffer size (default is the chunk size)\n" );
	fprintf( stream, "\t-q:        quiet shows no status information\n" );
	fprintf( stream, "\t-v:        verbose output to stderr\n" );
	fprintf( stream, "\t-V:        print version\n" );
	fprintf( stream, "\t-w:        wipe sectors on CRC error (mimic EnCase like behavior)\n" );
}

/* Reads the data to calculate the MD5 and SHA1 integrity hashes
 * Returns the amount of bytes read if successful or -1 on error
 */
ssize64_t ewfverify_read_input(
           verification_handle_t *verification_handle,
           uint8_t wipe_chunk_on_error,
           size_t data_buffer_size,
           void (*callback)( process_status_t *process_status, size64_t bytes_read, size64_t bytes_total ),
           liberror_error_t **error )
{
	storage_media_buffer_t *storage_media_buffer = NULL;
	static char *function                        = "ewfverify_read_input";
	off64_t read_offset                          = 0;
	size64_t media_size                          = 0;
	size32_t chunk_size                          = 0;
	size_t read_size                             = 0;
	ssize64_t total_read_count                   = 0;
	ssize_t read_count                           = 0;
#if defined( HAVE_RAW_ACCESS )
	uint8_t *raw_read_data                       = NULL;
	size_t raw_read_buffer_size                  = 0;
	uint32_t sectors_per_chunk                   = 0;
	uint32_t bytes_per_sector                    = 0;
#endif

	if( verification_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid verification handle.",
		 function );

		return( -1 );
	}
	if( data_buffer_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid data buffer size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( verification_handle_get_values(
	     verification_handle,
	     &media_size,
	     &chunk_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine verification handle values.\n",
		 function );

		return( -1 );
	}
	if( chunk_size == 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid chunk size.\n",
		 function );

		return( -1 );
	}
	if( chunk_size > (uint32_t) INT32_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid chunk size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( verification_handle_set_input_values(
	     verification_handle,
	     wipe_chunk_on_error,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set verification handle input values.\n",
		 function );

		return( -1 );
	}
#if defined( HAVE_RAW_ACCESS )
	if( verification_handle_get_raw_access_values(
	     verification_handle,
	     &sectors_per_chunk,
	     &bytes_per_sector,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine verification handle raw access values.\n",
		 function );

		return( -1 );
	}
#endif

#if defined( HAVE_RAW_ACCESS )
	/* Make sure SMART chunks fit in the storage media buffer
	 */
	data_buffer_size = chunk_size + 16;
#else
	if( data_buffer_size == 0 )
	{
		data_buffer_size = chunk_size;
	}
#endif

	if( storage_media_buffer_initialize(
	     &storage_media_buffer,
	     data_buffer_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create storage media buffer.\n",
		 function );

		return( -1 );
	}

	while( total_read_count < (ssize64_t) media_size )
	{
		read_size = chunk_size;

		if( ( media_size - total_read_count ) < read_size )
		{
			read_size = (size_t) ( media_size - total_read_count );
		}
		read_count = verification_handle_read_buffer(
		              verification_handle,
		              storage_media_buffer,
		              read_size,
		              error );

		if( read_count < 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			"%s: unable to read data from verification handle.\n",
			 function );

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
		if( read_count == 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unexpected end of data.\n",
			 function );

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
#if defined( HAVE_RAW_ACCESS )
		read_count = verification_handle_read_prepare_buffer(
		              verification_handle,
		              storage_media_buffer,
		              error );

		if( read_count < 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			"%s: unable to prepare buffer after read.\n",
			 function );

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
#endif
		if( read_count > (ssize_t) read_size )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: more bytes read than requested.\n",
			 function );

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
		if( verification_handle_update_integrity_hash(
		     verification_handle,
		     storage_media_buffer,
		     read_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GENERIC,
			 "%s: unable to update integrity hash(es).\n",
			 function );

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
		read_offset      += (off64_t) read_size;
		total_read_count += (ssize64_t) read_count;

		if( callback != NULL )
		{
			callback(
			 process_status,
			 (size64_t) total_read_count,
			 media_size );
		}
		if( ewfcommon_abort != 0 )
		{
			break;
		}
  	}
	if( storage_media_buffer_free(
	     &storage_media_buffer,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free storage media buffer.\n",
		 function );

		return( -1 );
	}
	return( total_read_count );
}

/* The main program
 */
#if defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
int wmain( int argc, wchar_t * const argv[] )
#else
int main( int argc, char * const argv[] )
#endif
{
	liberror_error_t *error                    = NULL;

#if !defined( HAVE_GLOB_H )
	glob_t *glob                               = NULL;
#endif
	character_t *calculated_md5_hash_string    = NULL;
	character_t *calculated_sha1_hash_string   = NULL;
	character_t *program                       = _CHARACTER_T_STRING( "ewfverify" );
	character_t *stored_md5_hash_string        = NULL;
	character_t *stored_sha1_hash_string       = NULL;

	system_character_t * const *argv_filenames = NULL;
	system_character_t **ewf_filenames         = NULL;
	system_character_t *log_filename           = NULL;

	verification_handle_t *verification_handle = NULL;

	FILE *log_file_stream                      = NULL;
	void *callback                             = &process_status_update;

	system_integer_t option                    = 0;
	ssize64_t verify_count                     = 0;
	size_t string_length                       = 0;
	uint64_t process_buffer_size               = 0;
	uint32_t amount_of_crc_errors              = 0;
	uint8_t calculate_md5                      = 1;
	uint8_t calculate_sha1                     = 0;
	uint8_t wipe_chunk_on_error                = 0;
	uint8_t verbose                            = 0;
	int amount_of_filenames                    = 0;
	int match_md5_hash                         = 0;
	int match_sha1_hash                        = 0;
	int result                                 = 0;
	int status                                 = 0;
	int stored_md5_hash_available              = 0;
	int stored_sha1_hash_available             = 0;

	ewfoutput_version_fprint(
	 stdout,
	 program );

	while( ( option = ewfgetopt(
	                   argc,
	                   argv,
	                   _SYSTEM_CHARACTER_T_STRING( "d:hl:p:qvVw" ) ) ) != (system_integer_t) -1 )
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

			case (system_integer_t) 'p':
				string_length = system_string_length(
				                 optarg );

				result = byte_size_string_convert_system_character(
				          optarg,
				          string_length,
				          &process_buffer_size );

				if( ( result != 1 )
				 || ( process_buffer_size > (uint64_t) SSIZE_MAX ) )
				{
					process_buffer_size = 0;

					fprintf( stderr, "Unsupported process buffer size defaulting to: chunk size.\n" );
				}
				break;

			case (system_integer_t) 'q':
				callback = NULL;

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
	notify_set_values(
	 stderr,
	 verbose );

	if( ewfsignal_attach(
	     ewfcommon_signal_handler ) != 1 )
	{
		fprintf( stderr, "Unable to attach signal handler.\n" );
	}
	amount_of_filenames = argc - optind;

#if !defined( HAVE_GLOB_H )
	if( glob_initialize(
	     &glob ) != 1 )
	{
		fprintf( stderr, "Unable to initialize glob.\n" );

		return( EXIT_FAILURE );
	}
	amount_of_filenames = glob_resolve(
	                       glob,
	                       &argv[ optind ],
	                       amount_of_filenames );

	if( ( amount_of_filenames <= 0 )
	 || ( amount_of_filenames > (int) UINT16_MAX ) )
	{
		fprintf( stderr, "Unable to resolve glob.\n" );

		glob_free(
		 &glob );

		return( EXIT_FAILURE );
	}
	argv_filenames = glob->result;
#else
	argv_filenames = &argv[ optind ];
#endif

	if( verification_handle_initialize(
	     &verification_handle,
	     calculate_md5,
	     calculate_sha1,
	     &error ) != 1 )
	{
		fprintf( stderr, "Unable to create verification handle.\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

#if !defined( HAVE_GLOB_H )
		glob_free(
		 &glob );
#endif

		return( EXIT_FAILURE );
	}
	if( amount_of_filenames == 1 )
	{
		amount_of_filenames = libewf_glob(
		                       argv_filenames[ 0 ],
		                       system_string_length(
		                        argv_filenames[ 0 ] ),
		                       LIBEWF_FORMAT_UNKNOWN,
		                       &ewf_filenames );

		if( amount_of_filenames <= 0 )
		{
			fprintf( stderr, "Unable to resolve EWF filename(s).\n" );

			verification_handle_free(
			 &verification_handle,
			 NULL );

#if !defined( HAVE_GLOB_H )
			glob_free(
			 &glob );
#endif

			return( EXIT_FAILURE );
		}
		argv_filenames = (system_character_t * const *) ewf_filenames;
	}
	result = verification_handle_open_input(
	          verification_handle,
	          argv_filenames,
	          amount_of_filenames,
	          &error );

#if !defined( HAVE_GLOB_H )
	glob_free(
	 &glob );
#endif

	if( ewf_filenames != NULL )
	{
		for( ; amount_of_filenames > 0; amount_of_filenames-- )
		{
fprintf( stderr, "X: %d: %s\n", amount_of_filenames - 1, ewf_filenames[ amount_of_filenames - 1 ] );
			memory_free(
			 ewf_filenames[ amount_of_filenames - 1 ] );
		}
		memory_free(
		 ewf_filenames );
	}
	if( ( ewfcommon_abort == 0 )
	 && ( result != 1 ) )
	{
		fprintf(
		 stderr,
		 "Unable to open EWF image file(s).\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		verification_handle_free(
		 &verification_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( ewfcommon_abort == 0 )
	{
		if( process_status_initialize(
		     &process_status,
		     _CHARACTER_T_STRING( "Verify" ),
		     _CHARACTER_T_STRING( "verified" ),
		     _CHARACTER_T_STRING( "Read" ),
		     stdout ) != 1 )
		{
			fprintf( stderr, "Unable to initialize process status.\n" );

			verification_handle_close(
			 verification_handle,
			 NULL );
			verification_handle_free(
			 &verification_handle,
			 NULL );

			return( EXIT_FAILURE );
		}
		if( process_status_start(
		     process_status ) != 1 )
		{
			fprintf( stderr, "Unable to start process status.\n" );

			process_status_free(
			 &process_status );

			verification_handle_close(
			 verification_handle,
			 NULL );
			verification_handle_free(
			 &verification_handle,
			 NULL );

			return( EXIT_FAILURE );
		}
		/* Start verifying data
		 */
#if defined( USE_LIBEWF_GET_MD5_HASH )
		if( calculate_sha1 == 1 )
		{
			if( libewf_parse_hash_values(
			     ewfcommon_libewf_handle ) != 1 )
			{
				fprintf( stderr, "Unable to get parse hash values.\n" );
			}
		}
#endif
#if defined( USE_LIBEWF_GET_HASH_VALUE_MD5 )
		if( libewf_parse_hash_values(
		     ewfcommon_libewf_handle ) != 1 )
		{
			fprintf( stderr, "Unable to get parse hash values.\n" );
		}
#endif
		verify_count = ewfverify_read_input(
		                verification_handle,
		                wipe_chunk_on_error,
		                (size_t) process_buffer_size,
		                callback,
		                &error );

		if( verify_count <= -1 )
		{
			status = PROCESS_STATUS_FAILED;
		}
		else
		{
			status = PROCESS_STATUS_COMPLETED;
		}
	}
	if( ewfcommon_abort != 0 )
	{
		status = PROCESS_STATUS_ABORTED;
	}
	if( process_status_stop(
	     process_status,
	     (size64_t) verify_count,
	     status ) != 1 )
	{
		fprintf( stderr, "Unable to stop process status.\n" );

		process_status_free(
		 &process_status );

		verification_handle_close(
		 verification_handle,
		 NULL );
		verification_handle_free(
		 &verification_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( process_status_free(
	     &process_status ) != 1 )
	{
		fprintf( stderr, "Unable to free process status.\n" );

		verification_handle_close(
		 verification_handle,
		 NULL );
		verification_handle_free(
		 &verification_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( status == PROCESS_STATUS_COMPLETED )
	{
		if( calculate_md5 == 1 )
		{
			stored_md5_hash_string = (character_t *) memory_allocate(
								  sizeof( character_t ) * EWFSTRING_DIGEST_HASH_LENGTH_MD5 );

			if( stored_md5_hash_string == NULL )
			{
				fprintf( stderr, "Unable to create stored MD5 hash string.\n" );

				verification_handle_close(
				 verification_handle,
				 NULL );
				verification_handle_free(
				 &verification_handle,
				 NULL );

				return( EXIT_FAILURE );
			}
			calculated_md5_hash_string = (character_t *) memory_allocate(
								      sizeof( character_t )* EWFSTRING_DIGEST_HASH_LENGTH_MD5 );

			if( calculated_md5_hash_string == NULL )
			{
				fprintf( stderr, "Unable to create calculated MD5 hash string.\n" );

				memory_free(
				 stored_md5_hash_string );

				verification_handle_close(
				 verification_handle,
				 NULL );
				verification_handle_free(
				 &verification_handle,
				 NULL );

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
				verification_handle_close(
				 verification_handle,
				 NULL );
				verification_handle_free(
				 &verification_handle,
				 NULL );

				return( EXIT_FAILURE );
			}
			calculated_sha1_hash_string = (character_t *) memory_allocate(
								       sizeof( character_t )* EWFSTRING_DIGEST_HASH_LENGTH_SHA1 );

			if( calculated_sha1_hash_string == NULL )
			{
				fprintf( stderr, "Unable to create calculated SHA1 hash string.\n" );

				memory_free(
				 stored_sha1_hash_string );

				if( calculate_md5 == 1 )
				{
					memory_free(
					 stored_md5_hash_string );
					memory_free(
					 calculated_md5_hash_string );
				}
				verification_handle_close(
				 verification_handle,
				 NULL );
				verification_handle_free(
				 &verification_handle,
				 NULL );

				return( EXIT_FAILURE );
			}
		}
		if( verification_handle_finalize(
		     verification_handle,
		     calculated_md5_hash_string,
		     EWFSTRING_DIGEST_HASH_LENGTH_MD5,
		     stored_md5_hash_string,
		     EWFSTRING_DIGEST_HASH_LENGTH_MD5,
		     &stored_md5_hash_available,
		     calculated_sha1_hash_string,
		     EWFSTRING_DIGEST_HASH_LENGTH_SHA1,
		     stored_sha1_hash_string,
		     EWFSTRING_DIGEST_HASH_LENGTH_SHA1,
		     &stored_sha1_hash_available,
		     &error ) != 1 )
		{
			fprintf( stderr, "Unable to finalize verification handle.\n" );

			notify_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			if( calculate_sha1 == 1 )
			{
				memory_free(
				 stored_sha1_hash_string );
				memory_free(
				 calculated_sha1_hash_string );
			}
			if( calculate_md5 == 1 )
			{
				memory_free(
				 stored_md5_hash_string );
				memory_free(
				 calculated_md5_hash_string );
			}
			verification_handle_close(
			 verification_handle,
			 NULL );
			verification_handle_free(
			 &verification_handle,
			 NULL );

			return( EXIT_FAILURE );
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
		fprintf(
		 stdout,
		 "\n" );

		/* TODO */
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
		if( calculate_md5 == 1 )
		{
			if( stored_md5_hash_available == 0 )
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
		}
		if( calculate_sha1 == 1 )
		{
			if( stored_sha1_hash_available == 0 )
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
		}
		ewfoutput_hash_values_fprint(
		 stdout,
		 verification_handle->input_handle,
		 "",
		 calculate_md5,
		 calculate_sha1 );

		if( log_file_stream != NULL )
		{
			ewfoutput_hash_values_fprint(
			 log_file_stream,
			 verification_handle->input_handle,
			 "",
			 calculate_md5,
			 calculate_sha1 );
		}
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
	if( verification_handle_close(
	     verification_handle,
	     &error ) != 0 )
	{
		fprintf( stderr, "Unable to close EWF file(s).\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		if( log_file_stream != NULL )
		{
			file_stream_io_fclose(
			 log_file_stream );
		}
		verification_handle_free(
		 &verification_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( verification_handle_free(
	     &verification_handle,
	     &error ) != 1 )
	{
		fprintf( stderr, "Unable to free verification handle.\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		if( log_file_stream != NULL )
		{
			file_stream_io_fclose(
			 log_file_stream );
		}
		return( EXIT_FAILURE );
	}
	if( status != PROCESS_STATUS_COMPLETED )
	{
		if( log_file_stream != NULL )
		{
			file_stream_io_fclose(
			 log_file_stream );
		}
		return( EXIT_FAILURE );
	}
	if( ewfsignal_detach() != 1 )
	{
		fprintf( stderr, "Unable to detach signal handler.\n" );
	}
	if( log_file_stream != NULL )
	{
		if( file_stream_io_fclose(
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
	  || ( stored_md5_hash_available == 0 )
	  || match_md5_hash )
	 && ( ( calculate_sha1 == 0 )
	  || ( stored_sha1_hash_available == 0 )
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

