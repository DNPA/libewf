/*
 * Segment file reading/writing functions
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
#include <file_io.h>
#include <memory.h>
#include <notify.h>
#include <types.h>

#include <libewf/definitions.h>

#include "libewf_hash_values.h"
#include "libewf_section.h"
#include "libewf_segment_file.h"
#include "libewf_segment_table.h"
#include "libewf_string.h"

#include "ewf_definitions.h"
#include "ewf_file_header.h"

const uint8_t dvf_file_signature[] = { 0x64, 0x76, 0x66, 0x09, 0x0D, 0x0A, 0xFF, 0x00 };
const uint8_t evf_file_signature[] = { 0x45, 0x56, 0x46, 0x09, 0x0D, 0x0A, 0xFF, 0x00 };
const uint8_t lvf_file_signature[] = { 0x4c, 0x56, 0x46, 0x09, 0x0D, 0x0A, 0xFF, 0x00 };

/* Detects if a file is an EWF file (check for the EWF file signature)
 * Returns 1 if true, 0 if not, or -1 on error
 */
int libewf_segment_file_check_file_signature(
     int file_descriptor )
{
	uint8_t signature[ 8 ];

	static char *function = "libewf_segment_file_check_file_signature";
	ssize_t read_count    = 0;

	if( file_descriptor == -1 )
	{
		notify_warning_printf( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	read_count = file_io_read(
	              file_descriptor,
	              signature,
	              8 );

	if( read_count <= -1 )
	{
		notify_warning_printf( "%s: error reading signature from file.\n",
		 function );

		return( -1 );
	}
	else if( read_count != 8 )
	{
		notify_warning_printf( "%s: unable to read signature from file.\n",
		 function );

		return( -1 );
	}
	/* The amount of EWF segment files will be the largest
	 */
	if( memory_compare(
	     evf_file_signature,
	     signature,
	     8 ) == 0 )
	{
		return( 1 );
	}
	else if( memory_compare(
	          lvf_file_signature,
	          signature,
	          8 ) == 0 )
	{
		return( 1 );
	}
	else if( memory_compare(
	          dvf_file_signature,
	          signature,
	          8 ) == 0 )
	{
		return( 1 );
	}
	return( 0 );
}

/* Reads the file header from a segment file
 * Returns the amount of bytes read if successful, or -1 on errror
 */
ssize_t libewf_segment_file_read_file_header(
         libewf_segment_file_handle_t *segment_file_handle,
         uint16_t *segment_number )
{
	ewf_file_header_t file_header;

	static char *function = "libewf_segment_file_read_file_header";
	ssize_t read_count    = 0;

	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file handle.\n",
		 function );

		return( -1 );
	}
	if( segment_number == NULL )
	{
		notify_warning_printf( "%s: invalid segment number.\n",
		 function );

		return( -1 );
	}
	read_count = libewf_segment_file_handle_read(
	              segment_file_handle,
	              &file_header,
	              sizeof( ewf_file_header_t ) );

	if( read_count != (ssize_t) sizeof( ewf_file_header_t ) )
	{
		notify_warning_printf( "%s: unable to read file header.\n",
		 function );

		return( -1 );
	}
	/* The amount of EWF segment files will be the largest
	 */
	if( memory_compare(
	     evf_file_signature,
	     file_header.signature,
	     8 ) == 0 )
	{
		segment_file_handle->file_type = LIBEWF_SEGMENT_FILE_TYPE_EWF;
	}
	else if( memory_compare(
	          lvf_file_signature,
	          file_header.signature,
	          8 ) == 0 )
	{
		segment_file_handle->file_type = LIBEWF_SEGMENT_FILE_TYPE_LWF;
	}
	else if( memory_compare(
	          dvf_file_signature,
	          file_header.signature,
	          8 ) == 0 )
	{
		segment_file_handle->file_type = LIBEWF_SEGMENT_FILE_TYPE_DWF;
	}
	else
	{
		notify_warning_printf( "%s: file signature does not match.\n",
		 function );

		return( -1 );
	}
	endian_little_convert_16bit(
	 *segment_number,
	 file_header.fields_segment );

	return( read_count );
}

/* Reads all sections from a segment file into the section list specific
 * for the segment file in the segment table in the handle
 * Returns 1 if successful, 0 if not, or -1 on error
 */
