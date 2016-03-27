/* 
 * Info handle
 *
 * Copyright (c) 2006-2010, Joachim Metz <jbmetz@users.sourceforge.net>
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

#if !defined( _INFO_HANDLE_H )
#define _INFO_HANDLE_H

#include <common.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#if defined( __cplusplus )
extern "C" {
#endif

typedef struct info_handle info_handle_t;

struct info_handle
{
	/* The libewf input handle
	 */
	libewf_handle_t *input_handle;
};

int info_handle_initialize(
     info_handle_t **info_handle,
     liberror_error_t **error );

int info_handle_free(
     info_handle_t **info_handle,
     liberror_error_t **error );

int info_handle_signal_abort(
     info_handle_t *info_handle,
     liberror_error_t **error );

int info_handle_open_input(
     info_handle_t *info_handle,
     libcstring_system_character_t * const * filenames,
     int number_of_filenames,
     liberror_error_t **error );

int info_handle_close(
     info_handle_t *info_handle,
     liberror_error_t **error );

int info_handle_get_header_value(
     info_handle_t *info_handle,
     char *header_value_identifier,
     size_t header_value_identifier_length,
     libcstring_system_character_t *header_value,
     size_t header_value_size,
     liberror_error_t **error );

int info_handle_set_header_codepage(
     info_handle_t *info_handle,
     int header_codepage,
     liberror_error_t **error );

int info_handle_header_values_fprint(
     info_handle_t *info_handle,
     uint8_t date_format,
     FILE *stream,
     liberror_error_t **error );

int info_handle_header_value_extents_fprint(
     libcstring_system_character_t *header_value,
     size_t header_value_length,
     FILE *stream,
     liberror_error_t **error );

int info_handle_media_information_fprint(
     info_handle_t *info_handle,
     FILE *stream,
     liberror_error_t **error );

int info_handle_hash_values_fprint(
     info_handle_t *info_handle,
     FILE *stream,
     liberror_error_t **error );

int info_handle_acquiry_errors_fprint(
     info_handle_t *info_handle,
     FILE *stream,
     liberror_error_t **error );

int info_handle_sessions_fprint(
     info_handle_t *info_handle,
     FILE *stream,
     liberror_error_t **error );

int info_handle_single_files_fprint(
     info_handle_t *info_handle,
     FILE *stream,
     liberror_error_t **error );

int info_handle_file_entry_fprint(
     info_handle_t *info_handle,
     libewf_file_entry_t *file_entry,
     FILE *stream,
     int level,
     liberror_error_t **error );

#if defined( __cplusplus )
}
#endif

#endif

