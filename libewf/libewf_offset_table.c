/*
 * Offset table functions
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

#include <common.h>
#include <endian.h>
#include <memory.h>
#include <notify.h>
#include <types.h>

#include <libewf/definitions.h>

#include "libewf_offset_table.h"

#include "ewf_definitions.h"

/* Initialize the offset table
 * Returns 1 if successful or -1 on error
 */
int libewf_offset_table_initialize(
     libewf_offset_table_t **offset_table,
     uint32_t amount_of_chunk_offsets )
{
	static char *function    = "libewf_offset_table_initialize";
	size_t chunk_offset_size = 0;

	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	if( *offset_table == NULL )
	{
		chunk_offset_size = sizeof( libewf_chunk_offset_t ) * amount_of_chunk_offsets;

		if( chunk_offset_size > (size_t) SSIZE_MAX )
		{
			notify_warning_printf( "%s: invalid chunk offset size value exceeds maximum.\n",
			 function );

			return( -1 );
		}
		*offset_table = (libewf_offset_table_t *) memory_allocate(
		                                           sizeof( libewf_offset_table_t ) );

		if( *offset_table == NULL )
		{
			notify_warning_printf( "%s: unable to create offset table.\n",
			 function );

			return( -1 );
		}
		if( memory_set(
		     *offset_table,
		     0,
		     sizeof( libewf_offset_table_t ) ) == NULL )
		{
			notify_warning_printf( "%s: unable to clear offset table.\n",
			 function );

			memory_free(
			 *offset_table );

			*offset_table = NULL;

			return( -1 );
		}
		if( amount_of_chunk_offsets > 0 )
		{
			( *offset_table )->chunk_offset = (libewf_chunk_offset_t *) memory_allocate(
			                                                             chunk_offset_size );

			if( ( *offset_table )->chunk_offset == NULL )
			{
				notify_warning_printf( "%s: unable to create chunk offsets.\n",
				 function );

				memory_free(
				 *offset_table );

				*offset_table = NULL;

				return( -1 );
			}
			if( memory_set(
			     ( *offset_table )->chunk_offset,
			     0,
			     chunk_offset_size ) == NULL )
			{
				notify_warning_printf( "%s: unable to clear chunk offsets.\n",
				 function );

				memory_free(
				 ( *offset_table )->chunk_offset );
				memory_free(
				 *offset_table );

				*offset_table = NULL;

				return( -1 );
			}
		}
		( *offset_table )->amount_of_chunk_offsets = amount_of_chunk_offsets;
	}
	return( 1 );
}

/* Frees the offset table including elements
 * Returns 1 if successful or -1 on error
 */
int libewf_offset_table_free(
     libewf_offset_table_t **offset_table )
{
	static char *function = "libewf_offset_table_free";

	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	if( *offset_table != NULL )
	{
		/* The segment file references are freed in the segment table
		 */
		if( ( *offset_table )->chunk_offset != NULL )
		{
			memory_free(
			 ( *offset_table )->chunk_offset );
		}
		memory_free(
		 *offset_table );

		*offset_table = NULL;
	}
	return( 1 );
}

/* Resizes the offset table
 * Returns 1 if successful or -1 on error
 */
int libewf_offset_table_resize(
     libewf_offset_table_t *offset_table,
     uint32_t amount_of_chunk_offsets )
{
	void *reallocation       = NULL;
	static char *function    = "libewf_offset_table_resize";
	size_t chunk_offset_size = 0;

	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset_table.\n",
		 function );

		return( -1 );
	}
	if( offset_table->amount_of_chunk_offsets < amount_of_chunk_offsets )
	{
		chunk_offset_size = sizeof( libewf_chunk_offset_t ) * amount_of_chunk_offsets;

		if( chunk_offset_size > (size_t) SSIZE_MAX )
		{
			notify_warning_printf( "%s: invalid chunk offset size value exceeds maximum.\n",
			 function );

			return( -1 );
		}
		reallocation = memory_reallocate(
				offset_table->chunk_offset,
				chunk_offset_size );

		if( reallocation == NULL )
		{
			notify_warning_printf( "%s: unable to resize chunk offsets.\n",
			 function );

			return( -1 );
		}
		offset_table->chunk_offset = (libewf_chunk_offset_t *) reallocation;

		if( memory_set(
		     &( offset_table->chunk_offset[ offset_table->amount_of_chunk_offsets ] ),
		     0,
		     ( sizeof( libewf_chunk_offset_t ) * ( amount_of_chunk_offsets - offset_table->amount_of_chunk_offsets ) ) ) == NULL )
		{
			notify_warning_printf( "%s: unable to clear chunk offsets.\n",
			 function );

			return( -1 );
		}
		offset_table->amount_of_chunk_offsets = amount_of_chunk_offsets;
	}
	return( 1 );
}

