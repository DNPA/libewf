/* 
 * Imaging handle
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

#if !defined( _IMAGING_HANDLE_H )
#define _IMAGING_HANDLE_H

#include <common.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>

#include <stdio.h>

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include "digest_context.h"
#include "digest_hash.h"
#include "md5.h"
#include "sha1.h"
#include "storage_media_buffer.h"

#if defined( __cplusplus )
extern "C" {
#endif

typedef struct imaging_handle imaging_handle_t;

struct imaging_handle
{
	/* The user input buffer
	 */
	libcstring_system_character_t *input_buffer; 

	/* The target filename
	 */
	libcstring_system_character_t *target_filename;

	/* The target filename size
	 */
	size_t target_filename_size;

	/* The secondary target filename
	 */
	libcstring_system_character_t *secondary_target_filename;

	/* The secondary target filename size
	 */
	size_t secondary_target_filename_size;

	/* The header codepage
	 */
	int header_codepage;

	/* The case number
	 */
	libcstring_system_character_t *case_number;

	/* The case number size
	 */
	size_t case_number_size;

	/* The description
	 */
	libcstring_system_character_t *description;

	/* The description size
	 */
	size_t description_size;

	/* The evidence number
	 */
	libcstring_system_character_t *evidence_number;

	/* The evidence number size
	 */
	size_t evidence_number_size;

	/* The examiner name
	 */
	libcstring_system_character_t *examiner_name;

	/* The examiner name size
	 */
	size_t examiner_name_size;

	/* The notes
	 */
	libcstring_system_character_t *notes;

	/* The notes size
	 */
	size_t notes_size;

	/* The compression level
	 */
	int8_t compression_level;

	/* The compression flags
	 */
	uint8_t compression_flags;

	/* The EWF format
	 */
	uint8_t ewf_format;

	/* The media type
	 */
	uint8_t media_type;

	/* The media flags
	 */
	uint8_t media_flags;

	/* The number of bytes per sector
	 */
	uint32_t bytes_per_sector;

	/* The number of sectors per chunk
	 */
	uint32_t sectors_per_chunk;

	/* The error granularity number of sectors
	 */
	uint32_t sector_error_granularity;

	/* The maximum segment size
	 */
	size64_t maximum_segment_size;

	/* The acquiry offset
	 */
	uint64_t acquiry_offset;

	/* The acquiry size
	 */
	uint64_t acquiry_size;

	/* Value to indicate if the MD5 digest hash should be calculated
	 */
	uint8_t calculate_md5;

	/* The MD5 digest context
	 */
	md5_context_t md5_context;

	/* Value to indicate the MD5 digest context was initialized
	 */
	uint8_t md5_context_initialized;

	/* The calculated MD5 digest hash string
	 */
	libcstring_system_character_t *calculated_md5_hash_string;

	/* Value to indicate if the SHA1 digest hash should be calculated
	 */
	uint8_t calculate_sha1;

	/* The SHA1 digest context
	 */
	sha1_context_t sha1_context;

	/* Value to indicate the SHA-1 digest context was initialized
	 */
	uint8_t sha1_context_initialized;

	/* The calculated SHA-1 digest hash string
	 */
	libcstring_system_character_t *calculated_sha1_hash_string;

	/* The libewf output handle
	 */
	libewf_handle_t *output_handle;

	/* The secondary libewf output handle
	 */
	libewf_handle_t *secondary_output_handle;

	/* The process buffer size
	 */
	size_t process_buffer_size;

	/* The nofication output stream
	 */
	FILE *notify_stream;

	/* Value to indicate if abort was signalled
	 */
	int abort;
};

int imaging_handle_initialize(
     imaging_handle_t **imaging_handle,
     uint8_t calculate_md5,
     uint8_t calculate_sha1,
     liberror_error_t **error );

int imaging_handle_free(
     imaging_handle_t **imaging_handle,
     liberror_error_t **error );

int imaging_handle_signal_abort(
     imaging_handle_t *imaging_handle,
     liberror_error_t **error );

int imaging_handle_open_output(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *filename,
     uint8_t resume,
     liberror_error_t **error );

int imaging_handle_open_secondary_output(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *filename,
     uint8_t resume,
     liberror_error_t **error );

int imaging_handle_close(
     imaging_handle_t *imaging_handle,
     liberror_error_t **error );

ssize_t imaging_handle_prepare_read_buffer(
         imaging_handle_t *imaging_handle,
         storage_media_buffer_t *storage_media_buffer,
         liberror_error_t **error );

ssize_t imaging_handle_read_buffer(
         imaging_handle_t *imaging_handle,
         storage_media_buffer_t *storage_media_buffer,
         size_t read_size,
         liberror_error_t **error );

ssize_t imaging_handle_prepare_write_buffer(
         imaging_handle_t *imaging_handle,
         storage_media_buffer_t *storage_media_buffer,
         liberror_error_t **error );

