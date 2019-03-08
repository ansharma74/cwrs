AC_PREREQ([2.63])

AC_INIT([libxklavier],
        [5.3],
        [http://bugs.freedesktop.org/enter_bug.cgi?product=libxklavier],
        [libxklavier])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([libxklavier/xklavier.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.10 dist-bzip2 -Wno-portability])
AM_MAINTAINER_MODE([enable])

# Support silent build rules, requires at least automake-1.11. Disable
# by either passing --disable-silent-rules to configure or passing V=1
# to make
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl for DLL
dnl http://sources.redhat.com/autobook/autobook/autobook_91.html
VERSION_INFO=19:0:3
AC_SUBST(VERSION_INFO)

# Check for programs
AC_PROG_CC
AM_ICONV

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT

# Gettext stuff, needed by AM_ICONV
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])

# Introspection
GOBJECT_INTROSPECTION_CHECK([1.30.0])

m4_ifset([GTK_DOC_CHECK], [
GTK_DOC_CHECK(1.0)
], [
AM_CONDITIONAL(ENABLE_GTK_DOC, false)
AM_CONDITIONAL(GTK_DOC_USE_LIBTOOL, false)
])

AC_DEFUN([jm_LANGINFO_CODESET],
[
  AC_CHECK_HEADERS(langinfo.h)
  AC_CHECK_FUNCS(nl_langinfo)

  AC_CACHE_CHECK([for nl_langinfo and CODESET], jm_cv_langinfo_codeset,
    [AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <langinfo.h>]],
                                     [[char* cs = nl_langinfo(CODESET);]])],
                    [jm_cv_langinfo_codeset=yes],
                    [jm_cv_langinfo_codeset=no])
    ])
  if test $jm_cv_langinfo_codeset = yes; then
    AC_DEFINE(HAVE_LANGINFO_CODESET, 1,
      [Define if you have <langinfo.h> and nl_langinfo(CODESET).])
  fi
])

dnl 

jm_LANGINFO_CODESET
AC_CHECK_FUNCS(setlocale)

PKG_CHECK_MODULES(X, \
	x11)
AC_SUBST(X_LIBS)
AC_SUBST(X_CFLAGS)

AC_ARG_WITH( xkb_base,
             [  --with-xkb-base=DIR        XKB base path (by default it is /usr/share/X11/xkb)],
             xkb_base="$withval",
             xorgdata=`$PKG_CONFIG --variable=datarootdir xorg-server`
             if test -z "$xorgdata" ; then
               xkb_base="/usr/share/X11/xkb"
             else
               xkb_base="$xorgdata/X11/xkb"
             fi )

AC_DEFINE_UNQUOTED(XKB_BASE,"${xkb_base}",Base for XKB configuration)

AC_PATH_PROG(XKBCOMP, xkbcomp, ,$PATH:/usr/X11R6/bin:/usr/bin/X11)

AC_ARG_WITH( xkb_bin_base,
             [  --with-xkb-bin-base=DIR        XKB executables base path],
             xkb_bin_base="$withval",
             if test -z "$XKBCOMP" ; then
               xkb_bin_base="${xkb_base}"
             else
               xkb_bin_base="`dirname $XKBCOMP`"
             fi )

AC_DEFINE_UNQUOTED(XKB_BIN_BASE,"${xkb_bin_base}",Base for XKB executables)

AC_ARG_WITH( xkb_default_ruleset,
             [  --with-xkb-default-ruleset=RULES        XKB default set of rules (by default it is base)],
             xkb_default_ruleset="$withval",
             xkb_default_ruleset="base" )

AC_DEFINE_UNQUOTED(XKB_DEFAULT_RULESET,"${xkb_default_ruleset}",Default XKB set of rules)

AC_ARG_WITH( xmodmap_default_ruleset,
             [  --with-xmodmap-default-ruleset=RULES        Xmodmap default set of rules (by default it is base)],
             xmodmap_default_ruleset="$withval",
             xmodmap_default_ruleset="base" )

AC_DEFINE_UNQUOTED(XMODMAP_DEFAULT_RULESET,"${xmodmap_default_ruleset}",Default Xmodmap set of rules)

