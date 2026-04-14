#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <vita2d.h>
#include <psp2/ctrl.h>

typedef enum { CP_MODE_GRID, CP_MODE_HEX } CPMode;

typedef struct {
    int hue_col;
    int sat_row;
    float brightness;
    unsigned int selected;
    CPMode mode;
    char hex_input[7];
    int hex_pos;
    int key_sel;
} ColorPicker;

void colorpicker_init(ColorPicker *cp);
void colorpicker_draw(ColorPicker *cp, vita2d_pgf *font);
void colorpicker_update(ColorPicker *cp, unsigned int pressed, float ly);

#endif
