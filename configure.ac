dnl | Copyright © 2012-2018 Jakub Wilk <jwilk@jwilk.net>
dnl | SPDX-License-Identifier: MIT

AC_PREREQ([2.61])
AC_INIT([dochttx], [0.3.1], [jwilk@jwilk.net])

AC_PROG_CC_C99

m4_pattern_forbid(
    [^PKG_CHECK_MODULES],
    [undefined macro PKG_CHECK_MODULES; install pkg-config and run private/autogen]
)

test -n "$NCURSESW_CFLAGS" || fix_ncurses_cflags=y
PKG_CHECK_MODULES(
    [NCURSESW],
    [ncursesw],
    [],
    [NCURSESW_CFLAGS=-I/usr/include/ncursesw; NCURSESW_LIBS=-lncursesw]
)
if test -n "$fix_ncurses_cflags" && test -n "$NCURSESW_CFLAGS"
then
    NCURSESW_CFLAGS=$(printf ' %s ' "$NCURSESW_CFLAGS" | sed -e 's/ -D_GNU_SOURCE / /; s/ -D_DEFAULT_SOURCE / /; s/^ *//; s/ *$//')
fi

PKG_CHECK_MODULES([ZVBI], [zvbi-0.2])

AC_CONFIG_HEADER([autoconf.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl vim:ts=4 sts=4 sw=4 et
