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

#define _XOPEN_SOURCE
/* needed for wcwidth(3) */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "anycurses.h"
#include "autoconf.h"
#include "render.h"
#include "ui.h"
#include "vbi.h"

static const wchar_t mosaic_to_braille[] = {
  0x2800, 0x2801, 0x2808, 0x2809, 0x2802, 0x2803, 0x280A, 0x280B,
  0x2810, 0x2811, 0x2818, 0x2819, 0x2812, 0x2813, 0x281A, 0x281B,
  0x2804, 0x2805, 0x280C, 0x280D, 0x2806, 0x2807, 0x280E, 0x280F,
  0x2814, 0x2815, 0x281C, 0x281D, 0x2816, 0x2817, 0x281E, 0x281F,
  0x2820, 0x2821, 0x2828, 0x2829, 0x2822, 0x2823, 0x282A, 0x282B,
  0x2830, 0x2831, 0x2838, 0x2839, 0x2832, 0x2833, 0x283A, 0x283B,
  0x2824, 0x2825, 0x282C, 0x282D, 0x2826, 0x2827, 0x282E, 0x282F,
  0x2834, 0x2835, 0x283C, 0x283D, 0x2836, 0x2837, 0x283E, 0x283F
};

#define ARRAY_LEN(x) (sizeof x / sizeof x[0])

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
  for (y = 0; y < pg->rows && y < lines; y++)
  {
    for (x = 0; x < pg->columns; x++, ch++)
    {
      wcs[0] = ch->unicode;
      if (ch->size > VBI_DOUBLE_SIZE || ch->conceal)
        wcs[0] = L' ';
      /* FIXME: Don't hardcode color palette.
       * Use zvbi's color_map instead.
       * */
      if (ch->foreground >= 8 || ch->background >= 8)
        attrset(dochttx_colors[7][0]);
      else
        attrset(dochttx_colors[ch->foreground][ch->background]);
      if (ch->bold)
        attron(A_BOLD);
      if (ch->flash)
        attron(A_BLINK);
      if (wcs[0] == 0x01B5) /* Ƶ (Latin capital letter Z with stroke) */
        wcs[0] = 0x017B;  /* Ż (Latin capital letter Z with dot above) */
      else if (wcs[0] >= 0xEE00 && wcs[0] < 0xEE80)
      {
        /* G1 Block Mosaic Set → braille patterns */
        unsigned int i = wcs[0] - 0xEE00;
        if (i & 0x20) {
          attron(A_BOLD);
          i -= 0x20;
        }
        if (i >= 0x40)
          i -= 0x20;
        assert(i < ARRAY_LEN(mosaic_to_braille));
        wcs[0] = mosaic_to_braille[i];
      }
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
