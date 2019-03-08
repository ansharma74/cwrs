dnl Process this file with autoconf to produce a configure script.
m4_pattern_forbid([^MAN_])

# Initialise and check we're in the correct directory.
AC_INIT([man-db], [2.6.2], [cjwatson@debian.org])
AC_CONFIG_AUX_DIR([tools])
AM_INIT_AUTOMAKE([1.10 -Wall -Wno-override -Werror foreign dist-xz])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_PREREQ([2.59])
AC_CONFIG_SRCDIR([src/man.c])
AC_GNU_SOURCE
LT_INIT([disable-static])

if test ! -f "$srcdir/gnulib/po/Makefile.in.in"; then
	AC_MSG_ERROR([you must run ./autogen.sh before ./configure when building from revision control])
fi

AC_CONFIG_HEADER([config.h])
AC_CANONICAL_HOST

# Define below date and version information to be put into man pages etc.
date=2012-06-18
AC_SUBST([date])dnl
roff_version=`echo AC_PACKAGE_VERSION | sed 's/-/\\-/g'`
AC_SUBST([roff_version])dnl

# We have to be a bit naughty here and supply options.
# The autoconf literature states that only features that can be separately
# 'built' should use --enable and friends. Oh well...
AC_ARG_ENABLE([setuid],
[AS_HELP_STRING([--enable-setuid[=ARG]], [install man setuid to user ARG [ARG=man]])
AS_HELP_STRING([--disable-setuid], [don't install man setuid])],
	  [if test "$enableval" = "yes" 
	   then
		enableval=man
	   fi
	   if test "$enableval" = "no" 
	   then
		man_owner=
		man_mode="755"
		AC_MSG_NOTICE([Man will not be installed setuid])
	   else
		man_owner=$enableval
		man_mode="4755"
		AC_MSG_NOTICE([Man will be installed setuid $enableval])
	   	AC_DEFINE_UNQUOTED([SECURE_MAN_UID], ["$man_owner"],
				   [Define as the setuid owner of man or undefine if not installing setuid.])
	   fi],
	  [man_owner=man
	   man_mode="4755"
	   AC_DEFINE_UNQUOTED([SECURE_MAN_UID], ["$man_owner"])])
AC_ARG_ENABLE([undoc],
[AS_HELP_STRING([--enable-undoc=COMMAND], [suggest COMMAND for missing manual pages])],
	[if test "$enableval" = "yes" || test "$enableval" = "no"
	 then
		AC_MSG_ERROR([--enable-undoc requires an argument])
	 else
		AC_MSG_NOTICE([Suggesting '$enableval' for missing manual pages])
		AC_DEFINE_UNQUOTED([UNDOC_COMMAND], ["$enableval"],
				   [Define as the name of a command you want to suggest when a non-existent page is requested.])
	 fi])
AC_ARG_WITH([device],
[AS_HELP_STRING([--with-device=DEVICE], [use nroff with the output device DEVICE])],
	[if test "$withval" = "yes" || test "$withval" = "no" 
	 then
	 	AC_MSG_ERROR([--with-device requires an argument])
	 else
	 	nroff_device=" -T$withval"
	 fi])
AC_ARG_WITH([db],
[AS_HELP_STRING([--with-db=LIBRARY], [use database library LIBRARY (db5, db4, db3, db2, db1, db, gdbm, ndbm)])],
	[if test "$withval" = "yes" || test "$withval" = "no" 
	 then
	 	AC_MSG_ERROR([--with-db requires an argument])
	 else
	 	db=$withval
	 fi],
	[: ${db=no}])
AC_ARG_WITH([config-file],
[AS_HELP_STRING([--with-config-file=CF], [use config file CF [CF=SYSCONFDIR/man_db.conf]])],
	[if test "$withval" = "yes" || test "$withval" = "no"
	 then
		AC_MSG_ERROR([--with-config-file requires an argument])
	 else
		config_file=$withval
	 fi],
	[: ${config_file=\$\{sysconfdir\}/man_db.conf}])
config_file_basename=${withval##*/}
config_file_dirname=`AS_DIRNAME(["$config_file"])`
AC_ARG_WITH([sections],
[AS_HELP_STRING([--with-sections=SECTIONS], [use manual page sections SECTIONS @<:@1 n l 8 3 0 2 5 4 9 6 7@:>@])],
	[if test "$withval" = "yes" || test "$withval" = "no"
	 then
		AC_MSG_ERROR([--with-sections requires an argument])
	 else
		sections="$withval"
	 fi],
	[: ${sections=1 n l 8 3 2 5 4 9 6 7}])
AC_ARG_ENABLE([automatic-create],
[AS_HELP_STRING([--enable-automatic-create], [allow man to create user databases on the fly])],
	[if test "$enableval" = "yes"
	 then
		AC_DEFINE([MAN_DB_CREATES], [1], [Allow man to create user databases on the fly.])
	 fi])
AC_ARG_ENABLE([automatic-update],
[AS_HELP_STRING([--disable-automatic-update], [don't allow man to update databases on the fly])],
	[if test "$enableval" = "yes"
	 then
		AC_DEFINE([MAN_DB_UPDATES], [1], [Allow man to update databases on the fly.])
	 fi],
	[AC_DEFINE([MAN_DB_UPDATES], [1], [Allow man to update databases on the fly.])])
AC_ARG_ENABLE([cats],
[AS_HELP_STRING([--disable-cats], [don't allow man to create/update cat files])],
	[if test "$enableval" = "yes"
	 then
		AC_DEFINE([MAN_CATS], [1], [Allow man to create/update cat files.])
	 fi],
	[AC_DEFINE([MAN_CATS], [1], [Allow man to create/update cat files.])])

# Finish the argument parsing.
AC_SUBST([man_owner])dnl
AC_SUBST([man_mode])dnl
AC_SUBST([config_file])dnl
AC_SUBST([config_file_basename])dnl
AC_SUBST([config_file_dirname])dnl
AC_SUBST([sections])dnl

# Check $PATH for the following programs and append suitable options.
AC_PROG_CC
gl_EARLY
AC_PROG_CPP
CFLAGS="$CFLAGS -Wall"
case $host_os in
	ultrix4.3*)
		dnl When compiled for BSD environment, each running `man'
		dnl increases the system load as reported by uptime(1) by
		dnl one.  The reason for this behaviour is currently
		dnl unknown, but the load increase does *not* reflect actual
		dnl resource usage.  To avoid it, compile for POSIX
		dnl environment:
		CFLAGS="$CFLAGS -YPOSIX"
		;;
esac
if test "$GCC" = yes
then
	gl_WARN_ADD([-W], [CFLAGS])
	gl_WARN_ADD([-Wpointer-arith], [CFLAGS])
	gl_WARN_ADD([-Wwrite-strings], [CFLAGS])
	gl_WARN_ADD([-Wstrict-prototypes], [CFLAGS])
	gl_WARN_ADD([-Wshadow], [CFLAGS])
	gl_WARN_ADD([-Wformat-security], [CFLAGS])
	gl_WARN_ADD([-Wredundant-decls], [CFLAGS])
	gl_WARN_ADD([-Wno-missing-field-initializers], [CFLAGS])
fi
AC_PROG_INSTALL
AC_PROG_LN_S
AC_CHECK_PROGS([cat], [cat])
MAN_CHECK_PROGS([browser], [BROWSER], [use BROWSER as default web browser], [www-browser lynx elinks w3m])
test -n "$browser" && browser="exec $browser"
AC_CHECK_PROGS([tr], [tr])
AC_CHECK_PROGS([grep], [grep])
MAN_CHECK_PROGS([pager], [PAGER], [use PAGER as default pager], [pager less more])
test -n "$pager" && pager="$pager -s"

# Define below (in list of preference) *roff macros to check for.
macros="andoc an doc"

# We have problems here, as different systems have different *roff
# formatters and they accept different options and do different things :(
MAN_CHECK_PROGS([nroff], [NROFF], [use NROFF as roff formatter for character devices], [nroff gnroff groff])
if test -n "$nroff"
then
	MAN_PROG_GNU_NROFF([$nroff])
	if test "$man_cv_prog_gnu_nroff" != "yes"
	then
		MAN_PROG_HEIRLOOM_NROFF([$nroff])
	fi
	if test -n "$nroff_device" 
	then
		AC_MSG_CHECKING([that nroff works with argument$nroff_device])
		# We cannot cache this result as it can change between runs
		# of configure.
		if $nroff $nroff_device </dev/null >/dev/null 2>&1 3>&1
		then 
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([nroff does not work with argument$nroff_device])
		fi
	fi
	AC_MSG_CHECKING([for appropriate nroff macro])
	AC_CACHE_VAL([man_cv_prog_nroff_macro], [
	
	for macro in $macros
	do
		if $nroff -m$macro $nroff_device </dev/null >/dev/null 2>&1 3>&1
		then
			man_cv_prog_nroff_macro=-m$macro
			break
		fi
	done])
	
	if test -n "$man_cv_prog_nroff_macro"
	then
		if test "$man_cv_prog_heirloom_nroff" = "yes"
		then
			# Heirloom works best with some extra options:
			#   -mg: groff compatibility
			#   -msafe: disable privileged operations
			#   -mpadj: clean up line breaking
			man_cv_prog_nroff_macro="-mg -msafe -mpadj ${man_cv_prog_nroff_macro}"
		fi
		nroff="$nroff ${man_cv_prog_nroff_macro}"
dnl		nroff="$nroff ${man_cv_prog_nroff_macro}${nroff_device}"
		AC_MSG_RESULT([$man_cv_prog_nroff_macro])
	else
		AC_MSG_RESULT([ambiguous])
		AC_MSG_WARN([please edit include/manconfig.h and add nroff definition])
	fi 
else
	AC_MSG_WARN([Cannot find an nroff-like program, formatting of manual page source will not be supported.])
	nroff="(nroff not installed)"
	AC_DEFINE([NROFF_MISSING], [1], [Define if you don't have nroff.])
fi

dnl It would be nice to use MAN_CHECK_PROGS here, but how do we determine
dnl TROFF_IS_GROFF?
AC_CHECK_PROGS([troff], [groff])
if test -n "$troff"
then
	troff_is_groff=yes
	AC_DEFINE([TROFF_IS_GROFF], [1], [Define if you are using groff as troff.])
	TROFF=groff
else
	troff_is_groff=no
	AC_CHECK_PROGS([troff], [troff gtroff])
	TROFF=troff
fi
AC_SUBST([TROFF])
if test -n "$troff"
then
	AC_DEFINE([HAS_TROFF], [1], [Define if you have troff.])
	AC_MSG_CHECKING([for appropriate $TROFF options])
	AC_CACHE_VAL([man_cv_prog_troff_options], [
	# Do a quick test to see if -t works [AIX needs this], groff doesn't
	# as it indicates pre-process with tbl.
	test "$TROFF" = "troff" && $troff -t </dev/null >/dev/null 2>&1 3>&1 \
				&& troff_filter="-t "
	for macro in $macros
	do
		if $troff $troff_filter -m$macro </dev/null >/dev/null 2>&1 3>&1
		then
			man_cv_prog_troff_options="${troff_filter}-m${macro}"
			break
		fi
	done])
	if test -n "$man_cv_prog_troff_options"
	then
		if test "$man_cv_prog_heirloom_nroff" = "yes"
		then
			# Heirloom works best with some extra options:
			#   -mg: groff compatibility
			#   -msafe: disable privileged operations
			#   -mpadj: clean up line breaking
			man_cv_prog_troff_options="-mg -msafe -mpadj ${man_cv_prog_troff_options}"
		fi
		troff="$troff $man_cv_prog_troff_options"
		AC_MSG_RESULT([$man_cv_prog_troff_options])
	else
		AC_MSG_RESULT([ambiguous])
		AC_MSG_WARN([please edit include/manconfig.h and add troff definition])
	fi
else
	troff="(troff not installed)"
fi

AC_CHECK_PROGS([preconv], [gpreconv preconv])

AC_MSG_CHECKING([for groff with Debian multibyte patch or real Unicode support])
man_mb_groff=no
AC_ARG_ENABLE([mb-groff],
[AS_HELP_STRING([--enable-mb-groff], [expect groff with Debian multibyte patch or real Unicode support])],
	[if test "$enableval" = "yes"
	 then
		man_mb_groff=yes
	 fi],
	[if test -n "$preconv"
	 then
		man_mb_groff=yes
	 elif test -n "$troff" && test "$troff_is_groff" = "yes"
	 then
		if $troff -Tnippon </dev/null >/dev/null 2>&1
		then
			man_mb_groff=yes
		fi
	 fi])
AC_MSG_RESULT([$man_mb_groff])
if test "$man_mb_groff" = "yes"
then
	AC_DEFINE([MULTIBYTE_GROFF], 1,
		  [Define if your groff installation has the Debian multibyte patch.])
fi

MAN_CHECK_PROGS([eqn], [EQN], [use EQN to preprocess equations], [eqn geqn])
MAN_CHECK_PROGS([neqn], [NEQN], [use NEQN to preprocess equations for character devices], [neqn gneqn])
# If we fail to find an neqn, use eqn and try to force it to output for an
# ascii device. As this is only relevant for equations (?), not using latin1
# should be acceptable. -Tlatin1 is ignored by some eqn implementations.
if test -z "$neqn"
then
	test -n "$eqn" && 
	(test -n "$nroff_device" && neqn="$eqn -T$nroff_device" || neqn="$eqn -Tascii")
fi
MAN_CHECK_PROGS([tbl], [TBL], [use TBL to preprocess tables], [tbl gtbl])
MAN_CHECK_PROGS([col], [COL], [use COL to filter formatting characters from output], [col gcol])
MAN_CHECK_PROGS([vgrind], [VGRIND], [use VGRIND to preprocess program sources], [vgrind gvgrind])
MAN_CHECK_PROGS([refer], [REFER], [use REFER to preprocess bibliographic references], [refer grefer])
MAN_CHECK_PROGS([grap], [GRAP], [use GRAP to preprocess graphs], [grap])
MAN_CHECK_PROGS([pic], [PIC], [use PIC to preprocess pictures], [pic gpic])
test -n "$pic" && pic="$pic -S"

MAN_CHECK_PROGS([gzip], [GZIP], [use GZIP as GNU compression utility], [gzip])
if test -n "$gzip"
then
	gunzip="$gzip -dc"
	compressor="$gzip -c7"
	compress_ext="gz"
fi
MAN_CHECK_PROGS([compress], [COMPRESS], [use COMPRESS as UNIX compression utility], [compress])
if test -n "$compress"
then
	uncompress="$compress -dc"
	if test -z "$gzip"
	then
		compressor="$compress -c"
		compress_ext="Z"
	fi
fi
MAN_CHECK_PROGS([bzip2], [BZIP2], [use BZIP2 as block-sorting compression utility], [bzip2])
if test -n "$bzip2"
then
	bunzip2="$bzip2 -dc"
fi
MAN_CHECK_PROGS([xz], [XZ], [use XZ as Lempel-Ziv-Markov chain-Algorithm compression utility], [xz])
if test -n "$xz"
then
	unxz="$xz -dc"
	unlzma=
else
	dnl lzma not used/needed if we have xz
	MAN_CHECK_PROGS([lzma], [LZMA], [use LZMA as Lempel-Ziv-Markov chain-Algorithm compression utility], [lzma])
	if test -n "$lzma"
	then
		unlzma="$lzma -dc"
	fi
fi
MAN_CHECK_PROGS([lzip], [LZIP], [use LZIP as Lempel-Ziv-Markov chain-Algorithm compression utility], [lzip])
if test -n "$lzip"
then
	unlzip="$lzip -dc"
fi
if test -n "$gzip" || test -n "$compress" || test -n "$bzip2" || test -n "$xz" || test -n "$lzip" || test -n "$lzma"
then
	AC_DEFINE([COMP_CAT], [1], [Define if you have compressors and want to support compressed cat files.])
	AC_DEFINE([COMP_SRC], [1], [Define if you have compressors and want to support compressed manual source.])
fi
AC_SUBST([compressor])
AC_SUBST([compress_ext])
AC_SUBST([gunzip])
AC_SUBST([uncompress])
AC_SUBST([bunzip2])
AC_SUBST([unlzma])
AC_SUBST([unxz])
AC_SUBST([unlzip])
MAN_COMPRESS_LIB([z], [gzopen])
dnl To add more decompressors just follow the scheme above.

# Work out which manual page hierarchy scheme might be in use.
AC_ARG_ENABLE([mandirs],
[AS_HELP_STRING([--enable-mandirs=OS], [select manual page hierarchy organization (GNU, HPUX, IRIX, Solaris, BSD)])],
	[AC_MSG_NOTICE([Using $enableval hierarchy organization(s)])
	 AC_DEFINE_UNQUOTED([MANDIR_LAYOUT], ["$enableval"],
			    [Define to the manual page hierarchy organization(s) in use.])],
	[case $host in
		*-gnu)          mandirs=GNU;;
		*-hpux*)        mandirs=HPUX;;
		*-irix*)        mandirs=IRIX;;
		*-solaris*)     mandirs=Solaris;;
		*-bsd*)         mandirs=BSD;;
		*)              mandirs=;;
	 esac
	 if test -n "$mandirs"; then
		AC_MSG_NOTICE([Using $mandirs hierarchy organization])
		AC_DEFINE_UNQUOTED([MANDIR_LAYOUT], ["$mandirs"])
	 else
		AC_MSG_NOTICE([Allowing any hierarchy organization])
		AC_DEFINE([MANDIR_LAYOUT], [""])
	 fi])

# Check for various header files and associated libraries.
AC_ISC_POSIX
dnl AC_PROG_LEX calls AC_TRY_LINK: must come after above 3
AC_PROG_LEX
gl_INIT
AC_HEADER_SYS_WAIT
AC_HEADER_DIRENT
AC_CHECK_HEADERS([fcntl.h sys/file.h])

# Internationalization support.
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])
AC_SUBST([LINGUAS])
AM_ICONV
MAN_PO4A
MAN_LINGUAS

# Checks for structures and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_UID_T
AC_TYPE_SIZE_T

# Check for pipeline library.
PKG_CHECK_MODULES([libpipeline], [libpipeline >= 1.1.0])

# Find a suitable database interface header and library.
#
# Check for GNU dbm routines.
if test "$db" = "no" || test "$db" = "gdbm"
then
  AC_CHECK_HEADER([gdbm.h], [
    for lib in gdbm c dbm
    do
      AC_CHECK_LIB([$lib], [gdbm_fetch],
		   test "$lib" = "c" || DBLIBS="-l$lib" 
		   [AC_DEFINE([GDBM], [1], [Define if you have, and want to use, gdbm interface routines.])
		    AC_SUBST([DBTYPE], [gdbm])]
		   db=yes, db=no)
      if test "$db" = "yes" 
      then
        man_save_LIBS="$LIBS"
        LIBS="$LIBS $DBLIBS"
        AC_CHECK_FUNCS([gdbm_exists])
        LIBS="$man_save_LIBS"
        break
      fi
    done], db=no)
fi

# Check for Berkeley db routines (first version API).
MAN_CHECK_BDB([db5 db], [db5/db_185.h db_185.h], [db5 db-5])
MAN_CHECK_BDB([db4 db], [db4/db_185.h db_185.h], [db4 db-4])
MAN_CHECK_BDB([db3 db], [db3/db_185.h db_185.h], [db3])
MAN_CHECK_BDB([db2 db], [db_185.h db2/db_185.h db2_185.h], [db2 db])
MAN_CHECK_BDB([db1 db], [db/db.h db.h db1/db.h], [db db1])

dnl MAN_CHECK_BDB([db2], [db2_185.h], [db2 db c], [AC_DEFINE(DB_ON_LIBC)])
dnl MAN_CHECK_BDB([db2], [db2/db_185.h], [db2 db c])
dnl # from package libdb1-dev
dnl MAN_CHECK_BDB([db], [db/db.h], [db c])
dnl # from package libc6-dev
dnl # ########## statically linked #####
dnl MAN_CHECK_BDB([db], [db1/db.h], [db c], [AC_DEFINE(DB_ON_LIBC)])

# Check for UNIX ndbm routines.
if test "$db" = "no" || test "$db" = "ndbm"
then
  AC_CHECK_HEADER([ndbm.h], [
    for lib in ndbm c dbm
    do
      AC_CHECK_LIB([$lib], [dbm_fetch],
		   test "$lib" = "c" || DBLIBS="-l$lib"
		   [AC_DEFINE([NDBM], [1], [Define if you have, and want to use, ndbm interface routines.])
		    AC_SUBST([DBTYPE], [ndbm])]
		   db=yes, db=no)
      test "$db" = "yes" && break
      done], db=no)
fi 

if test "$db" != "yes"
then 
    if test "$db" = "no"
    then
      AC_MSG_ERROR([Fatal: no supported database library/header found])
    else
      AC_MSG_ERROR([Fatal: $db: unsupported database library])
    fi
fi
AC_SUBST([DBLIBS])

dnl MAN_ECHO_VAR(ENV-VARIABLE)
define([MAN_ECHO_VAR], [AC_MSG_NOTICE([default $1 = "$$1"])])dnl
dnl
MAN_ECHO_VAR([CC])
MAN_ECHO_VAR([CPP])
MAN_ECHO_VAR([CPPFLAGS])
MAN_ECHO_VAR([CFLAGS])
MAN_ECHO_VAR([LDFLAGS])
MAN_ECHO_VAR([LIBS])
MAN_ECHO_VAR([DBLIBS])

# Transformed versions of program names for use in Automake variables.
MAN_TRANS_SUBST([apropos])
MAN_TRANS_SUBST([catman])
MAN_TRANS_SUBST([lexgrog])
MAN_TRANS_SUBST([man])
MAN_TRANS_SUBST([manconv])
MAN_TRANS_SUBST([mandb])
MAN_TRANS_SUBST([manpath])
MAN_TRANS_SUBST([whatis])
MAN_TRANS_SUBST([zsoelim])

# If we're cross-compiling, tests won't work.
AM_CONDITIONAL([CROSS_COMPILING], [test "x$cross_compiling" = xyes])

AC_CONFIG_FILES([Makefile
	gnulib/lib/Makefile
	gnulib/po/Makefile.in
	lib/Makefile
	src/Makefile
	src/man_db.conf
	src/tests/Makefile
	man/Makefile
	man/replace.sin
	man/po4a/Makefile
	man/de/Makefile
	man/es/Makefile
	man/fr/Makefile
	man/id/Makefile
	man/it/Makefile
	man/ja/Makefile
	man/nl/Makefile
	man/pl/Makefile
	man/ru/Makefile
	manual/Makefile
	libdb/Makefile
	docs/Makefile
	tools/Makefile
	include/comp_src.h
	include/manconfig.h
	po/Makefile.in])
AC_OUTPUT
