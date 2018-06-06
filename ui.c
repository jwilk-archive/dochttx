/* Copyright © 2005-2018 Jakub Wilk <jwilk@jwilk.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
