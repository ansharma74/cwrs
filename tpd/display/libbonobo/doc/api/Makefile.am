## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS = 1.6

SUBDIRS = figures

# The name of the module
DOC_MODULE=libbonobo

# The top-level SGML file.
DOC_MAIN_SGML_FILE=$(DOC_MODULE)-docs.sgml

# The directory containing the source code. Relative to $(srcdir).
# gtk-doc will search all .c & .h files beneath here for inline comments
# documenting functions and macros.
DOC_SOURCE_DIR=../../bonobo

# Extra options to supply to gtkdoc-scan.
SCAN_OPTIONS=--deprecated-guards="BONOBO_DISABLE_DEPRECATED"

# Extra options to supply to gtkdoc-mkdb.
MKDB_OPTIONS=--sgml-mode --output-format=xml

# Extra options to supply to gtkdoc-fixref.
FIXXREF_OPTIONS=

# Used for dependencies.
HFILE_GLOB=
CFILE_GLOB=

# Header files to ignore when scanning.
IGNORE_HFILES= Bonobo.h bonobo-marshal.h bonobo-macros.h bonobo-x-object.h\
	       bonobo-stream.h

# Images to copy into HTML directory.
HTML_IMAGES = figures/persist.png figures/persist-file.png\
	      figures/persist-storage.png figures/persist-stream.png\
              figures/stream.png figures/storage.png figures/unknown.png\
	      figures/running-context.png

# Extra SGML files that are included by $(DOC_MAIN_SGML_FILE).
content_files = faq.xml debugging.xml properties.xml refcounting.xml\
		monikers-overview.xml

# Other files to distribute.
extra_files =


# CFLAGS and LDFLAGS for compiling scan program. Only needed
# if $(DOC_MODULE).types is non-empty.
GTKDOC_CFLAGS =						\
	@CFLAGS@					\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	$(LIBBONOBO_CFLAGS)				\
	-I$(top_srcdir)					\
	-I$(top_builddir)

GTKDOC_LIBS = $(top_builddir)/bonobo/libbonobo-2.la $(LIBBONOBO_LIBS)


# If you need to override some of the declarations, place them in the
# $(DOC_MODULE)-overrides.txt file and uncomment the second line here.
DOC_OVERRIDES =
#DOC_OVERRIDES = $(DOC_MODULE)-overrides.txt


# include common portion ...
include $(top_srcdir)/gtk-doc.make

