#include "bgpicker.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void bgpicker_init(BgPicker *bp) {
    bp->count = 0;
    bp->selected = 0;
    bp->scroll_row = 0;
    bp->show_props = 0;
    bp->confirm_delete = 0;
    bp->error[0] = '\0';
    memset(bp->paths, 0, sizeof(bp->paths));
    memset(bp->thumbs, 0, sizeof(bp->thumbs));
}

static int is_blocked(const char *path) {
    const char *blocked[] = {
        "ux0:data/vitaDB",
        "ux0:data/VitaDB",
        "ux0:data/VITADB",
        "ux0:data/vhbb",
        "ux0:data/VHBB",
        "ux0:data/pkg",
        "ux0:data/PKG",
        "ux0:data/vitahbBrowser", "ux0:data/VitaHBBrowser",
        "ux0:data/vitahbbrowser",
        NULL
    };
    for (int i = 0; blocked[i] != NULL; i++) {
        if (strncmp(path, blocked[i], strlen(blocked[i])) == 0)
            return 1;
    }
    return 0;
}

static void scan_dir(BgPicker *bp, const char *dirpath) {
    if (is_blocked(dirpath)) return;

    SceUID dir = sceIoDopen(dirpath);
    if (dir < 0) return;
    SceIoDirent entry;
    while (sceIoDread(dir, &entry) > 0 && bp->count < MAX_BG) {
        char *name = entry.d_name;
        if (name[0] == '.') continue;
        char fullpath[256];
        snprintf(fullpath, sizeof(fullpath), "%s%s", dirpath, name);
        if (SCE_S_ISDIR(entry.d_stat.st_mode)) {
            char subdir[256];
            snprintf(subdir, sizeof(subdir), "%s%s/", dirpath, name);
            if (!is_blocked(subdir))
                scan_dir(bp, subdir);
            continue;
        }
        int len = strlen(name);
        if (len > 4 && (
            strcasecmp(name + len - 4, ".png") == 0 ||
            strcasecmp(name + len - 4, ".jpg") == 0)) {
            snprintf(bp->paths[bp->count], 256, "%s", fullpath);
            bp->thumbs[bp->count] = NULL;
            bp->count++;
        }
    }
    sceIoDclose(dir);
}

void bgpicker_scan(BgPicker *bp) {
    const char *roots[] = {
        "ux0:picture/",
        "uma0:picture/",
        NULL
    };
    for (int i = 0; roots[i] != NULL && bp->count < MAX_BG; i++) {
        SceUID test = sceIoDopen(roots[i]);
        if (test >= 0) { sceIoDclose(test); scan_dir(bp, roots[i]); }
    }
    if (bp->count == 0)
        snprintf(bp->error, sizeof(bp->error),
            "No images found. Add PNG/JPG to ux0:picture/");
    else
        snprintf(bp->error, sizeof(bp->error),
            "Found %d images  |  X: select  Triangle: properties",
            bp->count);
}

void bgpicker_load_thumbs(BgPicker *bp) {
    for (int i = 0; i < bp->count; i++) {
        if (bp->thumbs[i]) continue;
        int len = strlen(bp->paths[i]);
        if (strcasecmp(bp->paths[i] + len - 4, ".png") == 0)
            bp->thumbs[i] = vita2d_load_PNG_file(bp->paths[i]);
        else
            bp->thumbs[i] = vita2d_load_JPEG_file(bp->paths[i]);
    }
}

