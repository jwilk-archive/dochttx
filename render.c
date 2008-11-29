/* Copyright © 2005, 2006, 2008 Jakub Wilk
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; version 2 dated June, 1991.
 */

#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <ncursesw/ncurses.h>

#include "render.h"
#include "ui.h"
#include "vbi.h"

static void private_render(vbi_page *pg, int lines)
{
  int x, y, sx, sy;
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

// vim:ts=2 sw=2 et
