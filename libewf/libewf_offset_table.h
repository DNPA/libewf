/*
 * Offset table functions
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

#if !defined( _LIBEWF_OFFSET_TABLE_H )
#define _LIBEWF_OFFSET_TABLE_H

#include <common.h>
#include <types.h>

#include "libewf_chunk_offset.h"
#include "libewf_section_list.h"
#include "libewf_segment_file_handle.h"

#include "ewf_table.h"

#if defined( __cplusplus )
extern "C" {
#endif

typedef struct libewf_offset_table libewf_offset_table_t;

struct libewf_offset_table
{
	/* Stores the amount of chunks in the table
	 * There is an offset per chunk in the table
	 */
	uint32_t amount;

	/* The last chunk that was defined
	 */
	uint32_t last;

	/* Dynamic array of chunk offsets
	 */
	libewf_chunk_offset_t *chunk_offset;
};

libewf_offset_table_t *libewf_offset_table_alloc(
                        uint32_t amount );

int libewf_offset_table_realloc(
     libewf_offset_table_t *offset_table,
     uint32_t amount );

void libewf_offset_table_free(
      libewf_offset_table_t *offset_table );

int libewf_offset_table_fill(
     libewf_offset_table_t *offset_table,
     off64_t base_offset,
     ewf_table_offset_t *offsets,
     uint32_t amount_of_chunks,
     libewf_segment_file_handle_t *segment_file_handle,
     uint8_t error_tollerance );

int libewf_offset_table_fill_offsets(
     libewf_offset_table_t *offset_table,
     uint32_t offset_table_index,
     uint32_t amount_of_chunk_offsets,
     off64_t base_offset,
     ewf_table_offset_t *offsets,
     uint32_t amount_of_offsets );

int libewf_offset_table_calculate_last_offset(
     libewf_offset_table_t *offset_table,
     libewf_section_list_t *section_list,
     uint8_t error_tollerance );

int libewf_offset_table_compare(
     libewf_offset_table_t *offset_table1,
     libewf_offset_table_t *offset_table2 );

off64_t libewf_offset_table_seek_chunk_offset(
         libewf_offset_table_t *offset_table,
         uint32_t chunk );

#if defined( __cplusplus )
}
#endif

#endif

