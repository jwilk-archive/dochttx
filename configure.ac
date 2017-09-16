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

AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl vim:ts=4 sts=4 sw=4 et
