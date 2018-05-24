/* Copyright © 2005-2018 Jakub Wilk <jwilk@jwilk.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "anycurses.h"
#include "vbi.h"

short dochttx_vbi_colors[8] =
{ COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
  COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE };

struct dochttx_vbi_state *dochttx_vbi_open(const char *dev, int region)
{
  struct dochttx_vbi_state *vbi;
  unsigned int services =
    VBI_SLICED_VBI_525 | VBI_SLICED_VBI_625 |
    VBI_SLICED_TELETEXT_B | VBI_SLICED_CAPTION_525 |
    VBI_SLICED_CAPTION_625 | VBI_SLICED_VPS |
    VBI_SLICED_WSS_625 | VBI_SLICED_WSS_CPR1204;
  const char *error = NULL;
  do
  {
    size_t lines;
    vbi = malloc(sizeof(*vbi));
    if (vbi == NULL)
      break;
    memset(vbi, 0, sizeof(*vbi));
    vbi->dec = vbi_decoder_new();
    if (vbi->dec == NULL)
      break;
    vbi_teletext_set_default_region(vbi->dec, region);
    vbi->cap = vbi_capture_v4l2_new(dev, 16, &services, -1, &vbi->err, 0);
    if (vbi->cap == NULL)
    {
      error = vbi->err;
      break;
    }
    vbi->par = vbi_capture_parameters(vbi->cap);
    vbi->fd = vbi_capture_fd(vbi->cap);
    lines = vbi->par->count[0] + vbi->par->count[1];
    vbi->raw = malloc(lines * vbi->par->bytes_per_line);
    if (vbi->raw == NULL)
      break;
    vbi->sliced = malloc(lines * sizeof(vbi_sliced));
    if (vbi->sliced == NULL)
      break;
    vbi->tv.tv_sec = 1;
    vbi->tv.tv_usec = 0;
    return vbi;
  }
  while (false);
  if (error == NULL)
    error = strerror(errno);
  dochttx_vbi_close(vbi);
  fprintf(stderr, "dochttx: %s\n", error);
  return NULL;
}

int dochttx_vbi_has_data(struct dochttx_vbi_state *vbi)
{
  int rc;
  int lines;
  rc = vbi_capture_read(vbi->cap, vbi->raw, vbi->sliced, &lines, &vbi->ts, &vbi->tv);
  vbi_decode(vbi->dec, vbi->sliced, lines, vbi->ts);
  return rc;
}

void dochttx_vbi_close(struct dochttx_vbi_state *vbi)
{
  if (vbi == NULL)
    return;
  free(vbi->sliced);
  free(vbi->raw);
  if (vbi->cap != NULL)
    vbi_capture_delete(vbi->cap);
  if (vbi->dec != NULL)
    vbi_decoder_delete(vbi->dec);
  free(vbi);
}

// vim:ts=2 sts=2 sw=2 et
