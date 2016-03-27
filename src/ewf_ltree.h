/*
 * EWF ltree section
 *
 * Copyright (c) 2006, Joachim Metz <forensics@hoffmannbv.nl>,
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
 * - All advertising materials mentioning features or use of this software
 *   must acknowledge the contribution by people stated in the acknowledgements.
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

#ifndef _EWF_LTREE_H
#define _EWF_LTREE_H

#include <inttypes.h>

#include "ewf_header.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ewf_ltree EWF_LTREE;

struct ewf_ltree
{
	/* Unknown
	 * consists of 16 bytes
	 */
	uint8_t unknown1[16];

	/* The size of the tree in bytes
	 * consists of 4 bytes (32 bits)
	 */
	uint8_t tree_size[4];

	/* Unknown
	 * consists of 4 bytes
	 * contains 0x00
	 */
	uint8_t unknown2[4];

	/* Unknown
	 * consists of 4 bytes
	 */
	uint8_t unknown3[4];

	/* Unknown
	 * consists of 20 bytes
	 * contains 0x00
	 */
	uint8_t unknown4[20];

} __attribute__((packed));

#define EWF_LTREE_SIZE sizeof( EWF_LTREE )

EWF_LTREE *ewf_ltree_alloc( void );
void ewf_ltree_free( EWF_LTREE *ltree );
EWF_LTREE *ewf_ltree_read( int file_descriptor );
int32_t ewf_ltree_write( EWF_LTREE *ltree, int file_descriptor );

EWF_HEADER *ewf_tree_data_read( int file_descriptor, uint32_t size );

#ifdef __cplusplus
}
#endif

#endif

