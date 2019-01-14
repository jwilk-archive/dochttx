/* Copyright © 2005-2019 Jakub Wilk <jwilk@jwilk.net>
 * SPDX-License-Identifier: MIT
 */

#define _POSIX_C_SOURCE 200809L

#include <langinfo.h>
#include <locale.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "locale.h"

char* dochttx_charset = NULL;
char* dochttx_lang = NULL;
bool dochttx_latin = true;

int dochttx_locale_init(void)
{
  const char *locale;
  locale = setlocale(LC_ALL, "");
  if (locale == NULL)
    return -1;
  dochttx_charset = strdup(nl_langinfo(CODESET));
  if (dochttx_charset == NULL)
    return -1;
  locale = setlocale(LC_CTYPE, "");
  if (locale == NULL)
    return -1;
  const char *tmp = strchr(locale, '_');
  if (tmp == NULL)
    dochttx_lang = strdup(locale);
  else
    dochttx_lang = strndup(locale, tmp - locale);
  if (dochttx_lang == NULL)
    return -1;
  locale = setlocale(LC_TIME, locale);
  if (locale == NULL)
    return -1;
  const char *text = nl_langinfo(MON_9);
  dochttx_latin = false;
  for (tmp=text; *tmp; tmp++)
    if ((*tmp >= 'a' && *tmp <= 'z') || (*tmp >= 'A' && *tmp <= 'Z')) {
      dochttx_latin = true;
      break;
    }
  locale = setlocale(LC_TIME, "");
  if (locale == NULL)
    return -1;
  return 0;
}

void dochttx_locale_quit(void)
{
  free(dochttx_charset);
}

// vim:ts=2 sts=2 sw=2 et
