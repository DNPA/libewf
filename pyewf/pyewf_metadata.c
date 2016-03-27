/*
 *  Metadata functions for the Python object definition of the libewf handle
 *
 * Copyright (c) 2008, David Collett <david.collett@gmail.com>
 * Copyright (c) 2009-2010, Joachim Metz <jbmetz@users.sourceforge.net>
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

#include <common.h>
#include <memory.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>

#if defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif

/* Fix HAVE_FSTAT define in pyport.h
 */
#undef HAVE_FSTAT

#include <Python.h>

#include <libewf.h>

#include "pyewf_handle.h"
#include "pyewf_metadata.h"

/* Retrieves the size of the media data
 * Returns a Python object holding the offset if successful or NULL on error
 */
PyObject* pyewf_handle_get_media_size(
           pyewf_handle_t *pyewf_handle )
{
	liberror_error_t *error = NULL;
	static char *function   = "pyewf_handle_get_media_size";
	size64_t media_size     = 0;

	if( libewf_handle_get_media_size(
	     pyewf_handle->handle,
	     &media_size,
	     &error ) != 1 )
	{
		/* TODO something with error */

		PyErr_Format(
		 PyExc_IOError,
		 "%s: failed to retrieve media size.",
		 function );

		liberror_error_free(
		 &error );

		return( NULL );
	}
	return( PyLong_FromLongLong(
	         media_size ) );
}

/* Retrieves a header value
 * Returns a Python object holding the offset if successful or NULL on error
 */
PyObject *pyewf_handle_get_header_value(
           pyewf_handle_t *pyewf_handle,
           PyObject *arguments,
           PyObject *keywords )
{
	liberror_error_t *error               = NULL;
	PyObject *string_object               = NULL;
	static char *function                 = "pyewf_handle_get_header_value";
	static char *keyword_list[]           = { "identifier", NULL };
	const char *errors                    = NULL;
	char *header_value_identifier         = NULL;
	char *header_value                    = NULL;
	size_t header_value_identifier_length = 0;
	size_t header_value_size              = 0;
	int result                            = 0;

	if( PyArg_ParseTupleAndKeywords(
	     arguments,
	     keywords,
	     "s",
	     keyword_list,
	     &header_value_identifier ) == 0 )
	{
		return( NULL );
	}
	header_value_identifier_length = libcstring_narrow_string_length(
	                                  header_value_identifier );

	result = libewf_handle_get_header_value_size(
	          pyewf_handle->handle,
	          (uint8_t *) header_value_identifier,
	          header_value_identifier_length,
	          &header_value_size,
	          &error );

	if( result == -1 )
	{
		/* TODO something with error */

		PyErr_Format(
		 PyExc_IOError,
	         "%s: unable to retrieve header value size: %s.",
		 function,
		 header_value_identifier );

		liberror_error_free(
		 &error );

		return( NULL );
	}
	/* Check if header value is present
	 */
	else if( result == 0 )
	{
		return( Py_None );
	}
	header_value = (char *) memory_allocate(
	                         sizeof( char ) * header_value_size );

	if( header_value == NULL )
	{
		PyErr_Format(
		 PyExc_MemoryError,
		 "%s: unable to create header value.",
		 function );

		return( NULL );
	}
	result = libewf_handle_get_header_value(
	          pyewf_handle->handle,
	          (uint8_t *) header_value_identifier,
	          header_value_identifier_length,
	          (uint8_t *) header_value,
	          header_value_size,
	          NULL );

	if( result == -1 )
	{
		/* TODO something with error */

		PyErr_Format(
		 PyExc_IOError,
	         "%s: unable to retrieve header value: %s.",
		 function,
		 header_value_identifier );

		liberror_error_free(
		 &error );
		memory_free(
		 header_value );

		return( NULL );
	}
	/* Check if the header value is present
	 */
	else if( result == 0 )
	{
		memory_free(
		 header_value );

		return( Py_None );
	}
	string_object = PyUnicode_DecodeUTF8(
	                 header_value,
	                 header_value_size,
	                 errors );

	memory_free(
	 header_value );

	return( string_object );
}

/* Retrieves the header values
 * Returns a Python object holding the offset if successful or NULL on error
 */
