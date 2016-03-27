/*
 * ewfacquirestream
 * Reads data from a stdin and writes it in EWF format
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

#include <errno.h>

#include <stdio.h>

#if defined( HAVE_SYS_IOCTL_H )
#include <sys/ioctl.h>
#endif

#if defined( HAVE_UNISTD_H )
#include <unistd.h>
#endif

#if defined( HAVE_FCNTL_H )
#include <fcntl.h>
#endif

#if defined( HAVE_IO_H )
#include <io.h>
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

#include "byte_size_string.h"
#include "ewfcommon.h"
#include "ewfgetopt.h"
#include "ewfinput.h"
#include "ewfoutput.h"
#include "ewfsignal.h"
#include "file_stream_io.h"
#include "imaging_handle.h"
#include "notify.h"
#include "process_status.h"
#include "storage_media_buffer.h"
#include "system_string.h"

imaging_handle_t *ewfacquirestream_imaging_handle = NULL;
int ewfacquirestream_abort                        = 0;

/* Prints the executable usage information to the stream
 */
void usage_fprint(
      FILE *stream )
{
	system_character_t default_segment_file_size_string[ 16 ];
	system_character_t minimum_segment_file_size_string[ 16 ];
	system_character_t maximum_32bit_segment_file_size_string[ 16 ];
	system_character_t maximum_64bit_segment_file_size_string[ 16 ];

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
	fprintf( stream, "Use ewfacquirestream to acquire data from a pipe and store it in the EWF format\n"
	                 "(Expert Witness Compression Format).\n\n" );

	fprintf( stream, "Usage: ewfacquirestream [ -A codepage ] [ -b amount_of_sectors ]\n"
	                 "                        [ -c compression_type ] [ -C case_number ]\n"
	                 "                        [ -d digest_type ] [ -D description ]\n"
	                 "                        [ -e examiner_name ] [ -E evidence_number ]\n"
	                 "                        [ -f format ] [ -l log_filename ] [ -m media_type ]\n"
	                 "                        [ -M volume_type ] [ -N notes ]\n"
	                 "                        [ -p process_buffer_size ] [ -S segment_file_size ]\n"
	                 "                        [ -t target ] [ -hqsvVw ]\n\n" );

	fprintf( stream, "\tReads data from stdin\n\n" );

	fprintf( stream, "\t-A: codepage of header section, options: ascii (default), windows-1250,\n"
	                 "\t    windows-1251, windows-1252, windows-1253, windows-1254,\n"
	                 "\t    windows-1255, windows-1256, windows-1257, windows-1258\n" );
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
	fprintf( stream, "\t-m: specify the media type, options: fixed (default), removable, optical, memory\n" );
	fprintf( stream, "\t-M: specify the volume type, options: logical, physical (default)\n" );
	fprintf( stream, "\t-N: specify the notes (default is notes).\n" );
	fprintf( stream, "\t-p: specify the process buffer size (default is the chunk size)\n" );
	fprintf( stream, "\t-q: quiet shows no status information\n" );
	fprintf( stream, "\t-s: swap byte pairs of the media data (from AB to BA)\n"
	                 "\t    (use this for big to little endian conversion and vice versa)\n" );

	if( result == 1 )
	{
		fprintf( stream, "\t-S: specify the segment file size in bytes (default is %" PRIs_SYSTEM ")\n"
		                 "\t    (minimum is %" PRIs_SYSTEM ", maximum is %" PRIs_SYSTEM " for encase6 format\n"
		                 "\t    and %" PRIs_SYSTEM " for other formats)\n",
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

/* Prints an overview of the aquiry parameters
 * Returns 1 if successful or -1 on error
 */
int ewfacquirestream_acquiry_parameters_fprint(
     FILE *stream,
     system_character_t *filename,
     system_character_t *case_number,
     system_character_t *description,
     system_character_t *evidence_number,
     system_character_t *examiner_name,
     system_character_t *notes,
     uint8_t media_type,
     uint8_t volume_type,
     int8_t compression_level,
     uint8_t compress_empty_block,
     uint8_t libewf_format,
     size64_t acquiry_size,
     size64_t segment_file_size,
     uint32_t bytes_per_sector,
     uint32_t sectors_per_chunk,
     uint32_t sector_error_granularity,
     uint8_t read_error_retry,
     uint8_t wipe_block_on_read_error,
     liberror_error_t **error )
{
	system_character_t acquiry_size_string[ 16 ];
	system_character_t segment_file_size_string[ 16 ];

	static char *function = "ewfacquirestream_acquiry_parameters_fprint";
	int result            = 0;

	if( stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid stream.",
		 function );

		return( -1 );
	}
	fprintf(
	 stream,
	 "Using the following acquiry parameters:\n" );

	fprintf(
	 stream,
	 "Image path and filename:\t%" PRIs_SYSTEM ".",
	 filename );

	if( libewf_format == LIBEWF_FORMAT_SMART )
	{
		fprintf(
		 stream,
		 "s01\n" );
	}
	else if( ( libewf_format == LIBEWF_FORMAT_EWF )
	      || ( libewf_format == LIBEWF_FORMAT_EWFX ) )
	{
		fprintf(
		 stream,
		 "e01\n" );
	}
	else
	{
		fprintf(
		 stream,
		 "E01\n" );
	}
	fprintf(
	 stream,
	 "Case number:\t\t\t" );

	if( case_number != NULL )
	{
		fprintf(
		 stream,
		 "%" PRIs_SYSTEM "",
		 case_number );
	}
	fprintf(
	 stream,
	 "\n" );

	fprintf(
	 stream,
	 "Description:\t\t\t" );

	if( description != NULL )
	{
		fprintf(
		 stream,
		 "%" PRIs_SYSTEM "",
		 description );
	}
	fprintf(
	 stream,
	 "\n" );

	fprintf(
	 stream,
	 "Evidence number:\t\t" );

	if( evidence_number != NULL )
	{
		fprintf(
		 stream,
		 "%" PRIs_SYSTEM "",
		 evidence_number );
	}
	fprintf(
	 stream,
	 "\n" );

	fprintf(
	 stream,
	 "Examiner name:\t\t\t" );

	if( examiner_name != NULL )
	{
		fprintf(
		 stream,
		 "%" PRIs_SYSTEM "",
		 examiner_name );
	}
	fprintf(
	 stream,
	 "\n" );

	fprintf(
	 stream,
	 "Notes:\t\t\t\t" );

	if( notes != NULL )
	{
		fprintf(
		 stream,
		 "%" PRIs_SYSTEM "",
		 notes );
	}
	fprintf(
	 stream,
	 "\n" );

	fprintf(
	 stream,
	 "Media type:\t\t\t" );

	if( media_type == LIBEWF_MEDIA_TYPE_FIXED )
	{
		fprintf(
		 stream,
		 "fixed disk\n" );
	}
	else if( media_type == LIBEWF_MEDIA_TYPE_REMOVABLE )
	{
		fprintf(
		 stream,
		 "removable disk\n" );
	}
	else if( media_type == LIBEWF_MEDIA_TYPE_OPTICAL )
	{
		fprintf(
		 stream,
		 "optical disk (CD/DVD/BD)\n" );
	}
	else if( media_type == LIBEWF_MEDIA_TYPE_MEMORY )
	{
		fprintf(
		 stream,
		 "memory (RAM)\n" );
	}
	fprintf(
	 stream,
	 "Volume type:\t\t\t" );

	if( volume_type == LIBEWF_VOLUME_TYPE_LOGICAL )
	{
		fprintf(
		 stream,
		 "logical\n" );
	}
	else if( volume_type == LIBEWF_VOLUME_TYPE_PHYSICAL )
	{
		fprintf(
		 stream,
		 "physical\n" );
	}
	fprintf(
	 stream,
	 "Compression used:\t\t" );

	if( compression_level == LIBEWF_COMPRESSION_FAST )
	{
		fprintf(
		 stream,
		 "fast\n" );
	}
	else if( compression_level == LIBEWF_COMPRESSION_BEST )
	{
		fprintf(
		 stream,
		 "best\n" );
	}
	else if( compression_level == LIBEWF_COMPRESSION_NONE )
	{
		if( compress_empty_block == 0 )
		{
			fprintf(
			 stream,
			 "none\n" );
		}
		else
		{
			fprintf(
			 stream,
			 "empty block\n" );
		}
	}
	fprintf(
	 stream,
	 "EWF file format:\t\t" );

	if( libewf_format == LIBEWF_FORMAT_EWF )
	{
		fprintf(
		 stream,
		 "original EWF\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_SMART )
	{
		fprintf(
		 stream,
		 "SMART\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_FTK )
	{
		fprintf(
		 stream,
		 "FTK Imager\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_ENCASE1 )
	{
		fprintf(
		 stream,
		 "EnCase 1\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_ENCASE2 )
	{
		fprintf(
		 stream,
		 "EnCase 2\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_ENCASE3 )
	{
		fprintf(
		 stream,
		 "EnCase 3\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_ENCASE4 )
	{
		fprintf(
		 stream,
		 "EnCase 4\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_ENCASE5 )
	{
		fprintf(
		 stream,
		 "EnCase 5\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_ENCASE6 )
	{
		fprintf(
		 stream,
		 "EnCase 6\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_LINEN5 )
	{
		fprintf(
		 stream,
		 "linen 5\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_LINEN6 )
	{
		fprintf(
		 stream,
		 "linen 6\n" );
	}
	else if( libewf_format == LIBEWF_FORMAT_EWFX )
	{
		fprintf(
		 stream,
		 "extended EWF (libewf)\n" );
	}
	else
	{
		fprintf(
		 stream,
		 "\n" );
	}
	result = byte_size_string_create(
	          acquiry_size_string,
	          16,
	          acquiry_size,
	          BYTE_SIZE_STRING_UNIT_MEBIBYTE,
	          NULL );

	fprintf(
	 stream,
	 "Amount of bytes to acquire:\t" );

	if( acquiry_size == 0 )
	{
		fprintf(
		 stream,
		 "%" PRIu64 " (until end of input)",
		 acquiry_size );
	}
	else if( result == 1 )
	{
		fprintf(
		 stream,
		 "%" PRIs_SYSTEM " (%" PRIu64 " bytes)",
		 acquiry_size_string, acquiry_size );
	}
	else
	{
		fprintf(
		 stream,
		 "%" PRIu64 " bytes",
		 acquiry_size );
	}
	fprintf(
	 stream,
	 "\n" );

	result = byte_size_string_create(
	          segment_file_size_string,
	          16,
	          segment_file_size,
	          BYTE_SIZE_STRING_UNIT_MEBIBYTE,
	          NULL );

	fprintf(
	 stream,
	 "Evidence segment file size:\t" );

	if( result == 1 )
	{
		fprintf(
		 stream, "%" PRIs_SYSTEM " (%" PRIu64 " bytes)",
		 segment_file_size_string,
		 segment_file_size );
	}
	else
	{
		fprintf(
		 stream,
		 "%" PRIu64 " bytes",
		 segment_file_size );
	}
	fprintf(
	 stream,
	 "\n" );

	fprintf(
	 stream,
	 "Bytes per sector:\t\t%" PRIu32 "\n",
	 bytes_per_sector );
	fprintf(
	 stream,
	 "Block size:\t\t\t%" PRIu32 " sectors\n",
	 sectors_per_chunk );
	fprintf(
	 stream,
	 "Error granularity:\t\t%" PRIu32 " sectors\n",
	 sector_error_granularity );
	fprintf(
	 stream,
	 "Retries on read error:\t\t%" PRIu8 "\n",
	 read_error_retry );

	fprintf(
	 stream,
	 "Wipe sectors on read error:\t" );

	if( wipe_block_on_read_error == 0 )
	{
		fprintf(
		 stream,
		 "no\n" );
	}
	else
	{
		fprintf(
		 stream,
		 "yes\n" );
	}
	fprintf(
	 stream,
	 "\n" );

	return( 1 );
}

/* Reads a chunk of data from the file descriptor into the buffer
 * Returns the amount of bytes read, 0 if at end of input or -1 on error
 */
ssize_t ewfacquirestream_read_chunk(
         libewf_handle_t *handle,
         int input_file_descriptor,
         uint8_t *buffer,
         size_t buffer_size,
         size32_t chunk_size,
         ssize64_t total_read_count,
         uint8_t read_error_retry,
         liberror_error_t **error )
{
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
	system_character_t *error_string  = NULL;
#endif
	static char *function             = "ewfacquirestream_read_chunk";
	ssize_t read_count                = 0;
	ssize_t buffer_offset             = 0;
	size_t read_size                  = 0;
	size_t bytes_to_read              = 0;
	int32_t read_amount_of_errors     = 0;
	uint32_t read_error_offset        = 0;

	if( handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.",
		 function );

		return( -1 );
	}
	if( input_file_descriptor == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid input file descriptor.",
		 function );

		return( -1 );
	}
	if( buffer == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid buffer.",
		 function );

		return( -1 );
	}
	if( buffer_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid buffer size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( chunk_size == 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid chunk size value zero or less.",
		 function );

		return( -1 );
	}
	if( total_read_count <= -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
		 "%s: invalid total read count value less than zero.",
		 function );

		return( -1 );
	}
	while( buffer_size > 0 )
	{
		/* Determine the amount of bytes to read from the input
		 * Read as much as possible in chunk sizes
		 */
		if( buffer_size < (size_t) chunk_size )
		{
			read_size = buffer_size;
		}
		else
		{
			read_size = chunk_size;
		}
		bytes_to_read = read_size;

		while( read_amount_of_errors <= read_error_retry )
		{
			read_count = file_io_read(
			              input_file_descriptor,
			              &( buffer[ buffer_offset + read_error_offset ] ),
			              bytes_to_read );

#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf(
			 "%s: read buffer at: %" PRIu64 " of size: %" PRIzd ".\n",
			 function,
			 total_read_count,
			 read_count );
#endif

			if( read_count <= -1 )
			{
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
				if( ( errno == ESPIPE )
				 || ( errno == EPERM )
				 || ( errno == ENXIO )
				 || ( errno == ENODEV ) )
				{
					error_string = ewfcommon_strerror(
					                errno );

					if( error_string != NULL )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_IO,
						 LIBERROR_IO_ERROR_READ_FAILED,
						 "%s: error reading data: " PRIs_SYSTEM "s.",
						 function,
						 error_string );

						memory_free(
						 error_string );

						return( -1 );
					}
				}
#endif
				if( errno == ESPIPE )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_IO,
					 LIBERROR_IO_ERROR_READ_FAILED,
					 "%s: error reading data: invalid seek.",
					 function );

					return( -1 );
				}
				else if( errno == EPERM )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_IO,
					 LIBERROR_IO_ERROR_READ_FAILED,
					 "%s: error reading data: operation not permitted.",
					 function );

					return( -1 );
				}
				else if( errno == ENXIO )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_IO,
					 LIBERROR_IO_ERROR_READ_FAILED,
					 "%s: error reading data: no such device or address.",
					 function );

					return( -1 );
				}
				else if( errno == ENODEV )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_IO,
					 LIBERROR_IO_ERROR_READ_FAILED,
					 "%s: error reading data: no such device.",
					 function );

					return( -1 );
				}
			}
			else
			{
				/* The last read is OK, correct read_count
				 */
				if( read_count == (ssize_t) bytes_to_read )
				{
					read_count = read_error_offset + bytes_to_read;
				}
				/* The entire read is OK
				 */
				if( read_count == (ssize_t) read_size )
				{
					break;
				}
				/* If no end of input can be determined
				 */
				/* If some bytes were read it is possible that the end of the input reached
				 */
				if( read_count > 0 )
				{
					return( (ssize32_t) ( buffer_offset + read_count ) );
				}
				/* No bytes were read
				 */
				if( read_count == 0 )
				{
					return( 0 );
				}
#if defined( HAVE_VERBOSE_OUTPUT )
				notify_verbose_printf(
				 "%s: read error at offset %" PRIjd " after reading %" PRIzd " bytes.\n",
				 function,
				 total_read_count,
				 read_count );
#endif

				/* There was a read error at a certain offset
				 */
				read_error_offset += read_count;
				bytes_to_read     -= read_count;
			}
			read_amount_of_errors++;

			if( read_amount_of_errors > read_error_retry )
			{
				return( 0 );
			}
		}
		buffer_size   -= read_count;
		buffer_offset += read_count;

		/* At the end of the input
		 */
		if( ewfacquirestream_abort != 0 )
		{
			break;
		}
	}
	return( buffer_offset );
}

/* Reads data from a file descriptor and writes it in EWF format
 * Returns the amount of bytes written or -1 on error
 */
ssize64_t ewfacquirestream_read_input(
           imaging_handle_t *imaging_handle,
           int input_file_descriptor,
           size64_t write_size,
           uint32_t bytes_per_sector,
           uint8_t swap_byte_pairs,
           uint8_t read_error_retry,
           uint8_t wipe_block_on_read_error,
           size_t process_buffer_size,
           system_character_t *calculated_md5_hash_string,
           size_t calculated_md5_hash_string_size,
           system_character_t *calculated_sha1_hash_string,
           size_t calculated_sha1_hash_string_size,
           void (*callback)( process_status_t *process_status, size64_t bytes_read, size64_t bytes_total ),
           liberror_error_t **error )
{
	storage_media_buffer_t *storage_media_buffer = NULL;
	static char *function                        = "ewfacquirestream_read_input";
	ssize64_t total_write_count                  = 0;
	size32_t chunk_size                          = 0;
	ssize_t read_count                           = 0;
	ssize_t process_count                        = 0;
	ssize_t write_count                          = 0;
	uint32_t amount_of_chunks                    = 0;

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
	if( input_file_descriptor == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file descriptor.",
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
#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
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

	while( write_size == 0 )
	{
		/* Read a chunk from the file descriptor
		 */
		read_count = ewfacquirestream_read_chunk(
		              imaging_handle->output_handle,
		              input_file_descriptor,
		              storage_media_buffer->raw_buffer,
		              storage_media_buffer->raw_buffer_size,
		              process_buffer_size,
		              total_write_count,
		              read_error_retry,
		              error );

#if defined( HAVE_DEBUG_OUTPUT )
		notify_verbose_printf(
		 "%s: read chunk: %" PRIi32 " with size: %" PRIzd ".\n",
		 function,
		 amount_of_chunks + 1,
		 read_count );
#endif

		if( read_count < 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: error reading data from input.",
			 function );

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
		if( read_count == 0 )
		{
			break;
		}
		amount_of_chunks++;

#if defined( HAVE_LOW_LEVEL_FUNCTIONS )
		storage_media_buffer->data_in_compression_buffer = 0;
#endif
		storage_media_buffer->raw_buffer_amount = read_count;

		/* Swap byte pairs
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

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
		/* Digest hashes are calcultated after swap
		 */
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

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
		process_count = imaging_handle_write_prepare_buffer(
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

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
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

			storage_media_buffer_free(
			 &storage_media_buffer,
			 NULL );

			return( -1 );
		}
		total_write_count += read_count;

		/* Callback for status update
		 */
		if( callback != NULL )
		{
			callback(
		         process_status,
		         (size64_t) total_write_count,
		         write_size );
		}
		if( ewfacquirestream_abort != 0 )
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
	write_count = imaging_handle_finalize(
	               imaging_handle,
	               calculated_md5_hash_string,
	               calculated_md5_hash_string_size,
	               calculated_sha1_hash_string,
	               calculated_sha1_hash_string_size,
	               error );

	if( write_count == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_WRITE_FAILED,
		 "%s: unable to finalize write.\n",
		 function );

		return( -1 );
	}
	total_write_count += write_count;

	return( total_write_count );
}

/* Signal handler for ewfacquire
 */
void ewfacquirestream_signal_handler(
      ewfsignal_t signal )
{
	liberror_error_t *error = NULL;
	static char *function   = "ewfacquirestream_signal_handler";

	ewfacquirestream_abort = 1;

	if( ( ewfacquirestream_imaging_handle != NULL )
	 && ( imaging_handle_signal_abort(
	       ewfacquirestream_imaging_handle,
	       &error ) != 1 ) )
	{
		notify_warning_printf(
		 "%s: unable to signal imaging handle to abort.\n",
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
	system_character_t acquiry_operating_system[ 32 ];

	imaging_handle_t *imaging_handle                = NULL;

	liberror_error_t *error                         = NULL;

	system_character_t *acquiry_software_version    = NULL;
	system_character_t *calculated_md5_hash_string  = NULL;
	system_character_t *calculated_sha1_hash_string = NULL;
	system_character_t *case_number                 = NULL;
	system_character_t *description                 = NULL;
	system_character_t *evidence_number             = NULL;
	system_character_t *examiner_name               = NULL;
	system_character_t *log_filename                = NULL;
	system_character_t *notes                       = NULL;
	system_character_t *option_case_number          = NULL;
	system_character_t *option_description          = NULL;
	system_character_t *option_examiner_name        = NULL;
	system_character_t *option_evidence_number      = NULL;
	system_character_t *option_notes                = NULL;
	system_character_t *program                     = _SYSTEM_CHARACTER_T_STRING( "ewfacquirestream" );
	system_character_t *target_filename             = _SYSTEM_CHARACTER_T_STRING( "stream" );

	FILE *log_file_stream                           = NULL;
	void *callback                                  = &process_status_update_unknown_total;

	system_integer_t option                         = 0;
	size_t string_length                            = 0;
	int64_t write_count                             = 0;
	uint64_t acquiry_size                           = 0;
	uint64_t process_buffer_size                    = 0;
	uint64_t segment_file_size                      = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;
	uint32_t bytes_per_sector                       = 512;
	uint32_t sectors_per_chunk                      = 64;
	uint32_t sector_error_granularity               = 64;
	int8_t compression_level                        = LIBEWF_COMPRESSION_NONE;
	uint8_t calculate_md5                           = 1;
	uint8_t calculate_sha1                          = 0;
	uint8_t compress_empty_block                    = 0;
	uint8_t libewf_format                           = LIBEWF_FORMAT_ENCASE5;
	uint8_t media_type                              = LIBEWF_MEDIA_TYPE_FIXED;
	uint8_t read_error_retry                        = 2;
	uint8_t swap_byte_pairs                         = 0;
	uint8_t verbose                                 = 0;
	uint8_t volume_type                             = LIBEWF_VOLUME_TYPE_PHYSICAL;
	uint8_t wipe_block_on_read_error                = 0;
	int error_abort                                 = 0;
	int header_codepage                             = LIBEWF_CODEPAGE_ASCII;
	int result                                      = 0;
	int status                                      = 0;

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

#if defined( WINAPI )
	if( _setmode(
	     _fileno(
	      stdin ),
	     _O_BINARY ) == -1 )
	{
		fprintf(
		 stderr,
		 "Unable to set stdin to binary mode.\n" );

		usage_fprint(
		 stdout );

		return( EXIT_FAILURE );
	}
#endif

	while( ( option = ewfgetopt(
	                   argc,
	                   argv,
	                   _SYSTEM_CHARACTER_T_STRING( "A:b:c:C:d:D:e:E:f:hl:m:M:N:p:qsS:t:vVw" ) ) ) != (system_integer_t) -1 )
	{
		switch( option )
		{
			case (system_integer_t) '?':
			default:
				fprintf(
				 stderr,
				 "Invalid argument: %" PRIs_SYSTEM "\n",
				 argv[ optind ] );

				usage_fprint(
				 stdout );

				return( EXIT_FAILURE );

			case (system_integer_t) 'A':
				if( ewfinput_determine_header_codepage(
				     optarg,
				     &header_codepage ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unsuported header codepage defaulting to: ascii.\n" );

					header_codepage = LIBEWF_CODEPAGE_ASCII;
				}
				break;

			case (system_integer_t) 'b':
				if( ewfinput_determine_sectors_per_chunk(
				     optarg,
				     &sectors_per_chunk ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unsuported amount of sectors per chunk defaulting to: 64.\n" );

					sectors_per_chunk = 64;
				}
				break;

			case (system_integer_t) 'c':
				if( ewfinput_determine_compression_level(
				     optarg,
				     &compression_level,
				     &compress_empty_block ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unsupported compression type defaulting to: none.\n" );

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
					fprintf(
					 stderr,
					 "Unsupported digest type.\n" );
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
				if( ( ewfinput_determine_libewf_format(
				       optarg,
				       &libewf_format ) != 1 )
				 || ( libewf_format == LIBEWF_FORMAT_EWF )
				 || ( libewf_format == LIBEWF_FORMAT_SMART ) )
				{
					fprintf(
					 stderr,
					 "Unsupported EWF file format type defaulting to: encase5.\n" );

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
				if( ewfinput_determine_media_type(
				     optarg,
				     &media_type ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unsupported media type defaulting to: fixed.\n" );

					media_type = LIBEWF_MEDIA_TYPE_FIXED;
				}
				break;

			case (system_integer_t) 'M':
				if( ewfinput_determine_volume_type(
				     optarg,
				     &volume_type ) != 1 )
				{
					fprintf(
					 stderr,
					 "Unsupported volume type defaulting to: physical.\n" );

					volume_type = LIBEWF_VOLUME_TYPE_PHYSICAL;
				}
				break;

			case (system_integer_t) 'N':
				option_notes = optarg;

				break;

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
				callback = NULL;
				break;

			case (system_integer_t) 's':
				swap_byte_pairs = 1;

				break;

			case (system_integer_t) 'S':
				string_length = system_string_length(
				                 optarg );

				result = byte_size_string_convert(
				          optarg,
				          string_length,
				          &segment_file_size,
				          &error );

				if( result != 1 )
				{
					notify_error_backtrace(
					 error );
					liberror_error_free(
					 &error );
				}
				if( ( result != 1 )
				 || ( segment_file_size < EWFCOMMON_MINIMUM_SEGMENT_FILE_SIZE )
				 || ( ( libewf_format == LIBEWF_FORMAT_ENCASE6 )
				  && ( segment_file_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_64BIT ) )
				 || ( ( libewf_format != LIBEWF_FORMAT_ENCASE6 )
				  && ( segment_file_size >= (uint64_t) EWFCOMMON_MAXIMUM_SEGMENT_FILE_SIZE_32BIT ) ) )
				{
					segment_file_size = EWFCOMMON_DEFAULT_SEGMENT_FILE_SIZE;

					fprintf(
					 stderr,
					 "Unsupported segment file size defaulting to: %" PRIu64 ".\n",
					 segment_file_size );
				}
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

			case (system_integer_t) 'w':
				wipe_block_on_read_error = 1;

				break;
		}
	}
	notify_set_values(
	 stderr,
	 verbose );
	libewf_set_notify_values(
	 stderr,
	 verbose );

	if( option_case_number != NULL )
	{
		string_length = system_string_length(
		                 option_case_number );

		if( string_length > 0 )
		{
			case_number = (system_character_t *) memory_allocate(
			                                      sizeof( system_character_t ) * ( string_length + 1 ) );

			if( case_number == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to create case number string.\n" );

				error_abort = 1;
			}
			else if( memory_copy(
			          case_number,
			          option_case_number,
			          string_length + 1 ) == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to set case number string.\n" );

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
			description = (system_character_t *) memory_allocate(
			                                      sizeof( system_character_t ) * ( string_length + 1 ) );

			if( description == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to create description string.\n" );

				error_abort = 1;
			}
			else if( memory_copy(
			          description,
			          option_description,
			          string_length + 1 ) == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to set description string.\n" );

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
			examiner_name = (system_character_t *) memory_allocate(
			                                        sizeof( system_character_t ) * ( string_length + 1 ) );

			if( examiner_name == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to create examiner name string.\n" );

				error_abort = 1;
			}
			else if( memory_copy(
			          examiner_name,
			          option_examiner_name,
			          string_length + 1 ) == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to set examiner name string.\n" );

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
			evidence_number = (system_character_t *) memory_allocate(
			                                          sizeof( system_character_t ) * ( string_length + 1 ) );

			if( evidence_number == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to create evidence number string.\n" );

				error_abort = 1;
			}
			else if( memory_copy(
			          evidence_number,
			          option_evidence_number,
			          string_length + 1 ) == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to set evidence number string.\n" );

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
			notes = (system_character_t *) memory_allocate(
			                                sizeof( system_character_t ) * ( string_length + 1 ) );

			if( notes == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to create notes string.\n" );

				error_abort = 1;
			}
			else if( memory_copy(
			          notes,
			          option_notes,
			          string_length + 1 ) == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to set notes string.\n" );

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
	     ewfacquirestream_signal_handler ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to attach signal handler.\n" );
	}
	if( ewfacquirestream_abort == 0 )
	{
		if( ewfcommon_determine_operating_system_string(
		     acquiry_operating_system,
		     32,
		     &error ) != 1 )
		{
			fprintf(
			 stdout,
			 "Unable to determine operating system string.\n" );

			notify_error_backtrace(
			 error );
			liberror_error_free(
			 &error );

			acquiry_operating_system[ 0 ] = 0;
		}
		acquiry_software_version = _SYSTEM_CHARACTER_T_STRING( LIBEWF_VERSION_STRING );

		if( ewfacquirestream_acquiry_parameters_fprint(
		     stdout,
		     target_filename,
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
		     (size64_t) acquiry_size,
		     (size64_t) segment_file_size,
		     bytes_per_sector,
		     sectors_per_chunk,
		     sector_error_granularity,
		     read_error_retry,
		     wipe_block_on_read_error,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to print acquiry parameters.\n" );

			error_abort = 1;
		}
		else if( imaging_handle_initialize(
		          &imaging_handle,
		          calculate_md5,
		          calculate_sha1,
		          &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to create imaging handle.\n" );

			error_abort = 1;
		}
		else if( imaging_handle_open_output(
		          imaging_handle,
		          target_filename,
		          &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to open output file(s).\n" );

			imaging_handle_free(
			 &imaging_handle,
			 NULL );

			error_abort = 1;
		}
		else if( imaging_handle_set_output_values(
		          imaging_handle,
			  case_number,
			  description,
			  evidence_number,
			  examiner_name,
			  notes,
			  acquiry_operating_system,
			  program,
			  acquiry_software_version,
		          bytes_per_sector,
		          acquiry_size,
		          media_type,
		          volume_type,
		          compression_level,
		          compress_empty_block,
		          libewf_format,
		          segment_file_size,
		          sectors_per_chunk,
		          sector_error_granularity,
		          &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to initialize output settings.\n" );

			imaging_handle_close(
			 imaging_handle,
			 NULL );
			imaging_handle_free(
			 &imaging_handle,
			 NULL );

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
		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

		return( EXIT_FAILURE );
	}
	if( calculate_md5 == 1 )
	{
		calculated_md5_hash_string = (system_character_t *) memory_allocate(
		                                                     sizeof( system_character_t ) * DIGEST_HASH_STRING_SIZE_MD5 );

		if( calculated_md5_hash_string == NULL )
		{
			fprintf(
			 stderr,
			 "Unable to create calculated MD5 hash string.\n" );

			imaging_handle_close(
			 imaging_handle,
			 NULL );
			imaging_handle_free(
			 &imaging_handle,
			 NULL );

			return( EXIT_FAILURE );
		}
	}
	if( calculate_sha1 == 1 )
	{
		calculated_sha1_hash_string = (system_character_t *) memory_allocate(
		                                                      sizeof( system_character_t ) * DIGEST_HASH_STRING_SIZE_SHA1 );

		if( calculated_sha1_hash_string == NULL )
		{
			fprintf(
			 stderr,
			 "Unable to create calculated SHA1 hash string.\n" );

			memory_free(
			 calculated_md5_hash_string );

			imaging_handle_close(
			 imaging_handle,
			 NULL );
			imaging_handle_free(
			 &imaging_handle,
			 NULL );

			return( EXIT_FAILURE );
		}
	}
	if( ewfacquirestream_abort == 0 )
	{
		if( process_status_initialize(
		     &process_status,
		     _SYSTEM_CHARACTER_T_STRING( "Acquiry" ),
		     _SYSTEM_CHARACTER_T_STRING( "acquired" ),
		     _SYSTEM_CHARACTER_T_STRING( "Written" ),
		     stdout ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to initialize process status.\n" );

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
			imaging_handle_close(
			 imaging_handle,
			 NULL );
			imaging_handle_free(
			 &imaging_handle,
			 NULL );

			return( EXIT_FAILURE );
		}
		if( process_status_start(
		     process_status ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to start process status.\n" );

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
			imaging_handle_close(
			 imaging_handle,
			 NULL );
			imaging_handle_free(
			 &imaging_handle,
			 NULL );

			return( EXIT_FAILURE );
		}
		/* Start acquiring data
		 */
		write_count = ewfacquirestream_read_input(
		               imaging_handle,
		               0,
		               acquiry_size,
		               bytes_per_sector,
		               swap_byte_pairs,
		               read_error_retry,
		               wipe_block_on_read_error,
		               (size_t) process_buffer_size,
		               calculated_md5_hash_string,
		               DIGEST_HASH_STRING_SIZE_MD5,
		               calculated_sha1_hash_string,
		               DIGEST_HASH_STRING_SIZE_SHA1,
		               callback,
		               &error );

		if( write_count <= -1 )
		{
			notify_error_backtrace(
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
	if( ewfacquirestream_abort != 0 )
	{
		status = PROCESS_STATUS_ABORTED;
	}
	if( process_status_stop(
	     process_status,
	     (size64_t) write_count,
	     status ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to stop process status.\n" );

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
		imaging_handle_close(
		 imaging_handle,
		 NULL );
		imaging_handle_free(
		 &imaging_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( process_status_free(
	     &process_status ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free process status.\n" );

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
		imaging_handle_close(
		 imaging_handle,
		 NULL );
		imaging_handle_free(
		 &imaging_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( status == PROCESS_STATUS_COMPLETED )
	{
		if( log_filename != NULL )
		{
			log_file_stream = system_string_fopen(
					   log_filename,
					   _SYSTEM_CHARACTER_T_STRING( "a" ) );

			if( log_file_stream == NULL )
			{
				fprintf(
				 stderr,
				 "Unable to open log file: %s.\n",
				 log_filename );
			}
		}
		if( imaging_handle_acquiry_errors_fprint(
		     imaging_handle,
		     stdout,
		     &error ) != 1 )
		{
			fprintf(
			 stderr,
			 "Unable to print acquiry errors.\n" );

			notify_error_backtrace(
			 error );
			liberror_error_free(
			 &error );
		}
		if( ( log_file_stream != NULL )
		 && ( imaging_handle_acquiry_errors_fprint(
		       imaging_handle,
		       log_file_stream,
		       &error ) != 1 ) )
		{
			fprintf(
			 stderr,
			 "Unable to write acquiry errors in log file.\n" );

			notify_error_backtrace(
			 error );
			liberror_error_free(
			 &error );
		}
	}
	if( imaging_handle_close(
	     imaging_handle,
	     &error ) != 0 )
	{
		fprintf(
		 stderr,
		 "Unable to close output file(s).\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

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
		imaging_handle_free(
		 &imaging_handle,
		 NULL );

		return( EXIT_FAILURE );
	}
	if( imaging_handle_free(
	     &imaging_handle,
	     &error ) != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to free imaging handle.\n" );

		notify_error_backtrace(
		 error );
		liberror_error_free(
		 &error );

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
	if( ewfsignal_detach() != 1 )
	{
		fprintf(
		 stderr,
		 "Unable to detach signal handler.\n" );
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
		fprintf(
		 stdout,
		 "MD5 hash calculated over data:\t%" PRIs_SYSTEM "\n",
		 calculated_md5_hash_string );

		if( log_file_stream != NULL )
		{
			fprintf(
			 log_file_stream,
			 "MD5 hash calculated over data:\t%" PRIs_SYSTEM "\n",
			 calculated_md5_hash_string );
		}
		memory_free(
		 calculated_md5_hash_string );
	}
	if( calculate_sha1 == 1 )
	{
		fprintf(
		 stdout,
		 "SHA1 hash calculated over data:\t%" PRIs_SYSTEM "\n",
		 calculated_sha1_hash_string );

		if( log_file_stream != NULL )
		{
			fprintf(
			 log_file_stream,
			 "SHA1 hash calculated over data:\t%" PRIs_SYSTEM "\n",
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
			fprintf(
			 stderr,
			 "Unable to close log file: %s.\n",
			 log_filename );
		}
	}
	return( EXIT_SUCCESS );
}