/* Fills the offset table from the table offsets
 * Returns 1 if successful or -1 on error
 */
int libewf_offset_table_fill(
     libewf_offset_table_t *offset_table,
     off64_t base_offset,
     ewf_table_offset_t *offsets,
     uint32_t amount_of_chunks,
     libewf_segment_file_handle_t *segment_file_handle,
     uint8_t error_tollerance )
{
#if defined( HAVE_VERBOSE_OUTPUT )
	char *chunk_type        = NULL;
#endif
	static char *function   = "libewf_offset_table_fill";
	uint32_t chunk_size     = 0;
	uint32_t current_offset = 0;
	uint32_t next_offset    = 0;
	uint32_t raw_offset     = 0;
	uint32_t iterator       = 0;
	uint8_t compressed      = 0;
	uint8_t overflow        = 0;

	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	if( base_offset < 0 )
	{
		notify_warning_printf( "%s: invalid base offset.\n",
		 function );

		return( -1 );
	}
	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file.\n",
		 function );

		return( -1 );
	}
	/* Correct the last offset, to fill the table it should point to the first empty entry
	 * the the last filled entry
	 */
	if( offset_table->last_chunk_offset_filled > 0 )
	{
		offset_table->last_chunk_offset_filled++;
	}
	/* Allocate additional entries in the offset table if needed
	 * - a single reallocation saves processing time
	 */
	if( offset_table->amount_of_chunk_offsets < ( offset_table->last_chunk_offset_filled + amount_of_chunks ) )
	{
		if( libewf_offset_table_resize(
		     offset_table,
		     ( offset_table->last_chunk_offset_filled + amount_of_chunks ) ) != 1 )
		{
			notify_warning_printf( "%s: unable to resize offset table.\n",
			 function );

			return( -1 );
		}
	}
	endian_little_convert_32bit(
	 raw_offset,
	 offsets[ iterator ].offset );

	/* The size of the last chunk must be determined differently
	 */
	while( iterator < ( amount_of_chunks - 1 ) )
	{
		if( overflow == 0 )
		{
			compressed     = (uint8_t) ( raw_offset >> 31 );
			current_offset = raw_offset & EWF_OFFSET_COMPRESSED_READ_MASK;
		}
		else
		{
			current_offset = raw_offset;
		}
		endian_little_convert_32bit(
		 raw_offset,
		 offsets[ iterator + 1 ].offset );

		if( overflow == 0 )
		{
			next_offset = raw_offset & EWF_OFFSET_COMPRESSED_READ_MASK;
		}
		else
		{
			next_offset = raw_offset;
		}

		/* This is to compensate for the crappy >2Gb segment file
		 * solution in EnCase 6.7
		 */
		if( next_offset < current_offset )
		{
			if( raw_offset < current_offset )
			{
				notify_warning_printf( "%s: chunk offset %" PRIu32 " larger than raw %" PRIu32 ".\n",
				 function, current_offset, raw_offset );

				return( -1 );
			}
#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf( "%s: chunk offset %" PRIu32 " larger than next %" PRIu32 ".\n",
			 function, current_offset, next_offset );
#endif

			chunk_size = raw_offset - current_offset;
		}
		else
		{
			chunk_size = next_offset - current_offset;
		}
		if( chunk_size == 0 )
		{
			notify_warning_printf( "%s: invalid chunk size - size is zero.\n",
			 function );

			if( error_tollerance < LIBEWF_ERROR_TOLLERANCE_COMPENSATE )
			{
				return( -1 );
			}
		}
		if( chunk_size > (uint32_t) INT32_MAX )
		{
			notify_warning_printf( "%s: invalid chunk size value exceeds maximum.\n",
			 function );

			return( -1 );
		}
