#include "colorpicker.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define GRID_COLS 16
#define GRID_ROWS 8
#define CELL_W 40
#define CELL_H 36
#define GRID_X 60
#define GRID_Y 120

static unsigned int hsv_to_rgba(float h, float s, float v) {
    float r, g, b;
    int i = (int)(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r=v; g=t; b=p; break;
        case 1: r=q; g=v; b=p; break;
        case 2: r=p; g=v; b=t; break;
        case 3: r=p; g=q; b=v; break;
        case 4: r=t; g=p; b=v; break;
        default: r=v; g=p; b=q; break;
    }
    return RGBA8((int)(r*255),(int)(g*255),(int)(b*255),255);
}

static unsigned int hex_str_to_color(const char *hex) {
    unsigned int r=255, g=255, b=255;
    if (strlen(hex) == 6) {
        unsigned int val;
        if (sscanf(hex, "%06X", &val) == 1) {
            r = (val >> 16) & 0xFF;
            g = (val >> 8)  & 0xFF;
            b = (val >> 0)  & 0xFF;
        }
    }
    return RGBA8(r, g, b, 255);
}

void colorpicker_init(ColorPicker *cp) {
    cp->hue_col    = 0;
    cp->sat_row    = 0;
    cp->brightness = 1.0f;
    cp->selected   = RGBA8(255, 0, 0, 255);
    cp->mode       = CP_MODE_GRID;
    memset(cp->hex_input, '0', 6);
    cp->hex_input[6] = '\0';
    cp->hex_pos  = 0;
    cp->key_sel  = 0;
}

static void draw_grid(ColorPicker *cp) {
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            float h = col / (float)GRID_COLS;
            float s = 1.0f - (row / (float)(GRID_ROWS-1)) * 0.85f;
            unsigned int c = hsv_to_rgba(h, s, cp->brightness);
            float x = GRID_X + col * CELL_W;
            float y = GRID_Y + row * CELL_H;
            vita2d_draw_rectangle(x, y, CELL_W-2, CELL_H-2, c);
            if (col == cp->hue_col && row == cp->sat_row) {
                vita2d_draw_rectangle(x-2, y-2, CELL_W+2, 2, RGBA8(255,255,255,255));
                vita2d_draw_rectangle(x-2, y+CELL_H-2, CELL_W+2, 2, RGBA8(255,255,255,255));
                vita2d_draw_rectangle(x-2, y-2, 2, CELL_H+2, RGBA8(255,255,255,255));
                vita2d_draw_rectangle(x+CELL_W-2, y-2, 2, CELL_H+2, RGBA8(255,255,255,255));
            }
        }
    }
}

static void draw_brightness_slider(ColorPicker *cp, vita2d_pgf *font) {
    int sx = GRID_X, sy = GRID_Y + GRID_ROWS * CELL_H + 20;
    int sw = GRID_COLS * CELL_W, sh = 20;
    for (int i = 0; i < sw; i++) {
        float v = i / (float)sw;
        float h = cp->hue_col / (float)GRID_COLS;
        float s = 1.0f - (cp->sat_row / (float)(GRID_ROWS-1)) * 0.85f;
        vita2d_draw_rectangle(sx+i, sy, 1, sh, hsv_to_rgba(h, s, v));
    }
    vita2d_pgf_draw_text(font, sx, sy - 18, RGBA8(150,150,150,255), 0.9f, "Brightness");
    int knob_x = sx + (int)(cp->brightness * sw);
    vita2d_draw_rectangle(knob_x-3, sy-4, 6, sh+8, RGBA8(255,255,255,255));
    vita2d_draw_rectangle(knob_x-2, sy-3, 4, sh+6, RGBA8(50,50,50,255));
    char pct[8];
    snprintf(pct, sizeof(pct), "%d%%", (int)(cp->brightness * 100));
    vita2d_pgf_draw_text(font, sx + sw + 12, sy + 14, RGBA8(200,200,200,255), 1.0f, pct);
}

static void draw_hex_keyboard(ColorPicker *cp, vita2d_pgf *font) {
    vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(0,0,0,210));
    vita2d_draw_rectangle(200, 80, 560, 380, RGBA8(30,30,30,255));
    vita2d_draw_rectangle(200, 80, 560, 2, RGBA8(0,200,160,255));
    vita2d_draw_rectangle(200, 458, 560, 2, RGBA8(0,200,160,255));
    vita2d_draw_rectangle(200, 80, 2, 380, RGBA8(0,200,160,255));
    vita2d_draw_rectangle(758, 80, 2, 380, RGBA8(0,200,160,255));
    vita2d_pgf_draw_text(font, 230, 112, RGBA8(0,200,160,255), 1.0f, "Hex Color Input");
    char disp[10];
    snprintf(disp, sizeof(disp), "#%s", cp->hex_input);
    vita2d_pgf_draw_text(font, 280, 155, RGBA8(255,255,255,255), 1.2f, disp);
    vita2d_draw_rectangle(600, 128, 60, 38, hex_str_to_color(cp->hex_input));
    int cursor_x = 298 + cp->hex_pos * 19;
    const char *keys = "0123456789ABCDEF";
    for (int i = 0; i < 16; i++) {
        int col = i % 8, row = i / 8;
        float x = 220 + col * 66;
        float y = 250 + row * 70;
        int sel = (i == cp->key_sel);
        vita2d_draw_rectangle(x, y, 58, 60,
            sel ? RGBA8(0,200,160,255) : RGBA8(55,55,55,255));
        char k[2] = {keys[i], '\0'};
        vita2d_pgf_draw_text(font, x+22, y+38,
            sel ? RGBA8(0,0,0,255) : RGBA8(220,220,220,255), 1.0f, k);
    }
    vita2d_pgf_draw_text(font, 215, 440,
        RGBA8(150,150,150,255), 0.85f,
        "X:insert  L/R:move cursor  Sq:confirm  O:cancel");
}

