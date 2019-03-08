#! /bin/sh -e

# Build automatically generated files
./autogen.sh

# Basic configure to get 'make distcheck'
./configure

# Force regeneration with new version number
rm -f man/po4a/po/man-db-manpages.pot

# Make sure parsers are fresh
if [ src/lexgrog.l -nt src/lexgrog.c ]; then
	rm -f src/lexgrog.c
fi
if [ src/zsoelim.l -nt src/zsoelim.c ]; then
	rm -f src/zsoelim.c
fi

make distcheck