#if defined( HAVE_VERBOSE_OUTPUT )
		if( compressed == 0 )
		{
			chunk_type = "uncompressed";
		}
		else
		{
			chunk_type = "compressed";
		}
		notify_verbose_printf( "%s: %s chunk %" PRIu32 " read with: base %" PRIu64 ", offset %" PRIu32 " and size %" PRIu32 ".\n",
		 function, chunk_type, ( offset_table->last_chunk_offset_filled + 1 ), base_offset, current_offset, chunk_size );
#endif

		offset_table->chunk_offset[ offset_table->last_chunk_offset_filled ].segment_file_handle = segment_file_handle;
		offset_table->chunk_offset[ offset_table->last_chunk_offset_filled ].file_offset         = (off64_t) ( base_offset + current_offset );
		offset_table->chunk_offset[ offset_table->last_chunk_offset_filled ].size                = (size_t) chunk_size;
		offset_table->chunk_offset[ offset_table->last_chunk_offset_filled ].compressed          = compressed;

		offset_table->last_chunk_offset_filled++;

		/* This is to compensate for the crappy >2Gb segment file
		 * solution in EnCase 6.7
		 */
		if( ( overflow == 0 )
		 && ( ( current_offset + chunk_size ) > (uint32_t) INT32_MAX ) )
		{
#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf( "%s: chunk offset overflow at: %" PRIu32 ".\n",
			 function, current_offset );
#endif

			overflow   = 1;
			compressed = 0;
		}
		iterator++;
	}
	endian_little_convert_32bit(
	 raw_offset,
	 offsets[ iterator ].offset );

	if( overflow == 0 )
	{
		compressed     = (uint8_t) ( raw_offset >> 31 );
		current_offset = raw_offset & EWF_OFFSET_COMPRESSED_READ_MASK;
	}
	else
	{
		current_offset = raw_offset;
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	if( compressed == 0 )
	{
		chunk_type = "uncompressed";
	}
	else
	{
		chunk_type = "compressed";
	}
	notify_verbose_printf( "%s: %s last chunk %" PRIu32 " read with: base %" PRIu64 " and offset %" PRIu32 ".\n",
	 function, chunk_type, ( offset_table->last_chunk_offset_filled + 1 ), base_offset, current_offset );
#endif

	offset_table->chunk_offset[ offset_table->last_chunk_offset_filled ].segment_file_handle = segment_file_handle;
	offset_table->chunk_offset[ offset_table->last_chunk_offset_filled ].file_offset         = (off64_t) ( base_offset + current_offset );
	offset_table->chunk_offset[ offset_table->last_chunk_offset_filled ].compressed          = compressed;

	return( 1 );
}

/* Calculate the last offset and fills the offset table
 * Returns 1 if successful or -1 on error
 */