int libewf_segment_file_read_sections(
     libewf_segment_file_handle_t *segment_file_handle,
     int *last_segment_file,
     libewf_header_sections_t *header_sections,
     libewf_hash_sections_t *hash_sections,
     libewf_media_values_t *media_values,
     libewf_offset_table_t *offset_table,
     libewf_offset_table_t *secondary_offset_table,
     libewf_sector_table_t *sessions,
     libewf_sector_table_t *acquiry_errors,
     int8_t *compression_level,
     uint8_t *format,
     uint8_t *ewf_format,
     size64_t *segment_file_size,
     uint8_t error_tollerance  )
{
	ewf_section_t section;

	static char *function   = "libewf_segment_file_read_sections";
	off64_t previous_offset = 0;
	int result              = 0;

	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file handle.\n",
		 function );

		return( -1 );
	}
	if( last_segment_file == NULL )
	{
		notify_warning_printf( "%s: invalid last segment file.\n",
		 function );

		return( -1 );
	}
	*last_segment_file = 0;

	/* The first offset is directly after the file header (13 byte)
	 */
	previous_offset = (off64_t) sizeof( ewf_file_header_t );

	while( result != -1 )
	{
		result = libewf_section_read(
		          segment_file_handle,
		          header_sections,
		          hash_sections,
		          media_values,
		          offset_table,
		          secondary_offset_table,
		          sessions,
		          acquiry_errors,
		          compression_level,
		          format,
		          ewf_format,
		          segment_file_size,
		          &section,
		          &previous_offset,
		          error_tollerance );

		if( result != 1 )
		{
			notify_warning_printf( "%s: unable to read section.\n",
			 function );

			return( -1 );
		}
		/* The next and done sections point back at themselves
		 */
		if( memory_compare(
		     section.type,
		     "next",
		     5 ) == 0 )
		{
			return( 1 );
		}
		else if( memory_compare(
		          section.type,
		          "done",
		          5 ) == 0 )
		{
			*last_segment_file = 1;

			return( 1 );
		}
	}
	return( 0 );
}