ssize_t imaging_handle_write_buffer(
         imaging_handle_t *imaging_handle,
         storage_media_buffer_t *storage_media_buffer,
         size_t write_size,
         liberror_error_t **error );

off64_t imaging_handle_seek_offset(
         imaging_handle_t *imaging_handle,
         off64_t offset,
         liberror_error_t **error );

int imaging_handle_get_offset(
     imaging_handle_t *imaging_handle,
     off64_t *offset,
     liberror_error_t **error );

int imaging_handle_swap_byte_pairs(
     imaging_handle_t *imaging_handle,
     storage_media_buffer_t *storage_media_buffer,
     size_t read_size,
     liberror_error_t **error );

int imaging_handle_initialize_integrity_hash(
     imaging_handle_t *imaging_handle,
     liberror_error_t **error );

int imaging_handle_update_integrity_hash(
     imaging_handle_t *imaging_handle,
     uint8_t *buffer,
     size_t buffer_size,
     liberror_error_t **error );

int imaging_handle_finalize_integrity_hash(
     imaging_handle_t *imaging_handle,
     liberror_error_t **error );

int imaging_handle_finalize_integrity_hash_on_error(
     imaging_handle_t *imaging_handle,
     liberror_error_t **error );

int imaging_handle_get_chunk_size(
     imaging_handle_t *imaging_handle,
     size32_t *chunk_size,
     liberror_error_t **error );

int imaging_handle_prompt_for_string(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *request_string,
     libcstring_system_character_t **internal_string,
     size_t *internal_string_size,
     liberror_error_t **error );

int imaging_handle_prompt_for_compression_level(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *request_string,
     liberror_error_t **error );

int imaging_handle_prompt_for_format(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *request_string,
     liberror_error_t **error );

int imaging_handle_prompt_for_media_type(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *request_string,
     liberror_error_t **error );

int imaging_handle_prompt_for_media_flags(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *request_string,
     liberror_error_t **error );

int imaging_handle_prompt_for_bytes_per_sector(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *request_string,
     liberror_error_t **error );

int imaging_handle_prompt_for_sectors_per_chunk(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *request_string,
     liberror_error_t **error );

int imaging_handle_prompt_for_sector_error_granularity(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *request_string,
     liberror_error_t **error );

int imaging_handle_prompt_for_maximum_segment_size(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *request_string,
     liberror_error_t **error );

int imaging_handle_get_output_values(
     imaging_handle_t *imaging_handle,
     liberror_error_t **error );

int imaging_handle_set_string(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     libcstring_system_character_t **internal_string,
     size_t *internal_string_size,
     liberror_error_t **error );

int imaging_handle_set_compression_values(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error );

int imaging_handle_set_format(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error );

int imaging_handle_set_media_type(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error );

int imaging_handle_set_media_flags(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error );

int imaging_handle_set_bytes_per_sector(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error );

int imaging_handle_set_sectors_per_chunk(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error );

int imaging_handle_set_sector_error_granularity(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error );

int imaging_handle_set_maximum_segment_size(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error );

int imaging_handle_set_header_codepage(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error );

int imaging_handle_set_process_buffer_size(
     imaging_handle_t *imaging_handle,
     const libcstring_system_character_t *string,
     liberror_error_t **error );

int imaging_handle_set_output_values(
     imaging_handle_t *imaging_handle,
     libcstring_system_character_t *acquiry_software,
     libcstring_system_character_t *acquiry_software_version,
     libcstring_system_character_t *model,
     libcstring_system_character_t *serial_number,
     liberror_error_t **error );

int imaging_handle_get_header_value(
     imaging_handle_t *imaging_handle,
     const uint8_t *identifier,
     size_t identifier_size,
     libcstring_system_character_t **header_value,
     size_t *header_value_size,
     liberror_error_t **error );

int imaging_handle_set_header_value(
     imaging_handle_t *imaging_handle,
     const uint8_t *identifier,
     size_t identifier_length,
     const libcstring_system_character_t *header_value,
     liberror_error_t **error );

int imaging_handle_set_hash_value(
     imaging_handle_t *imaging_handle,
     char *hash_value_identifier,
     size_t hash_value_identifier_length,
     libcstring_system_character_t *hash_value,
     size_t hash_value_length,
     liberror_error_t **error );

int imaging_handle_append_read_error(
     imaging_handle_t *imaging_handle,
     off64_t start_offset,
     size64_t number_of_bytes,
     liberror_error_t **error );

int imaging_handle_append_session(
     imaging_handle_t *imaging_handle,
     uint64_t start_sector,
     uint64_t number_of_sectors,
     liberror_error_t **error );

ssize_t imaging_handle_finalize(
         imaging_handle_t *imaging_handle,
         liberror_error_t **error );

int imaging_handle_print_parameters(
     imaging_handle_t *imaging_handle,
     off64_t resume_acquiry_offset,
     uint8_t read_error_retries,
     uint8_t zero_block_on_read_error,
     uint8_t resume_acquiry,
     liberror_error_t **error );

#if defined( __cplusplus )
}
#endif

#endif