int libewf_offset_table_fill_last_offset(
     libewf_offset_table_t *offset_table,
     libewf_section_list_t *section_list,
     uint8_t error_tollerance )
{
	libewf_section_list_entry_t *section_list_entry = NULL;
	static char *function                           = "libewf_offset_table_fill_last_offset";
	off64_t last_offset                             = 0;
	off64_t chunk_size                              = 0;

	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	if( offset_table->chunk_offset == NULL )
	{
		notify_warning_printf( "%s: invalid offset table - missing chunk offsets.\n",
		 function );

		return( -1 );
	}
	if( section_list == NULL )
	{
		notify_warning_printf( "%s: invalid section list.\n",
		 function );

		return( -1 );
	}
	/* There is no indication how large the last chunk is. The only thing known is where it starts.
	 * However it can be determined where the next section starts within the file.
	 * The size of the last chunk is determined by subtracting the last offset from the offset of the section that follows.
	 */
	section_list_entry = section_list->first;
	last_offset        = offset_table->chunk_offset[ offset_table->last_chunk_offset_filled ].file_offset;

	while( section_list_entry != NULL )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		notify_verbose_printf( "%s: start offset: %" PRIi64 " last offset: %" PRIi64 " \n",
		 function, section_list_entry->start_offset, last_offset );
#endif

		if( ( section_list_entry->start_offset < last_offset )
		 && ( last_offset < section_list_entry->end_offset ) )
		{
			chunk_size = section_list_entry->end_offset - last_offset;

			if( last_offset > (off64_t) INT64_MAX )
			{
				notify_warning_printf( "%s: invalid last chunk offset value exceeds maximum.\n",
				 function );

				return( -1 );
			}
			if( chunk_size == 0 )
			{
				notify_warning_printf( "%s: invalid chunk size - size is zero.\n",
				 function );

				if( error_tollerance < LIBEWF_ERROR_TOLLERANCE_COMPENSATE )
				{
					return( -1 );
				}
			}
			if( chunk_size > (off64_t) INT32_MAX )
			{
				notify_warning_printf( "%s: invalid chunk size value exceeds maximum.\n",
				 function );

				return( -1 );
			}
#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf( "%s: last chunk %" PRIu32 " calculated with offset: %" PRIu64 " and size %" PRIzu ".\n",
			 function, ( offset_table->last_chunk_offset_filled + 1 ), last_offset, (size_t) chunk_size );
#endif

			offset_table->chunk_offset[ offset_table->last_chunk_offset_filled ].size = (size_t) chunk_size;

			break;
		}
		section_list_entry = section_list_entry->next;
	}
	return( 1 );
}

/* Fills the offsets from the offset table
 * amount_of_chunk_offsets contains the amount of chunk offsets to fill
 * Returns 1 if successful or -1 on error
 */
int libewf_offset_table_fill_offsets(
     libewf_offset_table_t *offset_table,
     uint32_t offset_table_index,
     uint32_t amount_of_chunk_offsets,
     off64_t base_offset,
     ewf_table_offset_t *offsets,
     uint32_t amount_of_offsets )
{
	static char *function   = "libewf_offset_table_fill_offsets";
	off64_t offset64_value  = 0;
	uint32_t offset32_value = 0;
	uint32_t iterator       = 0;

	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	if( offset_table->chunk_offset == NULL )
	{
		notify_warning_printf( "%s: invalid offset table - missing chunk offsets.\n",
		 function );

		return( -1 );
	}
	if( base_offset < 0 )
	{
		notify_warning_printf( "%s: invalid base offset.\n",
		 function );

		return( -1 );
	}
	if( offsets == NULL )
	{
		notify_warning_printf( "%s: invalid table offsets.\n",
		 function );

		return( -1 );
	}
	if( amount_of_offsets < amount_of_chunk_offsets )
	{
		notify_warning_printf( "%s: offsets too small.\n",
		 function );

		return( -1 );
	}
	for( iterator = 0; iterator < amount_of_chunk_offsets; iterator++ )
	{
		offset64_value = offset_table->chunk_offset[ offset_table_index + iterator ].file_offset
		               - base_offset;

		if( ( offset64_value < 0 )
		 || ( offset64_value > (off64_t) INT32_MAX ) )
		{
			notify_warning_printf( "%s: invalid chunk offset value.\n",
			 function );

			return( -1 );
		}
		offset32_value = (uint32_t) offset64_value;

		if( offset_table->chunk_offset[ offset_table_index + iterator ].compressed != 0 )
		{
			offset32_value |= EWF_OFFSET_COMPRESSED_WRITE_MASK;
		}
		endian_little_revert_32bit(
		 offsets[ iterator ].offset,
		 offset32_value );
	}
	return( 1 );
}

/* Compares the offsets with the ones in the offset table and makes corrections if necessary
 * Returns 1 if successful or -1 on error
 */