/* Write the headers to file
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_headers(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_header_sections_t *header_sections,
         int8_t compression_level,
         uint8_t format )
{
	static char *function     = "libewf_segment_file_write_headers";
	ssize_t write_count       = 0;
	ssize_t total_write_count = 0;
	size_t header_size        = 0;
	size_t header2_size       = 0;

	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file handle.\n",
		 function );

		return( -1 );
	}
	if( header_sections == NULL )
	{
		notify_warning_printf( "%s: invalid header sections.\n",
		 function );

		return( -1 );
	}
	if( ( header_sections->header == NULL )
	 || ( header_sections->header_size == 0 ) )
	{
		notify_warning_printf( "%s: invalid header.\n",
		 function );

		return( -1 );
	}
	/* The header size contains the end of string character
	 */
	header_size = header_sections->header_size - 1;

	if( ( format == LIBEWF_FORMAT_EWF )
	 || ( format == LIBEWF_FORMAT_SMART )
	 || ( format == LIBEWF_FORMAT_ENCASE1 ) )
	{
		/* The header should be written only once
		 * and using the compression used in the file
		 */
		write_count = libewf_section_header_write(
		               segment_file_handle,
		               header_sections->header,
		               header_size,
		               compression_level );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write single header section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		header_sections->amount_of_header_sections += 1;
	}
	else if( ( format == LIBEWF_FORMAT_ENCASE2 )
	 || ( format == LIBEWF_FORMAT_ENCASE3 )
	 || ( format == LIBEWF_FORMAT_LINEN5 )
	 || ( format == LIBEWF_FORMAT_LINEN6 )
	 || ( format == LIBEWF_FORMAT_FTK ) )
	{
		/* The header should be written twice
		 * the default compression is used
		 */
		write_count = libewf_section_header_write(
		               segment_file_handle,
		               header_sections->header,
		               header_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write first header section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		write_count = libewf_section_header_write(
		               segment_file_handle,
		               header_sections->header,
		               header_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write second header section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		header_sections->amount_of_header_sections += 2;
	}
	else if( ( format == LIBEWF_FORMAT_ENCASE4 )
	 || ( format == LIBEWF_FORMAT_ENCASE5 )
	 || ( format == LIBEWF_FORMAT_ENCASE6 ) )
	{
		if( ( header_sections->header2 == NULL )
		 && ( header_sections->header2_size == 0 ) )
		{
			notify_warning_printf( "%s: invalid header2.\n",
			 function );

			return( -1 );
		}
		/* The header2 size contains two end of string characters
		 */
		header2_size = header_sections->header2_size - 2;

		/* The header2 should be written twice
		 * the default compression is used
		 */
		write_count = libewf_section_header2_write(
		               segment_file_handle,
		               header_sections->header2,
		               header2_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write first header2 section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		write_count = libewf_section_header2_write(
		               segment_file_handle,
		               header_sections->header2,
		               header2_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write second header2 section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		/* The header should be written once
		 * the default compression is used
		 */
		write_count = libewf_section_header_write(
		               segment_file_handle,
		               header_sections->header,
		               header_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write third header section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		header_sections->amount_of_header_sections += 3;
	}
	/* EWFX uses the header and header2 for backwards compatibility
	 */
	else if( format == LIBEWF_FORMAT_EWFX )
	{
		if( ( header_sections->xheader == NULL )
		 && ( header_sections->xheader_size == 0 ) )
		{
			notify_warning_printf( "%s: invalid xheader.\n",
			 function );

			return( -1 );
		}
		if( ( header_sections->header2 == NULL )
		 && ( header_sections->header2_size == 0 ) )
		{
			notify_warning_printf( "%s: invalid header2.\n",
			 function );

			return( -1 );
		}
		/* The header2 size contains two end of string characters
		 */
		header2_size = header_sections->header2_size - 2;

		/* The xheader should be written once
		 * the default compression is used
		 */
		write_count = libewf_section_xheader_write(
		               segment_file_handle,
		               header_sections->xheader,
		               header_sections->xheader_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write firts xheader section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		/* The header2 should be written once
		 * the default compression is used
		 */
		write_count = libewf_section_header2_write(
		               segment_file_handle,
		               header_sections->header2,
		               header2_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write second header2 section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		/* The header should be written once
		 * the default compression is used
		 */
		write_count = libewf_section_header_write(
		               segment_file_handle,
		               header_sections->header,
		               header_size,
		               EWF_COMPRESSION_DEFAULT );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write third header section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		header_sections->amount_of_header_sections += 3;
	}
	return( total_write_count );
}

/* Write the last section at the end of the segment file
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_last_section(
         libewf_segment_file_handle_t *segment_file_handle,
         int last_segment_file,
         uint8_t format,
         uint8_t ewf_format )
{
	uint8_t *last_section_type = NULL;
	static char *function      = "libewf_segment_file_write_last_section";
	ssize_t write_count        = 0;

	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file handle.\n",
		 function );

		return( -1 );
	}
	if( last_segment_file == 0 )
	{
		last_section_type = (uint8_t *) "next";
	}
	else
	{
		last_section_type = (uint8_t *) "done";
	}
	/* Write next or done section
	 */
	write_count = libewf_section_last_write(
		       segment_file_handle,
		       last_section_type,
		       4,
		       format,
		       ewf_format );

	if( write_count == -1 )
	{
		notify_warning_printf( "%s: unable to write %s section.\n",
		 function, (char *) last_section_type );

		return( -1 );
	}
	return( write_count );
}

/* Write the necessary sections at the start of the segment file
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_start(
         libewf_segment_file_handle_t *segment_file_handle,
         uint16_t segment_number,
         uint8_t segment_file_type,
         libewf_media_values_t *media_values,
         libewf_header_sections_t *header_sections,
         int8_t compression_level,
         uint8_t format,
         uint8_t ewf_format,
         ewf_data_t **cached_data_section )
{
	ewf_file_header_t file_header;

	static char *function         = "libewf_segment_file_write_start";
	const uint8_t *file_signature = NULL;
	ssize_t total_write_count     = 0;
	ssize_t write_count           = 0;

	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file handle.\n",
		 function );

		return( -1 );
	}
	if( segment_number == 0 )
	{
		notify_warning_printf( "%s: invalid segment number.\n",
		 function );

		return( -1 );
	}
	if( segment_file_handle->file_descriptor == -1 )
	{
		notify_warning_printf( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	if( segment_file_handle->section_list == NULL )
	{
		notify_warning_printf( "%s: invalid section list.\n",
		 function );

		return( -1 );
	}
	if( cached_data_section == NULL )
	{
		notify_warning_printf( "%s: invalid cached data section.\n",
		 function );

		return( -1 );
	}

	/* Determine the segment file signature
	 */
	if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF )
	{
		file_signature = evf_file_signature;
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_LWF )
	{
		file_signature = lvf_file_signature;
	}
	else if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_DWF )
	{
		file_signature = dvf_file_signature;
	}
	else
	{
		notify_warning_printf( "%s: unsupported segment file type.\n",
		 function );

		return( -1 );
	}
	segment_file_handle->file_type = segment_file_type;

	if( memory_copy(
	     file_header.signature,
	     file_signature,
	     8 ) == NULL )
	{
		notify_warning_printf( "%s: unable to set file signature.\n",
		 function );

		return( -1 );
	}
	endian_little_revert_16bit(
	 file_header.fields_segment,
	 segment_number );

	file_header.fields_start    = 1;
	file_header.fields_end[ 0 ] = 0;
	file_header.fields_end[ 1 ] = 0;

	/* Write segment file header
	 */
	write_count = libewf_segment_file_handle_write(
	               segment_file_handle,
	               &file_header,
	               sizeof( ewf_file_header_t ) );

	if( write_count != (ssize_t) sizeof( ewf_file_header_t ) )
	{
		notify_warning_printf( "%s: unable to write file header to file.\n",
		 function );

		return( -1 );
	}
	total_write_count += write_count;

	if( segment_file_type == LIBEWF_SEGMENT_FILE_TYPE_EWF )
	{
		if( segment_number == 1 )
		{
			/* Write header section(s)
			 * The segment file offset is updated by the function
			 */
			write_count = libewf_segment_file_write_headers(
				       segment_file_handle,
				       header_sections,
				       compression_level,
				       format );

			if( write_count == -1 )
			{
				notify_warning_printf( "%s: unable to write header sections.\n",
				 function );

				return( -1 );
			}
			total_write_count += write_count;

			if( ewf_format == EWF_FORMAT_S01 )
			{
				/* Write volume (SMART) section
				 */
				write_count = libewf_section_volume_s01_write(
					       segment_file_handle,
					       media_values,
					       format,
					       0 );
			}
			else if( ewf_format == EWF_FORMAT_E01 )
			{
				/* Write volume section
				 */
				write_count = libewf_section_volume_e01_write(
					       segment_file_handle,
					       media_values,
					       compression_level,
					       format,
					       0 );
			}
			else
			{
				/* Fail safe
				 */
				write_count = -1;
			}
			if( write_count == -1 )
			{
				notify_warning_printf( "%s: unable to write volume section.\n",
				 function );

				return( -1 );
			}
			total_write_count += write_count;
		}
		else if( ewf_format == EWF_FORMAT_E01 )
		{
			/* Write data section
			 */
			write_count = libewf_section_data_write(
				       segment_file_handle,
				       media_values,
				       compression_level,
				       format,
				       cached_data_section,
				       0 );

			if( write_count == -1 )
			{
				notify_warning_printf( "%s: unable to write data section.\n",
				 function );

				return( -1 );
			}
			total_write_count += write_count;
		}
	}
	return( total_write_count );
}

/* Write the necessary sections before the actual data chunks to file
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_chunks_section_start(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_offset_table_t *offset_table,
         ewf_table_offset_t *table_offsets,
         uint32_t amount_of_table_offsets,
         size32_t chunk_size,
         uint32_t total_chunk_amount,
         uint32_t segment_chunk_amount,
         uint8_t format,
         uint8_t ewf_format )
{
	static char *function = "libewf_segment_file_write_chunks_section_start";
	ssize_t write_count   = 0;
	size_t section_size   = 0;

	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file handle.\n",
		 function );

		return( -1 );
	}
	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	if( table_offsets == NULL )
	{
		notify_warning_printf( "%s: invalid table offsets.\n",
		 function );

		return( -1 );
	}
	if( amount_of_table_offsets < segment_chunk_amount )
	{
		notify_warning_printf( "%s: table offsets too small.\n",
		 function );

		return( -1 );
	}
	/* The segment_chunk_amount contains the estimated amount of chunks for this section
	 */
	if( offset_table->amount < ( total_chunk_amount + segment_chunk_amount ) )
	{
		if( libewf_offset_table_realloc(
		     offset_table,
		     ( total_chunk_amount + segment_chunk_amount ) ) != 1 )
		{
			notify_warning_printf( "%s: unable to reallocate offset table.\n",
			 function );

			return( -1 );
		}
	}
	if( ( ewf_format == EWF_FORMAT_S01 )
	 || ( format == LIBEWF_FORMAT_ENCASE1 ) )
	{
		/* Write table section start
		 */
		write_count = libewf_section_table_write(
		               segment_file_handle,
		               0,
		               table_offsets,
		               segment_chunk_amount,
		               (uint8_t *) "table",
		               5,
		               0,
		               format,
		               ewf_format,
		               1 );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write table section.\n",
			 function );

			return( -1 );
		}
	}
	else if( ewf_format == EWF_FORMAT_E01 )
	{
		section_size = segment_chunk_amount 
		             * ( chunk_size + sizeof( ewf_crc_t ) );

		/* Write sectors section start
		 */
		write_count = libewf_section_sectors_write(
		               segment_file_handle,
		               section_size,
		               1 );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to write sectors section.\n",
			 function );

			return( -1 );
		}
	}
	return( write_count );
}

