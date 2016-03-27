/*
 * EWF SMART volume section (EWF-S01)
 *
 * Copyright (c) 2006-2007, Joachim Metz <forensics@hoffmannbv.nl>,
 * Hoffmann Investigations. All rights reserved.
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the creator, related organisations, nor the names of
 *   its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER, COMPANY AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "libewf_includes.h"

#include "libewf_common.h"
#include "libewf_endian.h"
#include "libewf_notify.h"

#include "ewf_crc.h"
#include "ewf_volume_smart.h"

/* Reads the volume from a file descriptor
 * Returns the amount of bytes read, or -1 on error
 */
ssize_t ewf_volume_smart_read( EWF_VOLUME_SMART *volume, int file_descriptor )
{
	static char *function = "ewf_volume_smart_read";
	ssize_t count         = 0;
	size_t size           = EWF_VOLUME_SMART_SIZE;

	if( volume == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid volume.\n",
		 function );

		return( -1 );
	}
	if( file_descriptor == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	count = libewf_common_read( file_descriptor, volume, size );

	if( count < (ssize_t) size )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to read volume.\n",
		 function );

		return( -1 );
	}
	return( count );
}

/* Writes the volume to a file descriptor
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t ewf_volume_smart_write( EWF_VOLUME_SMART *volume, int file_descriptor )
{
	static char *function = "ewf_volume_smart_write";
	EWF_CRC crc           = 0;
	ssize_t count         = 0;
	size_t size           = EWF_VOLUME_SMART_SIZE;

	if( volume == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid volume.\n",
		 function );

		return( -1 );
	}
	if( file_descriptor == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	if( ewf_crc_calculate( &crc, (uint8_t *) volume, ( size - EWF_CRC_SIZE ), 1 ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to calculate CRC.\n",
		 function );

		return( -1 );
	}
	if( libewf_endian_revert_32bit( crc, volume->crc ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to revert CRC value.\n",
		 function );

		return( -1 );
	}
	count = libewf_common_write( file_descriptor, volume, size );

	if( count < (ssize_t) size )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to write volume.\n",
		 function );

		return( -1 );
	}
	return( count );
}

/* Calculates the chunck size = sectors per chunk * bytes per sector
 * Returns the amount of bytes per sector, or -1 on error
 */
ssize32_t ewf_volume_smart_calculate_chunk_size( EWF_VOLUME_SMART *volume )
{
	static char *function      = "ewf_volume_smart_calculate_chunk_size";
	uint32_t sectors_per_chunk = 0;
	uint32_t bytes_per_sector  = 0;
	size64_t chunk_size        = 0;

	if( volume == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid volume.\n",
		 function );

		return( -1 );
	}
	if( libewf_endian_convert_32bit( &sectors_per_chunk, volume->sectors_per_chunk ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to convert sectors per chunk value.\n",
		 function );

		return( -1 );
	}
	if( libewf_endian_convert_32bit( &bytes_per_sector, volume->bytes_per_sector ) != 1 )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to conver bytes per sector value.\n",
		 function );

		return( -1 );
	}
	if( sectors_per_chunk > (uint32_t) INT32_MAX )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid sectors per chunk value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( bytes_per_sector > (uint32_t) INT32_MAX )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid bytes per sector value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	chunk_size = (size64_t) sectors_per_chunk * (size64_t) bytes_per_sector;

	if( chunk_size > (size64_t) INT32_MAX )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid chunk size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	return( (ssize32_t) chunk_size );
}

