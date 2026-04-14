#include "themebrowser.h"
#include <string.h>
#include <stdio.h>

#define THEME_ROOT "ux0:app/RETROVITA/assets/xmb/"

void themebrowser_init(ThemeBrowser *tb) {
    tb->count    = 0;
    tb->selected = 0;
    tb->scroll   = 0;
    memset(tb->names, 0, sizeof(tb->names));
}

void themebrowser_scan(ThemeBrowser *tb) {
    tb->count = 0;
    SceUID dir = sceIoDopen(THEME_ROOT);
    if (dir < 0) return;
    SceIoDirent entry;
    while (sceIoDread(dir, &entry) > 0 && tb->count < MAX_THEMES) {
        if (entry.d_name[0] == '.') continue;
        if (!SCE_S_ISDIR(entry.d_stat.st_mode)) continue;
        snprintf(tb->names[tb->count], 32, "%s", entry.d_name);
        tb->count++;
    }
    sceIoDclose(dir);
}

void themebrowser_draw(ThemeBrowser *tb, vita2d_pgf *font) {
    vita2d_pgf_draw_text(font, 40, 80,
        RGBA8(0,200,160,255), 1.0f, "Saved Themes");

    if (tb->count == 0) {
        vita2d_pgf_draw_text(font, 40, 200,
            RGBA8(150,150,150,255), 1.0f,
            "No themes saved yet.");
        vita2d_pgf_draw_text(font, 40, 240,
            RGBA8(100,100,100,255), 1.0f,
            "Go to Save screen and create one first.");
        return;
    }

    int visible = 8;
    for (int i = 0; i < tb->count; i++) {
        int row = i - tb->scroll;
        if (row < 0 || row >= visible) continue;
        float y = 130 + row * 48;
        int sel = (i == tb->selected);

        if (sel) {
            vita2d_draw_rectangle(30, y - 24, 900, 44,
                RGBA8(0,60,50,255));
            vita2d_draw_rectangle(30, y - 24, 4, 44,
                RGBA8(0,200,160,255));
        }

        vita2d_pgf_draw_text(font, 50, y,
            sel ? RGBA8(0,200,160,255) : RGBA8(200,200,200,255),
            1.0f, tb->names[i]);

        if (sel)
            vita2d_pgf_draw_text(font, 700, y,
                RGBA8(100,100,100,255), 1.0f, "X:apply  Sq:delete");
    }

    if (tb->scroll > 0)
        vita2d_pgf_draw_text(font, 460, 100,
            RGBA8(0,200,160,255), 1.0f, "^ UP");
    if (tb->scroll + visible < tb->count)
        vita2d_pgf_draw_text(font, 460, 520,
            RGBA8(0,200,160,255), 1.0f, "v DOWN");

    vita2d_pgf_draw_textf(font, 40, 530,
        RGBA8(100,100,100,255), 1.0f,
        "%d themes  |  X: apply  Square: delete  Circle: back",
        tb->count);
}

int themebrowser_update(ThemeBrowser *tb, unsigned int pressed) {
    if (tb->count == 0) return 0;
    int visible = 8;

    if (pressed & SCE_CTRL_DOWN) {
        tb->selected++;
        if (tb->selected >= tb->count) tb->selected = tb->count - 1;
        if (tb->selected >= tb->scroll + visible)
            tb->scroll = tb->selected - visible + 1;
    }
    if (pressed & SCE_CTRL_UP) {
        tb->selected--;
        if (tb->selected < 0) tb->selected = 0;
        if (tb->selected < tb->scroll)
            tb->scroll = tb->selected;
    }
    if (pressed & SCE_CTRL_CROSS)  return 1;
    if (pressed & SCE_CTRL_SQUARE) return 2;
    return 0;
}

void themebrowser_delete_selected(ThemeBrowser *tb) {
    if (tb->count == 0) return;
    char path[256];
    snprintf(path, sizeof(path), "%s%s", THEME_ROOT, tb->names[tb->selected]);
    SceUID dir = sceIoDopen(path);
    if (dir >= 0) {
        SceIoDirent entry;
        while (sceIoDread(dir, &entry) > 0) {
            char fpath[256];
            snprintf(fpath, sizeof(fpath), "%s/%s", path, entry.d_name);
            sceIoRemove(fpath);
        }
        sceIoDclose(dir);
    }
    sceIoRmdir(path);
    for (int i = tb->selected; i < tb->count - 1; i++)
        snprintf(tb->names[i], 32, "%s", tb->names[i+1]);
    tb->count--;
    if (tb->selected >= tb->count && tb->selected > 0)
        tb->selected--;
}
