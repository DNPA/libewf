/*
 * ewfalter
 * Alters media data in EWF files
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
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include "alteration_handle.h"
#include "byte_size_string.h"
#include "ewfgetopt.h"
#include "ewfinput.h"
#include "ewfoutput.h"
#include "ewfsignal.h"
#include "glob.h"
#include "notify.h"

#define EWFALTER_INPUT_BUFFER_SIZE	64

alteration_handle_t *ewfalter_alteration_handle = NULL;
int ewfalter_abort                              = 0;

/* Prints the executable usage information to the stream
 *  */
void usage_fprint(
      FILE *stream )
{
        if( stream == NULL )
        {
                return;
        }
	fprintf( stream, "Use ewfalter to test the libewf delta segment file support.\n\n" );

	fprintf( stream, "Usage: ewfalter [ -p process_buffer_size ] [ -t target_file ] [ -hqvV ]\n"
	                 "                ewf_files\n\n" );

	fprintf( stream, "\tewf_files: the first or the entire set of EWF segment files\n\n" );

	fprintf( stream, "\t-h:        shows this help\n" );
	fprintf( stream, "\t-q:        quiet shows no status information\n" );
	fprintf( stream, "\t-p:        specify the process buffer size (default is the chunk size)\n" );
	fprintf( stream, "\t-t:        specify the target delta path and base filename (default is the same\n"
	                 "\t           as the ewf_files)\n" );
	fprintf( stream, "\t-v:        verbose output to stderr\n" );
	fprintf( stream, "\t-V:        print version\n" );
}

/* Alters a specific size of the input starting from a specific offset 
 * Returns 1 the amount of bytes altered or -1 on error
 */
ssize64_t ewfalter_alter_input(
           alteration_handle_t *alteration_handle,
           size64_t alter_size,
           off64_t alter_offset,
           size_t process_buffer_size,
           liberror_error_t **error )
{
	storage_media_buffer_t *storage_media_buffer = NULL;
	static char *function                        = "ewfalter_alter_input";
	ssize64_t alter_count                        = 0;
	size_t write_size                            = 0;
	ssize_t process_count                        = 0;
	ssize_t write_count                          = 0;
	uint32_t chunk_size                          = 0;

	if( alteration_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid alteration handle.",
		 function );

		return( -1 );
	}
	if( process_buffer_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid process buffer size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( alteration_handle_get_chunk_size(
	     alteration_handle,
	     &chunk_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve chunk size.",
		 function );

		return( -1 );
	}
	if( chunk_size == 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_OUT_OF_RANGE,
		 "%s: invalid chunk size.",
		 function );

		return( -1 );
	}
#if defined( HAVE_RAW_ACCESS )
	/* Make sure SMART chunks fit in the storage media buffer
	 */
	process_buffer_size = (size_t) chunk_size;
#else
	if( process_buffer_size == 0 )
	{
		process_buffer_size = (size_t) chunk_size;
	}
#endif

	if( storage_media_buffer_initialize(
	     &storage_media_buffer,
	     process_buffer_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create storage media buffer.",
		 function );

		return( -1 );
	}
	/* Fill the storage media buffer with X
	 */
	if( memory_set(
	     storage_media_buffer->raw_buffer,
	     'X',
	     process_buffer_size ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to set storage media buffer.",
		 function );

		storage_media_buffer_free(
		 &storage_media_buffer,
		 NULL );

		return( -1 );
	}
	storage_media_buffer->raw_buffer_amount = process_buffer_size;

	/* Find the first alteration offset
	 */
	if( alteration_handle_seek_offset(
	     alteration_handle,
	     alter_offset,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek alteration offset.",
		 function );

		storage_media_buffer_free(
		 &storage_media_buffer,
		 NULL );

		return( -1 );
	}
	while( alter_count < (ssize64_t) alter_size )
	{
		if( alter_size > (size64_t) process_buffer_size )
		{
			write_size = process_buffer_size;
		}
		else
		{
			write_size = (size_t) alter_size;
		}
		process_count = alteration_handle_write_prepare_buffer(
		                 alteration_handle,
		                 storage_media_buffer,
		                 error );

		if( process_count < 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			"%s: unable to prepare buffer before write.",
			 function );

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
		write_count = alteration_handle_write_buffer(
		               alteration_handle,
		               storage_media_buffer,
		               process_count,
		               error );

		if( write_count < 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_WRITE_FAILED,
			 "%s: unable to write data to file.",
			 function );

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
		alter_count += write_size;

		if( ewfalter_abort != 0 )
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
		 "%s: unable to free storage media buffer.",
		 function );

		return( -1 );
	}
	return( alter_count );
}

