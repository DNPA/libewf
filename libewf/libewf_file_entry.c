/*
 * File entry functions
 *
 * Copyright (c) 2010, Joachim Metz <jbmetz@users.sourceforge.net>
 * Copyright (c) 2008-2010, Joachim Metz <forensics@hoffmannbv.nl>,
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

#include <common.h>
#include <byte_stream.h>
#include <memory.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>
#include <libnotify.h>

#include "libewf_extern.h"
#include "libewf_file_entry.h"
#include "libewf_handle.h"
#include "libewf_single_file_entry.h"
#include "libewf_tree_type.h"
#include "libewf_types.h"

/* Initializes the file entry and its values
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_initialize(
     libewf_file_entry_t **file_entry,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	static char *function                             = "libewf_file_entry_initialize";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	if( *file_entry == NULL )
	{
		internal_file_entry = (libewf_internal_file_entry_t *) memory_allocate(
		                                                        sizeof( libewf_internal_file_entry_t ) );

		if( internal_file_entry == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create internal file entry.",
			 function );

			return( -1 );
		}
		if( memory_set(
		     internal_file_entry,
		     0,
		     sizeof( libewf_internal_file_entry_t ) ) == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear internal file entry.",
			 function );

			memory_free(
			 internal_file_entry );

			return( -1 );
		}
		*file_entry = (libewf_file_entry_t *) internal_file_entry;
	}
	return( 1 );
}

/* Frees an file entry
 * Detaches the reference from the file if necessary
 * Return 1 if successful or -1 on error
 */
int libewf_file_entry_free(
     libewf_file_entry_t **file_entry,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	static char *function                             = "libewf_file_entry_free";
	int result                                        = 1;

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	if( *file_entry != NULL )
	{
		internal_file_entry = (libewf_internal_file_entry_t *) *file_entry;
		*file_entry         = NULL;

		/* The internal_handle reference is freed elsewhere
		 */
		/* If not managed the file_entry_tree_node reference is freed elsewhere
		 */
		if( libewf_file_entry_detach(
		     internal_file_entry,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_REMOVE_FAILED,
			 "%s: unable to detach internal file entry.",
			 function );

			result = -1;
		}
		memory_free(
		 internal_file_entry );
	}
	return( result );
}

/* Attaches the file entry to the handle
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_attach(
     libewf_internal_file_entry_t *internal_file_entry,
     libewf_internal_handle_t *internal_handle,
     libewf_tree_node_t *file_entry_tree_node,
     uint8_t flags,
     liberror_error_t **error )
{
	static char *function = "libewf_file_entry_attach";

	if( internal_file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid internal file entry.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->internal_handle != NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid internal file entry - internal handle already set.",
		 function );

		return( -1 );
	}
	if( ( flags & ~( LIBEWF_INTERNAL_FILE_ENTRY_FLAG_MANAGED_FILE_ENTRY_TREE_NODE ) ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported flags: 0x%02" PRIx8 ".",
		 function,
		 flags );

		return( -1 );
	}
	internal_file_entry->internal_handle = internal_handle;
	internal_file_entry->flags           = flags;

	if( ( flags & LIBEWF_INTERNAL_FILE_ENTRY_FLAG_MANAGED_FILE_ENTRY_TREE_NODE ) == 0 )
	{
		internal_file_entry->file_entry_tree_node = file_entry_tree_node;
	}
	else
	{
		if( libewf_tree_node_clone(
		     &( internal_file_entry->file_entry_tree_node ),
		     file_entry_tree_node,
		     &libewf_single_file_entry_clone,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
			 "%s: unable to copy file entry tree node.",
			 function );

			/* libewf_tree_node_clone can return a partial tree node
			 * but is freed by the libewf_file_entry_free function
			 * because the managed flag has been set
			 */

			return( -1 );
		}
	}
	return( 1 );
}

