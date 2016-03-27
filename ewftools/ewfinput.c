/*
 * User input functions for the ewftools
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
#include <character_string.h>
#include <memory.h>
#include <notify.h>
#include <system_string.h>
#include <types.h>

#if defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif

#if defined( HAVE_STRING_H )
#include <string.h>
#endif

/* If libtool DLL support is enabled set LIBEWF_DLL_IMPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include "ewfbyte_size_string.h"
#include "ewfinput.h"
#include "ewfstring.h"

/* Input selection defintions
 */
character_t *ewfinput_compression_levels[ 4 ] = \
 { _CHARACTER_T_STRING( "none" ),
   _CHARACTER_T_STRING( "empty-block" ),
   _CHARACTER_T_STRING( "fast" ),
   _CHARACTER_T_STRING( "best" ) };

character_t *ewfinput_format_types[ 12 ] = \
 { _CHARACTER_T_STRING( "ewf" ),
   _CHARACTER_T_STRING( "smart" ),
   _CHARACTER_T_STRING( "ftk" ),
   _CHARACTER_T_STRING( "encase1" ),
   _CHARACTER_T_STRING( "encase2" ),
   _CHARACTER_T_STRING( "encase3" ),
   _CHARACTER_T_STRING( "encase4" ),
   _CHARACTER_T_STRING( "encase5" ),
   _CHARACTER_T_STRING( "encase6" ),
   _CHARACTER_T_STRING( "linen5" ),
   _CHARACTER_T_STRING( "linen6" ),
   _CHARACTER_T_STRING( "ewfx" ) };

#if defined( LIBEWF_CD_SUPPORT )
character_t *ewfinput_media_types[ 3 ] = \
 { _CHARACTER_T_STRING( "fixed" ),
   _CHARACTER_T_STRING( "removable" ),
   _CHARACTER_T_STRING( "cd" ) };
#else
character_t *ewfinput_media_types[ 2 ] = \
 { _CHARACTER_T_STRING( "fixed" ),
   _CHARACTER_T_STRING( "removable" ) };
#endif

character_t *ewfinput_volume_types[ 2 ] = \
 { _CHARACTER_T_STRING( "logical" ),
   _CHARACTER_T_STRING( "physical" ) };

character_t *ewfinput_sector_per_block_sizes[ 10 ] = \
 { _CHARACTER_T_STRING( "64" ),
   _CHARACTER_T_STRING( "128" ),
   _CHARACTER_T_STRING( "256" ),
   _CHARACTER_T_STRING( "512" ),
   _CHARACTER_T_STRING( "1024" ),
   _CHARACTER_T_STRING( "2048" ),
   _CHARACTER_T_STRING( "4096" ),
   _CHARACTER_T_STRING( "8192" ),
   _CHARACTER_T_STRING( "16384" ),
   _CHARACTER_T_STRING( "32768" ) };

character_t *ewfinput_yes_no[ 2 ] = \
 { _CHARACTER_T_STRING( "yes" ),
   _CHARACTER_T_STRING( "no" ) };

/* Determines the sectors per chunk value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_libewf_format(
     const character_t *argument,
     uint8_t *libewf_format )
{
	static char *function = "ewfinput_determine_libewf_format";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( libewf_format == NULL )
	{
		notify_warning_printf( "%s: invalid libewf format.\n",
		 function );

		return( -1 );
	}
	if( string_compare(
	     argument,
	     _CHARACTER_T_STRING( "smart" ),
	     3 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_SMART;
		result         = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "ftk" ),
	          3 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_FTK;
		result         = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "encase1" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE1;
		result         = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "encase2" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE2;
		result         = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "encase3" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE3;
		result         = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "encase4" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE4;
		result         = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "encase5" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE5;
		result         = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "encase6" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE6;
		result         = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "linen5" ),
	          6 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_LINEN5;
		result         = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "linen6" ),
	          6 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_LINEN6;
		result         = 1;
	}
	/* This check must before the check for "ewf"
	 */
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "ewfx" ),
	          4 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_EWFX;
		result         = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "ewf" ),
	          3 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_EWF;
		result         = 1;
	}
	return( result );
}

