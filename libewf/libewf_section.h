/*
 * libewf section
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

#if !defined( _LIBEWF_SECTION_H )
#define _LIBEWF_SECTION_H

#include "libewf_includes.h"
#include "libewf_error_sector.h"
#include "libewf_internal_handle.h"
#include "libewf_offset_table.h"
#include "libewf_section_list.h"
#include "libewf_segment_table.h"

#include "ewf_char.h"
#include "ewf_crc.h"
#include "ewf_digest_hash.h"
#include "ewf_section.h"
#include "ewf_table.h"

#if defined( __cplusplus )
extern "C" {
#endif

ssize_t libewf_section_start_read( LIBEWF_SEGMENT_FILE *segment_file, EWF_SECTION *section, uint8_t error_tollerance );
ssize_t libewf_section_start_write( int file_descriptor, EWF_CHAR *section_type, size_t section_type_length, size_t section_data_size, off64_t start_offset );

ssize_t libewf_section_compressed_string_read( LIBEWF_SEGMENT_FILE *segment_file, size_t compressed_string_size, EWF_CHAR **uncompressed_string, size_t *uncompressed_string_size );
ssize_t libewf_section_compressed_string_write( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, off64_t start_offset, EWF_CHAR *section_type, size_t section_type_length, EWF_CHAR *uncompressed_string, size_t size, int8_t compression_level );

ssize_t libewf_section_header_read( LIBEWF_INTERNAL_HANDLE *internal_handle, LIBEWF_SEGMENT_FILE *segment_file, size_t section_size );
ssize_t libewf_section_header_write( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, off64_t start_offset, EWF_CHAR *header, size_t size, int8_t compression_level );

ssize_t libewf_section_header2_read( LIBEWF_INTERNAL_HANDLE *internal_handle, LIBEWF_SEGMENT_FILE *segment_file, size_t section_size );
ssize_t libewf_section_header2_write( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, off64_t start_offset, EWF_CHAR *header2, size_t size, int8_t compression_level );

ssize_t libewf_section_volume_s01_read( LIBEWF_SEGMENT_FILE *segment_file, uint32_t *amount_of_chunks, uint32_t *sectors_per_chunk, uint32_t *bytes_per_sector, uint32_t *amount_of_sectors, uint8_t *format, uint8_t error_tollerance );
ssize_t libewf_section_volume_s01_write( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, off64_t start_offset );
ssize_t libewf_section_volume_e01_read( LIBEWF_SEGMENT_FILE *segment_file, uint32_t *amount_of_chunks, uint32_t *sectors_per_chunk, uint32_t *bytes_per_sector, uint32_t *amount_of_sectors, uint32_t *error_granularity, uint8_t *media_type, uint8_t *media_flags, int8_t *compression_level, uint8_t *guid, uint8_t error_tollerance );
ssize_t libewf_section_volume_e01_write( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, off64_t start_offset );
ssize_t libewf_section_volume_read( LIBEWF_SEGMENT_FILE *segment_file, size_t section_size, uint32_t *amount_of_chunks, uint32_t *sectors_per_chunk, uint32_t *bytes_per_sector, uint32_t *amount_of_sectors, uint32_t *chunk_size, uint32_t *error_granularity, uint8_t *media_type, uint8_t *media_flags, int8_t *compression_level, uint8_t *guid, uint8_t *format, uint8_t *ewf_format, uint8_t error_tollerance );

int libewf_offset_table_read( LIBEWF_SEGMENT_FILE *segment_file, LIBEWF_OFFSET_TABLE *offset_table, uint32_t *amount_of_chunks, off64_t section_offset, size_t section_size, uint8_t ewf_format, uint8_t error_tollerance );

ssize_t libewf_section_table_read( LIBEWF_INTERNAL_HANDLE *internal_handle, LIBEWF_SEGMENT_FILE *segment_file, off64_t section_offset, size_t section_size );
ssize_t libewf_section_table_write( LIBEWF_SEGMENT_FILE *segment_file, off64_t base_offset, LIBEWF_OFFSET_TABLE *offset_table, uint32_t offset_table_index, uint32_t amount_of_offsets, EWF_CHAR *section_type, size_t section_type_length, size_t additional_size, uint8_t format, uint8_t ewf_format, uint8_t no_section_append );

ssize_t libewf_section_table2_read( LIBEWF_INTERNAL_HANDLE *internal_handle, LIBEWF_SEGMENT_FILE *segment_file, off64_t section_offset, size_t section_size );

ssize64_t libewf_section_sectors_read( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, size64_t size );

ssize_t libewf_section_ltree_read( LIBEWF_SEGMENT_FILE *segment_file, size_t section_size, uint8_t *ewf_format, uint8_t error_tollerance );

ssize_t libewf_section_session_read( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, size_t size );

ssize_t libewf_section_data_read( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, size_t size );
ssize_t libewf_section_data_write( LIBEWF_SEGMENT_FILE *segment_file, uint32_t amount_of_chunks, uint32_t sectors_per_chunk, uint32_t bytes_per_sector, uint32_t amount_of_sectors, uint32_t error_granularity, uint8_t media_type, uint8_t media_flags, int8_t compression_level, uint8_t *guid, uint8_t format, EWF_DATA **cached_data_section, uint8_t no_section_append );

ssize_t libewf_section_error2_read( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, size_t size );
ssize_t libewf_section_error2_write( LIBEWF_SEGMENT_FILE *segment_file, LIBEWF_ERROR_SECTOR *sectors, uint32_t amount_of_errors );

ssize_t libewf_section_hash_read( LIBEWF_SEGMENT_FILE *segment_file, EWF_DIGEST_HASH *md5_hash, uint8_t error_tollerance );
ssize_t libewf_section_hash_write( LIBEWF_SEGMENT_FILE *segment_file, EWF_DIGEST_HASH *md5_hash );

ssize_t libewf_section_last_write( LIBEWF_SEGMENT_FILE *segment_file, EWF_CHAR *section_type, size_t section_type_length, uint8_t format, uint8_t ewf_format );

ssize_t libewf_section_xheader_read( LIBEWF_INTERNAL_HANDLE *internal_handle, LIBEWF_SEGMENT_FILE *segment_file, size_t section_size );
ssize_t libewf_section_xheader_write( LIBEWF_INTERNAL_HANDLE *internal_handle, int file_descriptor, off64_t start_offset, EWF_CHAR *xheader, size_t size, int8_t compression_level );

ssize_t libewf_section_xhash_read( LIBEWF_INTERNAL_HANDLE *internal_handle, LIBEWF_SEGMENT_FILE *segment_file, size_t section_size );
#define libewf_section_xhash_write( internal_handle, file_descriptor, start_offset, xhash, size, compression_level ) \
	libewf_section_compressed_string_write( internal_handle, file_descriptor, start_offset, (EWF_CHAR *) "xhash", 5, xhash, size, compression_level )

ssize_t libewf_section_delta_chunk_read( LIBEWF_SEGMENT_FILE *segment_file, off64_t section_offset, size_t section_size, LIBEWF_OFFSET_TABLE *offset_table, uint8_t error_tollerance );
ssize_t libewf_section_delta_chunk_write( int file_descriptor, off64_t start_offset, uint32_t chunk, EWF_CHAR *chunk_data, size_t chunk_size, EWF_CRC *chunk_crc, uint8_t write_crc );

int libewf_section_read( LIBEWF_INTERNAL_HANDLE *internal_handle, LIBEWF_SEGMENT_FILE *segment_file, EWF_SECTION *section, off64_t *section_start_offset );

#if defined( __cplusplus )
}
#endif

#endif

