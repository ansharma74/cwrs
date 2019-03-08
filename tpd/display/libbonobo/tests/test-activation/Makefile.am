check_PROGRAMS=			\
	bonobo-activation-test		\
	bonobo-activation-test-async	\
	bonobo-activation-env-test	\
	bonobo-activation-empty-server

noinst_LTLIBRARIES=libtest-plugin.la

bin_PROGRAMS=bonobo-activation-run-query

bonobo_activation_test_async_SOURCES=   \
	bonobo-activation-test-async.c

bonobo_activation_test_SOURCES=		\
	bonobo-activation-test.c	\
	empty-common.c			\
	empty.h				\
	empty-stubs.c 

bonobo_activation_test_LDADD=libtest-plugin.la ../../bonobo/libbonobo-2.la
bonobo_activation_env_test_LDADD=../../bonobo/libbonobo-2.la


# Horrible, digusting and atrocious hack to
# get libtool to build a noinst shared lib.
libtest_plugin_la_LDFLAGS=					\
	-rpath $(shell (cd $(top_builddir);pwd))/test/.libs 	\
	-module -avoid-version -no-undefined

libtest_plugin_la_LIBADD=					\
	../../bonobo-activation/libbonobo-activation.la		\
	$(LIBBONOBO_LIBS)

libtest_plugin_la_SOURCES=		\
	plugin.h			\
	plugin-common.c			\
	plugin-skels.c			\
	plugin-stubs.c			\
	bonobo-activation-testplugin.c

bonobo_activation_run_query_SOURCES=	\
	bonobo-activation-run-query.c 	\
	empty-common.c			\
	empty.h				\
	empty-stubs.c

bonobo_activation_empty_server_SOURCES=	\
	empty-server.c			\
	empty.h				\
	empty-common.c			\
	empty-skels.c

BUILT_SOURCES=				\
	empty.h				\
	empty-common.c			\
	empty-stubs.c			\
	empty-skels.c			\
	plugin.h			\
	plugin-stubs.c			\
	plugin-skels.c			\
	plugin-common.c

empty.h empty-common.c empty-stubs.c empty-skels.c: empty_built
plugin.h plugin-common.c plugin-stubs.c plugin-skels.c: plugin_built

empty_built: empty.idl $(ORBIT_IDL)
	$(AM_V_GEN) $(ORBIT_IDL) $(srcdir)/empty.idl
	@touch empty_built

plugin_built: plugin.idl $(ORBIT_IDL)
	$(AM_V_GEN) $(ORBIT_IDL) $(srcdir)/plugin.idl
	@touch plugin_built

CLEANFILES=empty_built plugin_built $(BUILT_SOURCES)

INCLUDES= \
	-I$(top_srcdir)			\
	-I$(top_builddir)		\
	$(WARN_CFLAGS)			\
	$(DISABLE_DEPRECATED_CFLAGS) $(LIBBONOBO_CFLAGS)

LDADD = ../../bonobo-activation/libbonobo-activation.la \
	$(LIBBONOBO_LIBS)

serverfiles=empty.server broken.server plugin.server

noinst_DATA=$(serverfiles)

EXTRA_DIST= test.sh empty.idl plugin.idl $(serverfiles)

# TESTS= test.sh bonobo-activation-test-async
TESTS= test.sh

dist-hook:
	cd $(distdir); rm -f $(CLEANFILES)

TESTS_ENVIRONMENT = \
	BONOBO_ACTIVATION_DEBUG=1 \
	BONOBO_ACTIVATION_PATH="$(srcdir)$(SEARCHPATHSEP)$(top_builddir)/activation-server$(SEARCHPATHSEP)$$BONOBO_ACTIVATION_PATH"
