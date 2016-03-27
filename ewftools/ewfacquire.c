/*
 * ewfacquire
 * Reads data from a file or device and writes it in EWF format
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

/*
#define TOC_FILE
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
#include "device_handle.h"
#include "ewfcommon.h"
#include "ewfinput.h"
#include "ewfoutput.h"
#include "imaging_handle.h"
#include "log_handle.h"
#include "process_status.h"
#include "storage_media_buffer.h"

#if defined( __BORLANDC__ ) && ( __BORLANDC__ < 0x0560 )
#define EWFACQUIRE_2_TIB			0x20000000000UL
#else
#define EWFACQUIRE_2_TIB			0x20000000000ULL
#endif

#define EWFACQUIRE_INPUT_BUFFER_SIZE		64

device_handle_t *ewfacquire_device_handle   = NULL;
imaging_handle_t *ewfacquire_imaging_handle = NULL;
int ewfacquire_abort                        = 0;

/* Prints the executable usage information to the stream
 */
void ewfacquire_usage_fprint(
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
	fprintf( stream, "Use ewfacquire to acquire data from a file or device and store it in the EWF\n"
	                 "format (Expert Witness Compression Format).\n\n" );

	fprintf( stream, "Usage: ewfacquire [ -A codepage ] [ -b number_of_sectors ]\n"
	                 "                  [ -B number_of_bytes ] [ -c compression_level ]\n"
	                 "                  [ -C case_number ] [ -d digest_type ] [ -D description ]\n"
	                 "                  [ -e examiner_name ] [ -E evidence_number ] [ -f format ]\n"
	                 "                  [ -g number_of_sectors ] [ -l log_filename ]\n"
	                 "                  [ -m media_type ] [ -M media_flags ] [ -N notes ]\n"
	                 "                  [ -o offset ] [ -p process_buffer_size ]\n"
	                 "                  [ -P bytes_per_sector ] [ -r read_error_retries ]\n"
	                 "                  [ -S segment_file_size ] [ -t target ] [ -T toc_file ]\n"
	                 "                  [ -2 secondary_target ] [ -hqRsuvVw ] source\n\n" );

	fprintf( stream, "\tsource: the source file(s) or device\n\n" );

	fprintf( stream, "\t-A:     codepage of header section, options: ascii (default),\n"
	                 "\t        windows-874, windows-1250, windows-1251, windows-1252,\n"
	                 "\t        windows-1253, windows-1254, windows-1255, windows-1256,\n"
	                 "\t        windows-1257, windows-1258\n" );
	fprintf( stream, "\t-b:     specify the number of sectors to read at once (per chunk),\n"
	                 "\t        options: 16, 32, 64 (default), 128, 256, 512, 1024, 2048, 4096,\n"
	                 "\t        8192, 16384 or 32768\n" );
	fprintf( stream, "\t-B:     specify the number of bytes to acquire (default is all bytes)\n" );
	fprintf( stream, "\t-c:     specify the compression level, options: none (default),\n"
	                 "\t        empty-block, fast or best\n" );
	fprintf( stream, "\t-C:     specify the case number (default is case_number).\n" );
	fprintf( stream, "\t-d:     calculate additional digest (hash) types besides md5, options:\n"
	                 "\t        sha1\n" );
	fprintf( stream, "\t-D:     specify the description (default is description).\n" );
	fprintf( stream, "\t-e:     specify the examiner name (default is examiner_name).\n" );
	fprintf( stream, "\t-E:     specify the evidence number (default is evidence_number).\n" );
	fprintf( stream, "\t-g      specify the number of sectors to be used as error granularity,\n"
	                 "\t        options: 16, 32, 64 (default), 128, 256, 512, 1024, 2048, 4096,\n"
	                 "\t        8192, 16384 or 32768\n" );
	fprintf( stream, "\t-f:     specify the EWF file format to write to, options: ewf, smart,\n"
	                 "\t        ftk, encase2, encase3, encase4, encase5, encase6 (default),\n"
	                 "\t        linen5, linen6, ewfx\n" );
	fprintf( stream, "\t-h:     shows this help\n" );
	fprintf( stream, "\t-l:     logs acquiry errors and the digest (hash) to the log_filename\n" );
	fprintf( stream, "\t-m:     specify the media type, options: fixed (default), removable,\n"
	                 "\t        optical, memory\n" );
	fprintf( stream, "\t-M:     specify the media flags, options: logical, physical (default)\n" );
	fprintf( stream, "\t-N:     specify the notes (default is notes).\n" );
	fprintf( stream, "\t-o:     specify the offset to start to acquire (default is 0)\n" );
	fprintf( stream, "\t-p:     specify the process buffer size (default is the chunk size)\n" );
	fprintf( stream, "\t-P:     specify the number of bytes per sector (default is 512)\n"
	                 "\t        (use this to override the automatic bytes per sector detection)\n" );
	fprintf( stream, "\t-q:     quiet shows minimal status information\n" );
	fprintf( stream, "\t-r:     specify the number of retries when a read error occurs (default\n"
	                 "\t        is 2)\n" );
	fprintf( stream, "\t-R:     resume acquiry at a safe point\n" );
	fprintf( stream, "\t-s:     swap byte pairs of the media data (from AB to BA)\n"
	                 "\t        (use this for big to little endian conversion and vice versa)\n" );

	if( result == 1 )
	{
		fprintf( stream, "\t-S:     specify the segment file size in bytes (default is %" PRIs_LIBCSTRING_SYSTEM ")\n"
		                 "\t        (minimum is %" PRIs_LIBCSTRING_SYSTEM ", maximum is %" PRIs_LIBCSTRING_SYSTEM " for encase6 format\n"
		                 "\t        and %" PRIs_LIBCSTRING_SYSTEM " for other formats)\n",
		 default_segment_file_size_string,
		 minimum_segment_file_size_string,
		 maximum_64bit_segment_file_size_string,
		 maximum_32bit_segment_file_size_string );
	}
	else
	{
		fprintf( stream, "\t-S:     specify the segment file size in bytes (default is %" PRIu32 ")\n"
		                 "\t        (minimum is %" PRIu32 ", maximum is %" PRIu64 " for encase6 format\n"
		                 "\t        and %" PRIu32 " for other formats)\n",
		 (uint32_t) EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE,
		 (uint32_t) EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE,
		 (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT,
		 (uint32_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT );
	}

	fprintf( stream, "\t-t:     specify the target file (without extension) to write to\n" );
#ifdef TOC_FILE
	fprintf( stream, "\t-T:     specify the file containing the table of contents (TOC) of\n"
	                 "\t        an optical disc. The TOC file must be in the CUE format.\n" );
#endif
	fprintf( stream, "\t-u:     unattended mode (disables user interaction)\n" );
	fprintf( stream, "\t-v:     verbose output to stderr\n" );
	fprintf( stream, "\t-V:     print version\n" );
	fprintf( stream, "\t-w:     wipe sectors on read error (mimic EnCase like behavior)\n" );
	fprintf( stream, "\t-2:     specify the secondary target file (without extension) to write\n"
	                 "\t        to\n" );
}

/* Prints an overview of the acquiry parameters and asks the for confirmation
 * Returns 1 if confirmed by user, 0 otherwise or -1 on error
 */
int8_t ewfacquire_confirm_acquiry_parameters(
        FILE *stream,
        libcstring_system_character_t *input_buffer,
        size_t input_buffer_size,
        liberror_error_t **error )
{
	libcstring_system_character_t *fixed_string_variable = NULL;
	int8_t input_confirmed                               = -1;
	int result                                           = 0;

	if( stream == NULL )
	{
		return( -1 );
	}
	/* Ask for confirmation
	 */
	while( input_confirmed == -1 )
	{
		result = ewfinput_get_fixed_string_variable(
		          stream,
		          input_buffer,
		          input_buffer_size,
		          _LIBCSTRING_SYSTEM_STRING( "Continue acquiry with these values" ),
		          ewfinput_yes_no,
		          2,
		          0,
		          &fixed_string_variable,
		          error );

		if( result == -1 )
		{
			if( ( error != NULL )
			 && ( *error != NULL ) )
			{
				libsystem_notify_print_error_backtrace(
				 *error );
			}
			liberror_error_free(
			 error );

			fprintf(
			 stream,
			 "Unable to determine answer.\n" );
		}
		else
		{
			result = ewfinput_determine_yes_no(
				  fixed_string_variable,
				  (uint8_t *) &input_confirmed,
				  error );

			if( result != 1 )
			{
				if( ( error != NULL )
				 && ( *error != NULL ) )
				{
					libsystem_notify_print_error_backtrace(
					 *error );
				}
				liberror_error_free(
				 error );

				fprintf(
				 stream,
				 "Selected option not supported, please try again or terminate using Ctrl^C.\n" );

				input_confirmed = -1;
			}
		}
	}
	fprintf(
	 stream,
	 "\n" );

	return( input_confirmed );
}

/* Determines the sessions of an optical disc using the device handle
 * and appends them to the imaging handle, if the device is a file
 * a single session is simulated
 * Returns 1 if successful or -1 on error
 */
int ewfacquire_determine_sessions(
     imaging_handle_t *imaging_handle,
     device_handle_t *device_handle,
     uint64_t media_size,
     liberror_error_t **error )
{
	static char *function      = "ewfacquire_determine_sessions";
	uint64_t number_of_sectors = 0;
	uint64_t start_sector      = 0;
	uint8_t type               = 0;
	int number_of_sessions     = 0;
	int session_index          = 0;

	if( imaging_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid imaging handle.",
		 function );

		return( -1 );
	}
	if( imaging_handle->bytes_per_sector == 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid imaging handle - missing bytes per sector.",
		 function );

		return( -1 );
	}
	if( media_size > (uint64_t) UINT32_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid media size value out of bounds.",
		 function );

		return( -1 );
	}
	if( device_handle_get_number_of_sessions(
	     device_handle,
	     &number_of_sessions,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of sessions.",
		 function );

		return( -1 );
	}
	if( number_of_sessions == 0 )
	{
		if( device_handle_get_type(
		     device_handle,
		     &type,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve device handle type.",
			 function );

			return( -1 );
		}
		if( type != DEVICE_HANDLE_TYPE_FILE )
		{
			fprintf(
			 stderr,
			 "Unable to determine number of session on optical disc - defaulting to single session.\n" );
		}
		if( imaging_handle_append_session(
		     ewfacquire_imaging_handle,
		     (uint64_t) 0,
		     (uint64_t) ( media_size / imaging_handle->bytes_per_sector ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to append session to imaging handle.",
			 function );

			return( -1 );
		}
	}
	else
	{
		for( session_index = 0;
		     session_index < number_of_sessions;
		     session_index++ )
		{
			if( device_handle_get_session(
			     device_handle,
			     session_index,
			     &start_sector,
			     &number_of_sectors,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve session: %d from device handle.",
				 function,
				 session_index );

				return( -1 );
			}
			if( imaging_handle_append_session(
			     imaging_handle,
			     start_sector,
			     number_of_sectors,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to append session: %d to imaging handle.",
				 function,
				 session_index );

				return( -1 );
			}
		}
	}
	return( 1 );
}