int libewf_offset_table_compare(
     libewf_offset_table_t *offset_table,
     off64_t base_offset,
     ewf_table_offset_t *offsets,
     uint32_t amount_of_chunks,
     libewf_segment_file_handle_t *segment_file_handle,
     uint8_t correct_errors,
     uint8_t error_tollerance )
{
#if defined( HAVE_VERBOSE_OUTPUT )
	char *chunk_type        = NULL;
#endif
	static char *function   = "libewf_offset_table_compare";
	uint32_t chunk_size     = 0;
	uint32_t current_offset = 0;
	uint32_t next_offset    = 0;
	uint32_t raw_offset     = 0;
	uint32_t iterator       = 0;
	uint8_t compressed      = 0;
	uint8_t mismatch        = 0;
	uint8_t overflow        = 0;

	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	if( base_offset < 0 )
	{
		notify_warning_printf( "%s: invalid base offset.\n",
		 function );

		return( -1 );
	}
	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file.\n",
		 function );

		return( -1 );
	}
	/* Correct the last offset compared, to compare the table it should point to the first empty entry
	 * the the last filled entry
	 */
	if( offset_table->last_chunk_offset_compared > 0 )
	{
		offset_table->last_chunk_offset_compared++;
	}
	/* Allocate additional entries in the offset table if needed
	 * - a single reallocation saves processing time
	 */
	if( offset_table->amount_of_chunk_offsets < ( offset_table->last_chunk_offset_compared + amount_of_chunks ) )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		notify_verbose_printf( "%s: missing chunk offsets in offset table.\n",
		 function );
#endif

		if( libewf_offset_table_resize(
		     offset_table,
		     ( offset_table->last_chunk_offset_compared + amount_of_chunks ) ) != 1 )
		{
			notify_warning_printf( "%s: unable to resize offset table.\n",
			 function );

			return( -1 );
		}
	}
	endian_little_convert_32bit(
	 raw_offset,
	 offsets[ iterator ].offset );

	/* The size of the last chunk must be determined differently
	 */
	while( iterator < ( amount_of_chunks - 1 ) )
	{
		if( overflow == 0 )
		{
			compressed     = (uint8_t) ( raw_offset >> 31 );
			current_offset = raw_offset & EWF_OFFSET_COMPRESSED_READ_MASK;
		}
		else
		{
			current_offset = raw_offset;
		}
		endian_little_convert_32bit(
		 raw_offset,
		 offsets[ iterator + 1 ].offset );

		if( overflow == 0 )
		{
			next_offset = raw_offset & EWF_OFFSET_COMPRESSED_READ_MASK;
		}
		else
		{
			next_offset = raw_offset;
		}

		/* This is to compensate for the crappy >2Gb segment file
		 * solution in EnCase 6.7
		 */
		if( next_offset < current_offset )
		{
			if( raw_offset < current_offset )
			{
				notify_warning_printf( "%s: chunk offset %" PRIu32 " larger than raw %" PRIu32 ".\n",
				 function, current_offset, raw_offset );

				return( -1 );
			}
#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf( "%s: chunk offset %" PRIu32 " larger than next %" PRIu32 ".\n",
			 function, current_offset, next_offset );
#endif

			chunk_size = raw_offset - current_offset;
		}
		else
		{
			chunk_size = next_offset - current_offset;
		}

		if( chunk_size == 0 )
		{
			notify_warning_printf( "%s: invalid chunk size - size is zero.\n",
			 function );

			if( error_tollerance < LIBEWF_ERROR_TOLLERANCE_COMPENSATE )
			{
				return( -1 );
			}
		}
		if( chunk_size > (uint32_t) INT32_MAX )
		{
			notify_warning_printf( "%s: invalid chunk size value exceeds maximum.\n",
			 function );

			return( -1 );
		}
#if defined( HAVE_VERBOSE_OUTPUT )
		if( compressed == 0 )
		{
			chunk_type = "uncompressed";
		}
		else
		{
			chunk_type = "compressed";
		}
		notify_verbose_printf( "%s: %s chunk %" PRIu32 " read with: base %" PRIu64 ", offset %" PRIu32 " and size %" PRIu32 ".\n",
		 function, chunk_type, ( offset_table->last_chunk_offset_compared + 1 ), base_offset, current_offset, chunk_size );
#endif

		if( offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].file_offset != (off64_t) ( base_offset + current_offset ) )
		{
#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf( "%s: file offset mismatch for chunk offset: %" PRIu32 ".\n",
			 function, offset_table->last_chunk_offset_compared );
#endif

			mismatch = 1;
		}
		else if( offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].size != (size_t) chunk_size )
		{
#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf( "%s: chunk size mismatch for chunk offset: %" PRIu32 ".\n",
			 function, offset_table->last_chunk_offset_compared );
