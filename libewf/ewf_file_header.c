/*
 * EWF file header
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
#include "libewf_notify.h"

#include "ewf_file_header.h"

const uint8_t dvf_file_signature[] = { 0x64, 0x76, 0x66, 0x09, 0x0D, 0x0A, 0xFF, 0x00 };
const uint8_t evf_file_signature[] = { 0x45, 0x56, 0x46, 0x09, 0x0D, 0x0A, 0xFF, 0x00 };
const uint8_t lvf_file_signature[] = { 0x4c, 0x56, 0x46, 0x09, 0x0D, 0x0A, 0xFF, 0x00 };

/* Validates the EWF delta file signature (EWF-D01)
 * Returns 1 if the signature matches, 0 otherwise
 */
int dwf_file_header_check_signature( uint8_t *signature )
{
	static char *function = "dwf_file_header_check_signature";

	if( signature == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid signature.\n",
		 function );

		return( 0 );
	}
	if( libewf_common_memcmp( dvf_file_signature, signature, sizeof( dvf_file_signature ) ) == 0 )
	{
		return( 1 );
	}
	return( 0 );
}

/* Validates the EWF segment file signature (EWF-E01, EWF-S01)
 * Returns 1 if the signature matches, 0 otherwise
 */
int ewf_file_header_check_signature( uint8_t *signature )
{
	static char *function = "ewf_file_header_check_signature";

	if( signature == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid signature.\n",
		 function );

		return( 0 );
	}
	if( libewf_common_memcmp( evf_file_signature, signature, sizeof( evf_file_signature ) ) == 0 )
	{
		return( 1 );
	}
	return( 0 );
}

/* Validates the EWF logical evidence segment file signature (EWF-L01)
 * Returns 1 if the signature matches, 0 otherwise
 */
int lwf_file_header_check_signature( uint8_t *signature )
{
	static char *function = "lwf_file_header_check_signature";

	if( signature == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid signature.\n",
		 function );

		return( 0 );
	}
	if( libewf_common_memcmp( lvf_file_signature, signature, sizeof( lvf_file_signature ) ) == 0 )
	{
		return( 1 );
	}
	return( 0 );
}

/* Reads the file header from a file descriptor
 * Returns the amount of bytes read, or -1 on error
 */
ssize_t ewf_file_header_read( EWF_FILE_HEADER *file_header, int file_descriptor )
{
	static char *function = "ewf_file_header_read";
	ssize_t count         = 0;
	size_t size           = EWF_FILE_HEADER_SIZE;

	if( file_header == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file header.\n",
		 function );

		return( -1 );
	}
	if( file_descriptor == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	count = libewf_common_read( file_descriptor, file_header, size );

	if( count < (ssize_t) size )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to read file header.\n",
		 function );

		return( -1 );
	}
	return( count );
}

/* Writes the file header to a file descriptor
 * Returns the amount of bytes written, or -1 on error
 */
ssize_t ewf_file_header_write( EWF_FILE_HEADER *file_header, int file_descriptor )
{
	static char *function = "ewf_file_header_write";
	ssize_t count         = 0;
	size_t size           = EWF_FILE_HEADER_SIZE;

	if( file_header == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file header.\n",
		 function );

		return( -1 );
	}
	if( file_descriptor == -1 )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid file descriptor.\n",
		 function );

		return( -1 );
	}
	count = libewf_common_write( file_descriptor, file_header, size );

	if( count < (ssize_t) size )
	{
		LIBEWF_WARNING_PRINT( "%s: error writing file header.\n",
		 function );

		return( -1 );
	}
	return( count );
}

