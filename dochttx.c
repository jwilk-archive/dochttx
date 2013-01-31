/* Copyright © 2005, 2006, 2008, 2010, 2012 Jakub Wilk <jwilk@jwilk.net>
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

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "anycurses.h"
#include "ui.h"
#include "vbi.h"
#include "render.h"
#include "locale.h"

static vbi_pgno np_pgno = 0;
static vbi_subno np_subno = 0;
static bool np_drawn = true;

static void intercept(vbi_event *ev, void *dec)
{
  dec = dec;
  np_pgno = ev->ev.ttx_page.pgno;
  np_subno = ev->ev.ttx_page.subno;
  np_drawn = false;
}

static void show_display_info(vbi_decoder* dec, vbi_pgno pgno, vbi_subno subno)
{
  vbi_subno maxsubno;
  vbi_classify_page(dec, pgno, &maxsubno, NULL);
  mvhline(4, 43, ' ', COLS - 43);
  mvprintw(4, 43, "Showing page %03x", pgno);
  if (maxsubno == 0)
    printw(", no subpages");
  else if (maxsubno <= 0x3f7f)
    printw(", subpage %02x of %02x", subno, maxsubno);
}

int main(void)
{
  dochttx_locale_init();

  struct dochttx_vbi_state* vbi = dochttx_vbi_open("/dev/vbi0", 8);
  if (vbi == NULL) {
    return EXIT_FAILURE;
  }

  dochttx_ncurses_init();
 
  char lf[8];
  int lf_pos = 0;
  int lf_status = 0;
  bool lf_update = true;
 
  mvvline(0, 41, ACS_VLINE, 25);
  for (int y = 1; y < 25; y++)
    mvhline(y, 0, ACS_BOARD, 41);
  mvhline(25, 0, ACS_HLINE, COLS);
  mvaddch(25, 41, ACS_BTEE);
  mvprintw(0, 43, "Look for:");
  mvprintw(2, 43, "Looking for 100.*");
  wnoutrefresh(stdscr);
  
  vbi_event_handler_register(vbi->dec, VBI_EVENT_TTX_PAGE, intercept, (void*) vbi->dec);
  
  vbi_pgno pgno = 0x100;
  vbi_subno subno = VBI_ANY_SUBNO;
  bool drawn = false;
  memset(lf, 0, sizeof lf);
  while (true)
  {
    struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);
    FD_SET(vbi->fd, &rdfs);
    int rs = select(vbi->fd+1, &rdfs, NULL, NULL, &tv);
    if (rs == -1)
    {
      if (errno == EINTR)
        continue;
      break;
    }
    if (FD_ISSET(vbi->fd, &rdfs))
      dochttx_vbi_has_data(vbi);
    if (FD_ISSET(STDIN_FILENO, &rdfs))
    {
      bool do_quit = false;
      int chr = getch();
      switch (chr)
      {
      case L'X'-L'@': // Ctrl + X
      case L'C'-L'@': // Ctrl + C
        do_quit = true;
        break;
      case KEY_LEFT:
        if (lf_status == 2)
          lf_status = 1;
        if (lf_pos > 0)
          lf_pos--;
        lf_update = true;
        break;
      case KEY_RIGHT:
        if (lf_status == 2)
          lf_status = 1;
        if (lf[lf_pos] != '\0')
          lf_pos++;
        lf_update = true;
        break;
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9':
        if (lf_status == 2)
        {
          memset(lf, 0, sizeof(lf));
          lf[0] = (char)chr;
          lf_pos = 1;
        }
        else 
      case '.':
        if (lf[5] == '\0')
        {
          memmove(lf + lf_pos + 1, lf + lf_pos, 7 - lf_pos);
          lf[lf_pos++] = (char)chr;
        }
        lf_status = 0;
        lf_update = true;
        break;
      case KEY_DC:
        if (lf_pos >= 6)
          break;
        lf_pos++;
      case KEY_BACKSPACE:
      case '\x7f':
      case '\b':
        if (lf_pos == 0)
          break;
        memmove(lf + lf_pos - 1, lf + lf_pos, 7 - lf_pos);
        lf_pos--;
        lf_status = 0;
        lf_update = true;
        break;
      case KEY_ENTER:
      case '\n':
      case '\r':
        {
          unsigned int new_pgno = 0;
          unsigned int new_subno = VBI_ANY_SUBNO;
          if (sscanf(lf, "%x.%x", &new_pgno, &new_subno) >= 1 && 
              new_pgno >= 0x100 && 
              new_pgno <= 0x899 && 
              (new_subno <= 0x99 || new_subno == VBI_ANY_SUBNO))
          {
            pgno = new_pgno;
            subno = new_subno;
            drawn = false;
            lf_status = 2;
            char subnos[3] = "*";
            if (subno != VBI_ANY_SUBNO)
              sprintf(subnos, "%02x", subno);
            mvhline(2, 43, ' ', COLS - 43);
            mvprintw(2, 43, "Looking for %03x.%s", pgno, subnos);
          } 
          else
            lf_status = -1;
          lf_update = true;
        }
        break;
      }
      if (do_quit)
        break;
    }
    if (lf_update)
    {
      mvhline(0, 53, '_', 6);
      switch (lf_status)
      {
      case -1:
        attrset(dochttx_colors[7][1]);
      case 2:
      case 1:
        attron(A_BOLD);
        break;
      case 0:
      default:
        attrset(A_NORMAL);
      }
      mvprintw(0, 53, lf);
      attrset(A_NORMAL);
      wnoutrefresh(stdscr);
      setsyx(0, 53 + lf_pos);
      lf_update = false;
    }
    if (!drawn && vbi_is_cached(vbi->dec, pgno, subno))
    {
      vbi_subno tmp_subno = dochttx_vbi_render(vbi->dec, pgno, subno, 25);
      show_display_info(vbi->dec, pgno, tmp_subno);
      drawn = true;
    }
    else if (!np_drawn)
    {
      int lines = 1;
      if (pgno == np_pgno && (subno == VBI_ANY_SUBNO || subno == np_subno))
        lines = 25;
      dochttx_vbi_render(vbi->dec, np_pgno, np_subno, lines);
      if (lines == 25)
        show_display_info(vbi->dec, np_pgno, np_subno);
      np_drawn = true;
    }
    doupdate();
  }
  dochttx_ncurses_quit();
  dochttx_vbi_close(vbi);
  dochttx_locale_quit(); 
  return EXIT_SUCCESS;
}

// vim:ts=2 sw=2 et