/* Determines the sectors per chunk value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_libewf_format_system_character(
     const system_character_t *argument,
     uint8_t *libewf_format )
{
	static char *function = "ewfinput_determine_libewf_format_system_character";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( libewf_format == NULL )
	{
		notify_warning_printf( "%s: invalid libewf format.\n",
		 function );

		return( -1 );
	}
	if( system_string_compare(
	     argument,
	     _SYSTEM_CHARACTER_T_STRING( "smart" ),
	     3 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_SMART;
		result         = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "ftk" ),
	          3 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_FTK;
		result         = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "encase1" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE1;
		result         = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "encase2" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE2;
		result         = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "encase3" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE3;
		result         = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "encase4" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE4;
		result         = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "encase5" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE5;
		result         = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "encase6" ),
	          7 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_ENCASE6;
		result         = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "linen5" ),
	          6 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_LINEN5;
		result         = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "linen6" ),
	          6 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_LINEN6;
		result         = 1;
	}
	/* This check must before the check for "ewf"
	 */
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "ewfx" ),
	          4 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_EWFX;
		result         = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "ewf" ),
	          3 ) == 0 )
	{
		*libewf_format = LIBEWF_FORMAT_EWF;
		result         = 1;
	}
	return( result );
}

/* Determines the sectors per chunk value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_sectors_per_chunk(
     const character_t *argument,
     uint32_t *sectors_per_chunk )
{
	static char *function = "ewfinput_determine_sectors_per_chunk";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( sectors_per_chunk == NULL )
	{
		notify_warning_printf( "%s: invalid sectors per chunk.\n",
		 function );

		return( -1 );
	}
	if( string_compare(
	     argument,
	     _CHARACTER_T_STRING( "32768" ),
	     5 ) == 0 )
	{
		*sectors_per_chunk = 32768;
		result             = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "16384" ),
	          5 ) == 0 )
	{
		*sectors_per_chunk = 16384;
		result             = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "8192" ),
	          4 ) == 0 )
	{
		*sectors_per_chunk = 8192;
		result             = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "4096" ),
	          4 ) == 0 )
	{
		*sectors_per_chunk = 4096;
		result             = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "2048" ),
	          4 ) == 0 )
	{
		*sectors_per_chunk = 2048;
		result             = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "1024" ),
	          4 ) == 0 )
	{
		*sectors_per_chunk = 1024;
		result             = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "512" ),
	          3 ) == 0 )
	{
		*sectors_per_chunk = 512;
		result             = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "256" ),
	          3 ) == 0 )
	{
		*sectors_per_chunk = 256;
		result             = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "128" ),
	          3 ) == 0 )
	{
		*sectors_per_chunk = 128;
		result             = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "64" ),
	          2 ) == 0 )
	{
		*sectors_per_chunk = 64;
		result             = 1;
	}
	return( result );
}

/* Determines the sectors per chunk value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_sectors_per_chunk_system_character(
     const system_character_t *argument,
     uint32_t *sectors_per_chunk )
{
	static char *function = "ewfinput_determine_sectors_per_chunk_system_character";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( sectors_per_chunk == NULL )
	{
		notify_warning_printf( "%s: invalid sectors per chunk.\n",
		 function );

		return( -1 );
	}
	if( system_string_compare(
	     argument,
	     _SYSTEM_CHARACTER_T_STRING( "32768" ),
	     5 ) == 0 )
	{
		*sectors_per_chunk = 32768;
		result             = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "16384" ),
	          5 ) == 0 )
	{
		*sectors_per_chunk = 16384;
		result             = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "8192" ),
	          4 ) == 0 )
	{
		*sectors_per_chunk = 8192;
		result             = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "4096" ),
	          4 ) == 0 )
	{
		*sectors_per_chunk = 4096;
		result             = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "2048" ),
	          4 ) == 0 )
	{
		*sectors_per_chunk = 2048;
		result             = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "1024" ),
	          4 ) == 0 )
	{
		*sectors_per_chunk = 1024;
		result             = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "512" ),
	          3 ) == 0 )
	{
		*sectors_per_chunk = 512;
		result             = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "256" ),
	          3 ) == 0 )
	{
		*sectors_per_chunk = 256;
		result             = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "128" ),
	          3 ) == 0 )
	{
		*sectors_per_chunk = 128;
		result             = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "64" ),
	          2 ) == 0 )
	{
		*sectors_per_chunk = 64;
		result             = 1;
	}
	return( result );
}

/* Determines the compression level value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_compression_level(
     const character_t *argument,
     int8_t *compression_level,
     uint8_t *compress_empty_block )
{
	static char *function = "ewfinput_determine_compression_level";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( compression_level == NULL )
	{
		notify_warning_printf( "%s: invalid compression level.\n",
		 function );

		return( -1 );
	}
	if( compress_empty_block == NULL )
	{
		notify_warning_printf( "%s: invalid compress empty block.\n",
		 function );

		return( -1 );
	}
	if( string_compare(
	     argument,
	     _CHARACTER_T_STRING( "none" ),
	     4 ) == 0 )
	{
		*compression_level    = LIBEWF_COMPRESSION_NONE;
		*compress_empty_block = 0;
		result                = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "empty-block" ),
	          11 ) == 0 )
	{
		*compression_level    = LIBEWF_COMPRESSION_NONE;
		*compress_empty_block = 1;
		result                = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "empty_block" ),
	          11 ) == 0 )
	{
		*compression_level    = LIBEWF_COMPRESSION_NONE;
		*compress_empty_block = 1;
		result                = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "fast" ),
	          4 ) == 0 )
	{
		*compression_level    = LIBEWF_COMPRESSION_FAST;
		*compress_empty_block = 0;
		result                = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "best" ),
	          4 ) == 0 )
	{
		*compression_level    = LIBEWF_COMPRESSION_BEST;
		*compress_empty_block = 0;
		result                = 1;
	}
	return( result );
}

/* Determines the compression level value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_compression_level_system_character(
     const system_character_t *argument,
     int8_t *compression_level,
     uint8_t *compress_empty_block )
{
	static char *function = "ewfinput_determine_compression_level_system_character";
	int result            = 1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( compression_level == NULL )
	{
		notify_warning_printf( "%s: invalid compression level.\n",
		 function );

		return( -1 );
	}
	if( compress_empty_block == NULL )
	{
		notify_warning_printf( "%s: invalid compress empty block.\n",
		 function );

		return( -1 );
	}
	if( system_string_compare(
	     argument,
	     _SYSTEM_CHARACTER_T_STRING( "none" ),
	     4 ) == 0 )
	{
		*compression_level    = LIBEWF_COMPRESSION_NONE;
		*compress_empty_block = 0;
		result                = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "empty-block" ),
	          11 ) == 0 )
	{
		*compression_level    = LIBEWF_COMPRESSION_NONE;
		*compress_empty_block = 1;
		result                = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "empty_block" ),
	          11 ) == 0 )
	{
		*compression_level    = LIBEWF_COMPRESSION_NONE;
		*compress_empty_block = 1;
		result                = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "fast" ),
	          4 ) == 0 )
	{
		*compression_level    = LIBEWF_COMPRESSION_FAST;
		*compress_empty_block = 0;
		result                = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "best" ),
	          4 ) == 0 )
	{
		*compression_level    = LIBEWF_COMPRESSION_BEST;
		*compress_empty_block = 0;
		result                = 1;
	}
	return( result );
}

/* Determines the media type value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_media_type(
     const character_t *argument,
     uint8_t *media_type )
{
	static char *function = "ewfinput_determine_media_type";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( media_type == NULL )
	{
		notify_warning_printf( "%s: invalid media type.\n",
		 function );

		return( -1 );
	}
	if( string_compare(
	     argument,
	     _CHARACTER_T_STRING( "fixed" ),
	          5 ) == 0 )
	{
		*media_type = LIBEWF_MEDIA_TYPE_FIXED;
		result      = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "removable" ),
	          9 ) == 0 )
	{
		*media_type = LIBEWF_MEDIA_TYPE_REMOVABLE;
		result      = 1;
	}
#if defined( LIBEWF_CD_SUPPORT )
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "cd" ),
	          2 ) == 0 )
	{
		*media_type = LIBEWF_MEDIA_TYPE_CD;
		result      = 1;
	}
#endif
	return( result );
}

/* Determines the media type value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_media_type_system_character(
     const system_character_t *argument,
     uint8_t *media_type )
{
	static char *function = "ewfinput_determine_media_type_system_character";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( media_type == NULL )
	{
		notify_warning_printf( "%s: invalid media type.\n",
		 function );

		return( -1 );
	}
	if( system_string_compare(
	     argument,
	     _SYSTEM_CHARACTER_T_STRING( "fixed" ),
	          5 ) == 0 )
	{
		*media_type = LIBEWF_MEDIA_TYPE_FIXED;
		result      = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "removable" ),
	          9 ) == 0 )
	{
		*media_type = LIBEWF_MEDIA_TYPE_REMOVABLE;
		result      = 1;
	}
#if defined( LIBEWF_CD_SUPPORT )
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "cd" ),
	          2 ) == 0 )
	{
		*media_type = LIBEWF_MEDIA_TYPE_CD;
		result      = 1;
	}
#endif
	return( result );
}

/* Determines the volume type value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_volume_type(
     const character_t *argument,
     uint8_t *volume_type )
{
	static char *function = "ewfinput_determine_volume_type";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( volume_type == NULL )
	{
		notify_warning_printf( "%s: invalid volume type.\n",
		 function );

		return( -1 );
	}
	if( string_compare(
	     argument,
	     _CHARACTER_T_STRING( "logical" ),
	     7 ) == 0 )
	{
		*volume_type = LIBEWF_VOLUME_TYPE_LOGICAL;
		result       = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "physical" ),
	          8 ) == 0 )
	{
		*volume_type = LIBEWF_VOLUME_TYPE_PHYSICAL;
		result       = 1;
	}
	return( result );
}

/* Determines the volume type value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_volume_type_system_character(
     const system_character_t *argument,
     uint8_t *volume_type )
{
	static char *function = "ewfinput_determine_volume_type_system_character";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( volume_type == NULL )
	{
		notify_warning_printf( "%s: invalid volume type.\n",
		 function );

		return( -1 );
	}
	if( system_string_compare(
	     argument,
	     _SYSTEM_CHARACTER_T_STRING( "logical" ),
	     7 ) == 0 )
	{
		*volume_type = LIBEWF_VOLUME_TYPE_LOGICAL;
		result       = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "physical" ),
	          8 ) == 0 )
	{
		*volume_type = LIBEWF_VOLUME_TYPE_PHYSICAL;
		result       = 1;
	}
	return( result );
}

/* Determines the codepage from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_header_codepage_system_character(
     const system_character_t *argument,
     int *header_codepage )
{
	static char *function = "ewfinput_determine_header_codepage_system_character";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( header_codepage == NULL )
	{
		notify_warning_printf( "%s: invalid byte stream codepage.\n",
		 function );

		return( -1 );
	}
	if( system_string_compare(
	     argument,
	     _SYSTEM_CHARACTER_T_STRING( "ascii" ),
	          4 ) == 0 )
	{
		*header_codepage = LIBEWF_CODEPAGE_ASCII;
		result           = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "windows-1250" ),
	          12 ) == 0 )
	{
		*header_codepage = LIBEWF_CODEPAGE_WINDOWS_1250;
		result           = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "windows-1251" ),
	          12 ) == 0 )
	{
		*header_codepage = LIBEWF_CODEPAGE_WINDOWS_1251;
		result           = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "windows-1252" ),
	          12 ) == 0 )
	{
		*header_codepage = LIBEWF_CODEPAGE_WINDOWS_1252;
		result           = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "windows-1253" ),
	          12 ) == 0 )
	{
		*header_codepage = LIBEWF_CODEPAGE_WINDOWS_1253;
		result           = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "windows-1254" ),
	          12 ) == 0 )
	{
		*header_codepage = LIBEWF_CODEPAGE_WINDOWS_1254;
		result           = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "windows-1255" ),
	          12 ) == 0 )
	{
		*header_codepage = LIBEWF_CODEPAGE_WINDOWS_1255;
		result           = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "windows-1256" ),
	          12 ) == 0 )
	{
		*header_codepage = LIBEWF_CODEPAGE_WINDOWS_1256;
		result           = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "windows-1257" ),
	          12 ) == 0 )
	{
		*header_codepage = LIBEWF_CODEPAGE_WINDOWS_1257;
		result           = 1;
	}
	else if( system_string_compare(
	          argument,
	          _SYSTEM_CHARACTER_T_STRING( "windows-1258" ),
	          12 ) == 0 )
	{
		*header_codepage = LIBEWF_CODEPAGE_WINDOWS_1258;
		result           = 1;
	}
	return( result );
}

/* Determines the yes or no value from an argument string
 * Returns 1 if successful or -1 on error
 */
