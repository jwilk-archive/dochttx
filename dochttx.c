/* Copyright © 2005-2019 Jakub Wilk <jwilk@jwilk.net>
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include <curses.h>

#include "autoconf.h"
#include "locale.h"
#include "region.h"
#include "render.h"
#include "ui.h"
#include "vbi.h"

static const char default_device[] = "/dev/vbi0";

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

static void show_panel(vbi_decoder* dec, unsigned int pgno, unsigned int subno)
{
    vbi_subno maxsubno;
    vbi_classify_page(dec, pgno, &maxsubno, NULL);
    mvhline(4, 43, ' ', COLS - 43);
    mvprintw(4, 43, "Showing page %03x", pgno);
    if (maxsubno == 0)
        printw(", no subpages");
    else if (maxsubno == 0xFFFE || maxsubno == 0xFFFF)
        printw(", subpage %02x", subno);
    else if (maxsubno <= 0x3F7F)
        printw(", subpage %02x of %02x", subno, (unsigned int) maxsubno);
}

static void usage(FILE *fp)
{
    fprintf(fp, "Usage: dochttx [-d DEVICE] [-l LANG[,LANG...]]\n");
}

static void long_usage(FILE *fp)
{
    usage(fp);
    printf("\n"
        "Options:\n"
        "  -d DEVICE          VBI device to use (default: %s)\n"
        "  -l LANG[,LANG...]  use character sets for these languages\n"
        "                     (default: locale-dependent)\n"
        "  -h, --help         show this help message and exit\n"
        "  --version          show version information and exit\n",
        default_device
    );
}

static void print_version(void)
{
    unsigned int major, minor, micro;
    printf("%s\n", PACKAGE_STRING);
    vbi_version(&major, &minor, &micro);
    printf("+ ZVBI %u.%u.%u\n", major, minor, micro);
}

static int parse_pagespec(const char *pagespec, unsigned int *pgno, unsigned int *subno)
{
    int rc;
    static regex_t regexp;
    static bool regexp_initialized = false;
    if (!regexp_initialized) {
        rc = regcomp(&regexp,
                /* we don't use ranges such as 1-8 here,
                  * they're undefined outside the POSIX locale */
                "^[12345678]"
                "[0123456789abcdefABCDEF]{2}"
                "([.]([0123456789]|[01234567][0123456789]))?$",
                REG_EXTENDED | REG_NOSUB
            );
        assert(rc == 0);
        regexp_initialized = true;
    }
    rc = regexec(&regexp, pagespec, 0, NULL, 0);
    if (rc == REG_NOMATCH) {
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
    int region = -1;
    const char *device = default_device;
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
    while ((opt = getopt_long(argc, argv, "d:l:h", long_options, NULL)) != -1)
        switch (opt)
        {
        case 'd':
            device = optarg;
            break;
        case 'l':
            region = dochttx_region_for_lang(optarg);
            if (region < 0) {
                errno = EINVAL;
                perror("dochttx: -l");
                exit(EXIT_FAILURE);
            }
            break;
        case 'h':
            long_usage(stdout);
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

    int rc = dochttx_locale_init();
    if (rc < 0) {
        perror("dochttx: locale initialization failed");
        return EXIT_FAILURE;
    }

    if (region < 0)
        region = dochttx_region_for_locale();
    if (region < 0)
        region = dochttx_region_for_lang("en");
    assert(region >= 0);

    struct dochttx_vbi_state* vbi = dochttx_vbi_open(device, region);
    if (vbi == NULL)
        return EXIT_FAILURE;

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
    while (true) {
        struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
        fd_set rdfs;
        FD_ZERO(&rdfs);
        FD_SET(STDIN_FILENO, &rdfs);
        FD_SET(vbi->fd, &rdfs);
        int rs = select(vbi->fd+1, &rdfs, NULL, NULL, &tv);
        if (rs == -1) {
            if (errno == EINTR)
                continue;
            break;
        }
        if (FD_ISSET(vbi->fd, &rdfs))
            dochttx_vbi_has_data(vbi);
        if (FD_ISSET(STDIN_FILENO, &rdfs)) {
            bool do_quit = false;
            int chr = getch();
            switch (chr) {
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
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                chr = chr - 'a' + 'A';
                /* fall through */
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                if (lf_status == 2) {
                    memset(lf, 0, sizeof(lf));
                    lf[0] = (char)chr;
                    lf_pos = 1;
                }
                else
                /* fall through */
            case '.':
                if (lf[5] == '\0') {
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
                    if (parse_pagespec(lf, &new_pgno, &new_subno) >= 0) {
                        pgno = new_pgno;
                        subno = new_subno;
                        drawn = false;
                        lf_status = 2;
                        char subnos[3] = "*";
                        if (subno != VBI_ANY_SUBNO)
                            sprintf(subnos, "%02x", subno);
                        mvhline(2, 43, ' ', COLS - 43);
                        mvprintw(2, 43, "Looking for %03X.%s", pgno, subnos);
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
        if (lf_update) {
            mvhline(0, 53, '_', 6);
            switch (lf_status) {
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
        if (!drawn && vbi_is_cached(vbi->dec, pgno, subno)) {
            vbi_subno tmp_subno = dochttx_vbi_render(vbi->dec, pgno, subno, 25);
            show_panel(vbi->dec, pgno, tmp_subno);
            drawn = true;
        }
        else if (!np_drawn) {
            int lines = 1;
            if (pgno == np_pgno && (subno == VBI_ANY_SUBNO || subno == np_subno))
                lines = 25;
            dochttx_vbi_render(vbi->dec, np_pgno, np_subno, lines);
            if (lines == 25)
                show_panel(vbi->dec, np_pgno, np_subno);
            np_drawn = true;
        }
        doupdate();
    }
    dochttx_ncurses_quit();
    dochttx_vbi_close(vbi);
    dochttx_locale_quit();
    return EXIT_SUCCESS;
}

// vim:ts=4 sts=4 sw=4 et
