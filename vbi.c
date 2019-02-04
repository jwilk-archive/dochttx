/* Copyright © 2005-2019 Jakub Wilk <jwilk@jwilk.net>
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>

#include "vbi.h"

enum file_type {
    TYPE_DEV_NULL,
    TYPE_OTHER,
};

enum file_type classify_stat(const struct stat *st)
{
#if __linux__
    if (S_ISCHR(st->st_mode) && st->st_rdev == makedev(1, 3))
       return TYPE_DEV_NULL;
    return TYPE_OTHER;
#else
#error non-Linux systems are not supported yet
#endif
}

struct dochttx_vbi_state *dochttx_vbi_open(const char *dev, int region)
{
    struct dochttx_vbi_state *vbi;
    unsigned int services =
        VBI_SLICED_TELETEXT_B |
        VBI_SLICED_WSS_625;

    const char *econtext = NULL;
    const char *error = NULL;
    do {
        size_t lines;
        vbi = malloc(sizeof(*vbi));
        if (vbi == NULL)
            break;
        memset(vbi, 0, sizeof(*vbi));
        vbi->dec = vbi_decoder_new();
        if (vbi->dec == NULL)
            break;
        vbi_teletext_set_default_region(vbi->dec, region);
        struct stat st;
        int rc = stat(dev, &st);
        if (rc < 0) {
            econtext = dev;
            break;
        }
        vbi->err = NULL;
        if (classify_stat(&st) == TYPE_DEV_NULL)
            vbi->cap = vbi_capture_sim_new(625, &services, false, true);
        else
            vbi->cap = vbi_capture_v4l2_new(dev, 16, &services, -1, &vbi->err, 0);
        if (vbi->cap == NULL) {
            error = vbi->err;
            break;
        }
        vbi->par = vbi_capture_parameters(vbi->cap);
        vbi->fd = vbi_capture_fd(vbi->cap);
        if (classify_stat(&st) != TYPE_DEV_NULL && vbi->fd < 0) {
            errno = EBADF;
            break;
        }
        lines = vbi->par->count[0] + vbi->par->count[1];
        vbi->raw = malloc(lines * vbi->par->bytes_per_line);
        if (vbi->raw == NULL)
            break;
        vbi->sliced = malloc(lines * sizeof(vbi_sliced));
        if (vbi->sliced == NULL)
            break;
        return vbi;
    }
    while (false);
    if (error == NULL)
        error = strerror(errno);
    fprintf(stderr, "dochttx: ");
    if (econtext != NULL)
        fprintf(stderr, "%s: ", econtext);
    fprintf(stderr, "%s\n", error);
    dochttx_vbi_close(vbi);
    return NULL;
}

int dochttx_vbi_read_data(struct dochttx_vbi_state *vbi)
{
    int rc;
    int lines;
    double ts;
    struct timeval tv = { .tv_sec = 1 };
    rc = vbi_capture_read(vbi->cap, vbi->raw, vbi->sliced, &lines, &ts, &tv);
    if (rc > 0)
        vbi_decode(vbi->dec, vbi->sliced, lines, ts);
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

// vim:ts=4 sts=4 sw=4 et