static void draw_props_panel(BgPicker *bp, vita2d_pgf *font) {
    vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(0,0,0,180));
    vita2d_draw_rectangle(140, 120, 680, 300, RGBA8(30,30,30,255));
    vita2d_draw_rectangle(140, 120, 680, 2,   RGBA8(0,200,160,255));
    vita2d_draw_rectangle(140, 418, 680, 2,   RGBA8(0,200,160,255));
    vita2d_draw_rectangle(140, 120, 2,   300, RGBA8(0,200,160,255));
    vita2d_draw_rectangle(818, 120, 2,   300, RGBA8(0,200,160,255));

    vita2d_pgf_draw_text(font, 160, 150,
        RGBA8(0,200,160,255), 1.0f, "Image Properties");

    if (bp->thumbs[bp->selected]) {
        float sx = 120.0f / vita2d_texture_get_width(bp->thumbs[bp->selected]);
        float sy = 80.0f  / vita2d_texture_get_height(bp->thumbs[bp->selected]);
        vita2d_draw_texture_scale(bp->thumbs[bp->selected], 660, 170, sx, sy);
    }

    char *path = bp->paths[bp->selected];
    char *filename = path;
    for (char *p = path; *p; p++)
        if (*p == '/') filename = p + 1;

    SceIoStat stat;
    int file_size = -1;
    if (sceIoGetstat(path, &stat) == 0)
        file_size = stat.st_size;

    char folder[256];
    snprintf(folder, sizeof(folder), "%s", path);
    char *last_slash = folder;
    for (char *p = folder; *p; p++)
        if (*p == '/') last_slash = p;
    *last_slash = '\0';

    vita2d_pgf_draw_text(font, 160, 195,
        RGBA8(150,150,150,255), 1.0f, "Filename:");
    vita2d_pgf_draw_text(font, 160, 220,
        RGBA8(220,220,220,255), 1.0f, filename);
    vita2d_pgf_draw_text(font, 160, 255,
        RGBA8(150,150,150,255), 1.0f, "Location:");
    vita2d_pgf_draw_text(font, 160, 280,
        RGBA8(220,220,220,255), 1.0f, folder);
    vita2d_pgf_draw_text(font, 160, 315,
        RGBA8(150,150,150,255), 1.0f, "Full path:");
    vita2d_pgf_draw_text(font, 160, 340,
        RGBA8(220,220,220,255), 1.0f, path);

    if (file_size >= 0) {
        char size_str[64];
        if (file_size >= 1024*1024)
            snprintf(size_str, sizeof(size_str), "%.1f MB",
                file_size / (1024.0f*1024.0f));
        else
            snprintf(size_str, sizeof(size_str), "%d KB", file_size / 1024);
        vita2d_pgf_draw_text(font, 160, 375,
            RGBA8(150,150,150,255), 1.0f, "Size:");
        vita2d_pgf_draw_text(font, 240, 375,
            RGBA8(220,220,220,255), 1.0f, size_str);
    }

    vita2d_pgf_draw_text(font, 160, 408,
        RGBA8(200,80,80,255),  1.0f, "Square: DELETE");
    vita2d_pgf_draw_text(font, 500, 408,
        RGBA8(150,150,150,255), 1.0f, "Circle: close");
}

static void draw_confirm_dialog(vita2d_pgf *font) {
    vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(0,0,0,200));
    vita2d_draw_rectangle(280, 200, 400, 144, RGBA8(40,20,20,255));
    vita2d_draw_rectangle(280, 200, 400, 2,   RGBA8(200,80,80,255));
    vita2d_draw_rectangle(280, 342, 400, 2,   RGBA8(200,80,80,255));
    vita2d_draw_rectangle(280, 200, 2,   144, RGBA8(200,80,80,255));
    vita2d_draw_rectangle(678, 200, 2,   144, RGBA8(200,80,80,255));
    vita2d_pgf_draw_text(font, 340, 232,
        RGBA8(200,80,80,255),  1.0f, "Delete this image?");
    vita2d_pgf_draw_text(font, 320, 272,
        RGBA8(180,180,180,255), 1.0f, "This cannot be undone.");
    vita2d_pgf_draw_text(font, 300, 316,
        RGBA8(200,80,80,255),  1.0f, "Cross: YES delete");
    vita2d_pgf_draw_text(font, 480, 316,
        RGBA8(150,150,150,255), 1.0f, "Circle: NO");
}

