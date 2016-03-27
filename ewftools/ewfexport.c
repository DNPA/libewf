/*
 * ewfexport
 * Export media data from EWF files to a file
 *
 * Copyright (c) 2006-2011, Joachim Metz <jbmetz@users.sourceforge.net>
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

#include <libcstring.h>
#include <liberror.h>

#if defined( HAVE_STDLIB_H ) || defined( WINAPI )
#include <stdlib.h>
#endif

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include <libsystem.h>

#include "byte_size_string.h"
#include "digest_context.h"
#include "ewfcommon.h"
#include "ewfinput.h"
#include "ewfoutput.h"
#include "export_handle.h"
#include "log_handle.h"
#include "md5.h"
#include "platform.h"
#include "process_status.h"
#include "sha1.h"
#include "storage_media_buffer.h"

#define EWFEXPORT_INPUT_BUFFER_SIZE		64

export_handle_t *ewfexport_export_handle = NULL;
int ewfexport_abort                      = 0;

/* Prints the executable usage information to the stream
 */
void usage_fprint(
      FILE *stream )
{
	libcstring_system_character_t default_segment_file_size_string[ 16 ];
	libcstring_system_character_t minimum_segment_file_size_string[ 16 ];
	libcstring_system_character_t maximum_32bit_segment_file_size_string[ 16 ];
	libcstring_system_character_t maximum_64bit_segment_file_size_string[ 16 ];

	int result = 0;

	if( stream == NULL )
	{
		return;
	}
	result = byte_size_string_create(
	          default_segment_file_size_string,
	          16,
	          EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE,
	          BYTE_SIZE_STRING_UNIT_MEBIBYTE,
	          NULL );

	if( result == 1 )
	{
		result = byte_size_string_create(
			  minimum_segment_file_size_string,
			  16,
			  EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE,
			  BYTE_SIZE_STRING_UNIT_MEBIBYTE,
		          NULL );
	}
	if( result == 1 )
	{
		result = byte_size_string_create(
			  maximum_32bit_segment_file_size_string,
			  16,
			  EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT,
			  BYTE_SIZE_STRING_UNIT_MEBIBYTE,
		          NULL );
	}
	if( result == 1 )
	{
		result = byte_size_string_create(
			  maximum_64bit_segment_file_size_string,
			  16,
			  EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT,
			  BYTE_SIZE_STRING_UNIT_MEBIBYTE,
		          NULL );
	}
	fprintf( stream, "Use ewfexport to export data from the EWF format (Expert Witness Compression\n"
	                 "Format) to raw data or another EWF format.\n\n" );

	fprintf( stream, "Usage: ewfexport [ -A codepage ] [ -b number_of_sectors ]\n"
	                 "                 [ -B number_of_bytes ] [ -c compression_level ]\n"
	                 "                 [ -d digest_type ] [ -f format ] [ -l log_filename ]\n"
	                 "                 [ -o offset ] [ -p process_buffer_size ]\n"
	                 "                 [ -S segment_file_size ] [ -t target ] [ -hqsuvVw ] ewf_files\n\n" );

	fprintf( stream, "\tewf_files: the first or the entire set of EWF segment files\n\n" );

	fprintf( stream, "\t-A:        codepage of header section, options: ascii (default),\n"
	                 "\t           windows-874, windows-1250, windows-1251, windows-1252,\n"
	                 "\t           windows-1253, windows-1254, windows-1255, windows-1256,\n"
	                 "\t           windows-1257, windows-1258\n" );
	fprintf( stream, "\t-b:        specify the number of sectors to read at once (per chunk),\n"
	                 "\t           options: 16, 32, 64 (default), 128, 256, 512, 1024, 2048, 4096,\n"
	                 "\t           8192, 16384 or 32768 (not used for raw and files formats)\n" );
	fprintf( stream, "\t-B:        specify the number of bytes to export (default is all bytes)\n" );
	fprintf( stream, "\t-c:        specify the compression level, options: none (default),\n"
	                 "\t           empty-block, fast or best (not used for raw and files format)\n" );
	fprintf( stream, "\t-d:        calculate additional digest (hash) types besides md5,\n"
	                 "\t           options: sha1 (not used for raw and files format)\n" );
	fprintf( stream, "\t-f:        specify the output format to write to, options:\n"
	                 "\t           raw (default), files, ewf, smart, encase1, encase2, encase3,\n"
	                 "\t           encase4, encase5, encase6, linen5, linen6, ewfx\n"
	                 "\t           (the files format is restricted to logical volume files)\n" );
	fprintf( stream, "\t-h:        shows this help\n" );
	fprintf( stream, "\t-l:        logs export errors and the digest (hash) to the log_filename\n" );
	fprintf( stream, "\t-o:        specify the offset to start the export (default is 0)\n" );
	fprintf( stream, "\t-p:        specify the process buffer size (default is the chunk size)\n" );
	fprintf( stream, "\t-q:        quiet shows minimal status information\n" );
	fprintf( stream, "\t-s:        swap byte pairs of the media data (from AB to BA)\n"
	                 "\t           (use this for big to little endian conversion and vice\n"
	                 "\t           versa)\n" );

	if( result == 1 )
	{
		fprintf( stream, "\t-S:        specify the segment file size in bytes (default is %" PRIs_LIBCSTRING_SYSTEM ")\n"
		                 "\t           (minimum is %" PRIs_LIBCSTRING_SYSTEM ", maximum is %" PRIs_LIBCSTRING_SYSTEM " for raw and encase6\n"
		                 "\t           and %" PRIs_LIBCSTRING_SYSTEM " for other formats)\n"
		                 "\t           (not used for files format)\n",
		 default_segment_file_size_string,
		 minimum_segment_file_size_string,
		 maximum_64bit_segment_file_size_string,
		 maximum_32bit_segment_file_size_string );
	}
	else
	{
		fprintf( stream, "\t-S:        specify the segment file size in bytes (default is %" PRIu32 ")\n"
		                 "\t           (minimum is %" PRIu32 ", maximum is %" PRIu64 " for raw and encase6 format\n"
		                 "\t           and %" PRIu32 " for other formats)\n"
		                 "\t           (not used for files format)\n",
		 (uint32_t) EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE,
		 (uint32_t) EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE,
		 (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT,
		 (uint32_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT );
	}

	fprintf( stream, "\t-t:        specify the target file to export to, use - for stdout\n"
	                 "\t           (default is export) stdout is only supported for the raw\n"
	                 "\t           format\n" );
	fprintf( stream, "\t-u:        unattended mode (disables user interaction)\n" );
	fprintf( stream, "\t-v:        verbose output to stderr\n" );
	fprintf( stream, "\t-V:        print version\n" );
	fprintf( stream, "\t-w:        wipe sectors on checksum error (mimic EnCase like behavior)\n" );
}

/* Reads the media data and exports it
 * Returns the number of bytes read on success or -1 on error
 */
ssize64_t ewfexport_export_image(
           export_handle_t *export_handle,
           size64_t media_size,
           size64_t export_size,
           off64_t read_offset,
           uint8_t swap_byte_pairs,
           size_t process_buffer_size,
           process_status_t *process_status,
	   liberror_error_t **error )
{
	storage_media_buffer_t *storage_media_buffer        = NULL;
	static char *function                               = "ewfexport_export_image";
	ssize64_t export_count                              = 0;
	size32_t input_chunk_size                           = 0;
	size_t read_size                                    = 0;
	ssize_t read_count                                  = 0;
	ssize_t read_process_count                          = 0;
	ssize_t write_count                                 = 0;
	ssize_t write_process_count                         = 0;

#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
	storage_media_buffer_t *output_storage_media_buffer = NULL;
	uint8_t *input_buffer                               = NULL;
	size32_t output_chunk_size                          = 0;
	size_t write_size                                   = 0;
#endif

	if( export_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid export handle.",
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
	if( export_handle_get_input_chunk_size(
	     export_handle,
	     &input_chunk_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine the input chunk size.",
		 function );

		return( -1 );
	}
	if( input_chunk_size == 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid input chunk size.",
		 function );

		return( -1 );
	}
	if( ( export_size == 0 )
	 || ( export_size > media_size )
	 || ( export_size > (ssize64_t) INT64_MAX ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid size value out of bounds.",
		 function );

		return( -1 );
	}
	if( process_status == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid process status.",
		 function );

		return( -1 );
	}
	if( read_offset > 0 )
	{
		if( read_offset >= (off64_t) media_size )
			{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: invalid offset.",
			 function );

			goto on_error;
		}
		if( ( export_size + read_offset ) > media_size )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: unable to export beyond size of media.",
			 function );

			goto on_error;
		}
		if( export_handle_seek_offset(
		     export_handle,
		     read_offset,
		     error ) == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_SEEK_FAILED,
			 "%s: unable to seek offset.",
			 function );

			goto on_error;
		}
	}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
	if( export_handle_get_output_chunk_size(
	     export_handle,
	     &output_chunk_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to determine the output chunk size.",
		 function );

		goto on_error;
	}
	if( output_chunk_size == 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid output chunk size.",
		 function );

		goto on_error;
	}
	process_buffer_size = (size_t) input_chunk_size;
