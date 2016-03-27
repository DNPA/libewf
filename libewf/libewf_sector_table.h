/*
 * Error sector table definition for CRC and acquiry read errrors
 *
 * Copyright (c) 2006-2008, Joachim Metz <forensics@hoffmannbv.nl>,
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

#if !defined( _LIBEWF_SECTOR_TABLE_H )
#define _LIBEWF_SECTOR_TABLE_H

#include "libewf_includes.h"

#if defined( __cplusplus )
extern "C" {
#endif

typedef struct libewf_sector_table_entry libewf_sector_table_entry_t;

struct libewf_sector_table_entry
{
	/* The first sector
	 */
	off64_t first_sector;

	/* The amount of sectors
	 */
	uint32_t amount_of_sectors;
};

typedef struct libewf_sector_table libewf_sector_table_t;

struct libewf_sector_table
{
	/* The amount of sectors in the table
	 */
	uint32_t amount;

	/* A dynamic array containting references to the sectors
	 */
	libewf_sector_table_entry_t *sector;
};

libewf_sector_table_t *libewf_sector_table_alloc(
                        uint32_t amount );

int libewf_sector_table_realloc(
     libewf_sector_table_t *sector_table,
     uint32_t amount );

void libewf_sector_table_free(
      libewf_sector_table_t *sector_table );

int libewf_sector_table_get_sector(
     libewf_sector_table_t *sector_table,
     uint32_t index,
     off64_t *first_sector,
     uint32_t *amount_of_sectors );

int libewf_sector_table_add_sector(
     libewf_sector_table_t *sector_table,
     off64_t first_sector,
     uint32_t amount_of_sectors,
     int merge_continious_entries );

#if defined( __cplusplus )
}
#endif

#endif

