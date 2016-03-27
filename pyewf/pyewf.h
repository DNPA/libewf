/*
 * Python bindings for libewf (pyewf)
 *
 * Copyright (c) 2008, David Collett <david.collett@gmail.com>
 * Copyright (c) 2009-2010, Joachim Metz <forensics@hoffmannbv.nl>
 *
 * Integration into libewf package by Joachim Metz <forensics@hoffmannbv.nl>,
 * Hoffmann Investigations.
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

#if !defined( _PYEWF_H )
#define _PYEWF_H

#include <common.h>
#include <types.h>

/* Fix HAVE_FSTAT define in pyport.h
 */
#undef HAVE_FSTAT

#include <Python.h>

#if defined( __cplusplus )
extern "C" {
#endif

PyObject* pyewf_get_version(
           PyObject *self );

PyObject* pyewf_get_flags_read(
           PyObject *self );

PyObject* pyewf_get_flags_read_write(
           PyObject *self );

PyObject* pyewf_get_flags_write(
           PyObject *self );

PyObject* pyewf_get_flags_write_resume(
           PyObject *self );

PyObject* pyewf_check_file_signature(
           PyObject *self,
           PyObject *arguments,
           PyObject *keywords );

PyObject* pyewf_glob(
           PyObject *self,
           PyObject *arguments,
           PyObject *keywords );

PyObject* pyewf_set_notify_values(
           PyObject *self,
           PyObject *arguments,
           PyObject *keywords );

PyObject* pyewf_new_handle(
           PyObject *self );

PyMODINIT_FUNC initpyewf(
                void );

#if defined( __cplusplus )
}
#endif

#endif