/* Detaches the file entry from the handle
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_detach(
     libewf_internal_file_entry_t *internal_file_entry,
     liberror_error_t **error )
{
	static char *function = "libewf_file_entry_detach";

	if( internal_file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid internal file entry.",
		 function );

		return( -1 );
	}
	if( ( ( internal_file_entry->flags & LIBEWF_INTERNAL_FILE_ENTRY_FLAG_MANAGED_FILE_ENTRY_TREE_NODE ) == LIBEWF_INTERNAL_FILE_ENTRY_FLAG_MANAGED_FILE_ENTRY_TREE_NODE )
	 && ( internal_file_entry->file_entry_tree_node != NULL ) )
	{
		if( libewf_tree_node_free(
		     &( internal_file_entry->file_entry_tree_node ),
		     &libewf_single_file_entry_free,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free file entry tree node.",
			 function );

			return( -1 );
		}
	}
	internal_file_entry->internal_handle      = NULL;
	internal_file_entry->file_entry_tree_node = NULL;
	internal_file_entry->flags                = 0;

	return( 1 );
}

/* Retrieves the flags from the referenced file entry
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_get_flags(
     libewf_file_entry_t *file_entry,
     uint32_t *flags,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_single_file_entry_t *single_file_entry     = NULL;
	static char *function                             = "libewf_file_entry_get_flags";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->file_entry_tree_node->value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - invalid file entry tree node - missing value.",
		 function );

		return( -1 );
	}
	single_file_entry = (libewf_single_file_entry_t *) internal_file_entry->file_entry_tree_node->value;

	if( flags == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid flags.",
		 function );

		return( -1 );
	}
	*flags = single_file_entry->flags;

	return( 1 );
}

/* Retrieves the size of the UTF-8 formatted name from the referenced file entry
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_get_name_size(
     libewf_file_entry_t *file_entry,
     size_t *name_size,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_single_file_entry_t *single_file_entry     = NULL;
	static char *function                             = "libewf_file_entry_get_name_size";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->file_entry_tree_node->value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - invalid file entry tree node - missing value.",
		 function );

		return( -1 );
	}
	single_file_entry = (libewf_single_file_entry_t *) internal_file_entry->file_entry_tree_node->value;

	if( name_size == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid name size.",
		 function );

		return( -1 );
	}
	*name_size = single_file_entry->name_size;

	return( 1 );
}

/* Retrieves the UTF-8 formatted name value from the referenced file entry
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_get_name(
     libewf_file_entry_t *file_entry,
     uint8_t *name,
     size_t name_size,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_single_file_entry_t *single_file_entry     = NULL;
	static char *function                             = "libewf_file_entry_get_name_size";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->file_entry_tree_node->value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - invalid file entry tree node - missing value.",
		 function );

		return( -1 );
	}
	single_file_entry = (libewf_single_file_entry_t *) internal_file_entry->file_entry_tree_node->value;

	if( name == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid name.",
		 function );

		return( -1 );
	}
	if( name_size < single_file_entry->name_size )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: name too small.",
		 function );

		return( -1 );
	}
	if( libcstring_narrow_string_copy(
	     (char *) name,
	     (char *) single_file_entry->name,
	     single_file_entry->name_size ) == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_MEMORY,
		 LIBERROR_MEMORY_ERROR_COPY_FAILED,
		 "%s: unable to set name.",
		 function );

		return( -1 );
	}
	name[ single_file_entry->name_size - 1 ] = 0;

	return( 1 );
}

/* Retrieves the size from the referenced file entry
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_get_size(
     libewf_file_entry_t *file_entry,
     size64_t *size,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_single_file_entry_t *single_file_entry     = NULL;
	static char *function                             = "libewf_file_entry_get_size";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->file_entry_tree_node->value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - invalid file entry tree node - missing value.",
		 function );

		return( -1 );
	}
	single_file_entry = (libewf_single_file_entry_t *) internal_file_entry->file_entry_tree_node->value;

	if( size == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid size.",
		 function );

		return( -1 );
	}
	*size = single_file_entry->size;

	return( 1 );
}

/* Retrieves the creation date and time from the referenced file entry
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_get_creation_time(
     libewf_file_entry_t *file_entry,
     uint64_t *creation_time,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_single_file_entry_t *single_file_entry     = NULL;
	static char *function                             = "libewf_file_entry_get_creation_time";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->file_entry_tree_node->value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - invalid file entry tree node - missing value.",
		 function );

		return( -1 );
	}
	single_file_entry = (libewf_single_file_entry_t *) internal_file_entry->file_entry_tree_node->value;

	if( creation_time == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid creation time.",
		 function );

		return( -1 );
	}
	*creation_time = single_file_entry->creation_time;

	return( 1 );
}

/* Retrieves the (file) modification (last written) date and time from the referenced file entry
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_get_modification_time(
     libewf_file_entry_t *file_entry,
     uint64_t *modification_time,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_single_file_entry_t *single_file_entry     = NULL;
	static char *function                             = "libewf_file_entry_get_modification_time";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->file_entry_tree_node->value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - invalid file entry tree node - missing value.",
		 function );

		return( -1 );
	}
	single_file_entry = (libewf_single_file_entry_t *) internal_file_entry->file_entry_tree_node->value;

	if( modification_time == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid modification time.",
		 function );

		return( -1 );
	}
	*modification_time = single_file_entry->modification_time;

	return( 1 );
}

/* Retrieves the access date and time from the referenced file entry
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_get_access_time(
     libewf_file_entry_t *file_entry,
     uint64_t *access_time,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_single_file_entry_t *single_file_entry     = NULL;
	static char *function                             = "libewf_file_entry_get_access_time";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->file_entry_tree_node->value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - invalid file entry tree node - missing value.",
		 function );

		return( -1 );
	}
	single_file_entry = (libewf_single_file_entry_t *) internal_file_entry->file_entry_tree_node->value;

	if( access_time == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid access time.",
		 function );

		return( -1 );
	}
	*access_time = single_file_entry->access_time;

	return( 1 );
}

/* Retrieves the (file system entry) modification date and time from the referenced file entry
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_get_entry_modification_time(
     libewf_file_entry_t *file_entry,
     uint64_t *entry_modification_time,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_single_file_entry_t *single_file_entry     = NULL;
	static char *function                             = "libewf_file_entry_get_entry_modification_time";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->file_entry_tree_node->value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - invalid file entry tree node - missing value.",
		 function );

		return( -1 );
	}
	single_file_entry = (libewf_single_file_entry_t *) internal_file_entry->file_entry_tree_node->value;

	if( entry_modification_time == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid entry modification time.",
		 function );

		return( -1 );
	}
	*entry_modification_time = single_file_entry->entry_modification_time;

	return( 1 );
}

/* Retrieves the amount of sub file entries from a file entry
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_get_amount_of_sub_file_entries(
     libewf_file_entry_t *file_entry,
     int *amount_of_sub_file_entries,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	static char *function                             = "libewf_file_entry_get_amount_of_sub_file_entries";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( libewf_tree_node_get_amount_of_child_nodes(
	     internal_file_entry->file_entry_tree_node,
	     amount_of_sub_file_entries,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve amount of sub file entries.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Retrieves the sub file entry for the specific index from a file entry
 * Returns 1 if successful or -1 on error
 */
