/*
 * File handling functions
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

#if !defined( _LIBEWF_FILE_H )
#define _LIBEWF_FILE_H

#include <common.h>
#include <character_string.h>
#include <system_string.h>

/* If libtool DLL support is enabled set LIBEWF_DLL_EXPORT
 * before including libewf/extern.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_EXPORT
#endif

#include <libewf/extern.h>
#include <libewf/handle.h>

#if defined( __cplusplus )
extern "C" {
#endif

LIBEWF_EXTERN int libewf_check_file_signature(
                   const system_character_t *filename );

LIBEWF_EXTERN int libewf_glob(
                   const system_character_t *filename,
                   size_t length,
                   uint8_t format,
                   system_character_t **filenames[] );

LIBEWF_EXTERN libewf_handle_t *libewf_open(
                              system_character_t * const filenames[],
                              uint16_t amount_of_files,
                              uint8_t flags );

LIBEWF_EXTERN int libewf_close(
                   libewf_handle_t *handle );

LIBEWF_EXTERN off64_t libewf_seek_offset(
                       libewf_handle_t *handle,
                        off64_t offset );

LIBEWF_EXTERN off64_t libewf_get_offset(
                       libewf_handle_t *handle );

#if defined( __cplusplus )
}
#endif

#endif

