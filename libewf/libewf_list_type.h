/*
 * List type functions
 *
 * Copyright (C) 2008, Joachim Metz <forensics@hoffmannbv.nl>,
 * Hoffmann Investigations. All rights reserved.
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined( _LIBEWF_LIST_TYPE_H )
#define _LIBEWF_LIST_TYPE_H

#include <common.h>
#include <types.h>

#include <liberror.h>

#if defined( __cplusplus )
extern "C" {
#endif

typedef struct libewf_list_element libewf_list_element_t;

struct libewf_list_element
{
	/* The previous element
	 */
	libewf_list_element_t *previous;

	/* The next element
	 */
	libewf_list_element_t *next;

	/* The list element value
	 */
	intptr_t *value;
};

typedef struct libewf_list libewf_list_t;

struct libewf_list
{
	/* The amount of elements
	 */
	int amount_of_elements;

	/* The first list element
	 */
	libewf_list_element_t *first;

	/* The last list element
	 */
	libewf_list_element_t *last;
};

int libewf_list_free(
     libewf_list_t *list,
     int (*value_free_function)( intptr_t *value ),
     liberror_error_t **error );

int libewf_list_empty(
     libewf_list_t *list,
     int (*value_free_function)( intptr_t *value ),
     liberror_error_t **error );

int libewf_list_prepend_element(
     libewf_list_t *list,
     libewf_list_element_t *element,
     liberror_error_t **error );

int libewf_list_prepend_value(
     libewf_list_t *list,
     intptr_t *value,
     liberror_error_t **error );

int libewf_list_append_element(
     libewf_list_t *list,
     libewf_list_element_t *element,
     liberror_error_t **error );

int libewf_list_append_value(
     libewf_list_t *list,
     intptr_t *value,
     liberror_error_t **error );

int libewf_list_insert_element(
     libewf_list_t *list,
     libewf_list_element_t *element,
     int (*value_compare_function)( intptr_t *first, intptr_t *second ),
     liberror_error_t **error );

int libewf_list_insert_value(
     libewf_list_t *list,
     intptr_t *value,
     int (*value_compare_function)( intptr_t *first, intptr_t *second ),
     liberror_error_t **error );

int libewf_list_remove_element(
     libewf_list_t *list,
     libewf_list_element_t *element,
     liberror_error_t **error );

int libewf_list_get_amount_of_elements(
     libewf_list_t *list,
     liberror_error_t **error );

int libewf_list_get_element(
     libewf_list_t *list,
     int element_index,
     libewf_list_element_t **element,
     liberror_error_t **error );

#if defined( __cplusplus )
}
#endif

#endif

