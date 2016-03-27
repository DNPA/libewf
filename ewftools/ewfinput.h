/*
 * User input functions for the ewftools
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

#if !defined( _EWFINPUT_H )
#define _EWFINPUT_H

#include <common.h>
#include <character_string.h>
#include <system_string.h>
#include <types.h>

#if defined( __cplusplus )
extern "C" {
#endif

#define EWFINPUT_COMPRESSION_LEVELS_AMOUNT		3
#define EWFINPUT_COMPRESSION_LEVELS_DEFAULT		0

#define EWFINPUT_FORMAT_TYPES_AMOUNT			12
#define EWFINPUT_FORMAT_TYPES_DEFAULT			7

#if defined( LIBEWF_CD_SUPPORT )
#define EWFINPUT_MEDIA_TYPES_AMOUNT			3
#else
#define EWFINPUT_MEDIA_TYPES_AMOUNT			2
#endif
#define EWFINPUT_MEDIA_TYPES_DEFAULT			0

#define EWFINPUT_VOLUME_TYPES_AMOUNT			2
#define EWFINPUT_VOLUME_TYPES_DEFAULT			1

#define EWFINPUT_SECTOR_PER_BLOCK_SIZES_AMOUNT		10
#define EWFINPUT_SECTOR_PER_BLOCK_SIZES_DEFAULT		0

extern character_t *ewfinput_compression_levels[ 3 ];
extern character_t *ewfinput_format_types[ 12 ];
#if defined( LIBEWF_CD_SUPPORT )
extern character_t *ewfinput_media_types[ 3 ];
#else
extern character_t *ewfinput_media_types[ 2 ];
#endif
extern character_t *ewfinput_volume_types[ 2 ];
extern character_t *ewfinput_sector_per_block_sizes[ 10 ];
extern character_t *ewfinput_yes_no[ 2 ];

uint8_t ewfinput_determine_libewf_format(
         const character_t *argument );

uint8_t ewfinput_determine_libewf_format_system_character(
         const system_character_t *argument );

uint32_t ewfinput_determine_sectors_per_chunk(
          const character_t *argument );

uint32_t ewfinput_determine_sectors_per_chunk_system_character(
          const system_character_t *argument );

int8_t ewfinput_determine_compression_level(
        const character_t *argument );

int8_t ewfinput_determine_compression_level_system_character(
        const system_character_t *argument );

int8_t ewfinput_determine_media_type(
        const character_t *argument );

int8_t ewfinput_determine_volume_type(
        const character_t *argument );

int8_t ewfinput_determine_yes_no(
        const character_t *argument );

character_t *ewfinput_get_variable(
              FILE *stream,
              character_t *request_string );

system_character_t *ewfinput_get_variable_system_character(
                     FILE *stream,
                     character_t *request_string );

uint64_t ewfinput_get_size_variable(
          FILE *stream,
          character_t *request_string,
          uint64_t minimum,
          uint64_t maximum,
          uint64_t default_value );

uint64_t ewfinput_get_byte_size_variable(
          FILE *stream,
          character_t *request_string,
          uint64_t minimum,
          uint64_t maximum,
          uint64_t default_value );

character_t *ewfinput_get_fixed_value(
              FILE *stream,
              character_t *request_string,
              character_t **values,
              uint8_t amount,
              uint8_t default_value );

#if defined( __cplusplus )
}
#endif

#endif

