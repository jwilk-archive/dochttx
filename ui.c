#include <ncurses.h>

#include "vbi.h"
#include "ui.h"

int colors[8][8];

static void color_setup(void)
{
  start_color();
  int i, j, u;
  u = 1;
  for (i = 0; i < 8; i++)
  for (j = 0; j < 8; j++, u++)
  {
    init_pair(u, vbi_colors[i], vbi_colors[j]);
    colors[i][j] = COLOR_PAIR(u);
  }
}

void ncurses_init(void)
{
  initscr();
  raw(); noecho(); nonl();
  intrflush(NULL, FALSE);
  keypad(stdscr, TRUE);
  timeout(500);
  erase();
  color_setup();
}

void ncurses_quit(void)
{
  erase();
  endwin();
}

// vim:ts=2 sw=2 et
