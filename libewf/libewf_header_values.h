/*
 * Header values functions
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

#if !defined( _LIBEWF_HEADER_VALUES_H )
#define _LIBEWF_HEADER_VALUES_H

#include <common.h>
#include <character_string.h>
#include <types.h>

#include "libewf_values_table.h"

#include <time.h>

#include "ewf_char.h"

#if defined( __cplusplus )
extern "C" {
#endif

#define LIBEWF_HEADER_VALUES_DEFAULT_AMOUNT			16

#define LIBEWF_HEADER_VALUES_INDEX_CASE_NUMBER			0
#define LIBEWF_HEADER_VALUES_INDEX_DESCRIPTION			1
#define LIBEWF_HEADER_VALUES_INDEX_EXAMINER_NAME		2
#define LIBEWF_HEADER_VALUES_INDEX_EVIDENCE_NUMBER		3
#define LIBEWF_HEADER_VALUES_INDEX_NOTES			4
#define LIBEWF_HEADER_VALUES_INDEX_ACQUIRY_DATE			5
#define LIBEWF_HEADER_VALUES_INDEX_SYSTEM_DATE			6
#define LIBEWF_HEADER_VALUES_INDEX_ACQUIRY_OPERATING_SYSTEM	7
#define LIBEWF_HEADER_VALUES_INDEX_ACQUIRY_SOFTWARE_VERSION	8
#define LIBEWF_HEADER_VALUES_INDEX_PASSWORD			9
#define LIBEWF_HEADER_VALUES_INDEX_COMPRESSION_TYPE		10
#define LIBEWF_HEADER_VALUES_INDEX_MODEL			11
#define LIBEWF_HEADER_VALUES_INDEX_SERIAL_NUMBER		12

#define LIBEWF_HEADER_VALUES_INDEX_UNKNOWN_PID			13
#define LIBEWF_HEADER_VALUES_INDEX_UNKNOWN_DC			14
#define LIBEWF_HEADER_VALUES_INDEX_UNKNOWN_EXT			15

int libewf_header_values_initialize(
     libewf_values_table_t *header_values );

int libewf_header_values_realloc(
     libewf_values_table_t *header_values,
     uint32_t previous_amount,
     uint32_t new_amount );

void libewf_header_values_free(
      libewf_values_table_t *header_values );

int libewf_convert_timestamp(
     time_t timestamp,
     uint8_t date_format,
     character_t *date_string,
     size_t date_string_size );

int libewf_convert_date_header_value(
     character_t *header_value,
     size_t header_value_length,
     uint8_t date_format,
     character_t **date_string,
     size_t *date_string_length );

int libewf_generate_date_header_value(
     time_t timestamp,
     character_t **date_string,
     size_t *date_string_length );

int libewf_convert_date_header2_value(
     character_t *header_value,
     size_t header_value_length,
     uint8_t date_format,
     character_t **date_string,
     size_t *date_string_length );

int libewf_generate_date_header2_value(
     time_t timestamp,
     character_t **date_string,
     size_t *date_string_length );

int libewf_header_values_copy(
     libewf_values_table_t *destination_header_values,
     libewf_values_table_t *source_header_values );

int libewf_header_values_parse_header_string(
     libewf_values_table_t **header_values,
     character_t *header_string,
     size_t length,
     uint8_t date_format );

int libewf_header_values_parse_header(
     libewf_values_table_t **header_values,
     ewf_char_t *header,
     size_t size,
     uint8_t date_format );

int libewf_header_values_parse_header2(
     libewf_values_table_t **header_values,
     ewf_char_t *header2,
     size_t header2_size,
     uint8_t date_format );

int libewf_header_values_convert_header_string_to_header(
     character_t *header_string,
     size_t header_string_length,
     ewf_char_t **header,
     size_t *header_length );

int libewf_header_values_convert_header_string_to_header2(
     character_t *header_string,
     size_t header_string_size,
     ewf_char_t **header2,
     size_t *header2_size );

int libewf_header_values_generate_header_string_type1(
     libewf_values_table_t *header_values,
     time_t timestamp,
     int8_t compression_level,
     character_t *header_string_head,
     character_t *header_string_tail,
     character_t **header_string,
     size_t *header_string_length );

int libewf_header_values_generate_header_string_type2(
     libewf_values_table_t *header_values,
     time_t timestamp, 
     int8_t compression_level,
     character_t *header_string_head,
     character_t *header_string_tail,
     character_t **header_string,
     size_t *header_string_length );

int libewf_header_values_generate_header_string_type3(
     libewf_values_table_t *header_values,
     time_t timestamp,
     character_t *header_string_head,
     character_t *header_string_tail,
     character_t **header_string,
     size_t *header_string_length );

int libewf_header_values_generate_header_string_type4(
     libewf_values_table_t *header_values,
     time_t timestamp,
     character_t *header_string_head,
     character_t *header_string_tail,
     character_t **header_string,
     size_t *header_string_length );

int libewf_header_values_generate_header_string_type5(
     libewf_values_table_t *header_values,
     time_t timestamp,
     character_t *header_string_head,
     character_t *header_string_tail,
     character_t **header_string,
     size_t *header_string_length );

int libewf_header_values_generate_header_string_type6(
     libewf_values_table_t *header_values,
     time_t timestamp,
     character_t *header_string_head,
     character_t *header_string_tail,
     character_t **header_string,
     size_t *header_string_length );

int libewf_header_values_generate_header_string_type7(
     libewf_values_table_t *header_values,
     time_t timestamp,
     character_t *header_string_head,
     character_t *header_string_tail,
     character_t **header_string,
     size_t *header_string_length );

int libewf_header_values_generate_header_ewf(
     libewf_values_table_t *header_values,
     time_t timestamp,
     int8_t compression_level,
     ewf_char_t **header,
     size_t *header_length );

int libewf_header_values_generate_header_encase1(
     libewf_values_table_t *header_values,
     time_t timestamp,
     int8_t compression_level,
     ewf_char_t **header,
     size_t *header_length );

int libewf_header_values_generate_header_ftk(
     libewf_values_table_t *header_values,
     time_t timestamp,
     int8_t compression_level,
     ewf_char_t **header,
     size_t *header_length );

int libewf_header_values_generate_header_encase2(
     libewf_values_table_t *header_values,
     time_t timestamp,
     int8_t compression_level,
     ewf_char_t **header,
     size_t *header_length );

int libewf_header_values_generate_header_encase4(
     libewf_values_table_t *header_values,
     time_t timestamp,
     ewf_char_t **header,
     size_t *header_length );

int libewf_header_values_generate_header_linen5(
     libewf_values_table_t *header_values,
     time_t timestamp,
     ewf_char_t **header,
     size_t *header_length );

int libewf_header_values_generate_header_linen6(
     libewf_values_table_t *header_values,
     time_t timestamp,
     ewf_char_t **header,
     size_t *header_length );

int libewf_header_values_generate_header2_encase4(
     libewf_values_table_t *header_values,
     time_t timestamp,
     ewf_char_t **header2,
     size_t *header2_length );

int libewf_header_values_generate_header2_encase5(
     libewf_values_table_t *header_values,
     time_t timestamp,
     ewf_char_t **header2,
     size_t *header2_length );

int libewf_header_values_generate_header2_encase6(
     libewf_values_table_t *header_values,
     time_t timestamp,
     ewf_char_t **header2,
     size_t *header2_length );

int libewf_convert_date_xheader_value(
     character_t *header_value,
     size_t header_value_length,
     uint8_t date_format,
     character_t **date_string,
     size_t *date_string_length );

int libewf_generate_date_xheader_value(
     time_t timestamp,
     character_t **date_string,
     size_t *date_string_length );

int libewf_header_values_parse_header_string_xml(
     libewf_values_table_t **header_values,
     character_t *header_string_xml,
     size_t length,
     uint8_t date_format );

int libewf_header_values_parse_xheader(
     libewf_values_table_t **header_values,
     ewf_char_t *xheader,
     size_t size,
     uint8_t date_format );

int libewf_header_values_generate_header_string_xml(
     libewf_values_table_t *header_values,
     time_t timestamp,
     character_t **header_string,
     size_t *header_string_length );

int libewf_header_values_generate_header_ewfx(
     libewf_values_table_t *header_values,
     time_t timestamp,
     ewf_char_t **header,
     size_t *header_length );

int libewf_header_values_generate_header2_ewfx(
     libewf_values_table_t *header_values,
     time_t timestamp,
     ewf_char_t **header2,
     size_t *header2_length );

int libewf_header_values_generate_xheader_ewfx(
     libewf_values_table_t *header_values,
     time_t timestamp,
     ewf_char_t **xheader,
     size_t *xheader_length );

#if defined( __cplusplus )
}
#endif

#endif

