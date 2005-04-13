#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncursesw/ncurses.h>

#include "ui.h"
#include "vbi.h"
#include "render.h"
#include "locale.h"

static int np_pgno = 0;
static int np_subno = 0;
static bool np_drawn = true;

void intercept(vbi_event *ev, void *dec)
{
  dec = dec;
  np_pgno = ev->ev.ttx_page.pgno;
  np_subno = ev->ev.ttx_page.subno;
  np_drawn = false;
}

int main(void)
{
  locale_init();
  ncurses_init();
 
  char lf[8];
  int lfpos = 0;
  int lfstatus = 1;
 
  mvvline(0, 41, ACS_VLINE, 25);
  for (int y=1; y<25; y++)
    mvhline(y, 0, ACS_BOARD, 41);
  mvhline(25, 0, ACS_HLINE, COLS);
  mvaddch(25, 41, ACS_BTEE);
  mvprintw(0, 42, "Look for: (______)");
  wnoutrefresh(stdscr);
  setsyx(0, 53+lfpos);
  doupdate();
  
  struct vbi_state* vbi = vbi_open("/dev/vbi0", 8);
  vbi_event_handler_register(vbi->dec, VBI_EVENT_TTX_PAGE, intercept, (void*) vbi->dec);
  
  int pgno = 0x100;
  int subno = VBI_ANY_SUBNO;
  bool drawn = false;
  memset(lf, 0, sizeof(lf));
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
      vbi_hasdata(vbi);
    if (FD_ISSET(STDIN_FILENO, &rdfs))
    {
      bool do_quit = false;
      int chr = getch();
      switch (chr)
      {
      case L'C'-L'@': // Ctrl + C
        do_quit = true;
        break;
      case '0': case '1': case '2': case '3': case '4': 
      case '5': case '6': case '7': case '8': case '9':
      case '.':
        lf[lfpos++] = (char)chr;
        mvaddch(0, 52+lfpos, chr);
        if (lfpos >= 6)
          lfpos = 5;
        lfstatus = 0;
        wnoutrefresh(stdscr);
        setsyx(0, 53+lfpos);
        break;
      case KEY_BACKSPACE:
      case '\x7f':
      case '\b':
        if (lfpos > 0)
        {
          lf[--lfpos] = '\0';
          lfstatus = 0;
        }
        break;
      case KEY_ENTER:
      case '\n':
      case '\r':
        {
          unsigned int new_pgno = 0;
          unsigned int new_subno = VBI_ANY_SUBNO;
          if (sscanf(lf, "%x.%x", &new_pgno, &new_subno) < 1)
            break;
          if (new_pgno >= 0x100 && new_pgno <= 0x899 && (new_subno <= 0x99 || new_subno == VBI_ANY_SUBNO))
          {
            pgno = new_pgno;
            subno = new_subno;
            drawn = false;
            lfstatus = -1;
          } 
          else
            lfstatus = -2;
        }
        break;
      }
      if (do_quit)
        break;
      if (lfstatus <= 0)
      {
        lfstatus = -lfstatus;
        mvprintw(0, 53, "______");
        switch (lfstatus)
        {
        case 1:
          attrset(A_BOLD);
          break;
        case 2:
          attrset(A_BOLD | colors[7][1]);
          break;
        default:
          attrset(0);
        }
        mvprintw(0, 53, lf);
        attrset(0);
        wnoutrefresh(stdscr);
      }
    }
    if (!drawn && vbi_is_cached(vbi->dec, pgno, subno))
    {
      vbi_render(vbi->dec, pgno, subno, 25);
      drawn = true;
    }
    else if (!np_drawn)
    {
      int lines = 1;
      if (pgno == np_pgno && (subno == VBI_ANY_SUBNO || subno == np_subno))
        lines = 25;
      vbi_render(vbi->dec, np_pgno, np_subno, lines);
      np_drawn = true;
    }
    doupdate();
  }
  ncurses_quit();
  vbi_close(vbi);
  locale_quit(); 
  return EXIT_SUCCESS;
}

// vim:ts=2 sw=2 et
