/*
 * Date and time values functions
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
#include <liberror.h>

#if defined( TIME_WITH_SYS_TIME )
#include <sys/time.h>
#include <time.h>
#elif defined( HAVE_SYS_TIME_H )
#include <sys/time.h>
#else
#include <time.h>
#endif

#include "libewf_date_time.h"
#include "libewf_date_time_values.h"
#include "libewf_definitions.h"
#include "libewf_split_values.h"

/* Copies date and time values string from a timestamp
 * The string must be at least 20 characters + the length of the timezone string and/or timezone name of size including the end of string character
 * Returns 1 if successful or -1 on error
 */
int libewf_date_time_values_copy_from_timestamp(
     uint8_t *date_time_values_string,
     size_t date_time_values_string_size,
     time_t timestamp,
     liberror_error_t **error )
{
	struct tm time_elements;

	static char *function = "libewf_date_time_values_copy_from_timestamp";
	int print_count       = 0;

	if( date_time_values_string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid date time values string.",
		 function );

		return( -1 );
	}
	if( date_time_values_string_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid date time values string size.",
		 function );

		return( -1 );
	}
	if( date_time_values_string_size < 20 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: date time values string too small.",
		 function );

		return( -1 );
	}
	if( libewf_date_time_localtime(
	     &timestamp,
	     &time_elements,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to create time elements.",
		 function );

		return( -1 );
	}
	print_count = libcstring_narrow_string_snprintf(
		       (char *) date_time_values_string,
		       date_time_values_string_size,
		       "%04d %02d %02d %02d %02d %02d",
		       time_elements.tm_year + 1900,
		       time_elements.tm_mon + 1,
		       time_elements.tm_mday,
		       time_elements.tm_hour,
		       time_elements.tm_min,
		       time_elements.tm_sec );

	if( ( print_count <= -1 )
	 || ( (size_t) print_count > date_time_values_string_size ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set date time values string.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Copies date and time values string to a timestamp
 * Returns 1 if successful or -1 on error
 */
int libewf_date_time_values_copy_to_timestamp(
     const uint8_t *date_time_values_string,
     size_t date_time_values_string_length,
     time_t *timestamp,
     liberror_error_t **error )
{
	struct tm time_elements;

	libewf_split_values_t *date_time_elements = NULL;
	static char *function                     = "libewf_date_time_values_copy_to_timestamp";

	if( date_time_values_string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid date time values string.",
		 function );

		return( -1 );
	}
	if( date_time_values_string_length > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid date time values string length.",
		 function );

		return( -1 );
	}
	if( timestamp == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid timestamp.",
		 function );

		return( -1 );
	}
	if( libewf_split_values_parse_string(
	     &date_time_elements,
	     date_time_values_string,
	     date_time_values_string_length + 1,
	     (uint8_t) ' ',
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to split date time values string into elements.",
		 function );

		return( -1 );
	}
	if( date_time_elements->number_of_values < 6 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported number of elements in date time values string.",
		 function );

		libewf_split_values_free(
		 &date_time_elements,
	         NULL );

		return( -1 );
	}
	/* Set the year
	 */
	time_elements.tm_year = (int) ( ( ( ( date_time_elements->values[ 0 ] )[ 0 ] - (libcstring_character_t) '0' ) * 1000 )
	                      + ( ( ( date_time_elements->values[ 0 ] )[ 1 ] - (libcstring_character_t) '0' ) * 100 )
	                      + ( ( ( date_time_elements->values[ 0 ] )[ 2 ] - (libcstring_character_t) '0' ) * 10 )
	                      + ( ( date_time_elements->values[ 0 ] )[ 3 ] - (libcstring_character_t) '0' )
	                      - 1900 );

	/* Set the month
	 */
	time_elements.tm_mon = (int) ( ( ( ( date_time_elements->values[ 1 ] )[ 0 ] - (libcstring_character_t) '0' ) * 10 )
	                     + ( ( date_time_elements->values[ 1 ] )[ 1 ] - (libcstring_character_t) '0' )
	                     - 1 );

	/* Set the day of the month
	 */
	time_elements.tm_mday = (int) ( ( ( ( date_time_elements->values[ 2 ] )[ 0 ] - (libcstring_character_t) '0' ) * 10 )
	                      + ( ( date_time_elements->values[ 2 ] )[ 1 ] - (libcstring_character_t) '0' ) );

	/* Set the hour
	 */
	time_elements.tm_hour = (int) ( ( ( ( date_time_elements->values[ 3 ] )[ 0 ] - (libcstring_character_t) '0' ) * 10 )
	                      + ( ( date_time_elements->values[ 3 ] )[ 1 ] - (libcstring_character_t) '0' ) );

	/* Set the minutes
	 */
	time_elements.tm_min = (int) ( ( ( ( date_time_elements->values[ 4 ] )[ 0 ] - (libcstring_character_t) '0' ) * 10 )
	                     + ( ( date_time_elements->values[ 4 ] )[ 1 ] - (libcstring_character_t) '0' ) );

	/* Set the seconds
	 */
	time_elements.tm_sec = (int) ( ( ( ( date_time_elements->values[ 5 ] )[ 0 ] - (libcstring_character_t) '0' ) * 10 )
	                     + ( ( date_time_elements->values[ 5 ] )[ 1 ] - (libcstring_character_t) '0' ) );

	/* Set to ignore the daylight saving time
	 */
	time_elements.tm_isdst = -1;

	if( libewf_split_values_free(
	     &date_time_elements,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free split date time elements.",
		 function );

		return( -1 );
	}
	/* Create a timestamp
	 */
	*timestamp = libewf_date_time_mktime(
	              &time_elements );

	if( *timestamp == (time_t) -1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to create timestamp.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Copies a date time values string to a string in a specific date format
 * The string must be at least 32 characters of size this includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libewf_date_time_values_copy_to_string(
     libcstring_character_t *date_time_values_string,
     size_t date_time_values_string_length,
     int date_format,
     libcstring_character_t *string,
     size_t string_size,
     liberror_error_t **error )
{
	struct tm time_elements;

	const char *day_of_week = NULL;
	const char *month       = NULL;
	static char *function   = "libewf_date_time_values_copy_to_string";
	time_t timestamp        = 0;
	int print_count         = 0;

	if( date_time_values_string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid date time values string.",
		 function );

		return( -1 );
	}
	if( date_time_values_string_length > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid date time values string length.",
		 function );

		return( -1 );
	}
	if( ( date_format != LIBEWF_DATE_FORMAT_CTIME )
	 && ( date_format != LIBEWF_DATE_FORMAT_DAYMONTH )
	 && ( date_format != LIBEWF_DATE_FORMAT_MONTHDAY )
	 && ( date_format != LIBEWF_DATE_FORMAT_ISO8601 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported date format.",
		 function );

		return( -1 );
	}
	if( string == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid string.",
		 function );

		return( -1 );
	}
	if( string_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid string size.",
		 function );

		return( -1 );
	}
	if( string_size < 20 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
		 "%s: string too small.",
		 function );

		return( -1 );
	}
	if( libewf_date_time_values_copy_to_timestamp(
	     date_time_values_string,
	     date_time_values_string_length,
	     &timestamp,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to create timestamp.",
		 function );

		return( -1 );
	}
	if( libewf_date_time_localtime(
	     &timestamp,
	     &time_elements,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to create time elements.",
		 function );

		return( -1 );
	}
	if( date_format == LIBEWF_DATE_FORMAT_CTIME )
	{
		if( string_size < 25 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
			 LIBERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
			 "%s: string too small.",
			 function );

			return( -1 );
		}
		switch( time_elements.tm_wday )
		{
			case 0:
				day_of_week = "Sun";
				break;
			case 1:
				day_of_week = "Mon";
				break;
			case 2:
				day_of_week = "Tue";
				break;
			case 3:
				day_of_week = "Wed";
				break;
			case 4:
				day_of_week = "Thu";
				break;
			case 5:
				day_of_week = "Fri";
				break;
			case 6:
				day_of_week = "Sat";
				break;

			default:
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
				 "%s: unsupported day of the week value.",
				 function );

				return( -1 );
		}
		switch( time_elements.tm_mon )
		{
			case 0:
				month = "Jan";
				break;
			case 1:
				month = "Feb";
				break;
			case 2:
				month = "Mar";
				break;
			case 3:
				month = "Apr";
				break;
			case 4:
				month = "May";
				break;
			case 5:
				month = "Jun";
				break;
			case 6:
				month = "Jul";
				break;
			case 7:
				month = "Aug";
				break;
			case 8:
				month = "Sep";
				break;
			case 9:
				month = "Oct";
				break;
			case 10:
				month = "Nov";
				break;
			case 11:
				month = "Dec";
				break;

			default:
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
				 "%s: unsupported month value.",
				 function );

				return( -1 );
		}
		print_count = libcstring_string_snprintf(
			       string,
			       string_size,
			       "%s %s %2d %02d:%02d:%02d %04d",
			       (char *) day_of_week,
			       (char *) month,
			       time_elements.tm_mday,
			       time_elements.tm_hour,
			       time_elements.tm_min,
			       time_elements.tm_sec,
			       time_elements.tm_year + 1900 );
	}
	else if( date_format == LIBEWF_DATE_FORMAT_MONTHDAY )
	{
		print_count = libcstring_string_snprintf(
			       string,
			       string_size,
			       "%02d/%02d/%04d %02d:%02d:%02d",
			       time_elements.tm_mon + 1,
			       time_elements.tm_mday,
			       time_elements.tm_year + 1900,
			       time_elements.tm_hour,
			       time_elements.tm_min,
			       time_elements.tm_sec );
	}
	else if( date_format == LIBEWF_DATE_FORMAT_DAYMONTH )
	{
		print_count = libcstring_string_snprintf(
			       string,
			       string_size,
			       "%02d/%02d/%04d %02d:%02d:%02d",
			       time_elements.tm_mday,
			       time_elements.tm_mon + 1,
			       time_elements.tm_year + 1900,
			       time_elements.tm_hour,
			       time_elements.tm_min,
			       time_elements.tm_sec );
	}
	else if( date_format == LIBEWF_DATE_FORMAT_ISO8601 )
	{
		print_count = libcstring_string_snprintf(
			       string,
			       string_size,
			       "%04d-%02d-%02dT%02d:%02d:%02d",
			       time_elements.tm_year + 1900,
			       time_elements.tm_mon + 1,
			       time_elements.tm_mday,
			       time_elements.tm_hour,
			       time_elements.tm_min,
			       time_elements.tm_sec );
	}
	if( ( print_count <= -1 )
	 || ( (size_t) print_count > string_size ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set string.",
		 function );

		return( -1 );
	}
	if( date_time_values_string_length > 19 )
	{
		if( date_format == LIBEWF_DATE_FORMAT_ISO8601 )
		{
			if( ( date_time_values_string[ 20 ] == (libcstring_character_t ) '+' )
			 || ( date_time_values_string[ 20 ] == (libcstring_character_t ) '-' ) )
			{
				print_count = libcstring_string_snprintf(
					       &( string[ 19 ] ),
					       7,
					       "%" PRIs_LIBCSTRING "",
					       &( date_time_values_string[ 20 ] ) );
			}
		}
		else
		{
			print_count = libcstring_string_snprintf(
				       &( string[ print_count ] ),
				       string_size - print_count,
				       "%" PRIs_LIBCSTRING "",
				       &( date_time_values_string[ 19 ] ) );
		}
		if( ( print_count <= -1 )
		 || ( (size_t) print_count > string_size ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_SET_FAILED,
			 "%s: unable to set string.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