int libewf_file_entry_get_sub_file_entry(
     libewf_file_entry_t *file_entry,
     int sub_file_entry_index,
     libewf_file_entry_t **sub_file_entry,
     liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_tree_node_t *sub_file_entry_tree_node      = NULL;
	static char *function                             = "libewf_file_entry_get_sub_file_entry";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( sub_file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid sub file entry.",
		 function );

		return( -1 );
	}
	if( *sub_file_entry != NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: sub file entry already set.",
		 function );

		return( -1 );
	}
	if( libewf_tree_node_get_child_node(
	     internal_file_entry->file_entry_tree_node,
             sub_file_entry_index,
             &sub_file_entry_tree_node,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve sub file entry tree node.",
		 function );

		return( -1 );
	}
	if( sub_file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid sub file entry tree node.",
		 function );

		return( -1 );
	}
	if( libewf_file_entry_initialize(
	     sub_file_entry,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to initialize sub file entry.",
		 function );

		return( -1 );
	}
	/* TODO pass LIBEWF_FILE_ENTRY_FLAG_MANAGED_FILE_IO_HANDLE */
	if( libewf_file_entry_attach(
	     (libewf_internal_file_entry_t *) *sub_file_entry,
	     internal_file_entry->internal_handle,
	     sub_file_entry_tree_node,
	     0,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
		 "%s: unable to attach sub file entry.",
		 function );

		libewf_file_entry_free(
		 sub_file_entry,
		 NULL );

		return( -1 );
	}
	return( 1 );
}

