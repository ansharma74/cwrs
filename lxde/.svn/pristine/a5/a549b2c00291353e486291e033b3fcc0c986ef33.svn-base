AC_PREREQ(2.53)
AC_INIT(lxconf, 0.1, http://lxde.org/)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE


AC_CONFIG_SRCDIR([src/liblxconf.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AM_PROG_CC_C_O

# Checks for libraries.

pkg_modules="glib-2.0 >= 2.6.0 \
             gthread-2.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

pkg_modules="gtk+-2.0 >= 2.12.0"
PKG_CHECK_MODULES(GTK, [$pkg_modules])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)


AC_ARG_WITH([sqlite3],
  [  --with-sqlite3          Use sqlite3 for database],
  [],
  [with_sqlite3=yes])
if test "$with_sqlite3" = "yes"; then
  AC_CHECK_LIB([sqlite3], [sqlite3_open], , AC_MSG_ERROR([missing zlib]))
fi
AM_CONDITIONAL([HAVE_SQLITE3], [test "$with_sqlite3" = "yes"])



# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([bzero socket])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/liblxconf.pc
])
AC_OUTPUT

echo
echo LxCONF......................................... : Version $VERSION
if test "$with_sqlite3" = "yes"; then
echo DB............................................. : SQLITE3
else
echo DB............................................. : GLIB-KEYFILE
fi
echo Prefix....................................... : $prefix