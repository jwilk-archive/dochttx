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

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "anycurses.h"
#include "locale.h"
#include "render.h"
#include "ui.h"
#include "vbi.h"

static vbi_pgno np_pgno = 0;
static vbi_subno np_subno = 0;
static bool np_drawn = true;

static void intercept(vbi_event *ev, void *dec)
{
  (void) dec; /* unused */
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
  else if (maxsubno <= 0x3F7F)
    printw(", subpage %02x of %02x", subno, maxsubno);
}

void usage(FILE *fp)
{
  fprintf(fp, "Usage: dochttx [-d DEVICE]\n");
}

void print_version(void)
{
  unsigned int major, minor, micro;
  printf("%s\n", PACKAGE_STRING);
  vbi_version(&major, &minor, &micro);
  printf("+ ZVBI %u.%u.%u\n", major, minor, micro);
}

int parse_pagespec(const char *pagespec, unsigned int *pgno, unsigned int *subno)
{
  int rc;
  static regex_t regexp;
  static bool regexp_initialized = false;
  if (!regexp_initialized)
  {
    rc = regcomp(&regexp, "^[1-8][0-9]{2}([.][0-9]{1,2})?$", REG_EXTENDED | REG_NOSUB);
    assert(rc == 0);
    regexp_initialized = true;
  }
  rc = regexec(&regexp, pagespec, 0, NULL, 0);
  if (rc == REG_NOMATCH)
  {
    *pgno = *subno = 0;
    return -1;
  }
  *subno = VBI_ANY_SUBNO;
  rc = sscanf(pagespec, "%x.%x", pgno, subno);
  assert(rc == 1 || rc == 2);
  return 0;
}

int main(int argc, char **argv)
{
  const char *device = "/dev/vbi0";
  int opt;
  enum {
    OPT_DUMMY = CHAR_MAX,
    OPT_VERSION
  };
  static struct option long_options[] = {
    {"help", no_argument, NULL, 'h' },
    {"version", no_argument, NULL, OPT_VERSION },
    {NULL, 0, NULL, 0}
  };
  while ((opt = getopt_long(argc, argv, "d:h", long_options, NULL)) != -1)
    switch (opt)
    {
    case 'd':
      device = optarg;
      break;
    case 'h':
      usage(stdout);
      exit(EXIT_SUCCESS);
    case OPT_VERSION:
      print_version();
      exit(EXIT_SUCCESS);
    default: /* '?' */
      usage(stderr);
      exit(EXIT_FAILURE);
    }
  if (optind != argc) {
    usage(stderr);
    exit(EXIT_FAILURE);
  }

  dochttx_locale_init();

  struct dochttx_vbi_state* vbi = dochttx_vbi_open(device, 8);
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
        /* fall through */
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
        /* fall through */
      case KEY_BACKSPACE:
      case '\x7F':
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
          unsigned int new_pgno, new_subno;
          if (parse_pagespec(lf, &new_pgno, &new_subno) >= 0)
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

// vim:ts=2 sts=2 sw=2 et
