/* Copyright © 2005, 2006, 2008 Jakub Wilk
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; version 2 dated June, 1991.
 */

#include <langinfo.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "locale.h"

char* dochttx_charset = NULL;

bool dochttx_locale_init(void)
{
  if (setlocale(LC_ALL, "") == NULL)
    return false;
  dochttx_charset = strdup(nl_langinfo(CODESET));
  return dochttx_charset != NULL;
}

void dochttx_locale_quit()
{
  free(dochttx_charset);
}

// vim:ts=2 sw=2 et
