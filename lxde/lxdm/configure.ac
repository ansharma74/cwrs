#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([lxdm], [0.2.0], [http://lxde.org/])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([src/lxdm.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([Xmu], [XmuClientWindow])
AC_CHECK_LIB([crypt], [crypt])
AC_CHECK_LIB([pam], [pam_open_session])
AC_CHECK_LIB([ck-connector],[ck_connector_open_session])

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([shadow.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([atexit endpwent getspnam getusershell memset putenv strstr])

PKG_CONFIG=pkg-config # FIXME: is this OK?
# Check for ISO Codes, this is taken from gdm
AC_MSG_CHECKING([whether iso-codes has iso-639 domain])
if $PKG_CONFIG --variable=domains iso-codes | grep 639 >/dev/null ; then
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
fi
AC_DEFINE_UNQUOTED([ISO_CODES_PREFIX],["`$PKG_CONFIG --variable=prefix iso-codes`"],[ISO codes prefix])
ISO_CODES=iso-codes

IT_PROG_INTLTOOL([0.40.0])

pkg_modules="gtk+-2.0 >= 2.12.0"
PKG_CHECK_MODULES(GTK, [$pkg_modules])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(CONSOLEKIT, "ck-connector",[
	AC_SUBST(CONSOLEKIT_CFLAGS)
	AC_SUBST(CONSOLEKIT_LIBS)],[
	echo "ConsoleKit devel package not found"
	])

GETTEXT_PACKAGE=lxdm
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

AC_CONFIG_FILES([
	Makefile
	lxdm.spec
	src/Makefile
	po/Makefile.in
	pam/Makefile
	data/Makefile
    data/lxdm.conf
])
AC_CONFIG_FILES([data/lxdm], [chmod +x data/lxdm])
AC_OUTPUT

dnl sanity checks
if test "$sysconfdir" != "/etc"; then
   AC_MSG_WARN(sysconfdir is not /etc.)
   AC_MSG_WARN(Please consider passing --sysconfdir=/etc to configure.)
fi