/* Reads data from the curent offset into a buffer
 * Returns the amount of bytes read or -1 on error
 */
ssize_t libewf_file_entry_read_buffer(
         libewf_file_entry_t *file_entry,
         void *buffer,
         size_t buffer_size,
         liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_single_file_entry_t *single_file_entry     = NULL;
	static char *function                             = "libewf_file_entry_read_buffer";
	ssize_t read_count                                = 0;

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->file_entry_tree_node->value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - invalid file entry tree node - missing value.",
		 function );

		return( -1 );
	}
	single_file_entry = (libewf_single_file_entry_t *) internal_file_entry->file_entry_tree_node->value;

	if( buffer == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid buffer.",
		 function );

		return( -1 );
	}
	if( buffer_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid buffer size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( (off64_t) ( internal_file_entry->offset + buffer_size ) > (off64_t) single_file_entry->data_size )
	{
		buffer_size = (size_t) ( buffer_size - internal_file_entry->offset );
	}
	read_count = libewf_handle_read_buffer(
	              (libewf_handle_t *) internal_file_entry->internal_handle,
	              buffer,
	              buffer_size,
	              error );

	if( read_count <= -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read buffer.",
		 function );

		return( -1 );
	}
	return( read_count );
}

/* Reads data from an offset into a buffer
 * Returns the amount of bytes read or -1 on error
 */
ssize_t libewf_file_entry_read_random(
         libewf_file_entry_t *file_entry,
         void *buffer,
         size_t buffer_size,
         off64_t offset,
         liberror_error_t **error )
{
	static char *function = "libewf_file_entry_read_random";
	ssize_t read_count    = 0;

	if( libewf_file_entry_seek_offset(
	     file_entry,
	     offset,
	     SEEK_SET,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek offset.",
		 function );

		return( -1 );
	}
	read_count = libewf_file_entry_read_buffer(
	              file_entry,
	              buffer,
	              buffer_size,
	              error );

	if( read_count <= -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read buffer.",
		 function );

		return( -1 );
	}
	return( read_count );
}

/* Seeks a certain offset of the data
 * Returns the offset if seek is successful or -1 on error
 */
off64_t libewf_file_entry_seek_offset(
         libewf_file_entry_t *file_entry,
         off64_t offset,
         int whence,
         liberror_error_t **error )
{
	libewf_internal_file_entry_t *internal_file_entry = NULL;
	libewf_single_file_entry_t *single_file_entry     = NULL;
	static char *function                             = "libewf_file_entry_seek_offset";

	if( file_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file entry.",
		 function );

		return( -1 );
	}
	internal_file_entry = (libewf_internal_file_entry_t *) file_entry;

	if( internal_file_entry->file_entry_tree_node == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - missing file entry tree node.",
		 function );

		return( -1 );
	}
	if( internal_file_entry->file_entry_tree_node->value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid file entry - invalid file entry tree node - missing value.",
		 function );

		return( -1 );
	}
	single_file_entry = (libewf_single_file_entry_t *) internal_file_entry->file_entry_tree_node->value;

	if( ( whence != SEEK_CUR )
	 && ( whence != SEEK_END )
	 && ( whence != SEEK_SET ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported whence.",
		 function );

		return( -1 );
	}
	if( whence == SEEK_CUR )
	{	
		offset += internal_file_entry->offset;
	}
	else if( whence == SEEK_END )
	{	
		offset += (off64_t) single_file_entry->data_size;
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		libnotify_printf(
		 "%s: seeking offset: %" PRIu64 ".\n",
		 function,
		 offset );
	}
#endif

	if( ( offset < 0 )
	 || ( offset > (off64_t) single_file_entry->data_size ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
		 "%s: offset out of range.",
		 function );

		return( -1 );
	}
	internal_file_entry->offset = offset;

	if( libewf_handle_seek_offset(
	     (libewf_handle_t *) internal_file_entry->internal_handle,
	     single_file_entry->data_offset + offset,
	     SEEK_SET,
	     error ) == -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek offset.",
		 function );

		return( -1 );
	}
	return( offset );
}

