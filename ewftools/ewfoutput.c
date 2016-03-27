/*
 * Output functions
 *
 * Copyright (c) 2006-2010, Joachim Metz <jbmetz@users.sourceforge.net>
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
#include <memory.h>
#include <types.h>

#include <libcstring.h>

#if defined( HAVE_STDLIB_H ) || defined( WINAPI )
#include <stdlib.h>
#endif

#if defined( HAVE_STRING_H ) || defined( WINAPI )
#include <string.h>
#endif

#if defined( HAVE_SYS_UTSNAME_H )
#include <sys/utsname.h>
#endif

#include <libewf/features.h>

#if defined( HAVE_LOCAL_LIBUNA )
#include <libuna_definitions.h>
#elif defined( HAVE_LIBUNA_H )
#include <libuna.h>
#endif

#if defined( HAVE_LOCAL_LIBBFIO )
#include <libbfio_definitions.h>
#elif defined( HAVE_LIBBFIO )
#include <libbfio.h>
#endif

#if defined( HAVE_LOCAL_LIBFVALUE )
#include <libfvalue_definitions.h>
#elif defined( HAVE_LIBFVALUE )
#include <libfvalue.h>
#endif

#if defined( HAVE_ZLIB_H ) || defined( ZLIB_DLL )
#include <zlib.h>
#endif

#include <libewf.h>

#if defined( HAVE_LOCAL_LIBSMDEV )
#include <libsmdev_definitions.h>
#elif defined( HAVE_LIBSMDEV )
#include <libsmdev.h>
#endif

#if defined( HAVE_LOCAL_LIBSMRAW )
#include <libsmraw_definitions.h>
#elif defined( HAVE_LIBSMRAW )
#include <libsmraw.h>
#endif

#if defined( HAVE_LOCAL_LIBODTOC )
#include <libodtoc_definitions.h>
#elif defined( HAVE_LIBODTOC )
#include <libodtoc.h>
#endif

#if defined( HAVE_OPENSSL_OPENSSLV_H )
#include <openssl/opensslv.h>
#endif

#if defined( HAVE_UUID_UUID_H )
#include <uuid/uuid.h>
#endif

#include <libsystem.h>

#include "ewfoutput.h"

/* Prints the executable version information
 */
void ewfoutput_copyright_fprint(
      FILE *stream )
{
	static char *function = "ewfoutput_copyright_fprint";

	if( stream == NULL )
	{
		libsystem_notify_printf(
		 "%s: invalid stream.\n",
		 function );

		return;
	}
	fprintf(
	 stream,
	 "Copyright (c) 2006-2010, Joachim Metz <%s>.\n"
	 "This is free software; see the source for copying conditions. There is NO\n"
	 "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	 PACKAGE_BUGREPORT );
}

/* Prints the version information to a stream
 */
void ewfoutput_version_fprint(
      FILE *stream,
      const libcstring_system_character_t *program )
{
	static char *function = "ewfoutput_version_fprint";

	if( stream == NULL )
	{
		libsystem_notify_printf(
		 "%s: invalid stream.\n",
		 function );

		return;
	}
	if( program == NULL )
	{
		libsystem_notify_printf(
		 "%s: invalid program name.\n",
		 function );

		return;
	}
	fprintf(
	 stream,
	 "%" PRIs_LIBCSTRING_SYSTEM " %s\n\n",
	 program,
	 LIBEWF_VERSION_STRING );
}

/* Prints the detailed version information to a stream
 */
void ewfoutput_version_detailed_fprint(
      FILE *stream,
      const libcstring_system_character_t *program )
{
	static char *function = "ewfoutput_version_detailed_fprint";

	if( stream == NULL )
	{
		libsystem_notify_printf(
		 "%s: invalid stream.\n",
		 function );

		return;
	}
	if( program == NULL )
	{
		libsystem_notify_printf(
		 "%s: invalid program name.\n",
		 function );

		return;
	}
	fprintf(
	 stream,
	 "%" PRIs_LIBCSTRING_SYSTEM " %s (libewf %s",
	 program,
	 LIBEWF_VERSION_STRING,
	 LIBEWF_VERSION_STRING );

#if defined( HAVE_LIBUNA ) || defined( HAVE_LOCAL_LIBUNA )
	fprintf(
	 stream,
	 ", libuna %s",
	 LIBUNA_VERSION_STRING );
#endif

#if defined( HAVE_LIBBFIO ) || defined( HAVE_LOCAL_LIBBFIO )
	fprintf(
	 stream,
	 ", libbfio %s",
	 LIBBFIO_VERSION_STRING );
#endif

#if defined( HAVE_LIBFVALUE ) || defined( HAVE_LOCAL_LIBFVALUE )
	fprintf(
	 stream,
	 ", libfvalue %s",
	 LIBFVALUE_VERSION_STRING );
#endif

#if defined( HAVE_LIBZ )
	fprintf(
	 stream,
	 ", zlib %s",
	 ZLIB_VERSION );
#endif

#if defined( HAVE_LIBSMDEV ) || defined( HAVE_LOCAL_LIBSMDEV )
	fprintf(
	 stream,
	 ", libsmdev %s",
	 LIBSMDEV_VERSION_STRING );
#endif

#if defined( HAVE_LIBSMRAW ) || defined( HAVE_LOCAL_LIBSMRAW )
	fprintf(
	 stream,
	 ", libsmraw %s",
	 LIBSMRAW_VERSION_STRING );
#endif

#if defined( HAVE_LIBODTOC ) || defined( HAVE_LOCAL_LIBODTOC )
	fprintf(
	 stream,
	 ", libodtoc %s",
	 LIBODTOC_VERSION_STRING );
#endif

#if defined( HAVE_LIBCRYPTO )
	fprintf(
	 stream,
	 ", libcrypto %s",
	 SHLIB_VERSION_NUMBER );
#endif

#if defined( HAVE_LIBUUID )
	fprintf(
	 stream,
	 ", libuuid" );
#endif

	fprintf(
	 stream,
	 ")\n\n" );
}

