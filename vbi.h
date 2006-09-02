#ifndef _VBI_DATA_H
#define _VBI_DATA_H

#include <libzvbi.h>

struct vbi_state
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

extern short vbi_colors[8];

struct vbi_state *vbi_open(char *, int);
int vbi_has_data(struct vbi_state *);
void vbi_close(struct vbi_state *);

#endif

// vim:ts=2 sw=2 et
