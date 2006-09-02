/* Copyright (c) 2005, 2006 Jakub Wilk
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published 
 * by the Free Software Foundation.
 */

#include <langinfo.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "locale.h"

char* charset = NULL;

bool locale_init(void)
{
  if (setlocale(LC_ALL, "") == NULL)
    return false;
  charset = strdup(nl_langinfo(CODESET));
  return charset != NULL;
}

void locale_quit()
{
  free(charset);
}

// vim:ts=2 sw=2 et