int ewfinput_determine_yes_no(
     const character_t *argument,
     uint8_t *yes_no_value )
{
	static char *function = "ewfinput_determine_yes_no";
	int result            = -1;

	if( argument == NULL )
	{
		notify_warning_printf( "%s: invalid argument string.\n",
		 function );

		return( -1 );
	}
	if( yes_no_value == NULL )
	{
		notify_warning_printf( "%s: invalid yes no value.\n",
		 function );

		return( -1 );
	}
	if( string_compare(
	     argument,
	     _CHARACTER_T_STRING( "yes" ),
	     3 ) == 0 )
	{
		*yes_no_value = 1;
		result        = 1;
	}
	else if( string_compare(
	          argument,
	          _CHARACTER_T_STRING( "no" ),
	          2 ) == 0 )
	{
		*yes_no_value = 0;
		result        = 1;
	}
	return( result );
}

/* Get a string variable
 * Returns 1 if successful, 0 if no input was provided or -1 on error
 */
int ewfinput_get_string_variable(
     FILE *stream, 
     character_t *request_string,
     character_t *string_variable,
     size_t string_variable_size )
{
	character_t *end_of_input  = NULL;
	character_t *result_string = NULL;
	static char *function      = "ewfinput_get_variabl_string";
	ssize_t input_size         = 0;

	if( stream == NULL )
	{
		notify_warning_printf( "%s: invalid output stream.\n",
		 function );

		return( -1 );
	}
	if( request_string == NULL )
	{
		notify_warning_printf( "%s: invalid request string.\n",
		 function );

		return( -1 );
	}
	if( string_variable == NULL )
	{
		notify_warning_printf( "%s: invalid string variable.\n",
		 function );

		return( -1 );
	}
	if( string_variable_size > (size_t) SSIZE_MAX )
	{
		notify_warning_printf( "%s: invalid string variable size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	/* Safe guard the end of the input string
	 */
	string_variable[ string_variable_size ] = 0;

	while( 1 )
	{
		fprintf(
		 stream,
		 "%" PRIs ": ",
		 request_string );

		result_string = string_get_from_stream(
		                 string_variable,
		                 string_variable_size - 1,
		                 stdin );

		if( result_string != NULL )
		{
			end_of_input = string_search(
			                string_variable,
			                (character_t) '\n',
			                string_variable_size );

			/* Input was larger than size of buffer
			 */
			if( end_of_input == NULL )
			{
				/* Flush the stdin stream
				 */
				while( end_of_input == NULL )
				{
					result_string = string_get_from_stream(
					                 string_variable,
					                 string_variable_size - 1,
					                 stdin );

					end_of_input = string_search(
					                string_variable,
					                (character_t) '\n',
					                string_variable_size );

				}
				return( -1 );
			}
			input_size = (ssize_t) ( end_of_input - string_variable );

			if( input_size < 0 )
			{
				return( -1 );
			}
			/* Make sure the string is terminated with an end of string character
			 */
			string_variable[ input_size ] = 0;

			break;
		}
		else
		{
			fprintf( stream, "Error reading input, please try again or terminate using Ctrl^C.\n" );
		}
	}
	if( input_size == 0 )
	{
		return( 0 );
	}
	return( 1 );
}

/* Get a string variable
 * Returns 1 if successful, 0 if no input was provided or -1 on error
 */
int ewfinput_get_string_variable_system_character(
     FILE *stream,
     character_t *request_string,
     system_character_t *string_variable,
     size_t string_variable_size )
{
	system_character_t *result_string = NULL;
	static char *function             = "ewfinput_get_string_variable_system_character";
	int result                        = 0;

	if( sizeof( system_character_t ) != sizeof( character_t ) )
	{
		result_string = (character_t *) memory_allocate(
		                                 sizeof( character_t ) * string_variable_size );

		if( result_string == NULL )
		{
			notify_warning_printf( "%s: unable to create conversion string.\n",
			 function );

			return( -1 );
		}
	}
	else
	{
		result_string = string_variable;
	}
	result = ewfinput_get_string_variable(
	          stream,
	          request_string,
	          result_string,
	          string_variable_size );

	if( result == -1 )
	{
		notify_warning_printf( "%s: unable to get string variable.\n",
		 function );

		if( sizeof( system_character_t ) != sizeof( character_t ) )
		{
			memory_free(
			 result_string );
		}
		return( -1 );
	}
	else if( ( result == 1 )
	      && ( sizeof( system_character_t ) != sizeof( character_t ) ) )
	{
		if( ewfstring_copy_character_string_to_system_string(
		     string_variable,
		     result_string,
		     string_variable_size ) != 1 )
		{
			notify_warning_printf( "%s: unable to set conversion string.\n",
			 function );

			memory_free(
			 result_string );

			return( -1 );
		}
		memory_free(
		 result_string );
	}
	return( result );
}

/* Get a size variable
 * Returns 1 if successful, 0 if no input was provided or -1 on error
 */
int ewfinput_get_size_variable(
     FILE *stream,
     character_t *input_buffer,
     size_t input_buffer_size,
     character_t *request_string,
     uint64_t minimum,
     uint64_t maximum,
     uint64_t default_value,
     uint64_t *size_variable )
{
	character_t *end_of_input  = NULL;
	character_t *result_string = NULL;
	static char *function      = "ewfinput_get_size_variable";
	ssize_t input_size         = 0;

	if( stream == NULL )
	{
		notify_warning_printf( "%s: invalid output stream.\n",
		 function );

		return( -1 );
	}
	if( input_buffer == NULL )
	{
		notify_warning_printf( "%s: invalid input buffer.\n",
		 function );

		return( -1 );
	}
	if( input_buffer_size > (size_t) SSIZE_MAX )
	{
		notify_warning_printf( "%s: invalid input buffer size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( request_string == NULL )
	{
		notify_warning_printf( "%s: invalid request string.\n",
		 function );

		return( -1 );
	}
	if( size_variable == NULL )
	{
		notify_warning_printf( "%s: invalid size variable.\n",
		 function );

		return( -1 );
	}
	/* Safe guard the end of the input buffer
	 */
	input_buffer[ input_buffer_size ] = 0;

	while( 1 )
	{
		fprintf(
		 stream,
		 "%" PRIs " (%" PRIu64 " >= value >= %" PRIu64 ") [%" PRIu64 "]: ",
		 request_string,
		 minimum,
		 maximum,
		 default_value );

		result_string = string_get_from_stream(
		                 input_buffer,
		                 input_buffer_size - 1,
		                 stdin );

		if( result_string != NULL )
		{
			end_of_input = string_search(
			                input_buffer,
			                (character_t) '\n',
			                input_buffer_size );

			/* Input was larger than size of buffer
			 */
			if( end_of_input == NULL )
			{
				/* Flush the stdin stream
				 */
				while( end_of_input == NULL )
				{
					result_string = string_get_from_stream(
					                 input_buffer,
					                 input_buffer_size - 1,
					                 stdin );

					end_of_input = string_search(
					                input_buffer,
					                (character_t) '\n',
					                input_buffer_size );

				}
				return( -1 );
			}
			input_size = (ssize_t) ( end_of_input - input_buffer );

			if( input_size < 0 )
			{
				return( -1 );
			}
			else if( input_size == 0 )
			{
				*size_variable = default_value;

				return( 0 );
			}
			if( string_to_uint64(
			     input_buffer,
			     input_size,
			     size_variable ) != 1 )
			{
				fprintf( stream, "Unable to convert value into number, please try again or terminate using Ctrl^C.\n" );
			}
			else if( ( *size_variable >= minimum )
			      && ( *size_variable <= maximum ) )
			{
				break;
			}
			else
			{
				fprintf( stream, "Value not within specified range, please try again or terminate using Ctrl^C.\n" );
			}
		}
		else
		{
			fprintf( stream, "Error reading input, please try again or terminate using Ctrl^C.\n" );
		}
	}
	return( 1 );
}

/* Get a byte size variable
 * Returns 1 if successful, 0 if no input was provided or -1 on error
 */
int ewfinput_get_byte_size_variable(
     FILE *stream,
     character_t *input_buffer,
     size_t input_buffer_size,
     character_t *request_string,
     uint64_t minimum,
     uint64_t maximum,
     uint64_t default_value,
     uint64_t *byte_size_variable )
{
	character_t minimum_size_string[ 16 ];
	character_t maximum_size_string[ 16 ];
	character_t default_size_string[ 16 ];

	character_t *end_of_input  = NULL;
	character_t *result_string = NULL;
	static char *function      = "ewfinput_get_byte_size_variable";
	ssize_t input_size         = 0;

	if( stream == NULL )
	{
		notify_warning_printf( "%s: invalid output stream.\n",
		 function );

		return( -1 );
	}
	if( input_buffer == NULL )
	{
		notify_warning_printf( "%s: invalid input buffer.\n",
		 function );

		return( -1 );
	}
	if( input_buffer_size > (size_t) SSIZE_MAX )
	{
		notify_warning_printf( "%s: invalid input buffer size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( request_string == NULL )
	{
		notify_warning_printf( "%s: invalid request string.\n",
		 function );

		return( -1 );
	}
	if( byte_size_variable == NULL )
	{
		notify_warning_printf( "%s: invalid byte size variable.\n",
		 function );

		return( -1 );
	}
	if( ewfbyte_size_string_create(
	     minimum_size_string,
	     16,
	     minimum,
	     EWFBYTE_SIZE_STRING_UNIT_MEBIBYTE ) != 1 )
	{
		notify_warning_printf( "%s: unable to create minimum byte size string.\n",
		 function );

		return( -1 );
	}
	if( ewfbyte_size_string_create(
	     default_size_string,
	     16,
	     default_value,
	     EWFBYTE_SIZE_STRING_UNIT_MEBIBYTE ) != 1 )
	{
		notify_warning_printf( "%s: unable to create default byte size string.\n",
		 function );

		return( -1 );
	}
	if( ewfbyte_size_string_create(
	     maximum_size_string,
	     16,
	     maximum,
	     EWFBYTE_SIZE_STRING_UNIT_MEBIBYTE ) != 1 )
	{
		notify_warning_printf( "%s: unable to create maximum byte size string.\n",
		 function );

		return( -1 );
	}
	/* Safe guard the end of the input buffer
	 */
	input_buffer[ input_buffer_size ] = 0;

	while( 1 )
	{
		fprintf( stream, "%" PRIs " (%" PRIs " >= value >= %" PRIs ") [%" PRIs "]: ",
		 request_string, minimum_size_string, maximum_size_string, default_size_string );

		result_string = string_get_from_stream(
		                 input_buffer,
		                 input_buffer_size - 1,
		                 stdin );

		if( result_string != NULL )
		{
			end_of_input = string_search(
			                input_buffer,
			                (character_t) '\n',
			                input_buffer_size );

			/* Input was larger than size of buffer
			 */
			if( end_of_input == NULL )
			{
				/* Flush the stdin stream
				 */
				while( end_of_input == NULL )
				{
					result_string = string_get_from_stream(
					                 input_buffer,
					                 input_buffer_size - 1,
					                 stdin );

					end_of_input = string_search(
					                input_buffer,
					                (character_t) '\n',
					                input_buffer_size );

				}
				return( -1 );
			}
			input_size = (ssize_t) ( end_of_input - input_buffer );

			if( input_size < 0 )
			{
				return( -1 );
			}
			else if( input_size == 0 )
			{
				*byte_size_variable = default_value;

				return( 0 );
			}
			if( ewfbyte_size_string_convert(
			     input_buffer,
			     (size_t) input_size,
			     byte_size_variable ) != 1 )
			{
				fprintf( stream, "Invalid value, please try again or terminate using Ctrl^C.\n" );
			}
			else if( ( *byte_size_variable >= minimum )
			      && ( *byte_size_variable <= maximum ) )
			{
				break;
			}
			else
			{
				fprintf( stream, "Value not within specified range, please try again or terminate using Ctrl^C.\n" );
			}
		}
		else
		{
			fprintf( stream, "Error reading input, please try again or terminate using Ctrl^C.\n" );
		}
	}
	return( 1 );
}

/* Get a fixed value string variable
 * Returns 1 if successful, 0 if no input was provided or -1 on error
 */
int ewfinput_get_fixed_string_variable(
     FILE *stream,
     character_t *input_buffer,
     size_t input_buffer_size,
     character_t *request_string,
     character_t **values,
     uint8_t amount_of_values,
     uint8_t default_value,
     character_t **fixed_string_variable )
{

	character_t *end_of_input  = NULL;
	character_t *result_string = NULL;
	static char *function      = "ewfinput_get_fixed_value";
	size_t value_length        = 0;
	ssize_t input_size         = 0;
	uint8_t value_iterator     = 0;

	if( stream == NULL )
	{
		notify_warning_printf( "%s: invalid output stream.\n",
		 function );

		return( -1 );
	}
	if( input_buffer == NULL )
	{
		notify_warning_printf( "%s: invalid input buffer.\n",
		 function );

		return( -1 );
	}
	if( input_buffer_size > (size_t) SSIZE_MAX )
	{
		notify_warning_printf( "%s: invalid input buffer size value exceeds maximum.\n",
		 function );

		return( -1 );
	}
	if( request_string == NULL )
	{
		notify_warning_printf( "%s: invalid request string.\n",
		 function );

		return( -1 );
	}
	if( default_value >= amount_of_values )
	{
		notify_warning_printf( "%s: default value exceeds amount.\n",
		 function );

		return( -1 );
	}
	if( fixed_string_variable == NULL )
	{
		notify_warning_printf( "%s: invalid fixed string variable.\n",
		 function );

		return( -1 );
	}
	while( 1 )
	{
		fprintf(
		 stream,
		 "%" PRIs " (",
		 request_string );

		for( value_iterator = 0; value_iterator < amount_of_values; value_iterator++ )
		{
			if( value_iterator > 0 )
			{
				fprintf( stream, ", " );
			}
			fprintf(
			 stream,
			 "%" PRIs "",
			 values[ value_iterator ] );
		}
		fprintf(
		 stream,
		 ") [%" PRIs "]: ",
		 values[ default_value ] );

		result_string = string_get_from_stream(
		                 input_buffer,
		                 input_buffer_size - 1,
		                 stdin );

		if( result_string != NULL )
		{
			end_of_input = string_search(
			                input_buffer,
			                (character_t) '\n',
			                input_buffer_size );

			/* Input was larger than size of buffer
			 */
			if( end_of_input == NULL )
			{
				/* Flush the stdin stream
				 */
				while( end_of_input == NULL )
				{
					result_string = string_get_from_stream(
					                 input_buffer,
					                 input_buffer_size - 1,
					                 stdin );

					end_of_input = string_search(
					                input_buffer,
					                (character_t) '\n',
					                input_buffer_size );

				}
				return( -1 );
			}
			input_size = (ssize_t) ( end_of_input - input_buffer );

			if( input_size < 0 )
			{
				return( -1 );
			}
			else if( input_size == 0 )
			{
				*fixed_string_variable = values[ default_value ];

				return( 0 );
			}
			for( value_iterator = 0; value_iterator < amount_of_values; value_iterator++ )
			{
				value_length = string_length(
						values[ value_iterator ] );

				if( string_compare(
				     input_buffer,
				     values[ value_iterator ],
				     value_length ) == 0 )
				{
					/* Make sure no trailing characters were given
					 */
					if( input_buffer[ value_length ] == (character_t) '\n' )
					{
fprintf( stderr, "X: %d =? %d\n", input_size, value_length );

						*fixed_string_variable = values[ value_iterator ];

						break;
					}
				}
			}
			fprintf( stream, "Selected option not supported, please try again or terminate using Ctrl^C.\n" );
		}
		else
		{
			fprintf( stream, "Error reading input, please try again or terminate using Ctrl^C.\n" );
		}
	}
	return( 1 );
}

/* Get fixed value input from the user
 * The first value is considered the default value
 */
character_t *ewfinput_get_fixed_value(
              FILE *stream,
              character_t *request_string,
              character_t **values,
              uint8_t amount,
              uint8_t default_value )
{
	character_t user_input_buffer[ 1024 ];

	character_t *user_input_buffer_ptr = &user_input_buffer[ 0 ];
	character_t *user_input            = NULL;
	static char *function              = "ewfinput_get_fixed_value";
	size_t input_length                = 0;
	size_t value_length                = 0;
	uint8_t iterator                   = 0;
	uint8_t value_match                = 0;

	if( stream == NULL )
	{
		notify_warning_printf( "%s: invalid output stream.\n",
		 function );

		return( NULL );
	}
	if( request_string == NULL )
	{
		notify_warning_printf( "%s: invalid request string.\n",
		 function );

		return( NULL );
	}
	if( default_value >= amount )
	{
		notify_warning_printf( "%s: default value exceeds amount.\n",
		 function );

		return( NULL );
	}
	while( 1 )
	{
		fprintf( stream, "%" PRIs " (", request_string );

		for( iterator = 0; iterator < amount; iterator++ )
		{
			if( iterator > 0 )
			{
				fprintf( stream, ", " );
			}
			fprintf( stream, "%" PRIs "", values[ iterator ] );
		}
		fprintf( stream, ") [%" PRIs "]: ", values[ default_value ] );

		user_input_buffer_ptr = string_get_from_stream(
		                         user_input_buffer_ptr,
		                         1023,
		                         stdin );

		if( user_input_buffer_ptr != NULL )
		{
			iterator = 0;

			/* Remove the trailing newline character
			 */
			input_length = string_length(
			                user_input_buffer_ptr ) - 1;

			/* Check if the default value was selected
			 */
			if( input_length == 0 )
			{
				iterator = default_value;

				input_length = string_length(
				                values[ iterator ] );

				value_match  = 1;
			}
			else
			{
				while( iterator < amount )
				{
					value_length = string_length(
					                values[ iterator ] );

					if( string_compare(
					     user_input_buffer_ptr,
					     values[ iterator ],
					     value_length ) == 0 )
					{
						/* Make sure no trailing characters were given
						 */
						if( user_input_buffer_ptr[ value_length ] == (character_t) '\n' )
						{
							value_match = 1;

							break;
						}
					}
					iterator++;
				}
			}
		}
		else
		{
			fprintf( stream, "Error reading input, please try again or terminate using Ctrl^C.\n" );
		}
		if( value_match == 1 )
		{
			value_length = string_length(
			                values[ iterator ] );

#if defined( MEMWATCH )
			/* One additional character required for end of string
			 */
			user_input = memory_allocate(
			              sizeof( character_t ) * ( value_length + 1 ) );

			if( user_input == NULL )
			{
				notify_warning_printf( "%s: unable to create string.\n",
				 function );

				return( NULL );
			}
			if( string_copy(
			     user_input,
			     values[ iterator ],
			     value_length ) == NULL  )
			{
				notify_warning_printf( "%s: unable to copy string.\n",
				 function );

				memory_free(
				 user_input );

				return( NULL );
			}
			/* Make sure the string is terminated with an end of string character
			 */
			user_input[ input_length ] = 0;
#else
			user_input = string_duplicate(
			              values[ iterator ],
			              value_length + 1 );

			if( user_input == NULL )
			{
				notify_warning_printf( "%s: unable to create string.\n",
				 function );

				return( NULL );
			}
#endif
			break;
		}
		else
		{
			fprintf( stream, "Selected option not supported, please try again or terminate using Ctrl^C.\n" );
		}
	}
	return( user_input );
}

