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
      attrset(colors[ch->foreground][ch->background]);
      if (wcwidth(wcs[0]) != 1 || wcstombs(mbs, wcs, 1) == (size_t)-1)
        mvaddch(y, x, ACS_CKBOARD);
      else
        mvprintw(y, x, "%s", mbs); 
    }
  }
  wnoutrefresh(stdscr);
  setsyx(sy, sx);
  doupdate();
}

void vbi_render(vbi_decoder *dec, int pgno, int subno, int lines)
{
    vbi_page page;
    vbi_fetch_vt_page(dec, &page, pgno, subno, VBI_WST_LEVEL_1p5, lines, 1);
    private_render(&page, lines);
    vbi_unref_page(&page);
}

// vim:ts=2 sw=2 et