/* Write a chunk of data to a segment file and update the offset table
 * Set write_crc to a non 0 value if the CRC is not provided within the chunk data
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_chunks_data(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_offset_table_t *offset_table,
         uint32_t chunk,
         uint8_t *chunk_data,
         size_t size,
         int8_t is_compressed,
         ewf_crc_t *chunk_crc,
         int8_t write_crc )
{
	uint8_t calculated_crc_buffer[ 4 ];

#if defined( HAVE_VERBOSE_OUTPUT )
	char *chunk_type          = NULL;
#endif
	static char *function     = "libewf_segment_file_write_chunks_data";
	ssize_t write_count       = 0;
	ssize_t total_write_count = 0;
	size_t chunk_size         = size;

	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file handle.\n",
		 function );

		return( -1 );
	}
	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	if( chunk_crc == NULL )
	{
		notify_warning_printf( "%s: invalid chunk CRC.\n",
		 function );

		return( -1 );
	}
	if( size > (size_t) SSIZE_MAX )
	{
		notify_warning_printf( "%s: invalid size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( segment_file_handle->file_descriptor == -1 )
	{
		notify_warning_printf( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	if( write_crc != 0 )
	{
		chunk_size += sizeof( ewf_crc_t );
	}
	/* Make sure the chunk is available in the offset table
	 */
	if( offset_table->amount < ( chunk + 1 ) )
	{
		if( libewf_offset_table_realloc(
		     offset_table,
		     ( chunk + 1 ) ) != 1 )
		{
			notify_warning_printf( "%s: unable to reallocate offset table.\n",
			 function );

			return( -1 );
		}
	}
	/* Set the values in the offset table
	 */
	offset_table->chunk_offset[ chunk ].segment_file_handle = segment_file_handle;
	offset_table->chunk_offset[ chunk ].file_offset         = segment_file_handle->file_offset;
	offset_table->chunk_offset[ chunk ].size                = chunk_size;
	offset_table->chunk_offset[ chunk ].compressed          = is_compressed;

