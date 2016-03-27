/*
 * ewfacquirestream
 * Reads data from a stdin and writes it in EWF format
 *
 * Copyright (c) 2006-2008, Joachim Metz <forensics@hoffmannbv.nl>,
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
#include <file_stream_io.h>
#include <memory.h>
#include <system_string.h>
#include <types.h>

#include <stdio.h>

#if defined( HAVE_SYS_IOCTL_H )
#include <sys/ioctl.h>
#endif

#if defined( HAVE_UNISTD_H )
#include <unistd.h>
#endif

#if defined( HAVE_FCNLTL_H )
#include <fcntl.h>
#endif

#if defined( HAVE_IO_H )
#include <io.h>
#endif

#if defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif

#include <stdio.h>

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include "character_string.h"
#include "byte_size_string.h"
#include "ewfcommon.h"
#include "ewfgetopt.h"
#include "glob.h"
#include "ewfinput.h"
#include "ewfoutput.h"
#include "process_status.h"
#include "ewfsignal.h"
#include "ewfstring.h"

/* Prints the executable usage information to the stream
 */
void usage_fprint(
      FILE *stream )
{
	character_t default_segment_file_size_string[ 16 ];
	character_t minimum_segment_file_size_string[ 16 ];
	character_t maximum_32bit_segment_file_size_string[ 16 ];
	character_t maximum_64bit_segment_file_size_string[ 16 ];

	int result = 0;

	if( stream == NULL )
	{
		return;
	}
	result = byte_size_string_create(
	          default_segment_file_size_string,
	          16,
	          EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE,
	          BYTE_SIZE_STRING_UNIT_MEBIBYTE );

	if( result == 1 )
	{
		result = byte_size_string_create(
			  minimum_segment_file_size_string,
			  16,
			  EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE,
			  BYTE_SIZE_STRING_UNIT_MEBIBYTE );
	}
	if( result == 1 )
	{
		result = byte_size_string_create(
			  maximum_32bit_segment_file_size_string,
			  16,
			  EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT,
			  BYTE_SIZE_STRING_UNIT_MEBIBYTE );
	}
	if( result == 1 )
	{
		result = byte_size_string_create(
			  maximum_64bit_segment_file_size_string,
			  16,
			  EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT,
			  BYTE_SIZE_STRING_UNIT_MEBIBYTE );
	}
	fprintf( stream, "Usage: ewfacquirestream [ -b amount_of_sectors ] [ -c compression_type ]\n"
	                 "                        [ -C case_number ] [ -d digest_type ] [ -D description ]\n"
	                 "                        [ -e examiner_name ] [ -E evidence_number ]\n"
	                 "                        [ -f format ] [ -l log_filename ] [ -m media_type ]\n"
	                 "                        [ -M volume_type ] [ -N notes ]\n"
	                 "                        [ -p process_buffer_size ] [ -S segment_file_size ]\n"
	                 "                        [ -t target ] [ -hqsvVw ]\n\n" );

	fprintf( stream, "\tReads data from stdin\n\n" );

	fprintf( stream, "\t-b: specify the amount of sectors to read at once (per chunk), options:\n"
	                 "\t    64 (default), 128, 256, 512, 1024, 2048, 4096, 8192, 16384 or 32768\n" );
	fprintf( stream, "\t-c: specify the compression type, options: none (default), empty-block, fast\n"
	                 "\t    or best\n" );
	fprintf( stream, "\t-C: specify the case number (default is case_number).\n" );
	fprintf( stream, "\t-d: calculate additional digest (hash) types besides md5, options: sha1\n" );
	fprintf( stream, "\t-D: specify the description (default is description).\n" );
	fprintf( stream, "\t-e: specify the examiner name (default is examiner_name).\n" );
	fprintf( stream, "\t-E: specify the evidence number (default is evidence_number).\n" );
	fprintf( stream, "\t-f: specify the EWF file format to write to, options: ftk, encase2, encase3,\n"
	                 "\t    encase4, encase5 (default), encase6, linen5, linen6, ewfx\n" );
	fprintf( stream, "\t-h: shows this help\n" );
	fprintf( stream, "\t-l: logs acquiry errors and the digest (hash) to the log_filename\n" );
	fprintf( stream, "\t-m: specify the media type, options: fixed (default), removable\n" );
	fprintf( stream, "\t-M: specify the volume type, options: logical, physical (default)\n" );
	fprintf( stream, "\t-N: specify the notes (default is notes).\n" );
	fprintf( stream, "\t-p: specify the process buffer size (default is the chunk size)\n" );
	fprintf( stream, "\t-q: quiet shows no status information\n" );
	fprintf( stream, "\t-s: swap byte pairs of the media data (from AB to BA)\n"
	                 "\t    (use this for big to little endian conversion and vice versa)\n" );

	if( result == 1 )
	{
		fprintf( stream, "\t-S: specify the segment file size in bytes (default is %" PRIs ")\n"
		                 "\t    (minimum is %" PRIs ", maximum is %" PRIs " for encase6 format\n"
		                 "\t    and %" PRIs " for other formats)\n",
		 default_segment_file_size_string,
		 minimum_segment_file_size_string,
		 maximum_64bit_segment_file_size_string,
		 maximum_32bit_segment_file_size_string );
	}
	else
	{
		fprintf( stream, "\t-S: specify the segment file size in bytes (default is %" PRIu32 ")\n"
		                 "\t    (minimum is %" PRIu32 ", maximum is %" PRIu64 " for encase6 format\n"
		                 "\t    and %" PRIu32 " for other formats)\n",
		 (uint32_t) EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE,
		 (uint32_t) EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE,
		 (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT,
		 (uint32_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT );
	}
	fprintf( stream, "\t-t: specify the target file (without extension) to write to (default is stream)\n" );
	fprintf( stream, "\t-v: verbose output to stderr\n" );
	fprintf( stream, "\t-V: print version\n" );
	fprintf( stream, "\t-w: wipe sectors on read error (mimic EnCase like behavior)\n" );
}

/* The main program
 */
#if defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
int wmain( int argc, wchar_t * const argv[] )
#else
int main( int argc, char * const argv[] )
#endif
{
	character_t acquiry_operating_system[ 32 ];

	system_character_t *target_filenames[ 1 ]  = { _SYSTEM_CHARACTER_T_STRING( "stream" ) };

#if defined( HAVE_V2_API )
	libewf_error_t *libewf_error               = NULL;
#endif

	character_t *calculated_md5_hash_string    = NULL;
	character_t *calculated_sha1_hash_string   = NULL;
	character_t *case_number                   = NULL;
	character_t *description                   = NULL;
	character_t *evidence_number               = NULL;
	character_t *examiner_name                 = NULL;
	character_t *notes                         = NULL;
	character_t *acquiry_software_version      = NULL;
	character_t *program                       = _CHARACTER_T_STRING( "ewfacquirestream" );

	system_character_t *log_filename           = NULL;
	system_character_t *option_case_number     = NULL;
	system_character_t *option_description     = NULL;
	system_character_t *option_examiner_name   = NULL;
	system_character_t *option_evidence_number = NULL;
	system_character_t *option_notes           = NULL;

	FILE *log_file_stream                      = NULL;
	void *callback                             = &process_status_update_unknown_total;

	system_integer_t option                    = 0;
	size_t string_length                       = 0;
	int64_t write_count                        = 0;
	uint64_t acquiry_offset                    = 0;
	uint64_t acquiry_size                      = 0;
	uint64_t process_buffer_size               = 0;
	uint64_t segment_file_size                 = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;
	uint32_t amount_of_acquiry_errors          = 0;
	uint32_t sectors_per_chunk                 = 64;
	uint32_t sector_error_granularity          = 64;
	int8_t compression_level                   = LIBEWF_COMPRESSION_NONE;
	uint8_t calculate_md5                      = 1;
	uint8_t calculate_sha1                     = 0;
	uint8_t compress_empty_block               = 0;
	uint8_t libewf_format                      = LIBEWF_FORMAT_ENCASE5;
	uint8_t media_type                         = LIBEWF_MEDIA_TYPE_FIXED;
	uint8_t read_error_retry                   = 2;
	uint8_t seek_on_error                      = 0;
	uint8_t swap_byte_pairs                    = 0;
	uint8_t verbose                            = 0;
	uint8_t volume_type                        = LIBEWF_VOLUME_TYPE_PHYSICAL;
	uint8_t wipe_chunk_on_error                = 0;
	int error_abort                            = 0;
	int result                                 = 0;
	int status                                 = 0;

	ewfoutput_version_fprint(
	 stdout,
	 program );

#if defined( WINAPI )
	if( _setmode(
	     _fileno(
	      stdin ),
	     _O_BINARY ) == -1 )
	{
		fprintf( stderr, "Unable to set stdin to binary mode.\n" );

		usage_fprint(
		 stdout );

		return( EXIT_FAILURE );
	}
#endif

	while( ( option = ewfgetopt(
	                   argc,
	                   argv,
	                   _SYSTEM_CHARACTER_T_STRING( "b:c:C:d:D:e:E:f:hl:m:M:N:p:qsS:t:vVw" ) ) ) != (system_integer_t) -1 )
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

			case (system_integer_t) 'b':
				if( ewfinput_determine_sectors_per_chunk_system_character(
				     optarg,
				     &sectors_per_chunk ) != 1 )
				{
					fprintf( stderr, "Unsuported amount of sectors per chunk defaulting to: 64.\n" );

					sectors_per_chunk = 64;
				}
				break;

			case (system_integer_t) 'c':
				if( ewfinput_determine_compression_level_system_character(
				     optarg,
				     &compression_level,
				     &compress_empty_block ) != 1 )
				{
					fprintf( stderr, "Unsupported compression type defaulting to: none.\n" );

					compression_level    = LIBEWF_COMPRESSION_NONE;
					compress_empty_block = 0;
				}
				break;

			case (system_integer_t) 'C':
				option_case_number = optarg;

				break;

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

			case (system_integer_t) 'D':
				option_description = optarg;

				break;

			case (system_integer_t) 'e':
				option_examiner_name = optarg;

				break;

			case (system_integer_t) 'E':
				option_evidence_number = optarg;

				break;

			case (system_integer_t) 'f':
				if( ( ewfinput_determine_libewf_format_system_character(
				       optarg,
				       &libewf_format ) != 1 )
				 || ( libewf_format == LIBEWF_FORMAT_EWF )
				 || ( libewf_format == LIBEWF_FORMAT_SMART ) )
				{
					fprintf( stderr, "Unsupported EWF file format type defaulting to: encase5.\n" );

					libewf_format = LIBEWF_FORMAT_ENCASE5;
				}
				break;

			case (system_integer_t) 'h':
				usage_fprint(
				 stdout );

				return( EXIT_SUCCESS );

			case (system_integer_t) 'l':
				log_filename = optarg;

				break;

			case (system_integer_t) 'm':
				if( ewfinput_determine_media_type_system_character(
				     optarg,
				     &media_type ) != 1 )
				{
					fprintf( stderr, "Unsupported media type defaulting to: fixed.\n" );

					media_type = LIBEWF_MEDIA_TYPE_FIXED;
				}
				break;

			case (system_integer_t) 'M':
				if( ewfinput_determine_volume_type_system_character(
				     optarg,
				     &volume_type ) != 1 )
				{
					fprintf( stderr, "Unsupported volume type defaulting to: logical.\n" );

					volume_type = LIBEWF_VOLUME_TYPE_LOGICAL;
				}
				break;

			case (system_integer_t) 'N':
				option_notes = optarg;

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

			case (system_integer_t) 's':
				swap_byte_pairs = 1;

				break;

			case (system_integer_t) 'S':
				string_length = system_string_length(
				                 optarg );

				result = byte_size_string_convert_system_character(
				          optarg,
				          string_length,
				          &segment_file_size );

				if( ( result != 1 )
				 || ( segment_file_size < EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE )
				 || ( ( libewf_format == LIBEWF_FORMAT_ENCASE6 )
				  && ( segment_file_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT ) )
				 || ( ( libewf_format != LIBEWF_FORMAT_ENCASE6 )
				  && ( segment_file_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT ) ) )
				{
					segment_file_size = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;

					fprintf( stderr, "Unsupported segment file size defaulting to: %" PRIu64 ".\n",
					 segment_file_size );
				}
				break;

			case (system_integer_t) 't':
				target_filenames[ 0 ] = optarg;

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
	libewf_set_notify_values(
	 stderr,
	 verbose );

	if( option_case_number != NULL )
	{
		string_length = system_string_length(
		                 option_case_number );

		if( string_length > 0 )
		{
			string_length += 1;
			case_number    = (character_t *) memory_allocate(
			                                  sizeof( character_t ) * string_length );

			if( case_number == NULL )
			{
				fprintf( stderr, "Unable to create case number string.\n" );

				error_abort = 1;
			}
			else if( ewfstring_copy_system_string_to_character_string(
			          case_number,
			          option_case_number,
			          string_length ) != 1 )
			{
				fprintf( stderr, "Unable to set case number string.\n" );

				error_abort = 1;
			}
		}
	}
	if( ( error_abort == 0 )
	 && ( option_description != NULL ) )
	{
		string_length = system_string_length(
		                 option_description );

		if( string_length > 0 )
		{
			string_length += 1;
			description    = (character_t *) memory_allocate(
			                                  sizeof( character_t ) * string_length );

			if( description == NULL )
			{
				fprintf( stderr, "Unable to create description string.\n" );

				error_abort = 1;
			}
			else if( ewfstring_copy_system_string_to_character_string(
			          description,
			          option_description,
			          string_length ) != 1 )
			{
				fprintf( stderr, "Unable to set description string.\n" );

				error_abort = 1;
			}
		}
	}
	if( ( error_abort == 0 )
	 && ( option_examiner_name != NULL ) )
	{
		string_length = system_string_length(
		                 option_examiner_name );

		if( string_length > 0 )
		{
			string_length += 1;
			examiner_name  = (character_t *) memory_allocate(
			                                  sizeof( character_t ) * string_length );

			if( examiner_name == NULL )
			{
				fprintf( stderr, "Unable to create examiner name string.\n" );

				error_abort = 1;
			}
			else if( ewfstring_copy_system_string_to_character_string(
			          examiner_name,
			          option_examiner_name,
			          string_length ) != 1 )
			{
				fprintf( stderr, "Unable to set examiner name string.\n" );

				error_abort = 1;
			}
		}
	}
	if( ( error_abort == 0 )
	 && ( option_evidence_number != NULL ) )
	{
		string_length = system_string_length(
		                 option_evidence_number );

		if( string_length > 0 )
		{
			string_length  += 1;
			evidence_number = (character_t *) memory_allocate(
			                                   sizeof( character_t ) * string_length );

			if( evidence_number == NULL )
			{
				fprintf( stderr, "Unable to create evidence number string.\n" );

				error_abort = 1;
			}
			else if( ewfstring_copy_system_string_to_character_string(
			          evidence_number,
			          option_evidence_number,
			          string_length ) != 1 )
			{
				fprintf( stderr, "Unable to set evidence number string.\n" );

				error_abort = 1;
			}
		}
	}
	if( ( error_abort == 0 )
	 && ( option_notes != NULL ) )
	{
		string_length = system_string_length(
		                 option_notes );

		if( string_length > 0 )
		{
			string_length += 1;
			notes          = (character_t *) memory_allocate(
			                                  sizeof( character_t ) * string_length );

			if( notes == NULL )
			{
				fprintf( stderr, "Unable to create notes string.\n" );

				error_abort = 1;
			}
			else if( ewfstring_copy_system_string_to_character_string(
			          notes,
			          option_notes,
			          string_length ) != 1 )
			{
				fprintf( stderr, "Unable to set notes string.\n" );

				error_abort = 1;
			}
		}
	}
	if( error_abort != 0 )
	{
		if( case_number != NULL )
		{
			memory_free(
			 case_number );
		}
		if( description != NULL )
		{
			memory_free(
			 description );
		}
		if( examiner_name != NULL )
		{
			memory_free(
			 examiner_name );
		}
		if( evidence_number != NULL )
		{
			memory_free(
			 evidence_number );
		}
		if( notes != NULL )
		{
			memory_free(
			 notes );
		}
		return( EXIT_FAILURE );
	}
	if( ewfsignal_attach(
	     ewfcommon_signal_handler ) != 1 )
	{
		fprintf( stderr, "Unable to attach signal handler.\n" );
	}
	if( ewfcommon_abort == 0 )
	{
		if( ewfcommon_determine_operating_system_string(
		     acquiry_operating_system,
		     32 ) != 1 )
		{
			fprintf( stdout, "Unable to determine operating system string.\n" );

			acquiry_operating_system[ 0 ] = 0;
		}
		acquiry_software_version = _CHARACTER_T_STRING( LIBEWF_VERSION_STRING );

		fprintf( stdout, "Using the following acquiry parameters:\n" );

		ewfoutput_acquiry_parameters_fprint(
		 stdout,
		 target_filenames[ 0 ],
		 case_number,
		 description,
		 evidence_number,
		 examiner_name,
		 notes,
		 media_type,
		 volume_type,
		 compression_level,
		 compress_empty_block,
		 libewf_format,
		 (off64_t) acquiry_offset,
		 (size64_t) acquiry_size,
		 (size64_t) segment_file_size,
		 sectors_per_chunk,
		 sector_error_granularity,
		 read_error_retry,
		 wipe_chunk_on_error );

#if defined( HAVE_V2_API )
		if( libewf_handle_initialize(
		     &ewfcommon_libewf_handle,
		     &libewf_error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to create libewf handle.\n" );

			libewf_error_backtrace_fprint(
			 libewf_error,
			 stderr );
			libewf_error_free(
			 &libewf_error );

			error_abort = 1;
		}
		else if( libewf_open(
		          ewfcommon_libewf_handle,
		          (system_character_t * const *) target_filenames,
		          1,
		          LIBEWF_OPEN_WRITE,
		          &libewf_error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to open EWF file(s).\n" );

			libewf_error_backtrace_fprint(
			 libewf_error,
			 stderr );
			libewf_error_free(
			 &libewf_error );

			error_abort = 1;
		}
#else
		ewfcommon_libewf_handle = libewf_open(
					   (system_character_t * const *) target_filenames,
			                   1,
			                   LIBEWF_OPEN_WRITE );

		if( ewfcommon_libewf_handle == NULL )
		{
			ewfoutput_error_fprint(
			 stderr, "Unable to create EWF file(s)" );

			error_abort = 1;
		}
#endif
		else if( ewfcommon_initialize_write(
			  ewfcommon_libewf_handle,
			  case_number,
			  description,
			  evidence_number,
			  examiner_name,
			  notes,
			  acquiry_operating_system,
			  program,
			  acquiry_software_version,
			  media_type,
			  volume_type,
			  compression_level,
			  compress_empty_block,
			  libewf_format,
			  (size64_t) segment_file_size,
			  (uint32_t) sector_error_granularity ) != 1 )
		{
			fprintf( stderr, "Unable to initialize settings for EWF file(s).\n" );

			error_abort = 1;
		}
	}
	if( case_number != NULL )
	{
		memory_free(
		 case_number );
	}
	if( description != NULL )
	{
		memory_free(
		 description );
	}
	if( evidence_number != NULL )
	{
		memory_free(
		 evidence_number );
	}
	if( examiner_name != NULL )
	{
		memory_free(
		 examiner_name );
	}
	if( notes != NULL )
	{
		memory_free(
		 notes );
	}
	if( error_abort != 0 )
	{
#if defined( HAVE_V2_API )
		libewf_close(
		 ewfcommon_libewf_handle,
		 NULL );
		libewf_handle_free(
		 &ewfcommon_libewf_handle,
		 NULL );
#else
		libewf_close(
		 ewfcommon_libewf_handle );
#endif

		return( EXIT_FAILURE );
	}
	if( calculate_md5 == 1 )
	{
		calculated_md5_hash_string = (character_t *) memory_allocate(
		                                              sizeof( character_t ) * EWFSTRING_DIGEST_HASH_LENGTH_MD5 );

		if( calculated_md5_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create calculated MD5 hash string.\n" );

#if defined( HAVE_V2_API )
			libewf_close(
			 ewfcommon_libewf_handle,
			 NULL );
			libewf_handle_free(
			 &ewfcommon_libewf_handle,
			 NULL );
#else
			libewf_close(
			 ewfcommon_libewf_handle );
#endif

			return( EXIT_FAILURE );
		}
	}
	if( calculate_sha1 == 1 )
	{
		calculated_sha1_hash_string = (character_t *) memory_allocate(
		                                               sizeof( character_t ) * EWFSTRING_DIGEST_HASH_LENGTH_SHA1 );

		if( calculated_sha1_hash_string == NULL )
		{
			fprintf( stderr, "Unable to create calculated SHA1 hash string.\n" );

			memory_free(
			 calculated_md5_hash_string );

#if defined( HAVE_V2_API )
			libewf_close(
			 ewfcommon_libewf_handle,
			 NULL );
			libewf_handle_free(
			 &ewfcommon_libewf_handle,
			 NULL );
#else
			libewf_close(
			 ewfcommon_libewf_handle );
#endif

			return( EXIT_FAILURE );
		}
	}
	if( ewfcommon_abort == 0 )
	{
		if( process_status_initialize(
		     &process_status,
		     _CHARACTER_T_STRING( "Acquiry" ),
		     _CHARACTER_T_STRING( "acquired" ),
		     _CHARACTER_T_STRING( "Written" ),
		     stdout ) != 1 )
		{
			fprintf( stderr, "Unable to initialize process status.\n" );

			if( calculate_sha1 == 1 )
			{
				memory_free(
				 calculated_sha1_hash_string );
			}
			if( calculate_md5 == 1 )
			{
				memory_free(
				 calculated_md5_hash_string );
			}
#if defined( HAVE_V2_API )
			libewf_close(
			 ewfcommon_libewf_handle,
			 NULL );
			libewf_handle_free(
			 &ewfcommon_libewf_handle,
			 NULL );
#else
			libewf_close(
			 ewfcommon_libewf_handle );
#endif

			return( EXIT_FAILURE );
		}
		if( process_status_start(
		     process_status ) != 1 )
		{
			fprintf( stderr, "Unable to start process status.\n" );

			process_status_free(
			 &process_status );

			if( calculate_sha1 == 1 )
			{
				memory_free(
				 calculated_sha1_hash_string );
			}
			if( calculate_md5 == 1 )
			{
				memory_free(
				 calculated_md5_hash_string );
			}
#if defined( HAVE_V2_API )
			libewf_close(
			 ewfcommon_libewf_handle,
			 NULL );
			libewf_handle_free(
			 &ewfcommon_libewf_handle,
			 NULL );
#else
			libewf_close(
			 ewfcommon_libewf_handle );
#endif

			return( EXIT_FAILURE );
		}
		/* Start acquiring data
		 */
		write_count = ewfcommon_write_from_file_descriptor(
			       ewfcommon_libewf_handle,
			       0,
			       acquiry_size,
			       acquiry_offset,
			       sectors_per_chunk,
			       512,
			       read_error_retry,
			       sector_error_granularity,
			       calculate_md5,
			       calculated_md5_hash_string,
			       EWFSTRING_DIGEST_HASH_LENGTH_MD5,
			       calculate_sha1,
			       calculated_sha1_hash_string,
			       EWFSTRING_DIGEST_HASH_LENGTH_SHA1,
			       swap_byte_pairs,
			       wipe_chunk_on_error,
			       seek_on_error,
			       (size_t) process_buffer_size,
			       callback );

		if( write_count <= -1 )
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
	     (size64_t) write_count,
	     status ) != 1 )
	{
		fprintf( stderr, "Unable to stop process status.\n" );

		process_status_free(
		 &process_status );

		if( calculate_sha1 == 1 )
		{
			memory_free(
			 calculated_sha1_hash_string );
		}
		if( calculate_md5 == 1 )
		{
			memory_free(
			 calculated_md5_hash_string );
		}
#if defined( HAVE_V2_API )
		libewf_close(
		 ewfcommon_libewf_handle,
		 NULL );
		libewf_handle_free(
		 &ewfcommon_libewf_handle,
		 NULL );
#else
		libewf_close(
		 ewfcommon_libewf_handle );
#endif

		return( EXIT_FAILURE );
	}
	if( process_status_free(
	     &process_status ) != 1 )
	{
		fprintf( stderr, "Unable to free process status.\n" );

		if( calculate_sha1 == 1 )
		{
			memory_free(
			 calculated_sha1_hash_string );
		}
		if( calculate_md5 == 1 )
		{
			memory_free(
			 calculated_md5_hash_string );
		}
#if defined( HAVE_V2_API )
		libewf_close(
		 ewfcommon_libewf_handle,
		 NULL );
		libewf_handle_free(
		 &ewfcommon_libewf_handle,
		 NULL );
#else
		libewf_close(
		 ewfcommon_libewf_handle );
#endif

		return( EXIT_FAILURE );
	}
	if( status == PROCESS_STATUS_COMPLETED )
	{
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
		ewfoutput_acquiry_errors_fprint(
		 stdout,
		 ewfcommon_libewf_handle,
		 &amount_of_acquiry_errors );

		if( log_file_stream != NULL )
		{
			ewfoutput_acquiry_errors_fprint(
			 log_file_stream,
			 ewfcommon_libewf_handle,
			 &amount_of_acquiry_errors );
		}
	}
#if defined( HAVE_V2_API )
	if( libewf_close(
	     ewfcommon_libewf_handle,
	     &libewf_error ) != 0 )
	{
		fprintf(
		 stderr,
		 "Unable to close EWF file(s).\n" );

		libewf_error_backtrace_fprint(
		 libewf_error,
		 stderr );
		libewf_error_free(
		 &libewf_error );

		if( log_file_stream != NULL )
		{
			file_stream_io_fclose(
			 log_file_stream );
		}
		if( calculate_sha1 == 1 )
		{
			memory_free(
			 calculated_sha1_hash_string );
		}
		if( calculate_md5 == 1 )
		{
			memory_free(
			 calculated_md5_hash_string );
		}
		libewf_handle_free(
		 &ewfcommon_libewf_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( libewf_handle_free(
	     &ewfcommon_libewf_handle,
	     &libewf_error ) != 0 )
	{
		fprintf(
		 stderr,
		 "Unable to free libewf handle.\n" );

		libewf_error_backtrace_fprint(
		 libewf_error,
		 stderr );
		libewf_error_free(
		 &libewf_error );

		if( log_file_stream != NULL )
		{
			file_stream_io_fclose(
			 log_file_stream );
		}
		if( calculate_sha1 == 1 )
		{
			memory_free(
			 calculated_sha1_hash_string );
		}
		if( calculate_md5 == 1 )
		{
			memory_free(
			 calculated_md5_hash_string );
		}
		return( EXIT_FAILURE );
	}
#else
	if( libewf_close(
	     ewfcommon_libewf_handle ) != 0 )
	{
		fprintf(
		 stderr,
		 "Unable to close EWF file(s).\n" );

		if( log_file_stream != NULL )
		{
			file_stream_io_fclose(
			 log_file_stream );
		}
		if( calculate_sha1 == 1 )
		{
			memory_free(
			 calculated_sha1_hash_string );
		}
		if( calculate_md5 == 1 )
		{
			memory_free(
			 calculated_md5_hash_string );
		}
		return( EXIT_FAILURE );
	}
#endif
	if( ewfsignal_detach() != 1 )
	{
		fprintf( stderr, "Unable to detach signal handler.\n" );
	}
        if( status != PROCESS_STATUS_COMPLETED )
        {
		if( log_file_stream != NULL )
		{
			file_stream_io_fclose(
			 log_file_stream );
		}
		if( calculate_sha1 == 1 )
		{
			memory_free(
			 calculated_sha1_hash_string );
		}
		if( calculate_md5 == 1 )
		{
			memory_free(
			 calculated_md5_hash_string );
		}
		return( EXIT_FAILURE );
	}
	if( calculate_md5 == 1 )
	{
		fprintf( stdout, "MD5 hash calculated over data:\t%" PRIs "\n",
		 calculated_md5_hash_string );

		if( log_file_stream != NULL )
		{
			fprintf( log_file_stream, "MD5 hash calculated over data:\t%" PRIs "\n",
			 calculated_md5_hash_string );
		}
		memory_free(
		 calculated_md5_hash_string );
	}
	if( calculate_sha1 == 1 )
	{
		fprintf( stdout, "SHA1 hash calculated over data:\t%" PRIs "\n",
		 calculated_sha1_hash_string );

		if( log_file_stream != NULL )
		{
			fprintf( log_file_stream, "SHA1 hash calculated over data:\t%" PRIs "\n",
			 calculated_sha1_hash_string );
		}
		memory_free(
		 calculated_sha1_hash_string );
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
	return( EXIT_SUCCESS );
}

