check_PROGRAMS=test_config test_monitor

test_config_SOURCES=test_config.c

test_monitor_SOURCES=test_monitor.c

AM_CFLAGS=-Wall -I$(top_srcdir) $(X_CFLAGS) $(GLIB_CFLAGS)

LDADD=$(top_builddir)/libxklavier/libxklavier.la $(X_LIBS) $(GLIB_LIBS)

EXTRA_DIST=test_gi.py