/* Signal handler for ewfalter
 */
void ewfalter_signal_handler(
      ewfsignal_t signal )
{
	liberror_error_t *error = NULL;
	static char *function   = "ewfalter_signal_handler";

	ewfalter_abort = 1;

	if( ( ewfalter_alteration_handle != NULL )
	 && ( alteration_handle_signal_abort(
	       ewfalter_alteration_handle,
	       &error ) != 1 ) )
	{
		notify_warning_printf(
		 "%s: unable to signal alteration handle to abort.\n",
		 function );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		return;
	}
	/* Force stdin to close otherwise any function reading it will remain blocked
	 */
	if( file_io_close(
	     0 ) != 0 )
	{
		notify_warning_printf(
		 "%s: unable to close stdin.\n",
		 function );
	}
}

/* The main program
 */
#if defined( HAVE_WIDE_SYSTEM_CHARACTER_T )
int wmain( int argc, wchar_t * const argv[] )
#else
int main( int argc, char * const argv[] )
#endif
{
	system_character_t input_buffer[ EWFALTER_INPUT_BUFFER_SIZE ];

	alteration_handle_t *alteration_handle     = NULL;

#if !defined( HAVE_GLOB_H )
	glob_t *glob                               = NULL;
#endif

	liberror_error_t *error                    = NULL;

	system_character_t * const *argv_filenames = NULL;

	system_character_t *program                = _SYSTEM_CHARACTER_T_STRING( "ewfalter" );
	system_character_t *target_filename        = NULL;

	system_integer_t option                    = 0;
	size64_t media_size                        = 0;
	ssize64_t alter_count                      = 0;
	size_t string_length                       = 0;
	uint64_t alter_offset                      = 0;
	uint64_t alter_size                        = 0;
	uint64_t process_buffer_size               = 0;
	uint8_t verbose                            = 0;
	int amount_of_filenames                    = 0;
	int result                                 = 0;

	notify_set_values(
	 stderr,
	 1 );

	if( system_string_initialize(
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to initialize system string.\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		return( EXIT_FAILURE );
	}
	ewfoutput_version_fprint(
	 stdout,
	 program );

	fprintf(
	 stdout,
	 "%" PRIs_SYSTEM " is for testing purposes only.\n",
	 program );

	while( ( option = ewfgetopt(
			   argc,
			   argv,
			   _SYSTEM_CHARACTER_T_STRING( "hp:qt:vV" ) ) ) != (system_integer_t) -1 )
	{
		switch( option )
		{
			case (system_integer_t) '?':
			default:
				fprintf(
				 stderr,
				 "Invalid argument: %" PRIs_SYSTEM ".\n",
				 argv[ optind ] );

				usage_fprint(
				 stdout );

				return( EXIT_FAILURE );

			case (system_integer_t) 'h':
				usage_fprint(
				 stdout );

				return( EXIT_SUCCESS );

			case (system_integer_t) 'p':
				string_length = system_string_length(
						 optarg );

				result = byte_size_string_convert(
					  optarg,
					  string_length,
					  &process_buffer_size,
					  &error );

				if( result != 1 )
				{
					notify_error_backtrace(
					 error );
					liberror_error_free(
					 &error );
				}
				if( ( result != 1 )
				 || ( process_buffer_size > (uint64_t) SSIZE_MAX ) )
				{
					process_buffer_size = 0;

					fprintf(
					 stderr,
					 "Unsupported process buffer size defaulting to: chunk size.\n" );
				}
				break;

			case (system_integer_t) 'q':
				break;

			case (system_integer_t) 't':
				target_filename = optarg;

				break;

			case (system_integer_t) 'v':
				verbose = 1;

				break;

			case (system_integer_t) 'V':
				ewfoutput_copyright_fprint(
				 stdout );

				return( EXIT_SUCCESS );
		}
	}
	if( optind == argc )
	{
		fprintf(
		 stderr,
		 "Missing EWF image file(s).\n" );

		usage_fprint(
		 stdout );

		return( EXIT_FAILURE );
	}
	notify_set_values(
	 stderr,
	 verbose );
	libewf_set_notify_values(
	 stderr,
	 verbose );

	if( ewfsignal_attach(
	     ewfalter_signal_handler ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to attach signal handler.\n" );
	}
#if !defined( HAVE_GLOB_H )
	if( glob_initialize(
	     &glob,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to initialize glob.\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		return( EXIT_FAILURE );
	}
	if( glob_resolve(
	     glob,
	     &argv[ optind ],
	     argc - optind,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to resolve glob.\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		glob_free(
		 &glob,
		 NULL );

		return( EXIT_FAILURE );
	}
	argv_filenames      = glob->result;
	amount_of_filenames = glob->amount_of_results;
#else
	argv_filenames      = &argv[ optind ];
	amount_of_filenames = argc - optind;
#endif

	if( alteration_handle_initialize(
	     &alteration_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to create alteration handle.\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

#if !defined( HAVE_GLOB_H )
		glob_free(
		 &glob,
		 NULL );
#endif

		return( EXIT_FAILURE );
	}
	result = alteration_handle_open_input(
	          alteration_handle,
	          argv_filenames,
	          amount_of_filenames,
	          &error );

#if !defined( HAVE_GLOB_H )
	if( glob_free(
	     &glob,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free glob.\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		return( EXIT_FAILURE );
	}
#endif

	if( ( ewfalter_abort == 0 )
	 && ( result != 1 ) )
	{
		fprintf(
		 stderr,
		 "Unable to open EWF image file(s).\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		alteration_handle_free(
		 &alteration_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( ( ewfalter_abort == 0 )
	 && ( alteration_handle_get_media_size(
	       alteration_handle,
	       &media_size,
	       &error ) != 1 ) )
	{
		fprintf(
		 stderr,
		 "Unable to determine media size.\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		alteration_handle_close(
		 alteration_handle,
		 NULL );
		alteration_handle_free(
		 &alteration_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( ewfalter_abort == 0 )
	{
		if( ewfsignal_detach() != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to detach signal handler.\n" );
		}
		/* Request the necessary case data
		 */
		fprintf(
		 stdout,
		 "Information for alter required, please provide the necessary input\n" );

		if( ewfinput_get_size_variable(
		     stdout,
		     input_buffer,
		     EWFALTER_INPUT_BUFFER_SIZE,
		     _SYSTEM_CHARACTER_T_STRING( "Start altering at offset" ),
		     0,
		     media_size,
		     0,
		     &alter_offset ) == -1 )
		{
			alter_offset = 0;

			fprintf(
			 stdout,
			 "Unable to determine the altertion offset defaulting to: %" PRIu64 ".\n",
			 alter_offset );
		}
		if( ewfinput_get_size_variable(
		     stdout,
		     input_buffer,
		     EWFALTER_INPUT_BUFFER_SIZE,
		     _SYSTEM_CHARACTER_T_STRING( "Amount of bytes to alter" ),
		     0,
		     ( media_size - alter_offset ),
		     ( media_size - alter_offset ),
		     &alter_size ) == -1 )
		{
			alter_size = media_size - alter_offset;

			fprintf(
			 stdout,
			 "Unable to determine the altertion size defaulting to: %" PRIu64 ".\n",
			 alter_size );
		}
		if( process_buffer_size == 0 )
		{
			if( ewfinput_get_size_variable(
			     stdout,
			     input_buffer,
			     EWFALTER_INPUT_BUFFER_SIZE,
			     _SYSTEM_CHARACTER_T_STRING( "Alteration buffer size" ),
			     1,
			     SSIZE_MAX,
			     ( 64 * 512 ),
			     &process_buffer_size ) == -1 )
			{
				process_buffer_size = 64 * 512;

				fprintf(
				 stdout,
				 "Unable to determine the altertion buffer size defaulting to: %" PRIu64 ".\n",
				 process_buffer_size );
			}
			if( process_buffer_size > (size_t) SSIZE_MAX )
			{
				process_buffer_size = 64 * 512;

				fprintf(
				 stdout,
				 "Invalid alteration buffer size defaulting to: %" PRIu64 ".\n",
				 process_buffer_size );
			}
		}
		if( target_filename != NULL )
		{
			if( alteration_handle_set_output_values(
			     alteration_handle,
			     target_filename,
			     system_string_length(
			      target_filename ),
			     &error ) != 1 )
			{
				fprintf(
				 stderr,
				 "Unable to set delta segment filename in handle.\n" );

				notify_error_backtrace(
				 error );
				liberror_error_free(
				 &error );

				alteration_handle_close(
				 alteration_handle,
				 NULL );
				alteration_handle_free(
				 &alteration_handle,
				 NULL );

				return( EXIT_FAILURE );
			}
		}
		fprintf(
		 stderr,
		 "\n" );

		if( ewfsignal_attach(
		     ewfalter_signal_handler ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to attach signal handler.\n" );
		}
	}
	if( ewfalter_abort == 0 )
	{
		/* First alteration run
		 */
		alter_count = ewfalter_alter_input(
		               alteration_handle,
		               (size64_t) alter_size,
		               (off64_t) alter_offset,
		               (size_t) process_buffer_size,
		               &error );

		if( alter_count <= -1 )
		{
			fprintf(
			 stdout,
			 "Alteration failed.\n" );

			notify_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			alteration_handle_close(
			 alteration_handle,
			 NULL );
			alteration_handle_free(
			 &alteration_handle,
			 NULL );

			return( EXIT_FAILURE );
		}
	}
	if( ewfalter_abort == 0 )
	{
		/* Second alteration run
		 */
		alter_count = ewfalter_alter_input(
		               alteration_handle,
		               (size64_t) alter_size,
		               (off64_t) alter_offset,
		               (size_t) process_buffer_size,
		               &error );

		if( alter_count <= -1 )
		{
			fprintf(
			 stdout,
			 "Alteration failed.\n" );

			notify_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			alteration_handle_close(
			 alteration_handle,
			 NULL );
			alteration_handle_free(
			 &alteration_handle,
			 NULL );

			return( EXIT_FAILURE );
		}
	}
	if( alteration_handle_close(
	     alteration_handle,
	     &error ) != 0 )
	{
		fprintf(
		 stderr,
		 "Unable to close EWF file(s).\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		alteration_handle_free(
		 &alteration_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( alteration_handle_free(
	     &alteration_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free alteration handle.\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		return( EXIT_FAILURE );
	}
	if( ewfsignal_detach() != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to detach signal handler.\n" );
	}
	if( ewfalter_abort != 0 )
	{
		fprintf(
		 stdout,
		 "%s: ABORTED\n",
		 program );

		return( EXIT_FAILURE );
	}
	fprintf(
	 stdout,
	 "Alteration completed.\n" );

	return( EXIT_SUCCESS );
}

