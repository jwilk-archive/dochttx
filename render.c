/* Copyright © 2005-2019 Jakub Wilk <jwilk@jwilk.net>
 * SPDX-License-Identifier: MIT
 */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
/* needed for wcwidth(3) */
#endif

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <curses.h>

#include "render.h"
#include "ui.h"

static int get_curses_color(const vbi_page *pg, int n, int fallback)
{
    const int color_map_size = sizeof pg->color_map / sizeof pg->color_map[0];
    if (n < 0 || n >= color_map_size) {
        assert(fallback >= 0);
        assert(fallback < 8);
        return fallback;
    }
    vbi_rgba rgb = pg->color_map[n];
    int min_d2 = INT_MAX;
    int j = -1;
    for (int i = 0; i < 8; i++) {
        int dr = (rgb & 0xFF) - (i & 1 ? 0xFF : 0);
        int dg = ((rgb >> 8) & 0xFF) - (i & 2 ? 0xFF : 0);
        int db = ((rgb >> 16) & 0xFF) - (i & 4 ? 0xFF : 0);
        int d2 = dr * dr + dg * dg + db * db;
        if (d2 < min_d2) {
            min_d2 = d2;
            j = i;
        }
    }
    assert(j >= 0);
    assert(j < 8);
    return j;
}

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
    for (y = 0; y < pg->rows && y < lines; y++) {
        for (x = 0; x < pg->columns; x++, ch++) {
            wcs[0] = ch->unicode;
            if (ch->size > VBI_DOUBLE_SIZE || ch->conceal)
                wcs[0] = L' ';
            int fg_color = get_curses_color(pg, ch->foreground, 7);
            int bg_color = get_curses_color(pg, ch->background, 0);
            attrset(dochttx_colors[fg_color][bg_color]);
            if (ch->bold)
                attron(A_BOLD);
            if (ch->flash)
                attron(A_BLINK);
            if (wcs[0] == 0x01B5) /* Ƶ (Latin capital letter Z with stroke) */
                wcs[0] = 0x017B;  /* Ż (Latin capital letter Z with dot above) */
            else if (wcs[0] >= 0xEE00 && wcs[0] < 0xEE80) {
                /* G1 Block Mosaic Set → braille patterns */
                int i = wcs[0] - 0xEE00;
                if (i & 0x20) {
                    attron(A_BOLD);
                    i -= 0x20;
                }
                if (i >= 0x40)
                    i -= 0x20;
                assert(i >= 0);
                assert(i < (1 << 6));
                wchar_t c = 0x2800;
                for (int x = 0; x < 2; x++)
                for (int y = 0; y < 3; y++)
                    c |= !!(i & (1 << (2 * y + x))) << (3 * x + y);
                assert(c >= 0x2800);
                assert(c <= 0x283F);
                wcs[0] = c;
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

// vim:ts=4 sts=4 sw=4 et
