#ifndef DOCHTTX_VBI_DATA_H
#define DOCHTTX_VBI_DATA_H

#include <libzvbi.h>

struct dochttx_vbi_state
{
    vbi_decoder *dec;
    vbi_capture *cap;
    vbi_raw_decoder *par;
    vbi_sliced *sliced;
    uint8_t *raw;
    char *err;
    int fd;
};

struct dochttx_vbi_state *dochttx_vbi_open(const char *, int);
int dochttx_vbi_read_data(struct dochttx_vbi_state *);
void dochttx_vbi_close(struct dochttx_vbi_state *);

#endif

// vim:ts=4 sts=4 sw=4 et
