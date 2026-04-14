#ifndef BGPICKER_H
#define BGPICKER_H

#include <vita2d.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/ctrl.h>

#define MAX_BG 48

typedef struct {
    int count;
    int selected;
    int scroll_row;
    int show_props;
    int confirm_delete;
    char error[128];
    char paths[MAX_BG][256];
    vita2d_texture *thumbs[MAX_BG];
} BgPicker;

void bgpicker_init(BgPicker *bp);
void bgpicker_scan(BgPicker *bp);
void bgpicker_load_thumbs(BgPicker *bp);
void bgpicker_draw(BgPicker *bp, vita2d_pgf *font);
int  bgpicker_update(BgPicker *bp, unsigned int pressed);
void bgpicker_free(BgPicker *bp);

#endif