void colorpicker_draw(ColorPicker *cp, vita2d_pgf *font) {
    draw_grid(cp);
    draw_brightness_slider(cp, font);
    vita2d_draw_rectangle(700, 120, 180, 100, cp->selected);
    char hex[8];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X",
        (cp->selected >> 0) & 0xFF,
        (cp->selected >> 8) & 0xFF,
        (cp->selected >> 16)& 0xFF);
    vita2d_pgf_draw_text(font, 700, 240,
        RGBA8(220,220,220,255), 1.0f, hex);
    vita2d_pgf_draw_text(font, 60, 490,
        RGBA8(150,150,150,255), 1.0f,
        "D-Pad:navigate  Analog:brightness  Triangle:hex  L:color  R:background  Select:preview");
    if (cp->mode == CP_MODE_HEX)
        draw_hex_keyboard(cp, font);
}

void colorpicker_update(ColorPicker *cp, unsigned int pressed, float ly) {
    if (cp->mode == CP_MODE_HEX) {
        if (pressed & SCE_CTRL_RIGHT) { cp->key_sel++; if (cp->key_sel > 15) cp->key_sel = 0; }
        if (pressed & SCE_CTRL_LEFT)  { cp->key_sel--; if (cp->key_sel < 0)  cp->key_sel = 15; }
        if (pressed & SCE_CTRL_DOWN)  { cp->key_sel += 8; if (cp->key_sel > 15) cp->key_sel -= 16; }
        if (pressed & SCE_CTRL_UP)    { cp->key_sel -= 8; if (cp->key_sel < 0)  cp->key_sel += 16; }
        if (pressed & SCE_CTRL_LTRIGGER) { cp->hex_pos--; if (cp->hex_pos < 0) cp->hex_pos = 5; }
        if (pressed & SCE_CTRL_RTRIGGER) { cp->hex_pos++; if (cp->hex_pos > 5) cp->hex_pos = 0; }
        if (pressed & SCE_CTRL_CROSS) {
            const char *keys = "0123456789ABCDEF";
            cp->hex_input[cp->hex_pos] = keys[cp->key_sel];
            if (cp->hex_pos < 5) cp->hex_pos++;
        }
        if (pressed & SCE_CTRL_SQUARE) {
            unsigned int c = hex_str_to_color(cp->hex_input);
            float r = (c & 0xFF) / 255.0f;
            float g = ((c >> 8) & 0xFF) / 255.0f;
            float b = ((c >> 16) & 0xFF) / 255.0f;
            float mx = r>g ? (r>b?r:b) : (g>b?g:b);
            float mn = r<g ? (r<b?r:b) : (g<b?g:b);
            cp->brightness = mx;
            float s = mx == 0 ? 0 : (mx-mn)/mx;
            float h = 0;
            if (mx == r) h = (g-b)/(mx-mn+0.0001f) / 6.0f;
            else if (mx == g) h = (2.0f + (b-r)/(mx-mn+0.0001f)) / 6.0f;
            else h = (4.0f + (r-g)/(mx-mn+0.0001f)) / 6.0f;
            if (h < 0) h += 1.0f;
            cp->hue_col = (int)(h * GRID_COLS) % GRID_COLS;
            cp->sat_row = (int)((1.0f - s / 0.85f) * (GRID_ROWS-1));
            if (cp->sat_row < 0) cp->sat_row = 0;
            if (cp->sat_row >= GRID_ROWS) cp->sat_row = GRID_ROWS-1;
            cp->selected = hex_str_to_color(cp->hex_input);
            cp->mode = CP_MODE_GRID;
        }
        if (pressed & SCE_CTRL_CIRCLE) cp->mode = CP_MODE_GRID;
        return;
    }
    if (pressed & SCE_CTRL_RIGHT) { cp->hue_col++; if (cp->hue_col >= GRID_COLS) cp->hue_col = 0; }
    if (pressed & SCE_CTRL_LEFT)  { cp->hue_col--; if (cp->hue_col < 0) cp->hue_col = GRID_COLS-1; }
    if (pressed & SCE_CTRL_DOWN)  { cp->sat_row++; if (cp->sat_row >= GRID_ROWS) cp->sat_row = GRID_ROWS-1; }
    if (pressed & SCE_CTRL_UP)    { cp->sat_row--; if (cp->sat_row < 0) cp->sat_row = 0; }
    if (pressed & SCE_CTRL_TRIANGLE) { cp->mode = CP_MODE_HEX; cp->key_sel = 0; cp->hex_pos = 0; }
    if (fabsf(ly) > 0.1f) {
        cp->brightness -= ly * 0.02f;
        if (cp->brightness > 1.0f) cp->brightness = 1.0f;
        if (cp->brightness < 0.05f) cp->brightness = 0.05f;
    }
    float h = cp->hue_col / (float)GRID_COLS;
    float s = 1.0f - (cp->sat_row / (float)(GRID_ROWS-1)) * 0.85f;
    cp->selected = hsv_to_rgba(h, s, cp->brightness);
}
