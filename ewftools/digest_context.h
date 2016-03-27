/*
 * Crypographic digest context
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

#if !defined( _DIGEST_CONTEXT_H )
#define _DIGEST_CONTEXT_H

#include <common.h>
#include <types.h>

#if defined( HAVE_LIBCRYPTO ) && defined( HAVE_OPENSSL_EVP_H )
#include <openssl/evp.h>
#elif defined( HAVE_WINCPRYPT_H )
#include <windows.h>
#include <wincrypt.h>
#endif

#include "digest_hash.h"

#if defined( __cplusplus )
extern "C" {
#endif

#define DIGEST_CONTEXT_TYPE_MD5		(uint8_t) 'm'
#define DIGEST_CONTEXT_TYPE_SHA1	(uint8_t) 's'

#if defined( HAVE_LIBCRYPTO ) && defined( HAVE_OPENSSL_EVP_H )
typedef EVP_MD_CTX digest_context_t;

#elif defined( HAVE_WINCPRYPT_H )
typedef struct digest_context digest_context_t;

struct digest_context
{
	HCRYPTPROV crypt_provider;
	HCRYPTHASH hash;
};

#else
typedef int digest_context_t;
#endif

int digest_context_initialize(
     digest_context_t *digest_context,
     uint8_t type );

int digest_context_update(
     digest_context_t *digest_context,
     uint8_t *buffer,
     size_t size );

int digest_context_finalize(
     digest_context_t *digest_context,
     digest_hash_t *digest_hash,
     size_t *size );

#if defined( __cplusplus )
}
#endif

#endif

