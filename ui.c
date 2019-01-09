/* Copyright © 2005-2018 Jakub Wilk <jwilk@jwilk.net>
 * SPDX-License-Identifier: MIT
 */

#include <curses.h>

#include "ui.h"
#include "vbi.h"

int dochttx_colors[8][8];

static void color_setup(void)
{
  start_color();
  int i, j, u;
  u = 1;
  for (i = 0; i < 8; i++)
  for (j = 0; j < 8; j++, u++)
  {
    init_pair(u, dochttx_vbi_colors[i], dochttx_vbi_colors[j]);
    dochttx_colors[i][j] = COLOR_PAIR(u);
  }
}

void dochttx_ncurses_init(void)
{
  initscr();
  raw(); noecho(); nonl();
  intrflush(NULL, FALSE);
  keypad(stdscr, TRUE);
  timeout(500);
  erase();
  color_setup();
}

void dochttx_ncurses_quit(void)
{
  erase();
  endwin();
}

// vim:ts=2 sts=2 sw=2 et
