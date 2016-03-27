/*
 * User input functions for the ewftools
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

#if !defined( _EWFINPUT_H )
#define _EWFINPUT_H

#include <common.h>
#include <types.h>

#include <liberror.h>

#include "system_string.h"

#if defined( __cplusplus )
extern "C" {
#endif

#define EWFINPUT_COMPRESSION_LEVELS_AMOUNT		4
#define EWFINPUT_COMPRESSION_LEVELS_DEFAULT		0

#define EWFINPUT_FORMAT_TYPES_AMOUNT			12
#define EWFINPUT_FORMAT_TYPES_DEFAULT			8

#define EWFINPUT_MEDIA_TYPES_AMOUNT			4
#define EWFINPUT_MEDIA_TYPES_DEFAULT			0

#define EWFINPUT_MEDIA_FLAGS_AMOUNT			2
#define EWFINPUT_MEDIA_FLAGS_DEFAULT			1

#define EWFINPUT_SECTOR_PER_BLOCK_SIZES_AMOUNT		10
#define EWFINPUT_SECTOR_PER_BLOCK_SIZES_DEFAULT		0

extern system_character_t *ewfinput_compression_levels[ 4 ];
extern system_character_t *ewfinput_format_types[ 12 ];
extern system_character_t *ewfinput_media_types[ 4 ];
extern system_character_t *ewfinput_media_flags[ 2 ];
extern system_character_t *ewfinput_sector_per_block_sizes[ 10 ];
extern system_character_t *ewfinput_yes_no[ 2 ];

int ewfinput_determine_ewf_format(
     const system_character_t *argument,
     uint8_t *ewf_format );

int ewfinput_determine_sectors_per_chunk(
     const system_character_t *argument,
     uint32_t *sectors_per_chunk );

int ewfinput_determine_compression_level(
     const system_character_t *argument,
     int8_t *compression_level,
     uint8_t *compression_flags );

int ewfinput_determine_media_type(
     const system_character_t *argument,
     uint8_t *media_type );

int ewfinput_determine_media_flags(
     const system_character_t *argument,
     uint8_t *media_flags );

int ewfinput_determine_header_codepage(
     const system_character_t *argument,
     int *header_codepage );

int ewfinput_determine_yes_no(
     const system_character_t *argument,
     uint8_t *yes_no_value );

int ewfinput_get_string_variable(
     FILE *stream,
     system_character_t *request_string,
     system_character_t *string_variable,
     size_t string_variable_size );

int ewfinput_get_size_variable(
     FILE *stream,
     system_character_t *input_buffer,
     size_t input_buffer_size,
     system_character_t *request_string,
     uint64_t minimum,
     uint64_t maximum,
     uint64_t default_value,
     uint64_t *size_variable );

int ewfinput_get_byte_size_variable(
     FILE *stream,
     system_character_t *input_buffer,
     size_t input_buffer_size,
     system_character_t *request_string,
     uint64_t minimum,
     uint64_t maximum,
     uint64_t default_value,
     uint64_t *byte_size_variable,
     liberror_error_t **error );

int ewfinput_get_fixed_string_variable(
     FILE *stream,
     system_character_t *input_buffer,
     size_t input_buffer_size,
     system_character_t *request_string,
     system_character_t **values,
     uint8_t amount_of_values,
     uint8_t default_value,
     system_character_t **fixed_string_variable );

#if defined( __cplusplus )
}
#endif

#endif