#else
	if( process_buffer_size == 0 )
	{
		process_buffer_size = (size_t) input_chunk_size;
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

		goto on_error;
	}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
	if( storage_media_buffer_initialize(
	     &output_storage_media_buffer,
	     output_chunk_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create output storage media buffer.",
		 function );

		goto on_error;
	}
#endif
	while( export_count < (int64_t) export_size )
	{
		read_size = process_buffer_size;

		if( ( media_size - export_count ) < read_size )
		{
			read_size = (size_t) ( media_size - export_count );
		}
		read_count = export_handle_read_buffer(
		              export_handle,
		              storage_media_buffer,
		              read_size,
		              error );

		if( read_count < 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read data.",
			 function );

			goto on_error;
		}
		if( read_count == 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unexpected end of data.",
			 function );

			goto on_error;
		}
		read_process_count = export_handle_prepare_read_buffer(
		                      export_handle,
		                      storage_media_buffer,
		                      error );

		if( read_process_count < 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to prepare buffer after read.",
			 function );

			goto on_error;
		}
		if( read_process_count > (ssize_t) read_size )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: more bytes read than requested.",
			 function );

			goto on_error;
		}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
		/* Set the chunk data size in the compression buffer
		 */
		if( storage_media_buffer->data_in_compression_buffer == 1 )
		{
			storage_media_buffer->compression_buffer_data_size = (size_t) read_process_count;
		}