#if defined( HAVE_VERBOSE_OUTPUT )
	/* Print a verbose notification
	 */
	if( is_compressed == 0 )
	{
		chunk_type = "UNCOMPRESSED";
	}
	else
	{
		chunk_type = "COMPRESSED";
	}
	notify_warning_printf( "%s: writing %s chunk: %" PRIu32 " at offset: %" PRIjd " with size: %" PRIzu ", with CRC: %" PRIu32 ".\n",
	 function, chunk_type, ( chunk + 1 ), segment_file_handle->file_offset, chunk_size, *chunk_crc );
#endif

	/* Write the chunk data to the segment file
	 */
	write_count = libewf_segment_file_handle_write(
	               segment_file_handle,
	               chunk_data,
	               size );

	if( write_count != (ssize_t) size )
	{
		notify_warning_printf( "%s: unable to write data.\n",
		 function );

		return( -1 );
	}
	total_write_count += write_count;

	/* Write the CRC if necessary
	 */
	if( write_crc != 0 )
	{
		endian_little_revert_32bit(
		 calculated_crc_buffer,
		 *chunk_crc );

		write_count = libewf_segment_file_handle_write(
		               segment_file_handle,
		               calculated_crc_buffer,
		               sizeof( ewf_crc_t ) );

		if( write_count != (ssize_t) sizeof( ewf_crc_t ) )
		{
			notify_warning_printf( "%s: unable to write CRC.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;
	}
	return( total_write_count );
}

/* Correct the sections before the actual data chunks
 * Also write the necessary sections after the actual data chunks to file (like table and table2 sections for EWF-E01 format)
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_chunks_correction(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_offset_table_t *offset_table,
         ewf_table_offset_t *table_offsets,
         uint32_t amount_of_table_offsets,
         off64_t chunks_section_offset,
         size64_t chunks_section_size,
         uint32_t amount_of_chunks,
         uint32_t section_amount_of_chunks,
         uint8_t format,
         uint8_t ewf_format )
{
	uint8_t *table_section_string    = NULL;
	static char *function            = "libewf_segment_file_write_chunks_correction";
	off64_t last_segment_file_offset = 0;
	off64_t base_offset              = 0;
	ssize_t total_write_count        = 0;
	ssize_t write_count              = 0;

	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file handle.\n",
		 function );

		return( -1 );
	}
	if( format == LIBEWF_FORMAT_ENCASE6 )
	{
		if( chunks_section_size >= (size64_t) INT64_MAX )
		{
			notify_warning_printf( "%s: invalid chunk section size value exceeds maximum.\n",
			 function );

			return( -1 );
		}
	}
	else if( chunks_section_size >= (size64_t) INT32_MAX )
	{
		notify_warning_printf( "%s: invalid chunk section size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( ( format == LIBEWF_FORMAT_ENCASE6 )
	 || ( format == LIBEWF_FORMAT_LINEN6 ) )
	{
		base_offset = chunks_section_offset;
	}
	if( libewf_offset_table_fill_offsets(
	     offset_table,
	     ( amount_of_chunks - section_amount_of_chunks ),
	     section_amount_of_chunks,
	     base_offset,
	     table_offsets,
	     amount_of_table_offsets ) != 1 )
	{
		notify_warning_printf( "%s: unable to fill table offsets.\n",
		 function );

		return( -1 );
	}
	last_segment_file_offset = segment_file_handle->file_offset;

	/* Seek the start of the data chunks
	*/
#if defined( HAVE_VERBOSE_OUTPUT )
	notify_verbose_printf( "%s: setting file descriptor to start of chunks section offset: %" PRIu32 ".\n",
	 function, chunks_section_offset );
#endif

	if( libewf_segment_file_handle_seek_offset(
	     segment_file_handle,
	     chunks_section_offset ) == -1 )
	{
		notify_warning_printf( "%s: unable to find offset to correct sectors size.\n",
		 function );

		return( -1 );
	}
	if( ( ewf_format == EWF_FORMAT_S01 )
	 || ( format == LIBEWF_FORMAT_ENCASE1 ) )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		notify_verbose_printf( "%s: correcting table section size: %" PRIu64 " offset: %" PRIjd ".\n",
		 function, chunks_section_size, chunks_section_offset );
#endif

		/* Rewrite table section start
		 */
		write_count = libewf_section_table_write(
		               segment_file_handle,
		               0,
		               table_offsets,
		               section_amount_of_chunks,
		               (uint8_t *) "table",
		               5,
		               (size_t) chunks_section_size,
		               format,
		               ewf_format,
		               0 );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to rewrite %" PRIs " section.\n",
			 function, table_section_string );

			return( -1 );
		}
	}
	else if( ewf_format == EWF_FORMAT_E01 )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		notify_verbose_printf( "%s: correcting sectors section size: %" PRIzu " offset: %" PRIjd ".\n",
		 function, chunks_section_size, chunks_section_offset );
