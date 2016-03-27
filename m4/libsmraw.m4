dnl Functions for libsmraw

dnl Function to detect if libsmraw is available
AC_DEFUN([AC_CHECK_LIBSMRAW],
 [AC_CHECK_HEADERS([libsmraw.h])

 AS_IF(
  [test "x$ac_cv_header_libsmraw_h" = xno],
  [ac_cv_libsmraw=no],
  [ac_cv_libsmraw=yes
  AC_CHECK_LIB(
   smraw,
   libsmraw_get_version,
   [],
   [ac_cv_libsmraw=no])

  dnl Handle functions
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_initialize,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_free,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_signal_abort,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_open,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_close,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_read_buffer,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_seek_offset,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_write_buffer,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])

  AS_IF(
   [test "x$ac_cv_enable_wide_character_type" != xno],
   [AC_CHECK_LIB(
    smraw,
    libsmraw_handle_open_wide,
    [ac_cv_libsmraw_dummy=yes],
    [ac_cv_libsmraw=no])
   ])

  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_get_media_size,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_get_bytes_per_sector,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_get_utf8_information_value
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_get_utf16_information_value
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])

  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_set_maximum_segment_size,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_set_media_size,
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_set_utf8_integrity_hash_value
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  AC_CHECK_LIB(
   smraw,
   libsmraw_handle_set_utf16_integrity_hash_value
   [ac_cv_libsmraw_dummy=yes],
   [ac_cv_libsmraw=no])
  ])

 AS_IF(
  [test "x$ac_cv_libsmraw" = xyes],
  [AC_SUBST(
   [HAVE_LIBSMRAW],
   [1]) ],
  [AC_SUBST(
   [HAVE_LIBSMRAW],
   [0])
  ])
 ])

