/* Copyright © 2019 Jakub Wilk <jwilk@jwilk.net>
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "locale.h"
#include "region-table.h"
#include "region.h"

static int get_one_region(int rs)
{
    for (int r = -1; r <= 10; r++) {
        int r2 = r;
        if (r2 < 0)
            /* prefer ZVBI's default, i.e. Western Europe */
            r2 = 2;
        if ((1 << r2) & rs)
            return 8 * r2;
    }
    return -1;
}

int dochttx_region_for_lang(const char *lang)
{
    int rs = 0xFFFF;
    assert(lang != NULL);
    while (*lang) {
        const char *lang_end = strchr(lang, ',');
        size_t lang_len;
        if (lang_end == NULL)
            lang_len = strlen(lang);
        else
            lang_len = lang_end - lang;
        bool found = false;
        for (const struct langdatum *ld = langdata; ld->tag != NULL; ld++)
            if (strncmp(lang, ld->tag, lang_len) == 0 && ld->tag[lang_len] == '\0') {
                rs &= ld->regionset;
                found = true;
                break;
            }
        if (!found)
            return -1;
        lang += lang_len;
        while (*lang == ',')
            lang++;
    }
    return get_one_region(rs);
}

int dochttx_region_for_locale(void)
{
    assert(dochttx_lang != NULL);
    if (strlen(dochttx_lang) != 2)
        return -1;
    int rs = 0xFFFF;
    for (const struct langdatum *ld = langdata; ld->tag != NULL; ld++) {
        if (strncmp(dochttx_lang, ld->tag, 2) != 0)
            continue;
        if (ld->tag[2] == '-') {
            const char *script = dochttx_latin ? "Latn" : "Cyrl";
            if (strcmp(ld->tag + 3, script) != 0)
                continue;
        }
        rs &= ld->regionset;
    }
    return get_one_region(rs);
}

// vim:ts=4 sts=4 sw=4 et
