/*
 * ewfinfo
 * Shows information stored in an EWF file
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

#include "../libewf/libewf_includes.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>

#if defined( HAVE_UNISTD_H )
#include <unistd.h>
#endif

#if defined( HAVE_STDLIB_H )
#include <stdlib.h>
#endif

/* If libtool DLL support is enabled set LIBEWF_DLL_EXPORT
 * before including libewf.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBEWF_DLL_IMPORT
#endif

#include <libewf.h>

#include "../libewf/libewf_char.h"
#include "../libewf/libewf_common.h"
#include "../libewf/libewf_string.h"

#include "ewfcommon.h"
#include "ewfgetopt.h"
#include "ewfglob.h"
#include "ewfoutput.h"
#include "ewfsignal.h"
#include "ewfstring.h"

/* Prints the executable usage information
 */
void usage( void )
{
	fprintf( stdout, "Usage: ewfinfo [ -d date_format ] [ -himvV ] ewf_files\n\n" );

	fprintf( stdout, "\t-d: specify the date format, options: dm (day/month is default), md (month/day), iso8601\n" );
	fprintf( stdout, "\t-e: only show EWF read error information\n" );
	fprintf( stdout, "\t-h: shows this help\n" );
	fprintf( stdout, "\t-i: only show EWF acquiry information\n" );
	fprintf( stdout, "\t-m: only show EWF media information\n" );
	fprintf( stdout, "\t-v: verbose output to stderr\n" );
	fprintf( stdout, "\t-V: print version\n" );
}

/* The main program
 */