/* Reads data from a file descriptor and writes it in EWF format
 * Returns the number of bytes written or -1 on error
 */
ssize64_t ewfacquire_read_input(
           imaging_handle_t *imaging_handle,
           device_handle_t *device_handle,
           size64_t media_size,
           off64_t resume_acquiry_offset,
           uint8_t swap_byte_pairs,
           size_t process_buffer_size,
           process_status_t *process_status,
           liberror_error_t **error )
{
	storage_media_buffer_t *storage_media_buffer = NULL;
	static char *function                        = "ewfacquire_read_input";
	off64_t read_error_offset                    = 0;
	size64_t read_error_size                     = 0;
	ssize64_t acquiry_count                      = 0;
	size_t read_size                             = 0;
	ssize_t read_count                           = 0;
	ssize_t process_count                        = 0;
	ssize_t write_count                          = 0;
	uint32_t byte_error_granularity              = 0;
	uint32_t chunk_size                          = 0;
	int number_of_read_errors                    = 0;
	int read_error_iterator                      = 0;

	if( imaging_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid imaging handle.",
		 function );

		return( -1 );
	}
	if( device_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid device handle.",
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
        if( imaging_handle->acquiry_size > (ssize64_t) INT64_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid write size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( imaging_handle->acquiry_size > media_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid write size value out of bounds.",
		 function );

		return( -1 );
	}
	if( imaging_handle->acquiry_offset > 0 )
	{
		if( ( imaging_handle->acquiry_offset > media_size )
		 || ( ( imaging_handle->acquiry_size + imaging_handle->acquiry_offset ) > media_size ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: unable to acquire beyond media size.",
			 function );

			goto on_error;
		}
		if( device_handle_seek_offset(
		     device_handle,
		     imaging_handle->acquiry_offset,
		     SEEK_SET,
		     error ) == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_SEEK_FAILED,
			 "%s: unable to find acquiry offset.",
			 function );

			goto on_error;
		}
	}
	if( resume_acquiry_offset > 0 )
	{
		if( ( imaging_handle->acquiry_offset + resume_acquiry_offset ) > media_size )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
			 "%s: unable to resume acquire beyond media size.",
			 function );

			goto on_error;
		}
		if( device_handle_seek_offset(
		     device_handle,
		     resume_acquiry_offset,
		     SEEK_CUR,
		     error ) == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_SEEK_FAILED,
			 "%s: unable to find acquiry offset.",
			 function );

			goto on_error;
		}
		if( imaging_handle_seek_offset(
		     imaging_handle,
		     0,
		     error ) == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_SEEK_FAILED,
			 "%s: unable to seek imaging offset.",
			 function );

			goto on_error;
		}
	}
	byte_error_granularity = imaging_handle->sector_error_granularity * imaging_handle->bytes_per_sector;

	if( imaging_handle_get_chunk_size(
	     imaging_handle,
	     &chunk_size,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve chunk size.",
		 function );

		goto on_error;
	}
	if( chunk_size == 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid chunk size.",
		 function );

		goto on_error;
	}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
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

		goto on_error;
	}
	while( acquiry_count < (ssize64_t) imaging_handle->acquiry_size )
	{
		read_size = process_buffer_size;

		if( ( (ssize64_t) imaging_handle->acquiry_size - acquiry_count ) < (ssize64_t) read_size )
		{
			read_size = (size_t) ( (ssize64_t) imaging_handle->acquiry_size - acquiry_count );
		}
		if( acquiry_count >= resume_acquiry_offset )
		{
			read_count = device_handle_read_buffer(
				      device_handle,
				      storage_media_buffer->raw_buffer,
				      read_size,
				      error );

			if( read_count < 0 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: error reading data from input.",
				 function );

				goto on_error;
			}
			if( read_count == 0 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: unexpected end of input.",
				 function );

				goto on_error;
			}
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
			storage_media_buffer->data_in_compression_buffer = 0;
