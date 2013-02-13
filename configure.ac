#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([sshlpr], [0.99], [sqds2@cam.ac.uk])
AC_CONFIG_SRCDIR([src/sshlprd.cpp])
AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([socket])



# where to install the CUPS backend
AC_ARG_WITH([cups-backend],
	[AS_HELP_STRING([--with-cups-backend],[CUPS backend directory, e.g. /usr/lib/cups/backend])],
	[],
	[with_cups_backend=/usr/lib/cups/backend])
AC_SUBST([CUPS_BACKEND], ["$with_cups_backend"])

# where to have our socket
AC_ARG_WITH(
	[socket-dir],
	[AS_HELP_STRING([--with-socket-dir],[Socket directory, e.g. /var/sshlpr])],
	[],
	[with_socket_dir=/var/sshlpr])
AC_SUBST([SSHLPRD_SOCKDIR], ["$with_socket_dir"])
AC_DEFINE_UNQUOTED([SSHLPRD_SOCKPATH], ["$with_socket_dir/sshlprd.sock"], [Path to socket])

# which user to setuid our programs to
AC_ARG_WITH(
	[lpuser],
	[AS_HELP_STRING([--with-lpuser],
              [Username to setuid the CUPS backend and client])],
	[],
	[with_lpuser=lp])
AC_SUBST([LPUSER], ["$with_lpuser"])
AC_DEFINE_UNQUOTED([LPUSER], ["$with_lpuser"], [User to run sshlprd as])

AC_CONFIG_FILES([ src/Makefile Makefile])
AC_OUTPUT

