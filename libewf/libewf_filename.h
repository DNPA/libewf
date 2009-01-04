/*
 * Filename functions
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

#if !defined( _LIBEWF_FILENAME_H )
#define _LIBEWF_FILENAME_H

#include <common.h>
#include <file_io.h>
#include <memory.h>
#include <system_string.h>
#include <types.h>

#if defined( __cplusplus )
extern "C" {
#endif

int libewf_filename_set_extension(
     system_character_t *extension,
     uint16_t segment_number,
     uint16_t maximum_amount_of_segments,
     uint8_t segment_file_type,
     uint8_t format,
     uint8_t ewf_format );

int libewf_filename_create(
     system_character_t **filename,
     size_t *length_filename,
     system_character_t *basename,
     size_t length_basename,
     uint16_t segment_number,
     uint16_t maximum_amount_of_segments,
     uint8_t segment_file_type,
     uint8_t format,
     uint8_t ewf_format );

#if defined( HAVE_WIDE_SYSTEM_CHARACTER_T )
#define libewf_filename_open( filename, flags ) \
	file_io_wopen( filename, flags )

#else
#define libewf_filename_open( filename, flags ) \
	file_io_open( filename, flags )

#endif

#if defined( __cplusplus )
}
#endif

#endif

