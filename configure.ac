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

PKG_CHECK_MODULES([ZVBI], [zvbi-0.2])

AC_DEFINE([_XOPEN_SOURCE], [500], [Define to the level of X/Open that your system supports.])

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl vim:ts=4 sts=4 sw=4 et