AC_ARG_WITH( xmodmap_base,
             [  --with-xmodmap-base=DIR        xmodmap base path (by default it is /usr/share/xmodmap)],
             xmodmap_base="$withval",
             xmodmap_base="/usr/share/xmodmap" )

AC_DEFINE_UNQUOTED(XMODMAP_BASE,"${xmodmap_base}",Base for xmodmap configuration)

ac_xkblib_include="\
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>"

AC_CHECK_HEADER([X11/extensions/XKBrules.h],
                [libxkbfile_present=yes],[],$ac_xkblib_include)

AM_CONDITIONAL(LIBXKBFILE_PRESENT, test "$libxkbfile_present" = "yes")

AC_SUBST(libxkbfile_present)

AC_ARG_ENABLE(xkb-support,
[  --enable-xkb-support       Enable XKB support],
, enable_xkb_support=yes)

AM_CONDITIONAL(ENABLE_XKB_SUPPORT, test "$enable_xkb_support" = "yes")

AC_ARG_ENABLE(xmodmap-support,
[  --enable-xmodmap-support       Enable xmodmap support],
, enable_xmodmap_support=yes)

AM_CONDITIONAL(ENABLE_XMODMAP_SUPPORT, test "$enable_xmodmap_support" = "yes")

AC_MSG_CHECKING(whether at least one libxklavier backend is enabled)
if test \( "$enable_xkb_support" = "yes" -a \
          "$libxkbfile_present" = "yes" \) -o \
        "$enable_xmodmap_support" = "yes" ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  AC_MSG_ERROR([no libxklavier backends configured])
fi

dnl Checks for libraries.
PKG_CHECK_MODULES(XML, \
	libxml-2.0 >= 2.0.0)
AC_SUBST(XML_LIBS)
AC_SUBST(XML_CFLAGS)

PKG_CHECK_MODULES(GLIB, \
	glib-2.0 >= 2.16.0 gobject-2.0 >= 2.16.0)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

PKG_CHECK_MODULES(XINPUT, xi >= 1.1.3,
                  [AC_DEFINE(HAVE_XINPUT, 1, [Define if xinput dev lib is available])
                  have_xinput=yes], [have_xinput=no])

AC_SUBST(XINPUT_LIBS)
AC_SUBST(XINPUT_CFLAGS)

AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)

dnl -----------------------------------------------------------
dnl ISO codes
dnl -----------------------------------------------------------
AC_MSG_CHECKING([whether iso-codes exists])
if $PKG_CONFIG iso-codes > /dev/null ; then
   AC_MSG_RESULT([yes])
   iso_codes_prefix=`$PKG_CONFIG --variable=prefix iso-codes`
   iso_codes_pkgconfig=iso-codes
   have_iso_codes=yes
else
   AC_MSG_RESULT([no])
   AC_MSG_ERROR([You must have iso-codes.])
fi

AC_DEFINE_UNQUOTED([ISO_CODES_PREFIX],["$iso_codes_prefix"],[ISO codes prefix])

AC_CONFIG_FILES([
Makefile
libxklavier/Makefile
libxklavier.spec
doc/Makefile
doc/reference/Makefile
tests/Makefile
libxklavier.pc
])
AC_OUTPUT

echo '**********************************************************'
echo '  Libxklavier is configured with the following backends:'
if test "$enable_xkb_support" = "yes" ; then
  if test "$libxkbfile_present" = "yes" ; then
    echo "    XKB, libxkbfile is present"
    echo "         default ruleset: $xkb_default_ruleset"
    echo "         base: $xkb_base"
    echo "         xkbcomp is used from: $xkb_bin_base"
    echo "         xinput available: $have_xinput"
  else
    echo "    XKB, libxkbfile is NOT present"
  fi
fi
if test "$enable_xmodmap_support" = "yes" ; then
  echo "    xmodmap, default ruleset: $xmodmap_default_ruleset"
  echo "             base: $xmodmap_base"
fi

if test "$enable_gtk_doc" = "yes" ; then
  echo "  gtk-doc enabled"
else
  echo "  gtk-doc disabled"
fi
echo '**********************************************************'
