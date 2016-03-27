/*
 * Hash values functions
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

#if !defined( _LIBEWF_HASH_VALUES_H )
#define _LIBEWF_HASH_VALUES_H

#include <common.h>
#include <character_string.h>
#include <types.h>

#include "libewf_values_table.h"

#if defined( __cplusplus )
extern "C" {
#endif

#define LIBEWF_HASH_VALUES_DEFAULT_AMOUNT	1

int libewf_hash_values_initialize(
     libewf_values_table_t *hash_values );

int libewf_hash_values_parse_hash_string_xml(
     libewf_values_table_t **hash_values,
     character_t *hash_string_xml,
     size_t length );

int libewf_hash_values_parse_xhash( 
     libewf_values_table_t **hash_values,
     uint8_t *xhash, 
     size_t xhash_size );

int libewf_hash_values_convert_hash_string_to_hash(
     character_t *hash_string,
     size_t hash_string_length,
     uint8_t **hash,
     size_t *hash_length );

int libewf_hash_values_generate_hash_string_xml(
     libewf_values_table_t *hash_values,
     character_t **hash_string,
     size_t *hash_string_length );

int libewf_hash_values_generate_xhash_string_ewfx(
     libewf_values_table_t *hash_values,
     uint8_t **hash,
     size_t *hash_length );

#if defined( __cplusplus )
}
#endif

#endif