#endif

		/* Rewrite sectors section start
		 */
		write_count = libewf_section_sectors_write(
		               segment_file_handle,
		               chunks_section_size,
		               0 );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to rewrite sectors section.\n",
			 function );

			return( -1 );
		}
	}
	/* Seek the end of the chunks section
	 */
#if defined( HAVE_VERBOSE_OUTPUT )
	notify_verbose_printf( "%s: setting file descriptor back to end of data at offset: %" PRIu32 ".\n",
	 function, last_segment_file_offset );
#endif

	if( libewf_segment_file_handle_seek_offset(
	     segment_file_handle,
	     last_segment_file_offset ) == -1 )
	{
		notify_warning_printf( "%s: unable to find offset to continue.\n",
		 function );

		return( -1 );
	}
	if( ( ewf_format == EWF_FORMAT_E01 )
	 && ( format != LIBEWF_FORMAT_ENCASE1 ) )
	{
		/* Write table section start
		 */
		write_count = libewf_section_table_write(
		               segment_file_handle,
		               base_offset,
		               table_offsets,
		               section_amount_of_chunks,
		               (uint8_t *) "table",
		               5,
		               0,
		               format,
		               ewf_format,
		               0 );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to rewrite table section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;

		/* Write table2 section start
		 */
		write_count = libewf_section_table_write(
		               segment_file_handle,
		               base_offset,
		               table_offsets,
		               section_amount_of_chunks,
		               (uint8_t *) "table2",
		               6,
		               0,
		               format,
		               ewf_format,
		               0 );

		if( write_count == -1 )
		{
			notify_warning_printf( "%s: unable to rewrite table2 section.\n",
			 function );

			return( -1 );
		}
		total_write_count += write_count;
	}
	return( total_write_count );
}

