/* Copyright © 2005-2019 Jakub Wilk <jwilk@jwilk.net>
 * SPDX-License-Identifier: MIT
 */

#include <langinfo.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "locale.h"

char* dochttx_charset = NULL;

int dochttx_locale_init(void)
{
  if (setlocale(LC_ALL, "") == NULL)
    return -1;
  dochttx_charset = strdup(nl_langinfo(CODESET));
  if (dochttx_charset == NULL)
    return -1;
  return 0;
}

void dochttx_locale_quit(void)
{
  free(dochttx_charset);
}

// vim:ts=2 sts=2 sw=2 et
