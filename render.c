/* Copyright © 2005-2012 Jakub Wilk <jwilk@jwilk.net>
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

#define _XOPEN_SOURCE
/* needed for wcwidth(3) */

#include <wchar.h>
#include <string.h>
#include <stdlib.h>

#include "anycurses.h"
#include "config.h"
#include "render.h"
#include "ui.h"
#include "vbi.h"

static void private_render(vbi_page *pg, int lines)
{
  int x, y, sx = -1, sy = 1;
  vbi_char *ch;
  wchar_t wcs[2];
  char mbs[16];

  getsyx(sy, sx);

  memset(wcs, 0, sizeof(wcs));
  memset(mbs, 0, sizeof(mbs));
  ch = pg->text;
  for (y = 0; y < lines; y++)
  {
    for (x = 0; x <= 40; x++, ch++)
    {
      wcs[0] = ch->unicode;
      if (ch->size > VBI_DOUBLE_SIZE || ch->conceal)
        wcs[0] = L' ';
      attrset(dochttx_colors[ch->foreground][ch->background]);
      if (ch->bold)
        attron(A_BOLD);
      if (ch->flash)
        attron(A_BLINK);
      if (wcwidth(wcs[0]) != 1 || wcstombs(mbs, wcs, sizeof(mbs)) == (size_t)-1)
        mvaddch(y, x, ACS_CKBOARD);
      else
        mvprintw(y, x, "%s", mbs);
    }
  }
  attrset(A_NORMAL);
  wnoutrefresh(stdscr);
  setsyx(sy, sx);
  doupdate();
}

vbi_subno dochttx_vbi_render(vbi_decoder *dec, vbi_pgno pgno, vbi_subno subno, int lines)
{
  vbi_page page;
  vbi_fetch_vt_page(dec, &page, pgno, subno, VBI_WST_LEVEL_1p5, lines, 1);
  subno = page.subno;
  private_render(&page, lines);
  vbi_unref_page(&page);
  return subno;
}

// vim:ts=2 sts=2 sw=2 et
