#ifndef DOCHTTX_REGION_TABLE_H
#define DOCHTTX_REGION_TABLE_H

struct langdatum {
    const char *tag;
    int regionset;
};

extern const struct langdatum langdata[];

#endif

// vim:ts=4 sts=4 sw=4 et
