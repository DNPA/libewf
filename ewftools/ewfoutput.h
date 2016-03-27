/*
 * ewfoutput
 * User output functions for the libewf tools
 *
 * Copyright (c) 2006-2008, Joachim Metz <forensics@hoffmannbv.nl>,
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

#if !defined( _EWFOUTPUT_H )
#define _EWFOUTPUT_H

#include <common.h>
#include <character_string.h>
#include <system_string.h>

#include <stdio.h>

#include <libewf/handle.h>
#include <libewf/types.h>

#if defined( __cplusplus )
extern "C" {
#endif

void ewfoutput_version_fprint(
      FILE *stream,
      character_t *program );

void ewfoutput_copyright_fprint(
      FILE *stream );

void ewfoutput_error_fprint(
      FILE *stream,
      char *format,
      ... );

void ewfoutput_acquiry_parameters_fprint(
      FILE *stream,
      system_character_t *filename,
      character_t *case_number,
      character_t *description,
      character_t *evidence_number,
      character_t *examiner_name,
      character_t *notes,
      uint8_t media_type,
      uint8_t volume_type,
      int8_t compression_level,
      uint8_t compress_empty_block,
      uint8_t libewf_format,
      off64_t acquiry_offset,
      size64_t acquiry_size,
      size64_t segment_file_size,
      uint32_t sectors_per_chunk,
      uint32_t sector_error_granularity,
      uint8_t read_error_retry,
      uint8_t wipe_block_on_read_error );

void ewfoutput_acquiry_errors_fprint(
      FILE *stream,
      LIBEWF_HANDLE *handle,
      uint32_t *amount_of_errors );

void ewfoutput_crc_errors_fprint(
      FILE *stream,
      LIBEWF_HANDLE *handle,
      uint32_t *amount_of_errors );

void ewfoutput_sessions_fprint(
      FILE *stream,
      LIBEWF_HANDLE *handle,
      uint32_t *amount_of_sessions );

void ewfoutput_header_values_fprint(
      FILE *stream,
      LIBEWF_HANDLE *handle );

void ewfoutput_hash_values_fprint(
      FILE *stream,
      LIBEWF_HANDLE *handle );

void ewfoutput_timestamp_fprint(
      FILE *stream,
      time_t timestamp );

void ewfoutput_bytes_per_second_fprint(
      FILE *stream,
      size64_t bytes,
      time_t seconds );

void ewfoutput_bytes_fprint(
      FILE *stream,
      size64_t bytes );

void ewfoutput_process_status_initialize(
      FILE *stream,
      character_t *string,
      time_t timestamp_start );

void ewfoutput_process_status_fprint(
      size64_t bytes_read,
      size64_t bytes_total );

void ewfoutput_stream_process_status_fprint(
      size64_t bytes_read,
      size64_t bytes_total );

void ewfoutput_process_summary_fprint(
      FILE *stream,
      character_t *string,
      ssize64_t byte_count,
      time_t timestamp_start,
      time_t timestamp_end );

#if defined( __cplusplus )
}
#endif

#endif

