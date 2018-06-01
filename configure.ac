dnl | Copyright © 2012-2017 Jakub Wilk
dnl |
dnl | Permission is hereby granted, free of charge, to any person obtaining a
dnl | copy of this software and associated documentation files (the
dnl | “Software”), to deal in the Software without restriction, including
dnl | without limitation the rights to use, copy, modify, merge, publish,
dnl | distribute, sublicense, and/or sell copies of the Software, and to
dnl | permit persons to whom the Software is furnished to do so, subject to
dnl | the following conditions:
dnl |
dnl | The above copyright notice and this permission notice shall be included
dnl | in all copies or substantial portions of the Software.
dnl |
dnl | THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
dnl | OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
dnl | MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
dnl | IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
dnl | CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
dnl | TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
dnl | SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

AC_PREREQ([2.61])
AC_INIT([dochttx], [0.2.5], [jwilk@jwilk.net])

AC_PROG_CC_C99

AX_WITH_CURSES
if test "$ax_cv_curses" != yes || test "$ax_cv_curses_color" != yes; then
    AC_MSG_ERROR([an X/Open-compatible Curses library with color is required])
fi
if test "$ax_cv_ncursesw" != yes; then
    AC_MSG_WARN([the ncursesw library is recommended])
fi
# The variables were renamed in autoconf-archive v2016.09.16:
# * CURSES_CPPFLAGS -> CURSES_CFLAGS
# * CURSES_LIB -> CURSES_LIBS
if test -n "$CURSES_CPPFLAGS"; then
    CURSES_CFLAGS="$CURSES_CPPFLAGS"
    AC_SUBST([CURSES_CFLAGS])
fi
if test -n "$CURSES_LIB"; then
    CURSES_LIBS="$CURSES_LIB"
    AC_SUBST([CURSES_LIBS])
fi

PKG_CHECK_MODULES([ZVBI], [zvbi-0.2])

AC_CONFIG_HEADER([autoconf.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl vim:ts=4 sts=4 sw=4 et
