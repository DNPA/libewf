/*
 * libewf file reading
 *
 * Copyright (c) 2006, Joachim Metz <forensics@hoffmannbv.nl>,
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
 * - All advertising materials mentioning features or use of this software
 *   must acknowledge the contribution by people stated in the acknowledgements.
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

#ifndef _LIBEWF_FILE_READ_H
#define _LIBEWF_FILE_READ_H

#include "ewf_section.h"
#include "ewf_table.h"
#include "libewf_handle.h"
#include "libewf_offset_table.h"
#include "libewf_section_list.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t libewf_section_header_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size );
int32_t libewf_section_header2_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size );
int32_t libewf_section_volume_s01_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size );
int32_t libewf_section_volume_e01_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size );
int32_t libewf_section_volume_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size );
LIBEWF_OFFSET_TABLE *libewf_fill_offset_table( LIBEWF_OFFSET_TABLE *offset_table, EWF_TABLE_OFFSET *offsets, uint32_t chunk_amount, int file_descriptor, uint8_t error_tollerance );
LIBEWF_OFFSET_TABLE *libewf_calculate_last_offset( LIBEWF_OFFSET_TABLE *offset_table, LIBEWF_SECTION_LIST *section_list, int file_descriptor, uint8_t error_tollerance );
LIBEWF_OFFSET_TABLE *libewf_offset_table_read( LIBEWF_OFFSET_TABLE *offset_table, LIBEWF_SECTION_LIST *section_list, int file_descriptor, uint32_t size, uint8_t ewf_format, uint8_t error_tollerance );
uint8_t libewf_compare_offset_tables( LIBEWF_OFFSET_TABLE *offset_table1, LIBEWF_OFFSET_TABLE *offset_table2 );
int32_t libewf_section_table_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size, LIBEWF_SECTION_LIST *section_list );
int32_t libewf_section_table2_s01_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size, LIBEWF_SECTION_LIST *section_list );
int32_t libewf_section_table2_e01_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size, LIBEWF_SECTION_LIST *section_list );
int32_t libewf_section_table2_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size, LIBEWF_SECTION_LIST *section_list );
int32_t libewf_section_sectors_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size );
int32_t libewf_section_ltree_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size );
int32_t libewf_section_data_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size );
int32_t libewf_section_error2_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size );
int32_t libewf_section_hash_read( LIBEWF_HANDLE *handle, int file_descriptor, uint32_t size );
void libewf_section_data_read_segment( LIBEWF_HANDLE *handle, uint32_t segment, EWF_SECTION *section, int file_descriptor, LIBEWF_SECTION_LIST *section_list );
EWF_SECTION *libewf_sections_read_segment( LIBEWF_HANDLE *handle, uint32_t segment );

int64_t libewf_read_chunk( LIBEWF_HANDLE *handle, uint32_t chunk, void *buffer, uint64_t buffer_size );
int64_t libewf_read_random( LIBEWF_HANDLE *handle, void *buffer, uint64_t size, uint64_t offset );

int64_t libewf_read_to_file_descriptor( LIBEWF_HANDLE *handle, int output_file_descriptor, void (*callback)( uint64_t bytes_read, uint64_t bytes_total ) );

#ifdef __cplusplus
}
#endif

#endif

