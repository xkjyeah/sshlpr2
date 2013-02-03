#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src/sshlprd.cpp])
AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall])

AS_IF([test "x$with_foo" != "xno"],
      [CHECK_FOR_FOO([have_foo=yes], [have_foo=no])],
      [have_foo=no])

AS_IF([test "x$have_foo" = "xyes"],
      [do_whatever_needed],
      [AS_IF([test "x$with_foo" = "xyes"],
             [AC_MSG_ERROR([foo requested but not found])
      ])
])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([socket])

AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
