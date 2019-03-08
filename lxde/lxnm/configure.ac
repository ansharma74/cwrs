AC_PREREQ(2.53)
AC_INIT(lxnm, 0.3, http://lxde.org/)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AM_CONFIG_HEADER(config.h)

# Force to dynamic
AC_DISABLE_STATIC

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
dnl AC_PROG_INTLTOOL(, [no-xml])

AC_ARG_ENABLE(man,
       [AC_HELP_STRING([--enable-man],
               [regenerate roff man pages from Docbook @<:@default=no@:>@])],
       [enable_man="${enableval}"],
       [enable_man=no]
)

# Checks for libraries.
pkg_modules="glib-2.0 >= 2.6.0 \
             gthread-2.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

AC_CHECK_HEADERS(iwlib.h)

if test x"$enable_man" = x"yes"; then
	AC_PATH_PROG([XSLTPROC], [xsltproc])
	if test -z "$XSLTPROC"; then
		enable_man=no
	fi

	dnl check for DocBook DTD and stylesheets in the local catalog.
	JH_CHECK_XML_CATALOG([-//OASIS//DTD DocBook XML V4.1.2//EN],
		[DocBook XML DTD V4.1.2], [], enable_man=no)
	JH_CHECK_XML_CATALOG([http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl],
		[DocBook XSL Stylesheets >= 1.70.1], [], enable_man=no)
fi

AM_CONDITIONAL(ENABLE_REGENERATE_MAN, test "x$enable_man" != "xno")

dnl GETTEXT_PACKAGE=lxnm
dnl AM_GNU_GETTEXT_VERSION(0.14.1)
dnl AC_SUBST(GETTEXT_PACKAGE)
dnl AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

dnl ALL_LINGUAS=""
dnl AM_GLIB_GNU_GETTEXT

AC_OUTPUT([
  Makefile
  src/Makefile
  utils/Makefile
  data/Makefile
  data/lxnm.conf
  data/devlist.sh
  data/modeminfo.sh
  man/Makefile
])

echo
echo LxNM......................................... : Version $VERSION
echo
echo Prefix....................................... : $prefix