#endif
		/* Swap byte pairs
		 */
		if( swap_byte_pairs == 1 )
		{
			if( export_handle_swap_byte_pairs(
			     export_handle,
			     storage_media_buffer,
			     read_process_count,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_CONVERSION,
				 LIBERROR_CONVERSION_ERROR_GENERIC,
				 "%s: unable to swap byte pairs.",
				 function );

				goto on_error;
			}
		}
		/* Digest hashes are calcultated after swap
		 */
		if( export_handle_update_integrity_hash(
		     export_handle,
		     storage_media_buffer,
		     read_process_count,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GENERIC,
			 "%s: unable to update integrity hash(es).",
			 function );

			goto on_error;
		}
		export_count += read_process_count;

		while( read_process_count > 0 )
		{
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
			if( read_process_count > output_chunk_size )
			{
				write_size = output_chunk_size;
			}
			else
			{
				write_size = (size_t) read_process_count;
			}
			if( ( output_storage_media_buffer->raw_buffer_data_size + write_size ) > output_chunk_size )
			{
				write_size = output_chunk_size -  output_storage_media_buffer->raw_buffer_data_size;
			}
			if( storage_media_buffer->data_in_compression_buffer == 1 )
			{
				input_buffer = storage_media_buffer->compression_buffer;
			}
			else
			{
				input_buffer = storage_media_buffer->raw_buffer;
			}
			if( memory_copy(
			     &( output_storage_media_buffer->raw_buffer[ output_storage_media_buffer->raw_buffer_data_size ] ),
			     input_buffer,
			     write_size ) == NULL )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_MEMORY,
				 LIBERROR_MEMORY_ERROR_COPY_FAILED,
				 "%s: unable to copy data from input buffer to output raw buffer.",
				 function );

				goto on_error;
			}
			output_storage_media_buffer->raw_buffer_data_size += write_size;

			/* Make sure the output chunk is filled upto the output chunk size
			 */
			if( ( export_count < (int64_t) export_size )
			 && ( output_storage_media_buffer->raw_buffer_data_size < output_chunk_size ) )
			{
				continue;
			}
#endif
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
			write_process_count = export_handle_prepare_write_buffer(
			                       export_handle,
			                       output_storage_media_buffer,
			                       error );
#else
			write_process_count = export_handle_prepare_write_buffer(
			                       export_handle,
			                       storage_media_buffer,
			                       error );
#endif

			if( write_process_count < 0 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				"%s: unable to prepare buffer before write.",
				 function );

				goto on_error;
			}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
			write_count = export_handle_write_buffer(
				       export_handle,
				       output_storage_media_buffer,
				       write_process_count,
				       error );
#else
			write_count = export_handle_write_buffer(
				       export_handle,
				       storage_media_buffer,
				       write_process_count,
				       error );
#endif

			if( write_count < 0 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_WRITE_FAILED,
				 "%s: unable to write data to file.",
				 function );

				goto on_error;
			}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
			output_storage_media_buffer->raw_buffer_data_size = 0;
#endif
			read_process_count -= write_process_count;
		}
		if( process_status_update(
		     process_status,
		     (size64_t) export_count,
		     export_size,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to update process status.",
			 function );

			goto on_error;
		}
		if( ewfexport_abort != 0 )
		{
			break;
		}
  	}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
	if( storage_media_buffer_free(
	     &output_storage_media_buffer,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free output storage media buffer.",
		 function );

		goto on_error;
	}
#endif
	if( storage_media_buffer_free(
	     &storage_media_buffer,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free input storage media buffer.",
		 function );

		goto on_error;
	}
	write_count = export_handle_finalize(
	               export_handle,
	               error );

	if( write_count == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to finalize.",
		 function );

		goto on_error;
	}
	return( export_count );

on_error:
	if( storage_media_buffer != NULL )
	{
		storage_media_buffer_free(
		 &storage_media_buffer,
		 NULL );
	}
	return( -1 );

}

/* Signal handler for ewfexport
 */
void ewfexport_signal_handler(
      libsystem_signal_t signal )
{
	liberror_error_t *error = NULL;
	static char *function   = "ewfexport_signal_handler";

	ewfexport_abort = 1;

	if( ewfexport_export_handle != NULL )
	{
		if( export_handle_signal_abort(
		     ewfexport_export_handle,
		     &error ) != 1 )
		{
			libsystem_notify_printf(
			 "%s: unable to signal export handle to abort.\n",
			 function );

			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			return;
		}
	}
	/* Force stdin to close otherwise any function reading it will remain blocked
	 */
	if( libsystem_file_io_close(
	     0 ) != 0 )
	{
		libsystem_notify_printf(
		 "%s: unable to close stdin.\n",
		 function );
	}
}

