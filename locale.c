/* Copyright © 2005-2018 Jakub Wilk <jwilk@jwilk.net>
 * SPDX-License-Identifier: MIT
 */

#include <langinfo.h>
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "locale.h"

char* dochttx_charset = NULL;

bool dochttx_locale_init(void)
{
  if (setlocale(LC_ALL, "") == NULL)
    return false;
  dochttx_charset = strdup(nl_langinfo(CODESET));
  return dochttx_charset != NULL;
}

void dochttx_locale_quit(void)
{
  free(dochttx_charset);
}

// vim:ts=2 sts=2 sw=2 et
