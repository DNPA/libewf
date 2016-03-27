/*
 * Section reading/writing functions
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

#if !defined( _LIBEWF_SECTION_H )
#define _LIBEWF_SECTION_H

#include <common.h>
#include <types.h>

#include "libewf_hash_sections.h"
#include "libewf_header_sections.h"
#include "libewf_media_values.h"
#include "libewf_offset_table.h"
#include "libewf_section_list.h"
#include "libewf_sector_table.h"
#include "libewf_segment_file_handle.h"

#include "ewf_crc.h"
#include "ewf_data.h"
#include "ewf_digest_hash.h"
#include "ewf_section.h"
#include "ewf_table.h"

#if defined( __cplusplus )
extern "C" {
#endif

ssize_t libewf_section_start_read(
         libewf_segment_file_handle_t *segment_file_handle,
         ewf_section_t *section,
         uint8_t error_tollerance );

ssize_t libewf_section_start_write(
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *section_type,
         size_t section_type_length,
         size64_t section_data_size );

ssize_t libewf_section_compressed_string_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size_t compressed_string_size,
         uint8_t **uncompressed_string,
         size_t *uncompressed_string_size );

ssize_t libewf_section_write_compressed_string(
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *section_type,
         size_t section_type_length,
         uint8_t *uncompressed_string,
         size_t uncompressed_string_size,
         int8_t compression_level );

ssize_t libewf_section_header_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint8_t **cached_header,
         size_t *cached_header_size );

ssize_t libewf_section_header_write(
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *header,
         size_t header_size,
         int8_t compression_level );

ssize_t libewf_section_header2_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint8_t **cached_header2,
         size_t *cached_header2_size );

ssize_t libewf_section_header2_write(
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *header2,
         size_t header2_size,
         int8_t compression_level );

ssize_t libewf_section_volume_s01_read(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         uint8_t *format,
         uint8_t error_tollerance );

ssize_t libewf_section_volume_s01_write(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         uint8_t format,
         uint8_t no_section_append );

ssize_t libewf_section_volume_e01_read(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         int8_t *compression_level,
         uint8_t error_tollerance );

ssize_t libewf_section_volume_e01_write(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         int8_t compression_level,
         uint8_t format,
         uint8_t no_section_append );

ssize_t libewf_section_volume_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         libewf_media_values_t *media_values,
         int8_t *compression_level,
         uint8_t *format,
         uint8_t *ewf_format,
         uint8_t error_tollerance );

ssize_t libewf_section_table_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint32_t media_amount_of_chunks,
         libewf_offset_table_t *offset_table,
         uint8_t format,
         uint8_t ewf_format,
         uint8_t error_tollerance );

ssize_t libewf_section_table_write(
         libewf_segment_file_handle_t *segment_file_handle,
         off64_t base_offset,
         ewf_table_offset_t *offsets,
         uint32_t amount_of_offsets,
         uint8_t *section_type,
         size_t section_type_length,
         size_t additional_size,
         uint8_t format,
         uint8_t ewf_format,
         uint8_t no_section_append );

ssize64_t libewf_section_sectors_read(
           libewf_segment_file_handle_t *segment_file_handle,
           size64_t section_size,
           uint8_t ewf_format,
           uint8_t error_tollerance );

ssize_t libewf_section_sectors_write(
         libewf_segment_file_handle_t *segment_file_handle,
         size64_t sectors_data_size,
         uint8_t no_section_append );

ssize_t libewf_section_ltree_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint8_t *ewf_format,
         uint8_t error_tollerance );

ssize_t libewf_section_session_read(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         libewf_sector_table_t *sessions,
         size_t section_size,
         uint8_t ewf_format,
         uint8_t error_tollerance );

ssize_t libewf_section_session_write(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_sector_table_t *sessions );

ssize_t libewf_section_data_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         libewf_media_values_t *media_values,
         uint8_t ewf_format,
         uint8_t error_tollerance );

ssize_t libewf_section_data_write(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_media_values_t *media_values,
         int8_t compression_level,
         uint8_t format,
         ewf_data_t **cached_data_section,
         uint8_t no_section_append );

ssize_t libewf_section_error2_read(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_sector_table_t *acquiry_errors,
         size_t section_size,
         uint8_t ewf_format,
         uint8_t error_tollerance );

ssize_t libewf_section_error2_write(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_sector_table_t *acquiry_errors );

ssize_t libewf_section_hash_read(
         libewf_segment_file_handle_t *segment_file_handle,
         ewf_digest_hash_t *md5_hash,
         uint8_t error_tollerance );

ssize_t libewf_section_hash_write(
         libewf_segment_file_handle_t *segment_file_handle,
         ewf_digest_hash_t *md5_hash );

ssize_t libewf_section_last_write(
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *section_type,
         size_t section_type_length,
         uint8_t format,
         uint8_t ewf_format );

ssize_t libewf_section_xheader_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint8_t **cached_xheader,
         size_t *cached_xheader_size );

ssize_t libewf_section_xheader_write(
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *xheader,
         size_t xheader_size,
         int8_t compression_level );

ssize_t libewf_section_xhash_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         uint8_t **cached_xhash,
         size_t *cached_xhash_size );

ssize_t libewf_section_xhash_write(
         libewf_segment_file_handle_t *segment_file_handle,
         uint8_t *xhash,
         size_t xhash_size,
         int8_t compression_level );

ssize_t libewf_section_delta_chunk_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size_t section_size,
         libewf_offset_table_t *offset_table,
         libewf_offset_table_t *secondary_offset_table,
         uint8_t error_tollerance );

ssize_t libewf_section_delta_chunk_write(
         libewf_segment_file_handle_t *segment_file_handle,
         uint32_t chunk,
         uint8_t *chunk_data,
         size_t chunk_size,
         ewf_crc_t *chunk_crc,
         uint8_t write_crc,
         uint8_t no_section_append );

#if defined( HAVE_DEBUG_OUTPUT )
ssize_t libewf_section_debug_read(
         libewf_segment_file_handle_t *segment_file_handle,
         size64_t section_size );
#endif

int libewf_section_read(
     libewf_segment_file_handle_t *segment_file_handle,
     libewf_header_sections_t *header_sections,
     libewf_hash_sections_t *hash_sections,
     libewf_media_values_t *media_values,
     libewf_offset_table_t *offset_table,
     libewf_offset_table_t *secondary_offset_table,
     libewf_sector_table_t *sessions,
     libewf_sector_table_t *acquiry_errors,
     int8_t *compression_level,
     uint8_t *format,
     uint8_t *ewf_format,
     size64_t *segment_file_size,
     ewf_section_t *section,
     off64_t *section_start_offset,
     uint8_t error_tollerance );

#if defined( __cplusplus )
}
#endif

#endif