#endif
			storage_media_buffer->raw_buffer_data_size = (size_t) read_count;

			/* Swap byte pairs
			 * The digest hashes are calcultated after swap
			 */
			if( ( swap_byte_pairs == 1 )
			 && ( imaging_handle_swap_byte_pairs(
			       imaging_handle,
			       storage_media_buffer,
			       read_count,
			       error ) != 1 ) )
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
		else
		{
			/* Align with resume acquiry offset if necessary
			 */
			if( ( resume_acquiry_offset - (off64_t) acquiry_count ) < (off64_t) read_size )
			{
				read_size = (size_t) ( resume_acquiry_offset - acquiry_count );
			}
			read_count = imaging_handle_read_buffer(
				      imaging_handle,
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
			process_count = imaging_handle_prepare_read_buffer(
					 imaging_handle,
					 storage_media_buffer,
					 error );

			if( process_count < 0 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				"%s: unable to prepare buffer after read.",
				 function );

				goto on_error;
			}
			if( process_count > (ssize_t) read_size )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: more bytes read than requested.",
				 function,
				 process_count,
				 read_size );

				goto on_error;
			}
			read_count = process_count;

#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
			/* Set the chunk data size in the compression buffer
			 */
			if( storage_media_buffer->data_in_compression_buffer == 1 )
			{
				storage_media_buffer->compression_buffer_data_size = (size_t) process_count;
			}