/* Write a delta chunk of data to a segment file and update the offset table
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_delta_chunk(
         libewf_segment_file_handle_t *segment_file_handle,
         libewf_offset_table_t *offset_table,
         uint32_t chunk,
         uint8_t *chunk_data,
         size_t chunk_size,
         ewf_crc_t *chunk_crc,
         uint8_t write_crc,
	 uint8_t no_section_append )
{
	static char *function = "libewf_segment_file_write_delta_chunk";
	ssize_t write_count   = 0;

	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file handle.\n",
		 function );

		return( -1 );
	}
	if( offset_table == NULL )
	{
		notify_warning_printf( "%s: invalid offset table.\n",
		 function );

		return( -1 );
	}
	/* Make sure the chunk is available in the offset table
	 */
	if( chunk >= offset_table->amount )
	{
		notify_warning_printf( "%s: chunk not in offset table.\n",
		 function );

		return( -1 );
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	/* Print a verbose notification
	 */
	notify_warning_printf( "%s: writing UNCOMPRESSED delta chunk: %" PRIu32 " at offset: %" PRIjd " with size: %" PRIzu ", with CRC: %" PRIu32 ".\n",
	 function, ( chunk + 1 ), segment_file_handle->file_offset, chunk_size, *chunk_crc );
#endif

	/* Write the chunk in the delta segment file
	 */
	write_count = libewf_section_delta_chunk_write(
	               segment_file_handle,
	               chunk, 
	               chunk_data, 
	               chunk_size, 
	               chunk_crc,
	               write_crc,
	               no_section_append );

	/* Set the values in the offset table
	 * if the delta chunk was added newly
	 *  and the delta chunk was written correctly
	 */
	if( ( no_section_append == 0 )
	 && ( write_count > 0 ) )
	{
		offset_table->chunk_offset[ chunk ].segment_file_handle = segment_file_handle;
		offset_table->chunk_offset[ chunk ].file_offset         = segment_file_handle->file_offset - chunk_size - sizeof( ewf_crc_t );
		offset_table->chunk_offset[ chunk ].size                = chunk_size + sizeof( ewf_crc_t );
		offset_table->chunk_offset[ chunk ].compressed          = 0;
	}
	return( write_count );
}

