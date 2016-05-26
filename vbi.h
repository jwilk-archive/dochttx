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
  int lines, fd;
  double ts;
  struct timeval tv;
};

extern short dochttx_vbi_colors[8];

struct dochttx_vbi_state *dochttx_vbi_open(char *, int);
int dochttx_vbi_has_data(struct dochttx_vbi_state *);
void dochttx_vbi_close(struct dochttx_vbi_state *);

#endif

// vim:ts=2 sts=2 sw=2 et
