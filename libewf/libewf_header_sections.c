/*
 * libewf header sections
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
#include "libewf_header_sections.h"
#include "libewf_notify.h"

/* Allocates memory for a new header sections struct
 * Returns a pointer to the new instance, NULL on error
 */
LIBEWF_HEADER_SECTIONS *libewf_header_sections_alloc( void )
{
	LIBEWF_HEADER_SECTIONS *header_sections = NULL;
	static char *function                   = "libewf_header_sections_alloc";

	header_sections = (LIBEWF_HEADER_SECTIONS *) libewf_common_alloc( LIBEWF_HEADER_SECTIONS_SIZE );

	if( header_sections == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to allocate header sections.\n",
		 function );

		return( NULL );
	}
	if( libewf_common_memset(
	     header_sections,
	     0,
	     LIBEWF_HEADER_SECTIONS_SIZE ) == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: unable to clear media values.\n",
		 function );

		libewf_common_free( header_sections );

		return( NULL );
	}
	return( header_sections );
}

/* Frees memory of a header sections struct including elements
 */
void libewf_header_sections_free( LIBEWF_HEADER_SECTIONS *header_sections )
{
        static char *function = "libewf_header_sections_free";

	if( header_sections == NULL )
	{
		LIBEWF_WARNING_PRINT( "%s: invalid header sections.\n",
		 function );

		return;
	}
	libewf_common_free( header_sections->header );
	libewf_common_free( header_sections->header2 );
	libewf_common_free( header_sections->xheader );
	libewf_common_free( header_sections );
}