PyObject *pyewf_handle_get_header_values(
           pyewf_handle_t *pyewf_handle )
{
	liberror_error_t *error               = NULL;
	PyObject *dictionary_object           = NULL;
	PyObject *string_object               = NULL;
	static char *function                 = "pyewf_handle_get_header_values";
	const char *errors                    = NULL;
	char *header_value                    = NULL;
	char *header_value_identifier         = NULL;
	size_t header_value_identifier_length = 0;
	size_t header_value_identifier_size   = 0;
	size_t header_value_size              = 0;
	uint32_t number_of_header_values      = 0;
	uint32_t header_value_iterator        = 0;

	if( libewf_handle_get_number_of_header_values(
	     pyewf_handle->handle,
	     &number_of_header_values,
	     &error ) != 1 )
	{
		/* TODO something with error */

		PyErr_Format(
		 PyExc_IOError,
		 "%s: failed to retrieve number of header values.",
		 function );

		liberror_error_free(
		 &error );

		return( NULL );
	}
	dictionary_object = PyDict_New();

	for( header_value_iterator = 0;
	     header_value_iterator < number_of_header_values;
	     header_value_iterator++ )
	{
		if( libewf_handle_get_header_value_identifier_size(
		     pyewf_handle->handle,
		     header_value_iterator,
		     &header_value_identifier_size,
		     &error ) != 1 )
		{
			/* TODO something with error */
	
			PyErr_Format(
			 PyExc_IOError,
			 "%s: unable to retrieve header value identifier size: %d.",
			 function,
			 header_value_iterator + 1 );

			liberror_error_free(
			 &error );

			return( NULL );
		}
		header_value_identifier = (char *) memory_allocate(
		                                    sizeof( char ) * header_value_identifier_size );

		if( header_value_identifier == NULL )
		{
			PyErr_Format(
			 PyExc_MemoryError,
			 "%s: unable to create header value identifier.",
			 function );

			return( NULL );
		}
		if( libewf_handle_get_header_value_identifier(
		     pyewf_handle->handle,
		     header_value_iterator,
		     (uint8_t *) header_value_identifier,
		     header_value_identifier_size,
		     &error ) != 1 )
		{
			/* TODO something with error */

			PyErr_Format(
			 PyExc_IOError,
			 "%s: unable to retrieve header value identifier: %d.",
			 function,
			 header_value_iterator + 1 );

			liberror_error_free(
			 &error );
			memory_free(
			 header_value_identifier );

			return( NULL );
		}
		header_value_identifier_length = libcstring_narrow_string_length(
						  header_value_identifier );

		if( libewf_handle_get_header_value_size(
		     pyewf_handle->handle,
		     (uint8_t *) header_value_identifier,
		     header_value_identifier_length,
		     &header_value_size,
		     &error ) != 1 )
		{
			/* TODO something with error */

			PyErr_Format(
			 PyExc_IOError,
			 "%s: unable to retrieve header value size: %s.",
			 function,
			 header_value_identifier + 1 );

			liberror_error_free(
			 &error );
			memory_free(
			 header_value_identifier );

			return( NULL );
		}
		header_value = (char *) memory_allocate(
		                         sizeof( char ) * header_value_size );

		if( header_value == NULL )
		{
			memory_free(
			 header_value_identifier );

			PyErr_Format(
			 PyExc_MemoryError,
			 "%s: unable to create header value.",
			 function );

			return( NULL );
		}
		/* Ignore emtpy header values
		 */
		if( libewf_handle_get_header_value(
		     pyewf_handle->handle,
		     (uint8_t *) header_value_identifier,
		     header_value_identifier_length,
		     (uint8_t *) header_value,
		     header_value_size,
		     NULL ) == 1 )
		{
			string_object = PyUnicode_DecodeUTF8(
			                 header_value,
			                 header_value_size,
			                 errors );

			PyDict_SetItemString(
			 dictionary_object,
			 header_value_identifier,
			 string_object );

			Py_DECREF(
			 string_object );
		}
		memory_free(
		 header_value_identifier );
		memory_free(
		 header_value );
	}
	return( dictionary_object );
}