/* The main program
 */
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
int wmain( int argc, wchar_t * const argv[] )
#else
int main( int argc, char * const argv[] )
#endif
{
	libcstring_system_character_t acquiry_operating_system[ 32 ];
	libcstring_system_character_t input_buffer[ EWFEXPORT_INPUT_BUFFER_SIZE ];

	libcstring_system_character_t * const *argv_filenames      = NULL;

	liberror_error_t *error                                    = NULL;

#if !defined( LIBSYSTEM_HAVE_GLOB )
	libsystem_glob_t *glob                                     = NULL;
#endif

	libcstring_system_character_t *acquiry_software_version    = NULL;
	libcstring_system_character_t *log_filename                = NULL;
	libcstring_system_character_t *option_compression_level    = NULL;
	libcstring_system_character_t *option_format               = NULL;
	libcstring_system_character_t *option_header_codepage      = NULL;
	libcstring_system_character_t *option_maximum_segment_size = NULL;
	libcstring_system_character_t *option_offset               = NULL;
	libcstring_system_character_t *option_sectors_per_chunk    = NULL;
	libcstring_system_character_t *option_size                 = NULL;
	libcstring_system_character_t *option_target_filename      = NULL;
	libcstring_system_character_t *program                     = _LIBCSTRING_SYSTEM_STRING( "ewfexport" );
	libcstring_system_character_t *request_string              = NULL;

	log_handle_t *log_handle                                   = NULL;

	process_status_t *process_status                           = NULL;

	libcstring_system_integer_t option                         = 0;
	size64_t media_size                                        = 0;
	ssize64_t export_count                                     = 0;
	size_t string_length                                       = 0;
	uint64_t process_buffer_size                               = EWFCOMMON_PROCESS_BUFFER_SIZE;
	uint8_t calculate_md5                                      = 1;
	uint8_t calculate_sha1                                     = 0;
	uint8_t print_status_information                           = 1;
	uint8_t swap_byte_pairs                                    = 0;
	uint8_t verbose                                            = 0;
	uint8_t wipe_chunk_on_error                                = 0;
	int interactive_mode                                       = 1;
	int number_of_filenames                                    = 0;
	int result                                                 = 1;
	int status                                                 = 0;

/* TODO refactor */
	uint64_t export_offset                                     = 0;
	uint64_t export_size                                       = 0;

	libsystem_notify_set_stream(
	 stderr,
	 NULL );
	libsystem_notify_set_verbose(
	 1 );

	if( libsystem_initialize(
	     "ewftools",
	     &error ) != 1 )
	{
		ewfoutput_version_fprint(
		 stderr,
		 program );

		fprintf(
		 stderr,
		 "Unable to initialize system values.\n" );

		goto on_error;
	}
#if defined( WINAPI )
#if defined( _MSC_VER )
	if( _setmode(
	     _fileno(
	      stdout ),
	     _O_BINARY ) == -1 )
#else
	if( setmode(
	     _fileno(
	      stdout ),
	     _O_BINARY ) == -1 )
#endif
	{
		ewfoutput_version_fprint(
		 stderr,
		 program );

		fprintf(
		 stderr,
		 "Unable to set stdout to binary mode.\n" );

		usage_fprint(
		 stdout );

		goto on_error;
	}
#endif
	while( ( option = libsystem_getopt(
	                   argc,
	                   argv,
	                   _LIBCSTRING_SYSTEM_STRING( "A:b:B:c:d:f:hl:o:p:qsS:t:uvVw" ) ) ) != (libcstring_system_integer_t) -1 )
	{
		switch( option )
		{
			case (libcstring_system_integer_t) '?':
			default:
				ewfoutput_version_fprint(
				 stderr,
				 program );

				fprintf(
				 stderr,
				 "Invalid argument: %" PRIs_LIBCSTRING_SYSTEM ".\n",
				 argv[ optind ] );

				usage_fprint(
				 stderr );

				goto on_error;

			case (libcstring_system_integer_t) 'A':
				option_header_codepage = optarg;

				break;

			case (libcstring_system_integer_t) 'b':
				option_sectors_per_chunk = optarg;

				break;

			case (libcstring_system_integer_t) 'B':
				option_size = optarg;

				break;

			case (libcstring_system_integer_t) 'c':
				option_compression_level = optarg;

				break;

			case (libcstring_system_integer_t) 'd':
				if( libcstring_system_string_compare(
				     optarg,
				     _LIBCSTRING_SYSTEM_STRING( "sha1" ),
				     4 ) == 0 )
				{
					calculate_sha1 = 1;
				}
				else
				{
					fprintf(
					 stderr,
					 "Unsupported digest type.\n" );
				}
				break;

			case (libcstring_system_integer_t) 'f':
				option_format = optarg;

				break;

			case (libcstring_system_integer_t) 'h':
				ewfoutput_version_fprint(
				 stderr,
				 program );

				usage_fprint(
				 stderr );

				return( EXIT_SUCCESS );

			case (libcstring_system_integer_t) 'l':
				log_filename = optarg;

				break;

			case (libcstring_system_integer_t) 'o':
				option_offset = optarg;

				break;

			case (libcstring_system_integer_t) 'p':
				string_length = libcstring_system_string_length(
				                 optarg );

				result = byte_size_string_convert(
				          optarg,
				          string_length,
				          &process_buffer_size,
				          &error );

				if( result != 1 )
				{
					libsystem_notify_print_error_backtrace(
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

			case (libcstring_system_integer_t) 'q':
				print_status_information = 0;

				break;

			case (libcstring_system_integer_t) 's':
				swap_byte_pairs = 1;

				break;

			case (libcstring_system_integer_t) 'S':
				option_maximum_segment_size = optarg;

				break;

			case (libcstring_system_integer_t) 't':
				option_target_filename = optarg;

				break;

			case (libcstring_system_integer_t) 'u':
				interactive_mode = 0;

				break;

			case (libcstring_system_integer_t) 'v':
				verbose = 1;

				break;

			case (libcstring_system_integer_t) 'V':
				ewfoutput_version_fprint(
				 stderr,
				 program );

				ewfoutput_copyright_fprint(
				 stderr );

				return( EXIT_SUCCESS );

			case (libcstring_system_integer_t) 'w':
				wipe_chunk_on_error = 1;

				break;
		}
	}
	if( optind == argc )
	{
		ewfoutput_version_fprint(
		 stderr,
		 program );

		fprintf(
		 stderr,
		 "Missing EWF image file(s).\n" );

		usage_fprint(
		 stderr );

		goto on_error;
	}
	ewfoutput_version_fprint(
	 stderr,
	 program );

	libsystem_notify_set_verbose(
	 verbose );
	libewf_notify_set_verbose(
	 verbose );
	libewf_notify_set_stream(
	 stderr,
	 NULL );

#if !defined( LIBSYSTEM_HAVE_GLOB )
	if( libsystem_glob_initialize(
	     &glob,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to initialize glob.\n" );

		goto on_error;
	}
	if( libsystem_glob_resolve(
	     glob,
	     &( argv[ optind ] ),
	     argc - optind,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to resolve glob.\n" );

		goto on_error;
	}
	argv_filenames      = glob->result;
	number_of_filenames = glob->number_of_results;
#else
	argv_filenames      = &( argv[ optind ] );
	number_of_filenames = argc - optind;
#endif

	if( export_handle_initialize(
	     &ewfexport_export_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to create export handle.\n" );

		goto on_error;
	}
	if( libsystem_signal_attach(
	     ewfexport_signal_handler,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to attach signal handler.\n" );

		libsystem_notify_print_error_backtrace(
		 error );
		liberror_error_free(
		 &error );
	}
	result = export_handle_open_input(
	          ewfexport_export_handle,
	          argv_filenames,
	          number_of_filenames,
	          &error );

	if( ewfexport_abort != 0 )
	{
		goto on_abort;
	}
	if( result != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to open EWF file(s).\n" );

		goto on_error;
	}
#if !defined( LIBSYSTEM_HAVE_GLOB )
	if( libsystem_glob_free(
	     &glob,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free glob.\n" );

		goto on_error;
	}
#endif
	if( export_handle_get_input_media_size(
	     ewfexport_export_handle,
	     &media_size,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to retrieve input media size.\n" );

		goto on_error;
	}
	if( option_header_codepage != NULL )
	{
		result = export_handle_set_header_codepage(
			  ewfexport_export_handle,
		          option_header_codepage,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set header codepage.\n" );

			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf(
			 stderr,
			 "Unsuported header codepage defaulting to: ascii.\n" );
		}
	}
	if( option_target_filename != NULL )
	{
		if( export_handle_set_string(
		     ewfexport_export_handle,
		     option_target_filename,
		     &( ewfexport_export_handle->target_filename ),
		     &( ewfexport_export_handle->target_filename_size ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set target filename.\n" );

			goto on_error;
		}
	}
	else if( interactive_mode == 0 )
	{
		/* Make sure the target filename is set in unattended mode
		 */
		if( export_handle_set_string(
		     ewfexport_export_handle,
		     _LIBCSTRING_SYSTEM_STRING( "export" ),
		     &( ewfexport_export_handle->target_filename ),
		     &( ewfexport_export_handle->target_filename_size ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set target filename.\n" );

			goto on_error;
		}
	}
	if( option_compression_level != NULL )
	{
		result = export_handle_set_compression_values(
			  ewfexport_export_handle,
			  option_compression_level,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set compression values.\n" );

			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf(
			 stderr,
			 "Unsupported compression level defaulting to: none.\n" );
		}
	}
	if( option_format != NULL )
	{
		result = export_handle_set_format(
			  ewfexport_export_handle,
			  option_format,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set format.\n" );

			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf(
			 stderr,
			 "Unsupported output format defaulting to: raw.\n" );
		}
	}
	if( option_sectors_per_chunk != NULL )
	{
		result = export_handle_set_sectors_per_chunk(
			  ewfexport_export_handle,
			  option_sectors_per_chunk,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set sectors per chunk.\n" );

			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf(
			 stderr,
			 "Unsuported sectors per chunk defaulting to: 64.\n" );
		}
	}
	if( option_maximum_segment_size != NULL )
	{
		result = export_handle_set_maximum_segment_size(
			  ewfexport_export_handle,
			  option_maximum_segment_size,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set maximum segment size.\n" );

			goto on_error;
		}
		if( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_EWF )
		{
			if( ( result == 0 )
			 || ( ewfexport_export_handle->maximum_segment_size < EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE )
			 || ( ( ewfexport_export_handle->ewf_format == LIBEWF_FORMAT_ENCASE6 )
			  &&  ( ewfexport_export_handle->maximum_segment_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT ) )
			 || ( ( ewfexport_export_handle->ewf_format != LIBEWF_FORMAT_ENCASE6 )
			  &&  ( ewfexport_export_handle->maximum_segment_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT ) ) )
			{
				ewfexport_export_handle->maximum_segment_size = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;

				fprintf(
				 stderr,
				 "Unsuported maximum segment size defaulting to: %" PRIu64 ".\n",
				 ewfexport_export_handle->maximum_segment_size );
			}
		}
		else if( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_RAW )
		{
			if( ( result == 0 )
			 || ( ( ewfexport_export_handle->maximum_segment_size != 0 )
			  &&  ( ewfexport_export_handle->maximum_segment_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT ) ) )
			{
				ewfexport_export_handle->maximum_segment_size = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;

				fprintf(
				 stderr,
				 "Unsuported maximum segment size defaulting to: %" PRIu64 ".\n",
				 ewfexport_export_handle->maximum_segment_size );
			}
		}
	}
	if( option_offset != NULL )
	{
		string_length = libcstring_system_string_length(
				 option_offset );

		if( libsystem_string_to_uint64(
		     option_offset,
		     string_length + 1,
		     &export_offset,
		     &error ) != 1 )
		{
			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			export_offset = 0;

			fprintf(
			 stderr,
			 "Unsupported export offset defaulting to: %" PRIu64 ".\n",
			 export_offset );
		}
	}
	if( option_size != NULL )
	{
		string_length = libcstring_system_string_length(
				 option_size );

		if( libsystem_string_to_uint64(
		     option_size,
		     string_length + 1,
		     &export_size,
		     &error ) != 1 )
		{
			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			export_size = 0;

			fprintf(
			 stderr,
			 "Unsupported export size defaulting to: all bytes.\n" );
		}
	}
	/* Initialize values
	 */
	if( ( export_size == 0 )
	 || ( export_size > ( media_size - export_offset ) ) )
	{
		export_size = media_size - export_offset;
	}
	/* Request the necessary case data
	 */
	if( interactive_mode != 0 )
	{
		if( libsystem_signal_detach(
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to detach signal handler.\n" );

			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );
		}
		fprintf(
		 stderr,
		 "Information for export required, please provide the necessary input\n" );

		if( option_format == NULL )
		{
			result = export_handle_prompt_for_format(
				  ewfexport_export_handle,
			          _LIBCSTRING_SYSTEM_STRING( "Export to format" ),
				  &error );

			if( result == -1 )
			{
				fprintf(
				 stderr,
				 "Unable to determine format.\n" );

				goto on_error;
			}
		}
		if( option_target_filename == NULL )
		{
			if( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_EWF )
			{
				request_string = _LIBCSTRING_SYSTEM_STRING( "Target path and filename without extension" );
			}
			else if( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_FILES )
			{
				request_string = _LIBCSTRING_SYSTEM_STRING( "Target path" );
			}
			else if( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_RAW )
			{
				request_string = _LIBCSTRING_SYSTEM_STRING( "Target path and filename without extension or - for stdout" );
			}
		}
		if( request_string != NULL )
		{
			do
			{
				result = export_handle_prompt_for_string(
					  ewfexport_export_handle,
					  request_string,
					  &( ewfexport_export_handle->target_filename ),
					  &( ewfexport_export_handle->target_filename_size ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine target.\n" );

					goto on_error;
				}
				else if( result == 0 )
				{
					fprintf(
					 stdout,
					 "Target is required, please try again or terminate using Ctrl^C.\n" );
				}
			}
			while( result != 1 );
		}
		if( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_EWF )
		{
			if( option_compression_level == NULL )
			{
				result = export_handle_prompt_for_compression_level(
					  ewfexport_export_handle,
				          _LIBCSTRING_SYSTEM_STRING( "Use compression" ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine compression level.\n" );

					goto on_error;
				}
			}
			if( option_maximum_segment_size == NULL )
			{
				result = export_handle_prompt_for_maximum_segment_size(
					  ewfexport_export_handle,
				          _LIBCSTRING_SYSTEM_STRING( "Evidence segment file size in bytes" ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine maximum segment size.\n" );

					goto on_error;
				}
				if( ( ewfexport_export_handle->maximum_segment_size < EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE )
				 || ( ( ewfexport_export_handle->ewf_format == LIBEWF_FORMAT_ENCASE6 )
				  &&  ( ewfexport_export_handle->maximum_segment_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT ) )
				 || ( ( ewfexport_export_handle->ewf_format != LIBEWF_FORMAT_ENCASE6 )
				  &&  ( ewfexport_export_handle->maximum_segment_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT ) ) )
				{
					ewfexport_export_handle->maximum_segment_size = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;

					fprintf(
					 stderr,
					 "C. Unsuported maximum segment size defaulting to: %" PRIu64 ".\n",
					 ewfexport_export_handle->maximum_segment_size );
				}
			}
			if( option_sectors_per_chunk == NULL )
			{
				result = export_handle_prompt_for_sectors_per_chunk(
					  ewfexport_export_handle,
				          _LIBCSTRING_SYSTEM_STRING( "The number of sectors to read at once" ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine sectors per chunk.\n" );

					goto on_error;
				}
			}
		}
		else if( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_RAW )
		{
			if( ( ewfexport_export_handle->target_filename != NULL )
			 && ( ( ewfexport_export_handle->target_filename )[ 0 ] == (libcstring_system_character_t) '-' )
			 && ( ( ewfexport_export_handle->target_filename )[ 1 ] == 0 ) )
			{
				/* No need for segment files when exporting to stdout */
			}
			else if( option_maximum_segment_size == NULL )
			{
				result = export_handle_prompt_for_maximum_segment_size(
					  ewfexport_export_handle,
				          _LIBCSTRING_SYSTEM_STRING( "Evidence segment file size in bytes (0 is unlimited)" ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine maximum segment size.\n" );

					goto on_error;
				}
				if( ( ewfexport_export_handle->maximum_segment_size != 0 )
				 && ( ewfexport_export_handle->maximum_segment_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT ) )
				{
					ewfexport_export_handle->maximum_segment_size = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;

					fprintf(
					 stderr,
					 "Unsuported maximum segment size defaulting to: %" PRIu64 ".\n",
					 ewfexport_export_handle->maximum_segment_size );
				}
			}
		}
		if( ( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_EWF )
		 || ( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_RAW ) )
		{
			if( option_offset == NULL )
			{
				if( ewfinput_get_size_variable(
				     stderr,
				     input_buffer,
				     EWFEXPORT_INPUT_BUFFER_SIZE,
				     _LIBCSTRING_SYSTEM_STRING( "Start export at offset" ),
				     0,
				     media_size,
				     export_offset,
				     &export_offset,
				     &error ) == -1 )
				{
					libsystem_notify_print_error_backtrace(
					 error );
					liberror_error_free(
					 &error );

					export_offset = 0;

					fprintf(
					 stderr,
					 "Unable to determine export offset defaulting to: %" PRIu64 ".\n",
					 export_offset );
				}
			}
			if( option_size == NULL )
			{
				if( ewfinput_get_size_variable(
				     stderr,
				     input_buffer,
				     EWFEXPORT_INPUT_BUFFER_SIZE,
				     _LIBCSTRING_SYSTEM_STRING( "Number of bytes to export" ),
				     0,
				     media_size - export_offset,
				     export_size,
				     &export_size,
				     &error ) == -1 )
				{
					libsystem_notify_print_error_backtrace(
					 error );
					liberror_error_free(
					 &error );

					export_size = media_size - export_offset;

					fprintf(
					 stderr,
					 "Unable to determine export size defaulting to: %" PRIu64 ".\n",
					 export_size );
				}
			}
		}
		if( libsystem_signal_attach(
		     ewfexport_signal_handler,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to attach signal handler.\n" );

			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );
		}
	}
	else
	{
		if( ewfexport_export_handle->maximum_segment_size == 0 )
		{
			if( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_EWF )
			{
				if( ewfexport_export_handle->ewf_format == LIBEWF_FORMAT_ENCASE6 )
				{
					ewfexport_export_handle->maximum_segment_size = EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT;
				}
				else
				{
					ewfexport_export_handle->maximum_segment_size = EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT;
				}
			}
			else if( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_RAW )
			{
				ewfexport_export_handle->maximum_segment_size = EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT;
			}
		}
	}
	fprintf(
	 stderr,
	 "\n" );

	if( process_status_initialize(
	     &process_status,
	     _LIBCSTRING_SYSTEM_STRING( "Export" ),
	     _LIBCSTRING_SYSTEM_STRING( "exported" ),
	     _LIBCSTRING_SYSTEM_STRING( "Written" ),
	     stderr,
	     print_status_information,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to initialize process status.\n" );

		goto on_error;
	}
	if( process_status_start(
	     process_status,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to start process status.\n" );

		goto on_error;
	}
	if( ewfexport_export_handle->output_format == EXPORT_HANDLE_OUTPUT_FORMAT_FILES )
	{
		if( export_handle_export_single_files(
		     ewfexport_export_handle,
		     ewfexport_export_handle->target_filename,
		     NULL,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to export single files.\n" );

			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			status = PROCESS_STATUS_FAILED;
		}
		else
		{
			status = PROCESS_STATUS_COMPLETED;
		}
	}
	else
	{
		if( export_handle_set_processing_values(
		     ewfexport_export_handle,
		     calculate_md5,
		     calculate_sha1,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set processing values.\n" );

			goto on_error;
		}
		if( export_handle_open_output(
		     ewfexport_export_handle,
		     ewfexport_export_handle->target_filename,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to open output.\n" );

			goto on_error;
		}
		if( platform_get_operating_system(
		     acquiry_operating_system,
		     32,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to determine operating system.\n" );

			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			acquiry_operating_system[ 0 ] = 0;
		}
		acquiry_software_version = _LIBCSTRING_SYSTEM_STRING( LIBEWF_VERSION_STRING );

		if( export_handle_set_output_values(
		     ewfexport_export_handle,
		     acquiry_operating_system,
		     program,
		     acquiry_software_version,
		     export_size,
		     wipe_chunk_on_error,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set output values.\n" );

			goto on_error;
		}
		/* Exports image media data
		 */
		export_count = ewfexport_export_image(
				ewfexport_export_handle,
				media_size,
				export_size,
				export_offset,
				swap_byte_pairs,
				(size_t) process_buffer_size,
				process_status,
				&error );

		if( export_count <= -1 )
		{
			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			status = PROCESS_STATUS_FAILED;
		}
		else
		{
			status = PROCESS_STATUS_COMPLETED;
		}
	}
	if( process_status_stop(
	     process_status,
	     (size64_t) export_count,
	     status,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to stop process status.\n" );

		goto on_error;
	}
	if( process_status_free(
	     &process_status,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free process status.\n" );

		goto on_error;
	}
	if( ewfexport_abort != 0 )
	{
		status = PROCESS_STATUS_ABORTED;
	}
	if( status == PROCESS_STATUS_COMPLETED )
	{
		if( ewfexport_export_handle->output_format != EXPORT_HANDLE_OUTPUT_FORMAT_FILES )
		{
			if( log_filename != NULL )
			{
				if( log_handle_initialize(
				     &log_handle,
				     &error ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unable to create log handle.\n" );

					goto on_error;
				}
				if( log_handle_open(
				     log_handle,
				     log_filename,
				     &error ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unable to open log file: %" PRIs_LIBCSTRING_SYSTEM ".\n",
					 log_filename );

					goto on_error;
				}
			}
			if( export_handle_hash_values_fprint(
			     ewfexport_export_handle,
			     stdout,
			     &error ) != 1 )
			{
				fprintf(
				 stderr,
				 "Unable to print export hash values.\n" );

				goto on_error;
			}
			if( log_handle != NULL )
			{
				if( export_handle_hash_values_fprint(
				     ewfexport_export_handle,
				     log_handle->log_stream,
				     &error ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unable to write export hash values in log file.\n" );

					goto on_error;
				}
			}
			if( export_handle_checksum_errors_fprint(
			     ewfexport_export_handle,
			     stdout,
			     &error ) != 1 )
			{
				fprintf(
				 stderr,
				 "Unable to print export errors.\n" );

				goto on_error;
			}
			if( log_handle != NULL )
			{
				if( export_handle_checksum_errors_fprint(
				     ewfexport_export_handle,
				     log_handle->log_stream,
				     &error ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unable to write export errors in log file.\n" );

					goto on_error;
				}
			}
		}
	}
	if( log_handle != NULL )
	{
		if( log_handle_close(
		     log_handle,
		     &error ) != 0 )
		{
			fprintf(
			 stderr,
			 "Unable to close log file: %" PRIs_LIBCSTRING_SYSTEM ".\n",
			 log_filename );

			goto on_error;
		}
		if( log_handle_free(
		     &log_handle,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to free log handle.\n" );

			goto on_error;
		}
	}
on_abort:
	if( export_handle_close(
	     ewfexport_export_handle,
	     &error ) != 0 )
	{
		fprintf(
		 stderr,
		 "Unable to close export handle.\n" );

		goto on_error;
	}
	if( libsystem_signal_detach(
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to detach signal handler.\n" );

		libsystem_notify_print_error_backtrace(
		 error );
		liberror_error_free(
		 &error );
	}
	if( export_handle_free(
	     &ewfexport_export_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free export handle.\n" );

		goto on_error;
	}
	if( status != PROCESS_STATUS_COMPLETED )
	{
		return( EXIT_FAILURE );
	}
	return( EXIT_SUCCESS );

on_error:
	if( error != NULL )
	{
		libsystem_notify_print_error_backtrace(
		 error );
		liberror_error_free(
		 &error );
	}
	if( log_handle != NULL )
	{
		log_handle_close(
		 log_handle,
		 NULL );
		log_handle_free(
		 &log_handle,
		 NULL );
	}
	if( process_status != NULL )
	{
		process_status_free(
		 &process_status,
		 NULL );
	}
	if( ewfexport_export_handle != NULL )
	{
		export_handle_close(
		 ewfexport_export_handle,
		 NULL );
		export_handle_free(
		 &ewfexport_export_handle,
		 NULL );
	}
#if !defined( LIBSYSTEM_HAVE_GLOB )
	if( glob != NULL )
	{
		libsystem_glob_free(
		 &glob,
		 NULL );
	}
#endif
	return( EXIT_FAILURE );
}