#endif

			mismatch = 1;
		}
		else if( offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].compressed != compressed )
		{
#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf( "%s: compressed mismatch for chunk offset: %" PRIu32 ".\n",
			 function, offset_table->last_chunk_offset_compared );
#endif

			mismatch = 1;
		}
		else
		{
			mismatch = 0;
		}
		if( ( mismatch == 1 )
		 && ( correct_errors != 0 ) )
		{
#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf( "%s: correcting chunk offset: %" PRIu32 ".\n",
			 function, offset_table->last_chunk_offset_compared );
#endif

			offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].segment_file_handle = segment_file_handle;
			offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].file_offset         = (off64_t) ( base_offset + current_offset );
			offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].size                = (size_t) chunk_size;
			offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].compressed          = compressed;
		}
		offset_table->last_chunk_offset_compared++;

		/* This is to compensate for the crappy >2Gb segment file
		 * solution in EnCase 6.7
		 */
		if( ( overflow == 0 )
		 && ( ( current_offset + chunk_size ) > (uint32_t) INT32_MAX ) )
		{
#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf( "%s: chunk offset overflow at: %" PRIu32 ".\n",
			 function, current_offset );
#endif

			overflow   = 1;
			compressed = 0;
		}
		iterator++;
	}
	endian_little_convert_32bit(
	 raw_offset,
	 offsets[ iterator ].offset );

	if( overflow == 0 )
	{
		compressed     = (uint8_t) ( raw_offset >> 31 );
		current_offset = raw_offset & EWF_OFFSET_COMPRESSED_READ_MASK;
	}
	else
	{
		current_offset = raw_offset;
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	if( compressed == 0 )
	{
		chunk_type = "uncompressed";
	}
	else
	{
		chunk_type = "compressed";
	}
	notify_verbose_printf( "%s: %s last chunk %" PRIu32 " read with: base %" PRIu64 " and offset %" PRIu32 ".\n",
	 function, chunk_type, ( offset_table->last_chunk_offset_compared + 1 ), base_offset, current_offset );
#endif

	if( offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].file_offset != (off64_t) ( base_offset + current_offset ) )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		notify_verbose_printf( "%s: file offset mismatch for chunk offset: %" PRIu32 ".\n",
		 function, offset_table->last_chunk_offset_compared );
#endif

		mismatch = 1;
	}
	else if( offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].compressed != compressed )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		notify_verbose_printf( "%s: compressed mismatch for chunk offset: %" PRIu32 ".\n",
		 function, offset_table->last_chunk_offset_compared );
#endif

		mismatch = 1;
	}
	else
	{
		mismatch = 0;
	}
	if( ( mismatch == 1 )
	 && ( correct_errors != 0 ) )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		notify_verbose_printf( "%s: correcting chunk offset: %" PRIu32 ".\n",
		 function, offset_table->last_chunk_offset_compared );
#endif

		offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].segment_file_handle = segment_file_handle;
		offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].file_offset         = (off64_t) ( base_offset + current_offset );
		offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].compressed          = compressed;
	}
	return( 1 );
}

/* Calculate the last offset and compares it with the one in the offset table and makes corrections if necessary
 * Returns 1 if successful or -1 on error
 */
