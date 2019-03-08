#! /bin/sh -e

if type gnulib-tool >/dev/null 2>&1; then
	autopoint -f
	gnulib-tool --update >/dev/null
	export AUTOPOINT=true
fi
export LIBTOOLIZE_OPTIONS=--quiet
autoreconf -fi "$@"
