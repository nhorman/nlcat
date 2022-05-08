#! /bin/sh
mkdir -p ./m4
aclocal
autoheader
automake -a --add-missing --copy
autoconf

