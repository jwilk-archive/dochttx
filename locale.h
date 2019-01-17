#ifndef DOCHTTX_LOCALE_H
#define DOCHTTX_LOCALE_H

#include <stdbool.h>

extern char* dochttx_charset;
extern char* dochttx_lang;
extern bool dochttx_latin;

int dochttx_locale_init(void);
void dochttx_locale_quit(void);

#endif

// vim:ts=4 sts=4 sw=4 et
