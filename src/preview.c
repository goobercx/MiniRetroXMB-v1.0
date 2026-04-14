#include "preview.h"
#include <stdio.h>

void preview_draw(vita2d_pgf *font, ColorPicker *cp, BgPicker *bp) {
    int has_bg = (bp->count > 0 && bp->thumbs[bp->selected] != NULL);
    if (has_bg) {
        float sx = 960.0f / vita2d_texture_get_width(bp->thumbs[bp->selected]);
        float sy = 544.0f / vita2d_texture_get_height(bp->thumbs[bp->selected]);
        vita2d_draw_texture_scale(bp->thumbs[bp->selected], 0, 0, sx, sy);
        vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(0,0,0,80));
    } else {
        vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(20,20,20,255));
    }
    unsigned int tc = cp->selected;
    unsigned int text_color = RGBA8(
        (tc>>0)&0xFF,
        (tc>>8)&0xFF,
        (tc>>16)&0xFF,
        255);
    vita2d_draw_rectangle(0, 180, 960, 1, RGBA8(255,255,255,30));
    vita2d_draw_rectangle(0, 340, 960, 1, RGBA8(255,255,255,30));
    const char *items[] = {"Main Menu", "Settings", "History", "Favorites", "Image Viewer"};
    for (int i = 0; i < 5; i++) {
        int y = 200 + i * 28;
        unsigned int col = (i == 0) ? text_color : RGBA8(180,180,180,180);
        float scale = (i == 0) ? 1.1f : 0.9f;
        vita2d_pgf_draw_text(font, 80, y, col, scale, items[i]);
    }
    vita2d_draw_rectangle(60, 195, 4, 30, text_color);
    vita2d_pgf_draw_text(font, 500, 200, text_color, 1.0f, "Persona 4 Golden");
    vita2d_pgf_draw_text(font, 500, 230, RGBA8(180,180,180,200), 0.9f, "Atlus  |  2012");
    vita2d_pgf_draw_text(font, 500, 260, RGBA8(180,180,180,200), 0.9f, "Role Playing Game");
    vita2d_draw_rectangle(500, 290, 180, 2, text_color);
    vita2d_pgf_draw_text(font, 500, 320, text_color, 0.9f, "Start Game");
    vita2d_pgf_draw_text(font, 500, 350, RGBA8(180,180,180,180), 0.9f, "Resume");
    vita2d_pgf_draw_text(font, 500, 380, RGBA8(180,180,180,180), 0.9f, "Save State");
    vita2d_draw_rectangle(0, 510, 960, 34, RGBA8(0,0,0,160));
    char hex[16];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X",
        (tc>>0)&0xFF, (tc>>8)&0xFF, (tc>>16)&0xFF);
    vita2d_pgf_draw_text(font, 20, 532,
        RGBA8(180,180,180,255), 1.0f,
        "PREVIEW  |  Cross: save   Circle: back");
    vita2d_pgf_draw_text(font, 700, 532, text_color, 1.0f, hex);
    if (!has_bg)
        vita2d_pgf_draw_text(font, 350, 140,
            RGBA8(150,150,150,255), 0.9f, "No background selected");
}