/* Closes the segment file, necessary sections at the end of the segment file will be written
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t libewf_segment_file_write_close(
         libewf_segment_file_handle_t *segment_file_handle,
         uint16_t segment_number,
         uint32_t segment_amount_of_chunks,
         int last_segment_file,
         libewf_hash_sections_t *hash_sections,
         libewf_values_table_t *hash_values,
         libewf_media_values_t *media_values,
         libewf_sector_table_t *sessions,
         libewf_sector_table_t *acquiry_errors,
         int8_t compression_level,
         uint8_t format,
         uint8_t ewf_format,
         ewf_data_t **cached_data_section )
{
	static char *function     = "libewf_segment_file_write_close";
	ssize_t total_write_count = 0;
	ssize_t write_count       = 0;

	if( segment_file_handle == NULL )
	{
		notify_warning_printf( "%s: invalid segment file handle.\n",
		 function );

		return( -1 );
	}
	if( hash_sections == NULL )
	{
		notify_warning_printf( "%s: invalid hash sections.\n",
		 function );

		return( -1 );
	}
	if( media_values == NULL )
	{
		notify_warning_printf( "%s: invalid media values.\n",
		 function );

		return( -1 );
	}
	if( sessions == NULL )
	{
		notify_warning_printf( "%s: invalid sessions.\n",
		 function );

		return( -1 );
	}
	if( acquiry_errors == NULL )
	{
		notify_warning_printf( "%s: invalid acquiry errors.\n",
		 function );

		return( -1 );
	}
	if( last_segment_file != 0 )
	{
		/* Write the data section for a single segment file only for EWF-E01
		 */
		if( ( ewf_format == EWF_FORMAT_E01 )
		 && ( segment_number == 1 ) )
		{
			write_count = libewf_section_data_write(
				       segment_file_handle,
				       media_values,
				       compression_level,
				       format,
				       cached_data_section,
				       0 );

			if( write_count == -1 )
			{
				notify_warning_printf( "%s: unable to write data section.\n",
				 function );

				return( -1 );
			}
			total_write_count += write_count;
		}
		/* Write the session section if required 
		 */
		if( ( sessions->amount > 0 )
		 && ( ( format == LIBEWF_FORMAT_ENCASE5 )
		  || ( format == LIBEWF_FORMAT_ENCASE6 )
		  || ( format == LIBEWF_FORMAT_LINEN5 )
		  || ( format == LIBEWF_FORMAT_LINEN6 )
		  || ( format == LIBEWF_FORMAT_EWFX ) ) )
		{
			write_count = libewf_section_session_write(
			               segment_file_handle,
			               sessions );

			if( write_count == -1 )
			{
				notify_warning_printf( "%s: unable to write sessions section.\n",
				 function );

				return( -1 );
			}
			total_write_count += write_count;
		}
		/* Write the error2 section if required 
		 */
		if( ( acquiry_errors->amount > 0 )
		 && ( ( format == LIBEWF_FORMAT_ENCASE3 )
		  || ( format == LIBEWF_FORMAT_ENCASE4 )
		  || ( format == LIBEWF_FORMAT_ENCASE5 )
		  || ( format == LIBEWF_FORMAT_ENCASE6 )
		  || ( format == LIBEWF_FORMAT_LINEN5 )
		  || ( format == LIBEWF_FORMAT_LINEN6 )
		  || ( format == LIBEWF_FORMAT_EWFX ) ) )
		{
			write_count = libewf_section_error2_write(
			               segment_file_handle,
			               acquiry_errors );

			if( write_count == -1 )
			{
				notify_warning_printf( "%s: unable to write error2 section.\n",
				 function );

				return( -1 );
			}
			total_write_count += write_count;
		}
		/* Write the hash section if required
		 */
		if( hash_sections->md5_hash_set != 0 )
		{
			write_count = libewf_section_hash_write(
			               segment_file_handle,
			               hash_sections->md5_hash );

			if( write_count == -1 )
			{
				notify_warning_printf( "%s: unable to write hash section.\n",
				 function );

				return( -1 );
			}
			total_write_count += write_count;
		}

		/* Write the xhash section
		 */
		if( format == LIBEWF_FORMAT_EWFX )
		{
			if( hash_sections->xhash != NULL )
			{
				notify_warning_printf( "%s: xhash already set - cleaning previous defintion.\n",
				 function );

				memory_free(
				 hash_sections->xhash );

				hash_sections->xhash = NULL;
			}
			if( libewf_hash_values_generate_xhash_string_ewfx(
			     hash_values,
			     &( hash_sections->xhash ),
			     &( hash_sections->xhash_size ) ) != 1 )
			{
				notify_warning_printf( "%s: unable to generate xhash.\n",
				 function );

				return( -1 );
			}
			write_count = libewf_section_xhash_write(
			               segment_file_handle,
			               hash_sections->xhash,
			               hash_sections->xhash_size,
			               EWF_COMPRESSION_DEFAULT );

			if( write_count == -1 )
			{
				notify_warning_printf( "%s: unable to write xhash section.\n",
				 function );

				return( -1 );
			}
			total_write_count += write_count;
		}
	}
	/* Write the done or next section
	 * The segment file offset is updated by the function
	 */
	write_count = libewf_segment_file_write_last_section(
		       segment_file_handle,
	               last_segment_file,
	               format,
	               ewf_format );

	if( write_count == -1 )
	{
		notify_warning_printf( "%s: unable to write end of segment file.\n",
		 function );

		return( -1 );
	}
	total_write_count += write_count;

	segment_file_handle->amount_of_chunks = segment_amount_of_chunks;

	if( libewf_segment_file_handle_close(
	     segment_file_handle ) != 0 )
	{
		notify_warning_printf( "%s: unable to close segment file handle.\n",
		 function );

		return( -1 );
	}
	return( total_write_count );
}

