/*
 * Handle class of libewf .net managed wrapper
 *
 * Copyright (c) 2010-2011, Joachim Metz <jbmetz@users.sourceforge.net>
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
#pragma managed( push, off )
#include <common.h>
#include <memory.h>

#include <stdlib.h>

#include <libcstring.h>

#include <libewf.h>
#pragma managed( pop )

#include <vcclr.h>

#include "ewf.net.h"
#include "ewf.net_handle.h"

#using <mscorlib.dll>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace EWF {

Handle::Handle( System::IntPtr ewf_handle )
{
	this->ewf_handle = ewf_handle;
}

Handle::Handle( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::Handle";

	if( libewf_handle_initialize(
	     &handle,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to create ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	this->ewf_handle = Marshal::ReadIntPtr(
	                      (IntPtr) &handle );
}

Handle::~Handle( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::~Handle";

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_free(
	     &handle,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to free ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
}

int Handle::GetAccessFlagsRead( void )
{
	return( libewf_get_access_flags_read() );
}

int Handle::GetAccessFlagsReadWrite( void )
{
	return( libewf_get_access_flags_read_write() );
}

int Handle::GetAccessFlagsWrite( void )
{
	return( libewf_get_access_flags_write() );
}

int Handle::GetAccessFlagsWriteResume( void )
{
	return( libewf_get_access_flags_write_resume() );
}

bool Handle::CheckFileSignature( System::String^ filename )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error               = NULL;
	pin_ptr<const wchar_t> ewf_filename = nullptr;
	System::String^ error_string        = nullptr;
	System::String^ function            = "Handle::CheckFileSignature";
	int result                          = 0;

	ewf_filename = PtrToStringChars(
	                filename );

	result = libewf_check_file_signature_wide(
	          ewf_filename,
	          &error );

	if( result == -1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to check file signature." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	else if( result == 0 )
	{
		return( false );
	}
	return( true );
}

array<System::String^>^ Handle::Glob( System::String^ filename )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	array<System::String^>^ filenames   = nullptr;
	libewf_error_t *error               = NULL;
	wchar_t **ewf_filenames             = NULL;
	pin_ptr<const wchar_t> ewf_filename = nullptr;
	System::String^ error_string        = nullptr;
	System::String^ function            = "Handle::Glob";
	size_t ewf_filename_length          = 0;
	int ewf_filename_index              = 0;
	int ewf_number_of_filenames         = 0;

	ewf_filename = PtrToStringChars(
	                filename );

	ewf_filename_length = wcslen(
	                       ewf_filename );

	if( libewf_glob_wide(
	     ewf_filename,
	     ewf_filename_length,
	     LIBEWF_FORMAT_UNKNOWN,
	     &ewf_filenames,
	     &ewf_number_of_filenames,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to glob filenames." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	filenames = gcnew array<System::String^, 1>(
	                   ewf_number_of_filenames );

	for( ewf_filename_index = 0;
	     ewf_filename_index < ewf_number_of_filenames;
	     ewf_filename_index++ )
	{
		filenames[ ewf_filename_index ] = gcnew System::String(
		                                           ewf_filenames[ ewf_filename_index ] );
	}
	if( libewf_glob_wide_free(
	     ewf_filenames,
	     ewf_number_of_filenames,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to free globbed filenames." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( filenames );
}

Handle^ Handle::Clone( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error               = NULL;
	libewf_handle_t *destination_handle = NULL;
	libewf_handle_t *source_handle      = NULL;
	System::String^ error_string        = nullptr;
	System::String^ function            = "Handle::Clone";

	Marshal::WriteIntPtr(
	 (IntPtr) &source_handle,
	 this->ewf_handle );

	if( libewf_handle_clone(
	     &destination_handle,
	     source_handle,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to clone ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( gcnew Handle( Marshal::ReadIntPtr(
	                       (IntPtr) &destination_handle ) ) );
}

void Handle::Open( array<System::String^>^ filenames,
                   int access_flags )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_handle_t *handle         = NULL;
	libewf_error_t *error           = NULL;
	System::String^ error_string    = nullptr;
	System::String^ function        = "Handle::Open";
	wchar_t **ewf_filenames         = NULL;
	pin_ptr<const wchar_t> filename = nullptr;
	int ewf_filename_index          = 0;
	int ewf_number_of_filenames     = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	ewf_number_of_filenames = filenames->Length;

	if( ewf_number_of_filenames <= 0 )
	{
		throw gcnew System::Exception(
			"ewf.net " + function + ": missing filenames." );
	}
	ewf_filenames = (wchar_t **) memory_allocate(
	                                sizeof( wchar_t* ) * ewf_number_of_filenames );

	if( ewf_filenames == NULL )
	{
		throw gcnew System::Exception(
		             "ewf.net " + function + ": unable to create filenames." );
	}
	for( ewf_filename_index = 0;
	     ewf_filename_index < ewf_number_of_filenames;
	     ewf_filename_index++ )
	{
		filename = PtrToStringChars(
		            filenames[ ewf_filename_index ] );

		ewf_filenames[ ewf_filename_index ] = (wchar_t *) filename;
	}
	if( libewf_handle_open_wide(
	     handle,
	     (wchar_t * const *) ewf_filenames,
	     ewf_number_of_filenames,
	     access_flags,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to open ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		memory_free(
		 ewf_filenames );

		throw gcnew System::Exception(
			     error_string );
	}
	memory_free(
	 ewf_filenames );
}

void Handle::Close( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::Close";

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_close(
	     handle,
	     &error ) != 0 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to close ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
}

int Handle::ReadBuffer( array<System::Byte>^ buffer,
                        int size )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::ReadBuffer";
	pin_ptr<uint8_t> ewf_buffer  = nullptr;
	size_t read_count            = 0;

	if( size < 0 )
	{
		throw gcnew System::ArgumentException(
			     "ewf.net " + function + ": invalid size" );
	}
	if( size == 0 )
	{
		return( 0 );
	}
	if( size > buffer->Length )
	{
		throw gcnew System::ArgumentException(
			     "ewf.net " + function + ": buffer too small" );
	}
	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	ewf_buffer = &( buffer[ 0 ] );

	read_count = libewf_handle_read_buffer(
	              handle,
	              ewf_buffer,
	              (size_t) size,
	              &error );

	if( read_count == -1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to read buffer from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( (int) read_count );
}

int Handle::ReadRandom( array<System::Byte>^ buffer,
                        int size,
                        System::Int64 offset )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::ReadRandom";
	pin_ptr<uint8_t> ewf_buffer  = nullptr;
	off64_t ewf_offset           = 0;
	size_t read_count            = 0;

	if( size < 0 )
	{
		throw gcnew System::ArgumentException(
			     "ewf.net " + function + ": invalid size" );
	}
	if( size == 0 )
	{
		return( 0 );
	}
	if( size > buffer->Length )
	{
		throw gcnew System::ArgumentException(
			     "ewf.net " + function + ": buffer too small" );
	}
	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	Marshal::WriteInt64(
	 (IntPtr) &ewf_offset,
	 offset );

	ewf_buffer = &( buffer[ 0 ] );

	read_count = libewf_handle_read_random(
	              handle,
	              ewf_buffer,
	              (size_t) size,
	              ewf_offset,
	              &error );

	if( read_count == -1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to read random from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( (int) read_count );
}

int Handle::WriteBuffer( array<System::Byte>^ buffer,
                         int size )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error             = NULL;
	libewf_handle_t *handle           = NULL;
	System::String^ error_string      = nullptr;
	System::String^ function          = "Handle::WriteBuffer";
	pin_ptr<const uint8_t> ewf_buffer = nullptr;
	size_t write_count                = 0;

	if( size < 0 )
	{
		throw gcnew System::ArgumentException(
			     "ewf.net " + function + ": invalid size" );
	}
	if( size == 0 )
	{
		return( 0 );
	}
	if( size > buffer->Length )
	{
		throw gcnew System::ArgumentException(
			     "ewf.net " + function + ": buffer too small" );
	}
	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	ewf_buffer = &( buffer[ 0 ] );

	write_count = libewf_handle_write_buffer(
	               handle,
	               ewf_buffer,
	               (size_t) size,
	               &error );

	if( write_count == -1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to write buffer to ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( (int) write_count );
}

int Handle::WriteRandom( array<System::Byte>^ buffer,
                         int size,
                         System::Int64 offset )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error             = NULL;
	libewf_handle_t *handle           = NULL;
	System::String^ error_string      = nullptr;
	System::String^ function          = "Handle::WriteRandom";
	pin_ptr<const uint8_t> ewf_buffer = nullptr;
	off64_t ewf_offset                = 0;
	size_t write_count                = 0;

	if( size < 0 )
	{
		throw gcnew System::ArgumentException(
			     "ewf.net " + function + ": invalid size" );
	}
	if( size == 0 )
	{
		return( 0 );
	}
	if( size > buffer->Length )
	{
		throw gcnew System::ArgumentException(
			     "ewf.net " + function + ": buffer too small" );
	}
	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	Marshal::WriteInt64(
	 (IntPtr) &ewf_offset,
	 offset );

	ewf_buffer = &( buffer[ 0 ] );

	write_count = libewf_handle_write_random(
	               handle,
	               ewf_buffer,
	               (size_t) size,
	               ewf_offset,
	               &error );

	if( write_count == -1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to write random to ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( (int) write_count );
}

System::Int64 Handle::SeekOffset( System::Int64 offset,
                                  System::IO::SeekOrigin origin )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::SeekOffset";
	off64_t ewf_offset           = 0;
	int ewf_whence               = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	Marshal::WriteInt64(
	 (IntPtr) &ewf_offset,
	 offset );

	if( origin == System::IO::SeekOrigin::Begin )
	{
		ewf_whence = SEEK_SET;
	}
	else if( origin == System::IO::SeekOrigin::Current )
	{
		ewf_whence = SEEK_CUR;
	}
	else if( origin == System::IO::SeekOrigin::End )
	{
		ewf_whence = SEEK_END;
	}
	else
	{
		throw gcnew System::ArgumentException(
			     "ewf.net " + function + ": unsupported origin" );
	}
	ewf_offset = libewf_handle_seek_offset(
	                handle,
	                ewf_offset,
	                ewf_whence,
	                &error );

	if( ewf_offset == -1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to seek offset in ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	offset = Marshal::ReadInt64(
	          (IntPtr) &ewf_offset );

	return( offset );
}

System::Int64 Handle::GetOffset( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::GetOffset";
	off64_t ewf_offset           = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_get_offset(
	     handle,
	     &ewf_offset,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve offset from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( Marshal::ReadInt64(
	         (IntPtr) &ewf_offset ) );
}

System::UInt32 Handle::GetSectorsPerChunk( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error          = NULL;
	libewf_handle_t *handle        = NULL;
	System::String^ error_string   = nullptr;
	System::String^ function       = "Handle::GetSectorsPerChunk";
	uint32_t ewf_sectors_per_chunk = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_get_sectors_per_chunk(
	     handle,
	     &ewf_sectors_per_chunk,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve sectors per chunk from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( Marshal::ReadInt32(
	         (IntPtr) &ewf_sectors_per_chunk ) );
}

void Handle::SetSectorsPerChunk( System::UInt32 sectors_per_chunk )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error          = NULL;
	libewf_handle_t *handle        = NULL;
	System::String^ error_string   = nullptr;
	System::String^ function       = "Handle::SetSectorsPerChunk";
	uint32_t ewf_sectors_per_chunk = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	Marshal::WriteInt32(
	 (IntPtr) &ewf_sectors_per_chunk,
	 sectors_per_chunk );

	if( libewf_handle_set_sectors_per_chunk(
	     handle,
	     ewf_sectors_per_chunk,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to set sectors per chunk in ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
}

System::UInt32 Handle::GetBytesPerSector( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error         = NULL;
	libewf_handle_t *handle       = NULL;
	System::String^ error_string  = nullptr;
	System::String^ function      = "Handle::GetBytesPerSector";
	uint32_t ewf_bytes_per_sector = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_get_bytes_per_sector(
	     handle,
	     &ewf_bytes_per_sector,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve bytes per sector from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( Marshal::ReadInt32(
	         (IntPtr) &ewf_bytes_per_sector ) );
}

void Handle::SetBytesPerSector( System::UInt32 bytes_per_sector )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error         = NULL;
	libewf_handle_t *handle       = NULL;
	System::String^ error_string  = nullptr;
	System::String^ function      = "Handle::SetBytesPerSector";
	uint32_t ewf_bytes_per_sector = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	Marshal::WriteInt32(
	 (IntPtr) &ewf_bytes_per_sector,
	 bytes_per_sector );

	if( libewf_handle_set_bytes_per_sector(
	     handle,
	     ewf_bytes_per_sector,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to set bytes per sectors in ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
}

System::UInt64 Handle::GetNumberOfSectors( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error          = NULL;
	libewf_handle_t *handle        = NULL;
	System::String^ error_string   = nullptr;
	System::String^ function       = "Handle::GetNumberOfSectors";
	uint64_t ewf_number_of_sectors = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_get_number_of_sectors(
	     handle,
	     &ewf_number_of_sectors,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve number of sectors from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( Marshal::ReadInt64(
	         (IntPtr) &ewf_number_of_sectors ) );
}

System::UInt32 Handle::GetChunkSize( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::GetChunkSize";
	size32_t ewf_chunk_size      = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_get_chunk_size(
	     handle,
	     &ewf_chunk_size,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve chunk size from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( Marshal::ReadInt32(
	         (IntPtr) &ewf_chunk_size ) );
}

System::UInt32 Handle::GetErrorGranularity( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error          = NULL;
	libewf_handle_t *handle        = NULL;
	System::String^ error_string   = nullptr;
	System::String^ function       = "Handle::GetErrorGranularity";
	uint32_t ewf_error_granularity = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_get_error_granularity(
	     handle,
	     &ewf_error_granularity,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve error granularity from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( Marshal::ReadInt32(
	         (IntPtr) &ewf_error_granularity ) );
}

void Handle::SetErrorGranularity( System::UInt32 error_granularity )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error          = NULL;
	libewf_handle_t *handle        = NULL;
	System::String^ error_string   = nullptr;
	System::String^ function       = "Handle::SetErrorGranularity";
	uint32_t ewf_error_granularity = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	Marshal::WriteInt32(
	 (IntPtr) &ewf_error_granularity,
	 error_granularity );

	if( libewf_handle_set_error_granularity(
	     handle,
	     ewf_error_granularity,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to set error granularity in ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
}

System::UInt64 Handle::GetMediaSize( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::GetMediaSize";
	size64_t ewf_media_size      = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_get_media_size(
	     handle,
	     &ewf_media_size,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve media size from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( Marshal::ReadInt64(
	         (IntPtr) &ewf_media_size ) );
}

void Handle::SetMediaSize( System::UInt64 media_size )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error          = NULL;
	libewf_handle_t *handle        = NULL;
	System::String^ error_string   = nullptr;
	System::String^ function       = "Handle::SetMediaSize";
	size64_t ewf_media_size        = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	Marshal::WriteInt64(
	 (IntPtr) &ewf_media_size,
	 media_size );

	if( libewf_handle_set_media_size(
	     handle,
	     ewf_media_size,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to set media size in ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
}

System::Byte Handle::GetMediaType( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::GetMediaType";
	uint8_t ewf_media_type       = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_get_media_type(
	     handle,
	     &ewf_media_type,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve media type from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( Marshal::ReadByte(
	         (IntPtr) &ewf_media_type ) );
}

void Handle::SetMediaType( System::Byte media_type )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error          = NULL;
	libewf_handle_t *handle        = NULL;
	System::String^ error_string   = nullptr;
	System::String^ function       = "Handle::SetMediaType";
	uint8_t ewf_media_type         = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	Marshal::WriteByte(
	 (IntPtr) &ewf_media_type,
	 media_type );

	if( libewf_handle_set_media_type(
	     handle,
	     ewf_media_type,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to set media type in ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
}

System::Byte Handle::GetMediaFlags( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::GetMediaFlags";
	uint8_t ewf_media_flags      = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_get_media_flags(
	     handle,
	     &ewf_media_flags,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve media flags from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( Marshal::ReadByte(
	         (IntPtr) &ewf_media_flags ) );
}

void Handle::SetMediaFlags( System::Byte media_flags )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error          = NULL;
	libewf_handle_t *handle        = NULL;
	System::String^ error_string   = nullptr;
	System::String^ function       = "Handle::SetMediaFlags";
	uint8_t ewf_media_flags        = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	Marshal::WriteByte(
	 (IntPtr) &ewf_media_flags,
	 media_flags );

	if( libewf_handle_set_media_flags(
	     handle,
	     ewf_media_flags,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to set media flags in ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
}

System::Byte Handle::GetFormat( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error        = NULL;
	libewf_handle_t *handle      = NULL;
	System::String^ error_string = nullptr;
	System::String^ function     = "Handle::GetFormat";
	uint8_t ewf_format           = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	if( libewf_handle_get_format(
	     handle,
	     &ewf_format,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve format from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	return( Marshal::ReadByte(
	         (IntPtr) &ewf_format ) );
}

void Handle::SetFormat( System::Byte format )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error          = NULL;
	libewf_handle_t *handle        = NULL;
	System::String^ error_string   = nullptr;
	System::String^ function       = "Handle::SetFormat";
	uint8_t ewf_format             = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	Marshal::WriteByte(
	 (IntPtr) &ewf_format,
	 format );

	if( libewf_handle_set_format(
	     handle,
	     ewf_format,
	     &error ) != 1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to set format in ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
}

int Handle::GetNumberOfHeaderValues( void )
{
	char ewf_error_string[ EWF_NET_ERROR_STRING_SIZE ];

	libewf_error_t *error            = NULL;
	libewf_handle_t *handle          = NULL;
	System::String^ error_string     = nullptr;
	System::String^ function         = "Handle::GetNumberOfHeaderValues";
	uint32_t number_of_header_values = 0;
	int result                       = 0;

	Marshal::WriteIntPtr(
	 (IntPtr) &handle,
	 this->ewf_handle );

	result = libewf_handle_get_number_of_header_values(
	          handle,
	          &number_of_header_values,
	          &error );

	if( result == -1 )
	{
		error_string = gcnew System::String(
		                      "ewf.net " + function + ": unable to retrieve number of header values from ewf handle." );

		if( libewf_error_backtrace_sprint(
		     error,
		     &( ewf_error_string[ 1 ] ),
		     EWF_NET_ERROR_STRING_SIZE - 1 ) > 0 )
		{
			ewf_error_string[ 0 ] = '\n';

			error_string = System::String::Concat(
			                error_string,
			                gcnew System::String(
			                       ewf_error_string ) );
		}
		libewf_error_free(
		 &error );

		throw gcnew System::Exception(
			     error_string );
	}
	if( number_of_header_values > (uint32_t) INT_MAX )
	{
		throw gcnew System::Exception(
			     "ewf.net " + function + ": number of header values exceeds maximum." );
	}
	return( (int) number_of_header_values );
}

} // namespace EWF