int libewf_offset_table_compare_last_offset(
     libewf_offset_table_t *offset_table,
     libewf_section_list_t *section_list,
     uint8_t correct_errors,
     uint8_t error_tollerance )
{
	libewf_section_list_entry_t *section_list_entry = NULL;
	static char *function                           = "libewf_offset_table_fill_last_offset";
	off64_t last_offset                             = 0;
	off64_t chunk_size                              = 0;
	uint8_t mismatch                                = 0;

	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	if( offset_table->chunk_offset == NULL )
	{
		notify_warning_printf( "%s: invalid offset table - missing chunk offsets.\n",
		 function );

		return( -1 );
	}
	if( section_list == NULL )
	{
		notify_warning_printf( "%s: invalid section list.\n",
		 function );

		return( -1 );
	}
	/* There is no indication how large the last chunk is. The only thing known is where it starts.
	 * However it can be determined where the next section starts within the file.
	 * The size of the last chunk is determined by subtracting the last offset from the offset of the section that follows.
	 */
	section_list_entry = section_list->first;
	last_offset        = offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].file_offset;

	while( section_list_entry != NULL )
	{
#if defined( HAVE_DEBUG_OUTPUT )
		notify_verbose_printf( "%s: start offset: %" PRIi64 " last offset: %" PRIi64 " \n",
		 function, section_list_entry->start_offset, last_offset );
#endif

		if( ( section_list_entry->start_offset < last_offset )
		 && ( last_offset < section_list_entry->end_offset ) )
		{
			chunk_size = section_list_entry->end_offset - last_offset;

			if( last_offset > (off64_t) INT64_MAX )
			{
				notify_warning_printf( "%s: invalid last chunk offset value exceeds maximum.\n",
				 function );

				return( -1 );
			}
			if( chunk_size == 0 )
			{
				notify_warning_printf( "%s: invalid chunk size - size is zero.\n",
				 function );

				if( error_tollerance < LIBEWF_ERROR_TOLLERANCE_COMPENSATE )
				{
					return( -1 );
				}
			}
			if( chunk_size > (off64_t) INT32_MAX )
			{
				notify_warning_printf( "%s: invalid chunk size value exceeds maximum.\n",
				 function );

				return( -1 );
			}
#if defined( HAVE_VERBOSE_OUTPUT )
			notify_verbose_printf( "%s: last chunk %" PRIu32 " calculated with offset: %" PRIu64 " and size %" PRIzu ".\n",
			 function, ( offset_table->last_chunk_offset_compared + 1 ), last_offset, (size_t) chunk_size );
#endif

			if( offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].size != (size_t) chunk_size )
			{
#if defined( HAVE_VERBOSE_OUTPUT )
				notify_verbose_printf( "%s: chunk size mismatch for chunk offset: %" PRIu32 ".\n",
				 function, offset_table->last_chunk_offset_compared );
#endif

				mismatch = 1;
			}
			else
			{
				mismatch = 0;
			}
			if( ( mismatch == 1 )
			 && ( correct_errors != 0 ) )
			{
#if defined( HAVE_VERBOSE_OUTPUT )
				notify_verbose_printf( "%s: correcting chunk offset: %" PRIu32 ".\n",
				 function, offset_table->last_chunk_offset_compared );
#endif

				offset_table->chunk_offset[ offset_table->last_chunk_offset_compared ].size = (size_t) chunk_size;
			}
			break;
		}
		section_list_entry = section_list_entry->next;
	}
	return( 1 );
}

/* Seeks a certain chunk offset within the offset table
 * Returns the chunk segment file offset if the seek is successful or -1 on error
 */
off64_t libewf_offset_table_seek_chunk_offset(
         libewf_offset_table_t *offset_table,
         uint32_t chunk,
         libewf_file_io_pool_t *file_io_pool )
{
	static char *function = "libewf_segment_table_seek_chunk_offset";

	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	if( offset_table->chunk_offset == NULL )
	{
		notify_warning_printf( "%s: invalid offset table - missing chunk offsets.\n",
		 function );

		return( -1 );
	}
	if( chunk >= offset_table->amount_of_chunk_offsets )
	{
		notify_warning_printf( "%s: chunk: %" PRIu32 " not in offset table.\n",
		 function, chunk );

		return( -1 );
	}
	if( offset_table->chunk_offset[ chunk ].segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: missing segment file handle for chunk: %" PRIu32 ".\n",
		 function, chunk );

		return( -1 );
	}
	if( libewf_file_io_pool_seek_offset(
	     file_io_pool,
	     offset_table->chunk_offset[ chunk ].segment_file_handle->file_io_pool_entry,
	     offset_table->chunk_offset[ chunk ].file_offset,
	     SEEK_SET ) == -1 )
	{
		notify_warning_printf( "%s: cannot find chunk offset: %" PRIjd ".\n",
		 function, offset_table->chunk_offset[ chunk ].file_offset );

		return( -1 );
	}
	return( offset_table->chunk_offset[ chunk ].file_offset );
}