#endif
		}
		if( imaging_handle_update_integrity_hash(
		     imaging_handle,
		     storage_media_buffer,
		     read_count,
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
		if( acquiry_count >= resume_acquiry_offset )
		{
			process_count = imaging_handle_prepare_write_buffer(
					 imaging_handle,
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

				goto on_error;
			}
			write_count = imaging_handle_write_buffer(
				       imaging_handle,
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

				goto on_error;
			}
		}
		acquiry_count += read_count;

		if( process_status_update(
		     process_status,
		     (size64_t) acquiry_count,
		     imaging_handle->acquiry_size,
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
		if( ewfacquire_abort != 0 )
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

		goto on_error;
	}
	if( acquiry_count >= resume_acquiry_offset )
	{
		if( device_handle_get_number_of_read_errors(
		     device_handle,
		     &number_of_read_errors,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve number of read errors.",
			 function );

			goto on_error;
		}
		for( read_error_iterator = 0;
		     read_error_iterator < number_of_read_errors;
		     read_error_iterator++ )
		{
			if( device_handle_get_read_error(
			     device_handle,
			     read_error_iterator,
			     &read_error_offset,
			     &read_error_size,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve read error: %d.",
				 function,
				 read_error_iterator );

				goto on_error;
			}
			if( imaging_handle_append_read_error(
			     imaging_handle,
			     read_error_offset,
			     read_error_size,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to append read error: %d to imaging handle.",
				 function,
				 read_error_iterator );

				goto on_error;
			}
		}
		write_count = imaging_handle_finalize(
			       imaging_handle,
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
		acquiry_count += write_count;
	}
	return( acquiry_count );

on_error:
	if( storage_media_buffer != NULL )
	{
		storage_media_buffer_free(
		 &storage_media_buffer,
		 NULL );
	}
	return( -1 );
}

/* Signal handler for ewfacquire
 */
void ewfacquire_signal_handler(
      libsystem_signal_t signal )
{
	liberror_error_t *error = NULL;
	static char *function   = "ewfacquire_signal_handler";

	ewfacquire_abort = 1;

	if( ewfacquire_device_handle != NULL )
	{
		if( device_handle_signal_abort(
		     ewfacquire_device_handle,
		     &error ) != 1 )
		{
			libsystem_notify_printf(
			 "%s: unable to signal device handle to abort.\n",
			 function );

			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );
		}
	}
	if( ewfacquire_imaging_handle != NULL )
	{
		if( imaging_handle_signal_abort(
		     ewfacquire_imaging_handle,
		     &error ) != 1 )
		{
			libsystem_notify_printf(
			 "%s: unable to signal imaging handle to abort.\n",
			 function );

			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );
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
	libcstring_system_character_t input_buffer[ EWFACQUIRE_INPUT_BUFFER_SIZE ];
	libcstring_system_character_t media_information_model[ 64 ];
	libcstring_system_character_t media_information_serial_number[ 64 ];

	liberror_error_t *error                                         = NULL;

	libcstring_system_character_t *log_filename                     = NULL;
	libcstring_system_character_t *option_bytes_per_sector          = NULL;
	libcstring_system_character_t *option_case_number               = NULL;
	libcstring_system_character_t *option_compression_level         = NULL;
	libcstring_system_character_t *option_description               = NULL;
	libcstring_system_character_t *option_examiner_name             = NULL;
	libcstring_system_character_t *option_evidence_number           = NULL;
	libcstring_system_character_t *option_format                    = NULL;
	libcstring_system_character_t *option_header_codepage           = NULL;
	libcstring_system_character_t *option_maximum_segment_size      = NULL;
	libcstring_system_character_t *option_media_flags               = NULL;
	libcstring_system_character_t *option_media_type                = NULL;
	libcstring_system_character_t *option_notes                     = NULL;
	libcstring_system_character_t *option_number_of_error_retries   = NULL;
	libcstring_system_character_t *option_offset                    = NULL;
	libcstring_system_character_t *option_secondary_target_filename = NULL;
	libcstring_system_character_t *option_sector_error_granularity  = NULL;
	libcstring_system_character_t *option_sectors_per_chunk         = NULL;
	libcstring_system_character_t *option_size                      = NULL;
	libcstring_system_character_t *option_target_filename           = NULL;
	libcstring_system_character_t *option_toc_filename              = NULL;
	libcstring_system_character_t *option_zero_buffer_on_error      = NULL;
	libcstring_system_character_t *program                          = _LIBCSTRING_SYSTEM_STRING( "ewfacquire" );
	libcstring_system_character_t *request_string                   = NULL;

	log_handle_t *log_handle                                        = NULL;

	process_status_t *process_status                                = NULL;

	libcstring_system_integer_t option                              = 0;
	off64_t resume_acquiry_offset                                   = 0;
	ssize64_t read_count                                            = 0;
	size_t string_length                                            = 0;
	uint64_t media_size                                             = 0;
	uint64_t process_buffer_size                                    = EWFCOMMON_PROCESS_BUFFER_SIZE;
	uint8_t calculate_md5                                           = 1;
	uint8_t calculate_sha1                                          = 0;
	uint8_t print_status_information                                = 1;
	uint8_t resume_acquiry                                          = 0;
	uint8_t swap_byte_pairs                                         = 0;
	uint8_t verbose                                                 = 0;
	int8_t acquiry_parameters_confirmed                             = 0;
	int interactive_mode                                            = 1;
	int result                                                      = 0;
	int status                                                      = 0;

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
		 stdout,
		 program );

		fprintf(
		 stderr,
		 "Unable to initialize system values.\n" );

		goto on_error;
	}
	while( ( option = libsystem_getopt(
	                   argc,
	                   argv,
	                   _LIBCSTRING_SYSTEM_STRING( "A:b:B:c:C:d:D:e:E:f:g:hl:m:M:N:o:p:P:qr:RsS:t:T:uvVw2:" ) ) ) != (libcstring_system_integer_t) -1 )
	{
		switch( option )
		{
			case (libcstring_system_integer_t) '?':
			default:
				ewfoutput_version_fprint(
				 stdout,
				 program );

				fprintf(
				 stderr,
				 "Invalid argument: %" PRIs_LIBCSTRING_SYSTEM "\n",
				 argv[ optind ] );

				ewfacquire_usage_fprint(
				 stdout );

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

			case (libcstring_system_integer_t) 'C':
				option_case_number = optarg;

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

			case (libcstring_system_integer_t) 'D':
				option_description = optarg;

				break;

			case (libcstring_system_integer_t) 'e':
				option_examiner_name = optarg;

				break;

			case (libcstring_system_integer_t) 'E':
				option_evidence_number = optarg;

				break;

			case (libcstring_system_integer_t) 'f':
				option_format = optarg;

				break;

			case (libcstring_system_integer_t) 'g':
				option_sector_error_granularity = optarg;

				break;

			case (libcstring_system_integer_t) 'h':
				ewfoutput_version_fprint(
				 stdout,
				 program );

				ewfacquire_usage_fprint(
				 stdout );

				return( EXIT_SUCCESS );

			case (libcstring_system_integer_t) 'l':
				log_filename = optarg;

				break;

			case (libcstring_system_integer_t) 'm':
				option_media_type = optarg;

				break;

			case (libcstring_system_integer_t) 'M':
				option_media_flags = optarg;

				break;

			case (libcstring_system_integer_t) 'N':
				option_notes = optarg;

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

			case (libcstring_system_integer_t) 'P':
				option_bytes_per_sector = optarg;

				break;

			case (libcstring_system_integer_t) 'q':
				print_status_information = 0;

				break;

			case (libcstring_system_integer_t) 'r':
				option_maximum_segment_size = optarg;

				break;

			case (libcstring_system_integer_t) 'R':
				resume_acquiry = 1;

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

			case (libcstring_system_integer_t) 'T':
				option_toc_filename = optarg;

				break;

			case (libcstring_system_integer_t) 'u':
				interactive_mode = 0;

				break;

			case (libcstring_system_integer_t) 'v':
				verbose = 1;

				break;

			case (libcstring_system_integer_t) 'V':
				ewfoutput_version_fprint(
				 stdout,
				 program );

				ewfoutput_copyright_fprint(
				 stdout );

				return( EXIT_SUCCESS );

			case (libcstring_system_integer_t) 'w':
				option_zero_buffer_on_error = optarg;

				break;

			case (libcstring_system_integer_t) '2':
				option_secondary_target_filename = optarg;

				break;
		}
	}
	if( optind == argc )
	{
		ewfoutput_version_fprint(
		 stdout,
		 program );

		fprintf(
		 stderr,
		 "Missing source file or device.\n" );

		ewfacquire_usage_fprint(
		 stdout );

		goto on_error;
	}
	ewfoutput_version_fprint(
	 stdout,
	 program );

	libsystem_notify_set_verbose(
	 verbose );
	libewf_notify_set_verbose(
	 verbose );
	libewf_notify_set_stream(
	 stderr,
	 NULL );

	/* Check if to read from stdin
	 */
	if( libcstring_system_string_compare(
	     argv[ optind ],
	     _LIBCSTRING_SYSTEM_STRING( "-" ),
	     1 ) == 0 )
	{
		fprintf(
		 stderr,
		 "Reading from stdin not supported.\n" );

		goto on_error;
	}
	if( ( option_target_filename != NULL )
	 && ( option_secondary_target_filename != NULL ) )
	{
		string_length = libcstring_system_string_length(
				 option_secondary_target_filename );

		if( libcstring_system_string_length(
		     option_target_filename ) == string_length )
		{
			if( libcstring_system_string_compare(
			     option_target_filename,
			     option_secondary_target_filename,
			     string_length ) == 0 )
			{
				fprintf(
				 stderr,
				 "Primary and secondary target cannot be the same.\n" );

				goto on_error;
			}
		}
	}
	if( device_handle_initialize(
	     &ewfacquire_device_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to create device handle.\n" );

		goto on_error;
	}
#ifdef TOC_FILE
	if( option_toc_filename != NULL )
	{
		if( device_handle_set_string(
		     ewfacquire_device_handle,
		     option_toc_filename,
		     &( ewfacquire_device_handle->toc_filename ),
		     &( ewfacquire_device_handle->toc_filename_size ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set TOC filename.\n" );

			goto on_error;
		}
	}
#endif
	if( option_number_of_error_retries != NULL )
	{
		result = device_handle_set_number_of_error_retries(
			  ewfacquire_device_handle,
			  option_sectors_per_chunk,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set number of error retries.\n" );

			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf(
			 stderr,
			 "Unsuported number of error retries defaulting to: %" PRIu8 ".\n",
			 ewfacquire_device_handle->number_of_error_retries );
		}
	}
	if( option_zero_buffer_on_error != NULL )
	{
		ewfacquire_device_handle->zero_buffer_on_error = 1;
	}
	/* Open the input file or device size
	 */
	if( device_handle_open_input(
	     ewfacquire_device_handle,
	     &( argv[ optind ] ),
	     argc - optind,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to open file(s) or device.\n" );

		goto on_error;
	}
	if( device_handle_get_media_size(
	     ewfacquire_device_handle,
	     &media_size,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to retrieve media size.\n" );

		goto on_error;
	}
	if( device_handle_media_information_fprint(
	     ewfacquire_device_handle,
	     stdout,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to print media information.\n" );

		libsystem_notify_print_error_backtrace(
		 error );
		liberror_error_free(
		 &error );
	}
	/* Create the imaging handle and set the desired values
	 */
	if( imaging_handle_initialize(
	     &ewfacquire_imaging_handle,
	     calculate_md5,
	     calculate_sha1,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to create imaging handle.\n" );

		goto on_error;
	}
	if( option_header_codepage != NULL )
	{
		result = imaging_handle_set_header_codepage(
			  ewfacquire_imaging_handle,
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
		if( imaging_handle_set_string(
		     ewfacquire_imaging_handle,
		     option_target_filename,
		     &( ewfacquire_imaging_handle->target_filename ),
		     &( ewfacquire_imaging_handle->target_filename_size ),
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
		if( imaging_handle_set_string(
		     ewfacquire_imaging_handle,
		     _LIBCSTRING_SYSTEM_STRING( "image" ),
		     &( ewfacquire_imaging_handle->target_filename ),
		     &( ewfacquire_imaging_handle->target_filename_size ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set target filename.\n" );

			goto on_error;
		}
	}
	if( option_secondary_target_filename != NULL )
	{
		if( imaging_handle_set_string(
		     ewfacquire_imaging_handle,
		     option_secondary_target_filename,
		     &( ewfacquire_imaging_handle->secondary_target_filename ),
		     &( ewfacquire_imaging_handle->secondary_target_filename_size ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set secondary target filename.\n" );

			goto on_error;
		}
	}
	if( option_case_number != NULL )
	{
		if( imaging_handle_set_string(
		     ewfacquire_imaging_handle,
		     option_case_number,
		     &( ewfacquire_imaging_handle->case_number ),
		     &( ewfacquire_imaging_handle->case_number_size ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set case number.\n" );

			goto on_error;
		}
	}
	if( option_description != NULL )
	{
		if( imaging_handle_set_string(
		     ewfacquire_imaging_handle,
		     option_description,
		     &( ewfacquire_imaging_handle->description ),
		     &( ewfacquire_imaging_handle->description_size ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set description.\n" );

			goto on_error;
		}
	}
	if( option_evidence_number != NULL )
	{
		if( imaging_handle_set_string(
		     ewfacquire_imaging_handle,
		     option_evidence_number,
		     &( ewfacquire_imaging_handle->evidence_number ),
		     &( ewfacquire_imaging_handle->evidence_number_size ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set evidence number.\n" );

			goto on_error;
		}
	}
	if( option_examiner_name != NULL )
	{
		if( imaging_handle_set_string(
		     ewfacquire_imaging_handle,
		     option_examiner_name,
		     &( ewfacquire_imaging_handle->examiner_name ),
		     &( ewfacquire_imaging_handle->examiner_name_size ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set examiner name.\n" );

			goto on_error;
		}
	}
	if( option_notes != NULL )
	{
		if( imaging_handle_set_string(
		     ewfacquire_imaging_handle,
		     option_notes,
		     &( ewfacquire_imaging_handle->notes ),
		     &( ewfacquire_imaging_handle->notes_size ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to set notes.\n" );

			goto on_error;
		}
	}
	if( option_compression_level != NULL )
	{
		result = imaging_handle_set_compression_values(
			  ewfacquire_imaging_handle,
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
		result = imaging_handle_set_format(
			  ewfacquire_imaging_handle,
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
			 "Unsupported EWF format defaulting to: encase6.\n" );
		}
	}
	if( option_media_type != NULL )
	{
		result = imaging_handle_set_media_type(
			  ewfacquire_imaging_handle,
			  option_media_type,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set media type.\n" );

			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf(
			 stderr,
			 "Unsupported media type defaulting to: fixed.\n" );
		}
	}
	else
	{
		 if( device_handle_get_media_type(
		      ewfacquire_device_handle,
		      &( ewfacquire_imaging_handle->media_type ),
		      &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to retrieve media type from device.\n" );

			goto on_error;
		}
	}
	if( option_media_flags != NULL )
	{
		result = imaging_handle_set_media_flags(
			  ewfacquire_imaging_handle,
			  option_media_flags,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set media flags.\n" );

			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf(
			 stderr,
			 "Unsupported media flags defaulting to: physical.\n" );
		}
	}
	if( option_bytes_per_sector != NULL )
	{
		result = imaging_handle_set_bytes_per_sector(
			  ewfacquire_imaging_handle,
			  option_bytes_per_sector,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set bytes per sector.\n" );

			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf(
			 stderr,
			 "Unsupported bytes per sector defaulting to: %" PRIu32 ".\n",
			 ewfacquire_imaging_handle->bytes_per_sector );
		}
	}
	else
	{
		if( device_handle_get_bytes_per_sector(
		     ewfacquire_device_handle,
		     &( ewfacquire_imaging_handle->bytes_per_sector ),
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to retrieve bytes per sector from device.\n" );

			goto on_error;
		}
	}
	if( option_sectors_per_chunk != NULL )
	{
		result = imaging_handle_set_sectors_per_chunk(
			  ewfacquire_imaging_handle,
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
			 "Unsuported sectors per chunk defaulting to: %" PRIu32 ".\n",
			 ewfacquire_imaging_handle->sectors_per_chunk );
		}
	}
	if( option_sector_error_granularity != NULL )
	{
		result = imaging_handle_set_sector_error_granularity(
			  ewfacquire_imaging_handle,
			  option_sector_error_granularity,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set sector error granularity.\n" );

			goto on_error;
		}
		else if( result == 0 )
		{
			fprintf(
			 stderr,
			 "Unsuported sector error granularity defaulting to: %" PRIu32 ".\n",
			 ewfacquire_imaging_handle->sector_error_granularity );
		}
	}
	if( option_maximum_segment_size != NULL )
	{
		result = imaging_handle_set_maximum_segment_size(
			  ewfacquire_imaging_handle,
			  option_maximum_segment_size,
			  &error );

		if( result == -1 )
		{
			fprintf(
			 stderr,
			 "Unable to set maximum segment size.\n" );

			goto on_error;
		}
		else if( ( result == 0 )
		      || ( ewfacquire_imaging_handle->maximum_segment_size < EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE )
		      || ( ( ewfacquire_imaging_handle->ewf_format == LIBEWF_FORMAT_ENCASE6 )
		       &&  ( ewfacquire_imaging_handle->maximum_segment_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT ) )
		      || ( ( ewfacquire_imaging_handle->ewf_format != LIBEWF_FORMAT_ENCASE6 )
		       &&  ( ewfacquire_imaging_handle->maximum_segment_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT ) ) )
		{
			ewfacquire_imaging_handle->maximum_segment_size = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;

			fprintf(
			 stderr,
			 "Unsuported maximum segment size defaulting to: %" PRIu64 ".\n",
			 ewfacquire_imaging_handle->maximum_segment_size );
		}
	}
	if( option_offset != NULL )
	{
		string_length = libcstring_system_string_length(
				 option_offset );

		if( libsystem_string_to_uint64(
		     option_offset,
		     string_length + 1,
		     &( ewfacquire_imaging_handle->acquiry_offset ),
		     &error ) != 1 )
		{
			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			ewfacquire_imaging_handle->acquiry_offset = 0;

			fprintf(
			 stderr,
			 "Unsupported acquiry offset defaulting to: %" PRIu64 ".\n",
			 ewfacquire_imaging_handle->acquiry_offset );
		}
	}
	if( option_size != NULL )
	{
		string_length = libcstring_system_string_length(
				 option_size );

		if( libsystem_string_to_uint64(
		     option_size,
		     string_length + 1,
		     &( ewfacquire_imaging_handle->acquiry_size ),
		     &error ) != 1 )
		{
			libsystem_notify_print_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			ewfacquire_imaging_handle->acquiry_size = 0;

			fprintf(
			 stderr,
			 "Unsupported acquiry size defaulting to: all bytes.\n" );
		}
	}
	/* Initialize values
	 */
	if( ( ewfacquire_imaging_handle->acquiry_size == 0 )
	 || ( ewfacquire_imaging_handle->acquiry_size > ( media_size - ewfacquire_imaging_handle->acquiry_offset ) ) )
	{
		ewfacquire_imaging_handle->acquiry_size = media_size - ewfacquire_imaging_handle->acquiry_offset;
	}
	/* Request the necessary case data
	 */
	while( ( interactive_mode != 0 )
	    && ( acquiry_parameters_confirmed == 0 ) )
	{
		fprintf(
		 stdout,
		 "Acquiry parameters required, please provide the necessary input\n" );

		if( resume_acquiry != 0 )
		{
			request_string = _LIBCSTRING_SYSTEM_STRING( "Image path and filename with extension" );
		}
		else if( option_target_filename == NULL )
		{
			request_string = _LIBCSTRING_SYSTEM_STRING( "Image path and filename without extension" );
		}
		if( request_string != NULL )
		{
			do
			{
				result = imaging_handle_prompt_for_string(
				          ewfacquire_imaging_handle,
				          request_string,
				          &( ewfacquire_imaging_handle->target_filename ),
				          &( ewfacquire_imaging_handle->target_filename_size ),
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
		if( resume_acquiry != 0 )
		{
			if( libsystem_signal_attach(
			     ewfacquire_signal_handler,
			     &error ) != 1 )
			{
				fprintf(
				 stderr,
				 "Unable to attach signal handler.\n" );

				goto on_error;
			}
			if( imaging_handle_open_output(
			     ewfacquire_imaging_handle,
			     ewfacquire_imaging_handle->target_filename,
			     resume_acquiry,
			     &error ) != 1 )
			{
				fprintf(
				 stdout,
				 "Unable to resume acquire - starting from scratch.\n" );

#if defined( HAVE_VERBOSE_OUTPUT )
				libsystem_notify_print_error_backtrace(
				 error );
#endif
				liberror_error_free(
				 &error );

				/* TODO what about extension in target filename ? */

				resume_acquiry = 0;
			}
			if( libsystem_signal_detach(
			     &error ) != 1 )
			{
				fprintf(
				 stderr,
				 "Unable to detach signal handler.\n" );

				goto on_error;
			}
		}
		if( resume_acquiry != 0 )
		{
			if( imaging_handle_get_output_values(
			     ewfacquire_imaging_handle,
			     &error ) != 1 )
			{
				fprintf(
				 stdout,
				 "Unable to determine previous acquiry parameters.\n" );

				libsystem_notify_print_error_backtrace(
				 error );
				liberror_error_free(
				 &error );

				imaging_handle_close(
				 ewfacquire_imaging_handle,
				 NULL );

				resume_acquiry = 0;
			}
		}
		if( resume_acquiry != 0 )
		{
			if( imaging_handle_get_offset(
			     ewfacquire_imaging_handle,
			     &resume_acquiry_offset,
			     &error ) != 1 )
			{
				fprintf(
				 stdout,
				 "Unable to determine resume acquiry offset.\n" );

				libsystem_notify_print_error_backtrace(
				 error );
				liberror_error_free(
				 &error );

				imaging_handle_close(
				 ewfacquire_imaging_handle,
				 NULL );

				resume_acquiry = 0;
			}
			fprintf(
			 stdout,
			 "Resuming acquire at offset: %" PRIi64 ".\n",
			 resume_acquiry_offset );
		}
		if( resume_acquiry == 0 )
		{
			if( option_case_number == NULL )
			{
				if( imaging_handle_prompt_for_string(
				     ewfacquire_imaging_handle,
				     _LIBCSTRING_SYSTEM_STRING( "Case number" ),
				     &( ewfacquire_imaging_handle->case_number ),
				     &( ewfacquire_imaging_handle->case_number_size ),
				     &error ) == -1 )
				{
					fprintf(
					 stdout,
					 "Unable to determine case number.\n" );

					goto on_error;
				}
			}
			if( option_description == NULL )
			{
				if( imaging_handle_prompt_for_string(
				     ewfacquire_imaging_handle,
				     _LIBCSTRING_SYSTEM_STRING( "Description" ),
				     &( ewfacquire_imaging_handle->description ),
				     &( ewfacquire_imaging_handle->description_size ),
				     &error ) == -1 )
				{
					fprintf(
					 stdout,
					 "Unable to determine description.\n" );

					goto on_error;
				}
			}
			if( option_evidence_number == NULL )
			{
				if( imaging_handle_prompt_for_string(
				     ewfacquire_imaging_handle,
				     _LIBCSTRING_SYSTEM_STRING( "Evidence number" ),
				     &( ewfacquire_imaging_handle->evidence_number ),
				     &( ewfacquire_imaging_handle->evidence_number_size ),
				     &error ) == -1 )
				{
					fprintf(
					 stdout,
					 "Unable to determine evidence number.\n" );

					goto on_error;
				}
			}
			if( option_examiner_name == NULL )
			{
				if( imaging_handle_prompt_for_string(
				     ewfacquire_imaging_handle,
				     _LIBCSTRING_SYSTEM_STRING( "Examiner name" ),
				     &( ewfacquire_imaging_handle->examiner_name ),
				     &( ewfacquire_imaging_handle->examiner_name_size ),
				     &error ) == -1 )
				{
					fprintf(
					 stdout,
					 "Unable to determine examiner name.\n" );

					goto on_error;
				}
			}
			if( option_notes == NULL )
			{
				if( imaging_handle_prompt_for_string(
				     ewfacquire_imaging_handle,
				     _LIBCSTRING_SYSTEM_STRING( "Notes" ),
				     &( ewfacquire_imaging_handle->notes ),
				     &( ewfacquire_imaging_handle->notes_size ),
				     &error ) == -1 )
				{
					fprintf(
					 stdout,
					 "Unable to determine notes.\n" );

					goto on_error;
				}
			}
			if( option_media_type == NULL )
			{
				result = imaging_handle_prompt_for_media_type(
					  ewfacquire_imaging_handle,
				          _LIBCSTRING_SYSTEM_STRING( "Media type" ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine media type.\n" );

					goto on_error;
				}
			}
			if( option_media_flags == NULL )
			{
				result = imaging_handle_prompt_for_media_flags(
					  ewfacquire_imaging_handle,
				          _LIBCSTRING_SYSTEM_STRING( "Media characteristics" ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine media flags.\n" );

					goto on_error;
				}
			}
#ifdef TOC_FILE
/* TODO how to deal with this, because device handle is already open at this point
 */
			if( ( ewfacquire_imaging_handle->media_type == LIBEWF_MEDIA_TYPE_OPTICAL )
			 && ( option_toc_filename == NULL ) )
			{
				if( device_handle_prompt_for_string(
				     ewfacquire_device_handle,
				     _LIBCSTRING_SYSTEM_STRING( "File containing table of contents" ),
				     &( ewfacquire_device_handle->toc_filename ),
				     &( ewfacquire_device_handle->toc_filename_size ),
				     &error ) == -1 )
				{
					fprintf(
					 stdout,
					 "Unable to determine toc file.\n" );

					goto on_error;
				}
			}
#endif
			if( option_compression_level == NULL )
			{
				result = imaging_handle_prompt_for_compression_level(
					  ewfacquire_imaging_handle,
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
			if( option_format == NULL )
			{
				result = imaging_handle_prompt_for_format(
					  ewfacquire_imaging_handle,
				          _LIBCSTRING_SYSTEM_STRING( "Use EWF file format" ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine format.\n" );

					goto on_error;
				}
			}
		}
		if( ( resume_acquiry == 0 )
		 || ( ewfacquire_imaging_handle->acquiry_size != media_size ) )
		{
			if( option_offset == NULL )
			{
				if( ewfinput_get_size_variable(
				     stdout,
				     input_buffer,
				     EWFACQUIRE_INPUT_BUFFER_SIZE,
				     _LIBCSTRING_SYSTEM_STRING( "Start to acquire at offset" ),
				     0,
				     media_size,
				     0,
				     &( ewfacquire_imaging_handle->acquiry_offset ),
				     &error ) == -1 )
				{
					libsystem_notify_print_error_backtrace(
					 error );
					liberror_error_free(
					 &error );

					ewfacquire_imaging_handle->acquiry_offset = 0;

					fprintf(
					 stdout,
					 "Unable to determine acquiry offset defaulting to: %" PRIu64 ".\n",
					 ewfacquire_imaging_handle->acquiry_offset );
				}
			}
		}
		if( resume_acquiry == 0 )
		{
			if( option_size == NULL )
			{
				if( ewfinput_get_size_variable(
				     stdout,
				     input_buffer,
				     EWFACQUIRE_INPUT_BUFFER_SIZE,
				     _LIBCSTRING_SYSTEM_STRING( "The number of bytes to acquire" ),
				     0,
				     media_size - ewfacquire_imaging_handle->acquiry_offset,
				     media_size - ewfacquire_imaging_handle->acquiry_offset,
				     &( ewfacquire_imaging_handle->acquiry_size ),
				     &error ) == -1 )
				{
					libsystem_notify_print_error_backtrace(
					 error );
					liberror_error_free(
					 &error );

					ewfacquire_imaging_handle->acquiry_size = media_size - ewfacquire_imaging_handle->acquiry_offset;

					fprintf(
					 stdout,
					 "Unable to determine input size defaulting to: %" PRIu64 ".\n",
					 ewfacquire_imaging_handle->acquiry_size );
				}
			}
			if( option_maximum_segment_size == NULL )
			{
				result = imaging_handle_prompt_for_maximum_segment_size(
					  ewfacquire_imaging_handle,
				          _LIBCSTRING_SYSTEM_STRING( "Evidence segment file size in bytes" ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine maximum segment size.\n" );

					goto on_error;
				}
				if( ( ewfacquire_imaging_handle->maximum_segment_size < EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE )
				 || ( ( ewfacquire_imaging_handle->ewf_format == LIBEWF_FORMAT_ENCASE6 )
				  &&  ( ewfacquire_imaging_handle->maximum_segment_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT ) )
				 || ( ( ewfacquire_imaging_handle->ewf_format != LIBEWF_FORMAT_ENCASE6 )
				  &&  ( ewfacquire_imaging_handle->maximum_segment_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT ) ) )
				{
					ewfacquire_imaging_handle->maximum_segment_size = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;

					fprintf(
					 stderr,
					 "Unsuported maximum segment size defaulting to: %" PRIu64 ".\n",
					 ewfacquire_imaging_handle->maximum_segment_size );
				}
			}
			if( option_bytes_per_sector == NULL )
			{
				result = imaging_handle_prompt_for_bytes_per_sector(
					  ewfacquire_imaging_handle,
				          _LIBCSTRING_SYSTEM_STRING( "The number of bytes per sector" ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine bytes per sector.\n" );

					goto on_error;
				}
			}
			if( option_sectors_per_chunk == NULL )
			{
				result = imaging_handle_prompt_for_sectors_per_chunk(
					  ewfacquire_imaging_handle,
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
			if( option_sector_error_granularity == NULL )
			{
				result = imaging_handle_prompt_for_sector_error_granularity(
					  ewfacquire_imaging_handle,
				          _LIBCSTRING_SYSTEM_STRING( "The number of sectors to be used as error granularity" ),
					  &error );

				if( result == -1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine sector error granularity.\n" );

					goto on_error;
				}
/* TODO add range check */
			}
		}
		if( option_number_of_error_retries == NULL )
		{
			result = device_handle_prompt_for_number_of_error_retries(
				  ewfacquire_device_handle,
				  _LIBCSTRING_SYSTEM_STRING( "The number of retries when a read error occurs" ),
				  &error );

			if( result == -1 )
			{
				fprintf(
				 stderr,
				 "Unable to determine number of error retries.\n" );

				goto on_error;
			}
/* TODO add range check */
		}
		if( option_zero_buffer_on_error == NULL )
		{
			result = device_handle_prompt_for_zero_buffer_on_error(
				  ewfacquire_device_handle,
			          _LIBCSTRING_SYSTEM_STRING( "Wipe sectors on read error (mimic EnCase like behavior)" ),
				  &error );

			if( result == -1 )
			{
				fprintf(
				 stderr,
				 "Unable to determine zero buffer on error.\n" );

				goto on_error;
			}
/* TODO add range check */
		}
		fprintf(
		 stdout,
		 "\n" );

		fprintf(
		 stdout,
		 "The following acquiry parameters were provided:\n" );

		if( imaging_handle_print_parameters(
		     ewfacquire_imaging_handle,
		     resume_acquiry_offset,
		     ewfacquire_device_handle->number_of_error_retries,
		     ewfacquire_device_handle->zero_buffer_on_error,
		     resume_acquiry,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to print acquiry parameters.\n" );

			goto on_error;
		}
		/* Check if user is content with the acquiry parameters
		 */
		acquiry_parameters_confirmed = ewfacquire_confirm_acquiry_parameters(
						stdout,
						input_buffer,
						EWFACQUIRE_INPUT_BUFFER_SIZE,
		                                &error );

		if( acquiry_parameters_confirmed == -1 )
		{
			fprintf(
			 stdout,
			 "Unable to determine if acquiry parameters are correct aborting.\n" );

			goto on_error;
		}
		/* Reset all parameters provided as command line arguments
		 */
		else if( acquiry_parameters_confirmed == 0 )
		{
			option_case_number              = NULL;
			option_compression_level        = NULL;
			option_description              = NULL;
			option_evidence_number          = NULL;
			option_examiner_name            = NULL;
			option_format                   = NULL;
			option_maximum_segment_size     = NULL;
			option_media_flags              = NULL;
			option_media_type               = NULL;
			option_notes                    = NULL;
			option_number_of_error_retries  = NULL;
			option_offset                   = NULL;
			option_sectors_per_chunk        = NULL;
			option_sector_error_granularity = NULL;
			option_size                     = NULL;
			option_target_filename          = NULL;
			option_toc_filename             = NULL;
			option_zero_buffer_on_error     = NULL;

			if( resume_acquiry != 0 )
			{
				if( imaging_handle_close(
				     ewfacquire_imaging_handle,
				     &error ) != 0 )
				{
					fprintf(
					 stdout,
					 "Unable to close output file(s).\n" );

					goto on_error;
				}
			}
		}
		else if( ewfacquire_imaging_handle->acquiry_size > EWFACQUIRE_2_TIB )
		{
			if( ( ewfacquire_imaging_handle->ewf_format != LIBEWF_FORMAT_ENCASE6 )
			 && ( ewfacquire_imaging_handle->ewf_format != LIBEWF_FORMAT_EWFX ) )
			{
				fprintf(
				 stdout,
				 "Cannot acquire more than 2 TiB in selected EWF file format.\n" );

				acquiry_parameters_confirmed = 0;
			}
		}
	}
	if( interactive_mode == 0 )
	{
		if( ewfacquire_imaging_handle->maximum_segment_size == 0 )
		{
			if( ewfacquire_imaging_handle->ewf_format == LIBEWF_FORMAT_ENCASE6 )
			{
				ewfacquire_imaging_handle->maximum_segment_size = EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT;
			}
			else
			{
				ewfacquire_imaging_handle->maximum_segment_size = EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT;
			}
		}
	}
	if( ewfacquire_abort == 0 )
	{
		if( resume_acquiry == 0 )
		{
			result = device_handle_get_information_value(
			          ewfacquire_device_handle,
			          (uint8_t *) "model",
			          5,
			          media_information_model,
			          64,
			          &error );

			if( result == -1 )
			{
				fprintf(
				 stdout,
				 "Unable to retrieve model.\n" );

				libsystem_notify_print_error_backtrace(
				 error );
				liberror_error_free(
				 &error );
			}
			if( result != 1 )
			{
				media_information_model[ 0 ] = 0;
			}
			result = device_handle_get_information_value(
			          ewfacquire_device_handle,
			          (uint8_t *) "serial_number",
			          13,
			          media_information_serial_number,
			          64,
			          &error );

			if( result == -1 )
			{
				fprintf(
				 stdout,
				 "Unable to retrieve serial number.\n" );

				libsystem_notify_print_error_backtrace(
				 error );
				liberror_error_free(
				 &error );
			}
			if( result != 1 )
			{
				media_information_serial_number[ 0 ] = 0;
			}
			if( imaging_handle_open_output(
			     ewfacquire_imaging_handle,
			     ewfacquire_imaging_handle->target_filename,
			     resume_acquiry,
			     &error ) != 1 )
			{
				fprintf(
				 stderr,
				 "Unable to open output file(s).\n" );

				goto on_error;
			}
			if( ewfacquire_imaging_handle->secondary_target_filename != NULL )
			{
				if( imaging_handle_open_secondary_output(
				     ewfacquire_imaging_handle,
				     ewfacquire_imaging_handle->secondary_target_filename,
				     resume_acquiry,
				     &error ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unable to open secondary output file(s).\n" );

					goto on_error;
				}
			}
			if( device_handle_set_error_values(
			     ewfacquire_device_handle,
			     ewfacquire_imaging_handle->sector_error_granularity * ewfacquire_imaging_handle->bytes_per_sector,
			     &error ) != 1 )
			{
				fprintf(
				 stderr,
				 "Unable to initialize output settings.\n" );

				goto on_error;
			}
			if( imaging_handle_set_output_values(
			     ewfacquire_imaging_handle,
			     program,
			     _LIBCSTRING_SYSTEM_STRING( LIBEWF_VERSION_STRING ),
			     media_information_model,
			     media_information_serial_number,
			     &error ) != 1 )
			{
				fprintf(
				 stderr,
				 "Unable to initialize output settings.\n" );

				goto on_error;
			}
			if( ewfacquire_imaging_handle->media_type == LIBEWF_MEDIA_TYPE_OPTICAL )
			{
				if( ewfacquire_determine_sessions(
				     ewfacquire_imaging_handle,
				     ewfacquire_device_handle,
				     media_size,
				     &error ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unable to determine sessions.\n" );

					goto on_error;
				}
			}
		}
		if( process_status_initialize(
		     &process_status,
		     _LIBCSTRING_SYSTEM_STRING( "Acquiry" ),
		     _LIBCSTRING_SYSTEM_STRING( "acquired" ),
		     _LIBCSTRING_SYSTEM_STRING( "Written" ),
		     stdout,
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
		if( libsystem_signal_attach(
		     ewfacquire_signal_handler,
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
		/* Start acquiring data
		 */
		read_count = ewfacquire_read_input(
		              ewfacquire_imaging_handle,
		              ewfacquire_device_handle,
		              media_size,
		              resume_acquiry_offset,
		              swap_byte_pairs,
		              (size_t) process_buffer_size,
		              process_status,
		              &error );

		if( read_count <= -1 )
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
		if( process_status_stop(
		     process_status,
		     (size64_t) read_count,
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
	}
	if( ewfacquire_abort != 0 )
	{
		status = PROCESS_STATUS_ABORTED;
	}
	/* Done acquiring data
	 */
	if( status == PROCESS_STATUS_COMPLETED )
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
		if( device_handle_read_errors_fprint(
		     ewfacquire_device_handle,
		     stdout,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to print device read errors.\n" );

			goto on_error;
		}
		if( log_handle != NULL )
		{
			if( device_handle_read_errors_fprint(
			    ewfacquire_device_handle,
			    log_handle->log_stream,
			    &error ) != 1 )
			{
				fprintf(
				 stderr,
				 "Unable to write device read errors in log file.\n" );

				goto on_error;
			}
		}
		if( calculate_md5 == 1 )
		{
			fprintf(
			 stdout,
			 "MD5 hash calculated over data:\t%" PRIs_LIBCSTRING_SYSTEM "\n",
			 ewfacquire_imaging_handle->md5_hash_string );

			if( log_handle != NULL )
			{
				log_handle_printf(
				 log_handle,
				 "MD5 hash calculated over data:\t%" PRIs_LIBCSTRING_SYSTEM "\n",
				 ewfacquire_imaging_handle->md5_hash_string );
			}
		}
		if( calculate_sha1 == 1 )
		{
			fprintf(
			 stdout,
			 "SHA1 hash calculated over data:\t%" PRIs_LIBCSTRING_SYSTEM "\n",
			 ewfacquire_imaging_handle->sha1_hash_string );

			if( log_handle != NULL )
			{
				log_handle_printf(
				 log_handle,
				 "SHA1 hash calculated over data:\t%" PRIs_LIBCSTRING_SYSTEM "\n",
				 ewfacquire_imaging_handle->sha1_hash_string );
			}
		}
	}
	if( imaging_handle_close(
	     ewfacquire_imaging_handle,
	     &error ) != 0 )
	{
		fprintf(
		 stderr,
		 "Unable to close output file(s).\n" );

		goto on_error;
	}
	if( imaging_handle_free(
	     &ewfacquire_imaging_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free imaging handle.\n" );

		goto on_error;
	}
	if( device_handle_close(
	     ewfacquire_device_handle,
	     &error ) != 0 )
	{
		fprintf(
		 stderr,
		 "Unable to close input file or device.\n" );

		goto on_error;
	}
	if( device_handle_free(
	     &ewfacquire_device_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free device handle.\n" );

		goto on_error;
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
	if( ewfacquire_imaging_handle != NULL )
	{
		imaging_handle_close(
		 ewfacquire_imaging_handle,
		 NULL );
		imaging_handle_free(
		 &ewfacquire_imaging_handle,
		 NULL );
	}
	if( ewfacquire_device_handle != NULL )
	{
		device_handle_close(
		 ewfacquire_device_handle,
		 NULL );
		device_handle_free(
		 &ewfacquire_device_handle,
		 NULL );
	}
	return( EXIT_FAILURE );
}