void bgpicker_draw(BgPicker *bp, vita2d_pgf *font) {
    vita2d_pgf_draw_text(font, 20, 70,
        RGBA8(255,200,0,255), 1.0f, bp->error);
    if (bp->count == 0) return;

    int cols = 3, thumb_w = 280, thumb_h = 158, pad = 16;
    int visible_rows = 2;

    for (int i = 0; i < bp->count; i++) {
        int col = i % cols;
        int row = i / cols;
        int visible_row = row - bp->scroll_row;
        if (visible_row < 0 || visible_row >= visible_rows) continue;
        float x = pad + col * (thumb_w + pad);
        float y = 90 + visible_row * (thumb_h + pad);
        if (bp->thumbs[i]) {
            float sx = (float)thumb_w / vita2d_texture_get_width(bp->thumbs[i]);
            float sy = (float)thumb_h / vita2d_texture_get_height(bp->thumbs[i]);
            vita2d_draw_texture_scale(bp->thumbs[i], x, y, sx, sy);
        } else {
            vita2d_draw_rectangle(x, y, thumb_w, thumb_h, RGBA8(60,60,60,255));
            vita2d_pgf_draw_text(font, x+10, y+80,
                RGBA8(150,150,150,255), 1.0f, "...");
        }
        if (i == bp->selected) {
            vita2d_draw_rectangle(x-3, y-3, thumb_w+6, 3, RGBA8(0,200,160,255));
            vita2d_draw_rectangle(x-3, y+thumb_h, thumb_w+6, 3, RGBA8(0,200,160,255));
            vita2d_draw_rectangle(x-3, y-3, 3, thumb_h+6, RGBA8(0,200,160,255));
            vita2d_draw_rectangle(x+thumb_w, y-3, 3, thumb_h+6, RGBA8(0,200,160,255));
        }
    }

    int total_rows = (bp->count + cols - 1) / cols;
    if (bp->scroll_row > 0)
        vita2d_pgf_draw_text(font, 450, 95,
            RGBA8(0,200,160,255), 1.0f, "^ UP");
    if (bp->scroll_row + visible_rows < total_rows)
        vita2d_pgf_draw_text(font, 450, 500,
            RGBA8(0,200,160,255), 1.0f, "v DOWN");

    vita2d_pgf_draw_textf(font, 20, 530,
        RGBA8(150,150,150,255), 1.0f,
        "%d/%d  |  Triangle: properties",
        bp->selected + 1, bp->count);

    if (bp->confirm_delete)
        draw_confirm_dialog(font);
    else if (bp->show_props)
        draw_props_panel(bp, font);
}

int bgpicker_update(BgPicker *bp, unsigned int pressed) {
    if (bp->count == 0) return 0;
    int cols = 3, visible_rows = 2;

    if (bp->confirm_delete) {
        if (pressed & SCE_CTRL_CROSS) {
            sceIoRemove(bp->paths[bp->selected]);
            if (bp->thumbs[bp->selected])
                vita2d_free_texture(bp->thumbs[bp->selected]);
            for (int i = bp->selected; i < bp->count - 1; i++) {
                snprintf(bp->paths[i], 256, "%s", bp->paths[i+1]);
                bp->thumbs[i] = bp->thumbs[i+1];
            }
            bp->count--;
            if (bp->selected >= bp->count && bp->selected > 0)
                bp->selected--;
            snprintf(bp->error, sizeof(bp->error),
                "Deleted. %d images remaining.", bp->count);
            bp->confirm_delete = 0;
            bp->show_props = 0;
            return 0;
        }
        if (pressed & SCE_CTRL_CIRCLE) bp->confirm_delete = 0;
        return 0;
    }

    if (bp->show_props) {
        if (pressed & SCE_CTRL_SQUARE) bp->confirm_delete = 1;
        if (pressed & SCE_CTRL_CIRCLE) bp->show_props = 0;
        return 0;
    }

    if (pressed & SCE_CTRL_TRIANGLE) { bp->show_props = 1; return 0; }
    if (pressed & SCE_CTRL_CROSS)     return 1;

    if (pressed & SCE_CTRL_RIGHT) { bp->selected++; if (bp->selected >= bp->count) bp->selected = 0; }
    if (pressed & SCE_CTRL_LEFT)  { bp->selected--; if (bp->selected < 0) bp->selected = bp->count - 1; }
    if (pressed & SCE_CTRL_DOWN)  { bp->selected += cols; if (bp->selected >= bp->count) bp->selected = bp->count - 1; }
    if (pressed & SCE_CTRL_UP)    { bp->selected -= cols; if (bp->selected < 0) bp->selected = 0; }

    int selected_row = bp->selected / cols;
    if (selected_row < bp->scroll_row)
        bp->scroll_row = selected_row;
    if (selected_row >= bp->scroll_row + visible_rows)
        bp->scroll_row = selected_row - visible_rows + 1;

    return 0;
}

void bgpicker_free(BgPicker *bp) {
    for (int i = 0; i < bp->count; i++)
        if (bp->thumbs[i]) vita2d_free_texture(bp->thumbs[i]);
}