#if defined( HAVE_WIDE_CHARACTER_SUPPORT_FUNCTIONS )
int wmain( int argc, wchar_t * const argv[] )
#else
int main( int argc, char * const argv[] )
#endif
{
	uint8_t guid[ 16 ];

	LIBEWF_CHAR *program       = _S_LIBEWF_CHAR( "ewfinfo" );

#if !defined( HAVE_GLOB_H )
	EWFGLOB *glob              = NULL;
	int32_t glob_count         = 0;
#endif
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
        CHAR_T *error_string       = NULL;
#endif
	LIBEWF_HANDLE *handle      = NULL;
	char *file_format_string   = NULL;
	INT_T option               = 0;
	size64_t media_size        = 0;
	uint32_t bytes_per_sector  = 0;
	uint32_t amount_of_sectors = 0;
	uint32_t error_granularity = 0;
	int8_t format              = 0;
	int8_t compression_level   = 0;
	int8_t media_type          = 0;
	int8_t media_flags         = 0;
	int8_t volume_type         = 0;
	uint8_t verbose            = 0;
	uint8_t date_format        = LIBEWF_DATE_FORMAT_DAYMONTH;
	char info_option           = 'a';

	ewfsignal_initialize();

	ewfoutput_version_fprint( stdout, program );

	while( ( option = ewfgetopt( argc, argv, _S_CHAR_T( "d:himvV" ) ) ) != (INT_T) -1 )
	{
		switch( option )
		{
			case (INT_T) '?':
			default:
				fprintf( stderr, "Invalid argument: %" PRIs "\n", argv[ optind ] );

				usage();

				return( EXIT_FAILURE );

			case (INT_T) 'd':
				if( CHAR_T_COMPARE( optarg, _S_CHAR_T( "md" ), 3 ) == 0 )
				{
					date_format = LIBEWF_DATE_FORMAT_MONTHDAY;
				}
				else if( CHAR_T_COMPARE( optarg, _S_CHAR_T( "iso8601" ), 8 ) == 0 )
				{
					date_format = LIBEWF_DATE_FORMAT_ISO8601;
				}
				else if( CHAR_T_COMPARE( optarg, _S_CHAR_T( "dm" ), 3 ) != 0 )
				{
					fprintf( stderr, "Unsupported date format: %" PRIs " using default day/month.\n", optarg );
				}
				break;

			case (INT_T) 'e':
				if( info_option != 'a' )
				{
					fprintf( stderr, "Conflicting options: %" PRIc " and %c\n", option, info_option );

					usage();

					return( EXIT_FAILURE );
				}
				info_option = 'e';

				break;

			case (INT_T) 'h':
				usage();

				return( EXIT_SUCCESS );

			case (INT_T) 'i':
				if( info_option != 'a' )
				{
					fprintf( stderr, "Conflicting options: %" PRIc " and %c\n", option, info_option );

					usage();

					return( EXIT_FAILURE );
				}
				info_option = 'i';

				break;

			case (INT_T) 'm':
				if( info_option != 'a' )
				{
					fprintf( stderr, "Conflicting options: %" PRIc " and %c\n", option, info_option );

					usage();

					return( EXIT_FAILURE );
				}
				info_option = 'm';

				break;

			case (INT_T) 'v':
				verbose = 1;

				break;

			case (INT_T) 'V':
				ewfoutput_copyright_fprint( stdout );

				return( EXIT_SUCCESS );
		}
	}
	if( optind == argc )
	{
		fprintf( stderr, "Missing EWF image file(s).\n" );

		usage();

		return( EXIT_FAILURE );
	}
	libewf_set_notify_values( stderr, verbose );

#if !defined( HAVE_GLOB_H )
	glob = ewfglob_alloc();

	if( glob == NULL )
	{
		fprintf( stderr, "Unable to create glob.\n" );

		return( EXIT_FAILURE );
	}
	glob_count = ewfglob_resolve( glob, &argv[ optind ], ( argc - optind ) );

	if( glob_count <= 0 )
	{
		fprintf( stderr, "Unable to resolve glob.\n" );

		ewfglob_free( glob );

		return( EXIT_FAILURE );
	}
	handle = libewf_open( glob->results, glob->amount, LIBEWF_OPEN_READ );

	ewfglob_free( glob );
#else
	handle = libewf_open( &argv[ optind ], ( argc - optind ), LIBEWF_OPEN_READ );
#endif

	if( handle == NULL )
	{
#if defined( HAVE_STRERROR_R ) || defined( HAVE_STRERROR )
		if( errno != 0 )
		{
			error_string = ewfstring_strerror( errno );
		}
		if( error_string != NULL )
		{
			fprintf( stderr, "Unable to open EWF file(s) with failure: %" PRIs ".\n",
			 error_string );

			libewf_common_free( error_string );
		}
		else
		{
			fprintf( stderr, "Unable to open EWF file(s).\n" );
		}
#else
		fprintf( stderr, "Unable to open EWF file(s).\n" );
#endif

		return( EXIT_FAILURE );
	}
	if( libewf_parse_header_values( handle, date_format ) != 1 )
	{
		fprintf( stderr, "Unable to parse header values.\n" );
	}
	if( libewf_get_format( handle, &format ) != 1 )
	{
		fprintf( stderr, "Unable to determine format.\n" );
	}
	else if( verbose == 1 )
	{
		switch( format )
		{
			case LIBEWF_FORMAT_EWF:
				file_format_string = "original EWF";
				break;

			case LIBEWF_FORMAT_SMART:
				file_format_string = "SMART";
				break;

			case LIBEWF_FORMAT_FTK:
				file_format_string = "FTK Imager";
				break;

			case LIBEWF_FORMAT_ENCASE1:
				file_format_string = "EnCase 1";
				break;

			case LIBEWF_FORMAT_ENCASE2:
				file_format_string = "EnCase 2";
				break;

			case LIBEWF_FORMAT_ENCASE3:
				file_format_string = "EnCase 3";
				break;

			case LIBEWF_FORMAT_ENCASE4:
				file_format_string = "EnCase 4";
				break;

			case LIBEWF_FORMAT_ENCASE5:
				file_format_string = "EnCase 5";
				break;

			case LIBEWF_FORMAT_ENCASE6:
				file_format_string = "EnCase 6";
				break;

			case LIBEWF_FORMAT_LINEN5:
				file_format_string = "linen 5";
				break;

			case LIBEWF_FORMAT_LINEN6:
				file_format_string = "linen 6";
				break;

			case LIBEWF_FORMAT_EWFX:
				file_format_string = "extended EWF (libewf)";
				break;

			case LIBEWF_FORMAT_UNKNOWN:
			default:
				file_format_string = "unknown";
				break;

		}
		fprintf( stdout, "File format:\t\t\t%s\n\n", file_format_string );
	}
	if( ( info_option == 'a' ) || ( info_option == 'i' ) )
	{
		fprintf( stdout, "Acquiry information\n" );

		ewfoutput_header_values_fprint( stdout, handle );

		fprintf( stdout, "\n" );
	}
	if( ( info_option == 'a' ) || ( info_option == 'm' ) )
	{
		fprintf( stdout, "Media information\n" );

		if( ( format != LIBEWF_FORMAT_EWF )
		 && ( format != LIBEWF_FORMAT_SMART ) )
		{
			if( libewf_get_media_type( handle, &media_type ) != 1 )
			{
				fprintf( stderr, "Unable to determine media type.\n" );
			}
			else if( media_type == LIBEWF_MEDIA_TYPE_REMOVABLE )
			{
				fprintf( stdout, "\tMedia type:\t\tremovable disk\n" );
			}
			else if( media_type == LIBEWF_MEDIA_TYPE_FIXED )
			{
				fprintf( stdout, "\tMedia type:\t\tfixed disk\n" );
			}
			else
			{
				fprintf( stdout, "\tMedia type:\t\tunknown (0x%" PRIx8 ")\n", media_type );
			}
			if( libewf_get_media_flags( handle, &media_flags ) != 1 )
			{
				fprintf( stderr, "Unable to determine media flags.\n" );
			}
			else if( verbose == 1 )
			{
				fprintf( stdout, "\tMedia flags:\t\t0x%" PRIx8 "\n", media_flags );
			}
			if( libewf_get_volume_type( handle, &volume_type ) != 1 )
			{
				fprintf( stderr, "Unable to determine volume type.\n" );
			}
			else if( volume_type == LIBEWF_VOLUME_TYPE_LOGICAL )
			{
				fprintf( stdout, "\tMedia is physical:\tno\n" );
			}
			else if( volume_type == LIBEWF_VOLUME_TYPE_PHYSICAL )
			{
				fprintf( stdout, "\tMedia is physical:\tyes\n" );
			}
			else
			{
				fprintf( stdout, "\tVolume type:\t\tunknown (0x%" PRIx8 ")\n", volume_type );
			}
		}
		if( libewf_get_amount_of_sectors( handle, &amount_of_sectors ) == 1 )
		{
			fprintf( stdout, "\tAmount of sectors:\t%" PRIu32 "\n", amount_of_sectors );
		}
		else
		{
			fprintf( stderr, "Unable to determine amount of sectors.\n" );
		}
		if( libewf_get_bytes_per_sector( handle, &bytes_per_sector ) == 1 )
		{
			fprintf( stdout, "\tBytes per sector:\t%" PRIu32 "\n", bytes_per_sector );
		}
		else
		{
			fprintf( stderr, "Unable to determine bytes per sector.\n" );
		}
		if( libewf_get_media_size( handle, &media_size ) == 1 )
		{
			fprintf( stdout, "\tMedia size:\t\t%" PRIu64 "\n", media_size );
		}
		else
		{
			fprintf( stderr, "Unable to determine media size.\n" );
		}
		if( ( format == LIBEWF_FORMAT_ENCASE5 )
		 || ( format == LIBEWF_FORMAT_ENCASE6 )
		 || ( format == LIBEWF_FORMAT_LINEN5 )
		 || ( format == LIBEWF_FORMAT_LINEN6 )
		 || ( format == LIBEWF_FORMAT_EWFX ) )
		{
			if( libewf_get_error_granularity( handle, &error_granularity ) == 1 )
			{
				fprintf( stdout, "\tError granularity:\t%" PRIu32 "\n", error_granularity );
			}
			else
			{
				fprintf( stderr, "Unable to determine error granularity.\n" );
			}
			if( libewf_get_compression_level( handle, &compression_level ) == 1 )
			{
				if( compression_level == LIBEWF_COMPRESSION_NONE )
				{
					fprintf( stdout, "\tCompression type:\tno compression\n" );
				}
				else if( compression_level == LIBEWF_COMPRESSION_FAST )
				{
					fprintf( stdout, "\tCompression type:\tgood (fast) compression\n" );
				}
				else if( compression_level == LIBEWF_COMPRESSION_BEST )
				{
					fprintf( stdout, "\tCompression type:\tbest compression\n" );
				}
				else
				{
					fprintf( stdout, "\tCompression type:\tunknown compression\n" );
				}
			}
			else
			{
				fprintf( stderr, "Unable to determine compression level.\n" );
			}
			if( libewf_get_guid( handle, guid, 16 ) == 1 )
			{
				fprintf( stdout, "\tGUID:\t\t\t%.2" PRIx8 "%.2" PRIx8 "%.2" PRIx8 "%.2" PRIx8 "-%.2" PRIx8
				                 "%.2" PRIx8 "-%.2" PRIx8 "%.2" PRIx8 "-%.2" PRIx8 "%.2" PRIx8 "-%.2" PRIx8
				                 "%.2" PRIx8 "%.2" PRIx8 "%.2" PRIx8 "%.2" PRIx8 "%.2" PRIx8 "\n",
					guid[ 0 ], guid[ 1 ], guid[ 2 ], guid[ 3 ], guid[ 4 ], guid[ 5 ], guid[ 6 ], guid[ 7 ],
					guid[ 8 ], guid[ 9 ], guid[ 10 ], guid[ 11 ], guid[ 12 ], guid[ 13 ], guid[ 14 ], guid[ 15 ]
				);
			}
		}
		ewfoutput_hash_values_fprint( stdout, handle );

		fprintf( stdout, "\n" );
	}
	if( ( info_option == 'a' ) || ( info_option == 'e' ) )
	{
		ewfoutput_acquiry_errors_fprint( stdout, handle );
	}
	if( libewf_close( handle ) != 0 )
	{
		fprintf( stderr, "Unable to close EWF file(s).\n" );

		return( EXIT_FAILURE );
	}
	return( EXIT_SUCCESS );
}

