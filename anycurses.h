#ifndef DOCHTTX_ANYCURSES_H
#define DOCHTTX_ANYCURSES_H

#include "config.h"

#if defined HAVE_NCURSESW_CURSES_H
#include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
#include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
#include <ncurses.h>
#elif defined HAVE_CURSES_H
#include <curses.h>
#else
#error "SysV or X/Open-compatible Curses header file required"
#endif

#endif

// vim:ts=2 sts=2 sw=2 et
