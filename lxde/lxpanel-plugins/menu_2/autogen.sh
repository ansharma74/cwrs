#!/bin/sh
# Run this to generate all the initial makefiles, etc.

echo "+ aclocal"
aclocal
rm -rf autom4te.cache

echo "+ libtoolize --force --copy"
libtoolize --force --copy

echo "+ intltoolize --force --copy"
intltoolize --force --copy

echo "+ autoconf"
autoconf

echo "+ automake --add-missing --copy"
automake --add-missing --copy
