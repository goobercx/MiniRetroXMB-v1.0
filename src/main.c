#include <vita2d.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <string.h>
#include <stdio.h>
#include "colorpicker.h"
#include "bgpicker.h"
#include "export.h"
#include "preview.h"
#include "themebrowser.h"

typedef enum { SCREEN_COLOR, SCREEN_BG, SCREEN_PREVIEW, SCREEN_EXPORT, SCREEN_THEMES } Screen;

static void draw_export_screen(vita2d_pgf *font, int editing_name, int name_key_sel,
    ColorPicker *cp, BgPicker *bp,
    const char *theme_name, const char *status_msg) {

    vita2d_pgf_draw_text(font, 40, 100,
        RGBA8(200,200,200,255), 1.0f, "Theme Name:");
    vita2d_pgf_draw_text(font, 40, 130,
        RGBA8(255,255,255,255), 1.0f, theme_name);
    vita2d_draw_rectangle(500, 160, 440, 110, RGBA8(60,30,0,255));
    vita2d_draw_rectangle(500, 160, 440, 2, RGBA8(255,160,0,255));
    vita2d_draw_rectangle(500, 268, 440, 2, RGBA8(255,160,0,255));
    vita2d_draw_rectangle(500, 160, 2, 110, RGBA8(255,160,0,255));
    vita2d_draw_rectangle(938, 160, 2, 110, RGBA8(255,160,0,255));
    vita2d_pgf_draw_text(font, 515, 190, RGBA8(255,160,0,255), 0.9f, "NOTE: SAVING OR APPLYING");
    vita2d_pgf_draw_text(font, 515, 215, RGBA8(255,160,0,255), 0.9f, "A THEME MAY TAKE UP TO");
    vita2d_pgf_draw_text(font, 515, 240, RGBA8(255,200,100,255), 0.9f, "30 SECONDS. DONT PANIC :)");

    vita2d_pgf_draw_text(font, 40, 180,
        RGBA8(200,200,200,255), 1.0f, "Selected background:");
    if (bp->count > 0 && bp->selected < bp->count)
        vita2d_pgf_draw_text(font, 40, 210,
            RGBA8(255,255,255,255), 1.0f, bp->paths[bp->selected]);
    else
        vita2d_pgf_draw_text(font, 40, 210,
            RGBA8(150,150,150,255), 1.0f, "None selected");

    vita2d_pgf_draw_text(font, 40, 260,
        RGBA8(200,200,200,255), 1.0f, "Accent color:");
    vita2d_draw_rectangle(40, 280, 60, 40, cp->selected);
    char hex[16];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X",
        (cp->selected >> 24) & 0xFF,
        (cp->selected >> 16) & 0xFF,
        (cp->selected >> 8)  & 0xFF);
    vita2d_pgf_draw_text(font, 115, 305,
        RGBA8(220,220,220,255), 1.0f, hex);

    vita2d_draw_rectangle(40, 370, 450, 50, RGBA8(0,140,110,255));
    vita2d_pgf_draw_text(font, 130, 402,
        RGBA8(255,255,255,255), 1.0f, "Cross: SAVE THEME");
    vita2d_draw_rectangle(40, 432, 450, 50, RGBA8(60,40,120,255));
    vita2d_pgf_draw_text(font, 60, 462,
        RGBA8(255,255,255,255), 1.0f, "Square: SAVE + LAUNCH RETROARCH");

    if (status_msg[0] != '\0')
        vita2d_pgf_draw_text(font, 40, 470,
            RGBA8(0,200,160,255), 1.0f, status_msg);
    vita2d_pgf_draw_text(font, 40, 530, RGBA8(150,150,150,255), 1.0f, "Triangle: name   Cross: save   Square: save + launch   START: exit");
    if (editing_name) {
        vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(0,0,0,200));
        vita2d_draw_rectangle(100, 80, 760, 380, RGBA8(30,30,30,255));
        vita2d_draw_rectangle(100, 80, 760, 2, RGBA8(0,200,160,255));
        vita2d_draw_rectangle(100, 458, 760, 2, RGBA8(0,200,160,255));
        vita2d_draw_rectangle(100, 80, 2, 380, RGBA8(0,200,160,255));
        vita2d_draw_rectangle(858, 80, 2, 380, RGBA8(0,200,160,255));
        vita2d_pgf_draw_text(font, 130, 112, RGBA8(0,200,160,255), 1.0f, "Theme Name");
        vita2d_pgf_draw_text(font, 130, 150, RGBA8(255,255,255,255), 1.2f, theme_name);
        const char *keys = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
        for (int i = 0; i < 38; i++) {
            int col = i%10, row = i/10;
            float x = 115+col*73, y = 180+row*56;
            int sel = (i==name_key_sel);
            vita2d_draw_rectangle(x,y,65,46,sel?RGBA8(0,200,160,255):RGBA8(55,55,55,255));
            char k[2]={keys[i],0};
            vita2d_pgf_draw_text(font,x+22,y+30,sel?RGBA8(0,0,0,255):RGBA8(220,220,220,255),1.0f,k);
        }
        vita2d_pgf_draw_text(font,130,430,RGBA8(150,150,150,255),0.9f,"X:add  Tri:delete  Sq/O:confirm");
    }
}
static void draw_progress(vita2d_pgf *font, const char *msg, float pct) {
    vita2d_start_drawing();
    vita2d_clear_screen();
    vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(20,20,20,255));
    vita2d_pgf_draw_text(font, 40, 240, RGBA8(0,200,160,255), 1.0f, msg);
    vita2d_draw_rectangle(40, 280, 880, 20, RGBA8(50,50,50,255));
    vita2d_draw_rectangle(40, 280, (int)(880*pct), 20, RGBA8(0,200,160,255));
    char pct_str[16];
    snprintf(pct_str, sizeof(pct_str), "%d%%", (int)(pct*100));
    vita2d_pgf_draw_text(font, 460, 320, RGBA8(150,150,150,255), 1.0f, pct_str);
    vita2d_end_drawing();
    vita2d_swap_buffers();
}
int main() {
    vita2d_init();
    vita2d_set_clear_color(RGBA8(30, 30, 30, 255));
    vita2d_pgf *font = vita2d_load_default_pgf();
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    ColorPicker cp;
    colorpicker_init(&cp);

    BgPicker bp;
    bgpicker_init(&bp);
    draw_progress(font, "Scanning for images...", 0.3f);
    bgpicker_scan(&bp);
    ThemeBrowser tb;
    draw_progress(font, "Loading thumbnails...", 0.6f);
    themebrowser_init(&tb);
    draw_progress(font, "Loading themes...", 0.8f);
    themebrowser_scan(&tb);
    draw_progress(font, "Ready!", 1.0f);
    sceKernelDelayThread(400000);
    SceCtrlData pad;
    int show_welcome = 1;
    while (show_welcome) {
        sceCtrlReadBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_CROSS) show_welcome = 0;
        vita2d_start_drawing();
        vita2d_clear_screen();
        vita2d_draw_rectangle(100, 60, 760, 424, RGBA8(25,25,25,255));
        vita2d_draw_rectangle(100, 60, 760, 2, RGBA8(0,200,160,255));
        vita2d_draw_rectangle(100, 482, 760, 2, RGBA8(0,200,160,255));
        vita2d_draw_rectangle(100, 60, 2, 424, RGBA8(0,200,160,255));
        vita2d_draw_rectangle(858, 60, 2, 424, RGBA8(0,200,160,255));
        vita2d_pgf_draw_text(font, 380, 100, RGBA8(0,200,160,255), 1.2f, "MiniRetroXMB");
        vita2d_pgf_draw_text(font, 200, 145, RGBA8(200,200,200,255), 1.0f, "Welcome! Create custom RetroArch themes on your Vita.");
        vita2d_draw_rectangle(100, 165, 760, 1, RGBA8(0,200,160,80));
        vita2d_pgf_draw_text(font, 400, 200, RGBA8(0,200,160,255), 1.0f, "HOW TO USE");
        vita2d_pgf_draw_text(font, 160, 235, RGBA8(0,200,160,255), 1.0f, "L Trigger");
        vita2d_pgf_draw_text(font, 340, 235, RGBA8(180,180,180,255), 1.0f, "Pick your accent color");
        vita2d_pgf_draw_text(font, 160, 265, RGBA8(0,200,160,255), 1.0f, "R Trigger");
        vita2d_pgf_draw_text(font, 340, 265, RGBA8(180,180,180,255), 1.0f, "Pick your wallpaper");
        vita2d_pgf_draw_text(font, 160, 295, RGBA8(0,200,160,255), 1.0f, "Select");
        vita2d_pgf_draw_text(font, 340, 295, RGBA8(180,180,180,255), 1.0f, "Preview your theme");
        vita2d_pgf_draw_text(font, 160, 325, RGBA8(0,200,160,255), 1.0f, "Cross on Preview");
        vita2d_pgf_draw_text(font, 340, 325, RGBA8(180,180,180,255), 1.0f, "Go to Save screen");
        vita2d_pgf_draw_text(font, 160, 355, RGBA8(0,200,160,255), 1.0f, "Triangle");
        vita2d_pgf_draw_text(font, 340, 355, RGBA8(180,180,180,255), 1.0f, "Browse saved themes");
        vita2d_pgf_draw_text(font, 160, 385, RGBA8(0,200,160,255), 1.0f, "START");
        vita2d_pgf_draw_text(font, 340, 385, RGBA8(180,180,180,255), 1.0f, "Exit app");
        vita2d_draw_rectangle(100, 445, 760, 1, RGBA8(0,200,160,80));
        vita2d_pgf_draw_text(font, 370, 468, RGBA8(100,100,100,255), 1.0f, "Press Cross to continue");
        vita2d_end_drawing();
        vita2d_swap_buffers();
    }
    bgpicker_load_thumbs(&bp);

    Screen screen = SCREEN_COLOR;
    char status_msg[128] = "";
    char theme_name[32] = "mytheme";
    int theme_name_len = 7;
    int editing_name = 0;
    int start_count = 0;
    int name_key_sel = 0;

    SceCtrlData prev_pad;
    memset(&prev_pad, 0, sizeof(prev_pad));
    while (1) {
        sceCtrlReadBufferPositive(0, &pad, 1);
        unsigned int pressed = pad.buttons & ~prev_pad.buttons;
        if (pressed & SCE_CTRL_START) { start_count++; if (start_count >= 2) break; }

        if (cp.mode != CP_MODE_HEX && !editing_name) {
            if (pressed & SCE_CTRL_LTRIGGER) screen = SCREEN_COLOR;
            if (pressed & SCE_CTRL_RTRIGGER) screen = SCREEN_BG;
            if (pressed & SCE_CTRL_SELECT)   screen = SCREEN_PREVIEW;
        }
            if (pressed & SCE_CTRL_TRIANGLE && screen == SCREEN_COLOR) { themebrowser_scan(&tb); screen = SCREEN_THEMES; }
            if (pressed & SCE_CTRL_SELECT && screen == SCREEN_BG) { themebrowser_scan(&tb); screen = SCREEN_THEMES; }
            if (pressed & SCE_CTRL_CIRCLE && screen == SCREEN_THEMES) screen = SCREEN_COLOR;

        if (screen == SCREEN_COLOR) {
            float ly = (pad.ly - 128) / 128.0f;
            colorpicker_update(&cp, pressed, ly);
        } else if (screen == SCREEN_BG) {
            bgpicker_update(&bp, pressed);
        } else if (screen == SCREEN_PREVIEW) {
            if (pressed & SCE_CTRL_CROSS) screen = SCREEN_EXPORT;
            if (pressed & SCE_CTRL_TRIANGLE) { themebrowser_scan(&tb); screen = SCREEN_THEMES; }
            if (pressed & SCE_CTRL_CIRCLE) screen = SCREEN_BG;
        } else if (screen == SCREEN_EXPORT) {
            if (editing_name) {
                const char *keys = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
                int key_count = 38;
                if (pressed & SCE_CTRL_RIGHT) { name_key_sel++; if (name_key_sel >= key_count) name_key_sel = 0; }
                if (pressed & SCE_CTRL_LEFT)  { name_key_sel--; if (name_key_sel < 0) name_key_sel = key_count-1; }
                if (pressed & SCE_CTRL_DOWN)  { name_key_sel += 10; if (name_key_sel >= key_count) name_key_sel = key_count-1; }
                if (pressed & SCE_CTRL_UP)    { name_key_sel -= 10; if (name_key_sel < 0) name_key_sel = 0; }
                if (pressed & SCE_CTRL_CROSS && theme_name_len < 31) {
                    theme_name[theme_name_len++] = keys[name_key_sel];
                    theme_name[theme_name_len] = 0;
                }
                if (pressed & SCE_CTRL_TRIANGLE && theme_name_len > 0) {
                    theme_name[--theme_name_len] = 0;
                }
                if (pressed & SCE_CTRL_SQUARE) editing_name = 0;
                if (pressed & SCE_CTRL_CIRCLE) editing_name = 0;
            } else {
                if (pressed & SCE_CTRL_TRIANGLE) { editing_name = 1; name_key_sel = 0; }
                if (pressed & SCE_CTRL_CROSS) {
                    const char *bg = (bp.count > 0) ? bp.paths[bp.selected] : "";
                    ExportResult r = export_theme(theme_name, bg, cp.selected);
                    if (r == EXPORT_OK)
                        snprintf(status_msg, sizeof(status_msg), "Saved as: %s", theme_name);
                    else
                        snprintf(status_msg, sizeof(status_msg), "Error saving (code %d)", r);
                }
                if (pressed & SCE_CTRL_SQUARE) {
                    const char *bg = (bp.count > 0) ? bp.paths[bp.selected] : "";
                    ExportResult r = export_theme(theme_name, bg, cp.selected);
                    if (r == EXPORT_OK) {
                        draw_progress(font, "Saving theme...", 0.3f);
                        sceKernelDelayThread(300000);
                        draw_progress(font, "Writing RetroArch config...", 0.6f);
                        sceKernelDelayThread(300000);
                        draw_progress(font, "Launching RetroArch...", 0.9f);
                        sceKernelDelayThread(300000);
                        draw_progress(font, "Done!", 1.0f);
                        sceKernelDelayThread(200000);
                        launch_retroarch();
                    }
                }
                if (pressed & SCE_CTRL_CIRCLE) { themebrowser_scan(&tb); screen = SCREEN_THEMES; }
            }
        } else if (screen == SCREEN_THEMES) {
            int result = themebrowser_update(&tb, pressed);
            if (result == 1) {
                char cfg_path[256];
                snprintf(cfg_path, sizeof(cfg_path), "ux0:app/RETROVITA/assets/xmb/%s", tb.names[tb.selected]);
                snprintf(status_msg, sizeof(status_msg), "Applied: %s", tb.names[tb.selected]);
                screen = SCREEN_EXPORT;
            } else if (result == 2) {
                themebrowser_delete_selected(&tb);
            } else if (pressed & SCE_CTRL_CIRCLE) {
                screen = SCREEN_EXPORT;
            }
        }

        vita2d_start_drawing();
        vita2d_clear_screen();
        vita2d_draw_rectangle(0, 0, 960, 40, RGBA8(20,20,20,255));
        vita2d_pgf_draw_text(font, 16, 28, screen==SCREEN_COLOR   ? RGBA8(0,200,160,255) : RGBA8(100,100,100,255), 1.0f, "L  COLOR");
        vita2d_pgf_draw_text(font, 170, 28, screen==SCREEN_BG      ? RGBA8(0,200,160,255) : RGBA8(100,100,100,255), 1.0f, "R  BACKGROUND");
        vita2d_pgf_draw_text(font, 400, 28, screen==SCREEN_PREVIEW  ? RGBA8(0,200,160,255) : RGBA8(100,100,100,255), 1.0f, "SELECT  PREVIEW");
        vita2d_pgf_draw_text(font, 620, 28, screen==SCREEN_EXPORT   ? RGBA8(0,200,160,255) : RGBA8(100,100,100,255), 1.0f, "X  SAVE");
        vita2d_pgf_draw_text(font, 760, 28, screen==SCREEN_THEMES   ? RGBA8(0,200,160,255) : RGBA8(100,100,100,255), 1.0f, "TRIANGLE  THEMES");

        if (screen == SCREEN_COLOR) {
            colorpicker_draw(&cp, font);
            if (bp.count > 0 && bp.thumbs[bp.selected]) {
                vita2d_pgf_draw_text(font, 700, 290, RGBA8(150,150,150,255), 0.9f, "Selected BG:");
                float sx = 180.0f / vita2d_texture_get_width(bp.thumbs[bp.selected]);
                float sy = 100.0f / vita2d_texture_get_height(bp.thumbs[bp.selected]);
                vita2d_draw_texture_scale(bp.thumbs[bp.selected], 700, 300, sx, sy);
            } else {
                vita2d_pgf_draw_text(font, 700, 310, RGBA8(100,100,100,255), 0.9f, "No BG selected");
            }
            }
        else if (screen == SCREEN_PREVIEW)
            preview_draw(font, &cp, &bp);
        else if (screen == SCREEN_BG)
            bgpicker_draw(&bp, font);
        else if (screen == SCREEN_EXPORT)
            draw_export_screen(font, editing_name, name_key_sel, &cp, &bp, theme_name, status_msg);
        else if (screen == SCREEN_THEMES)
            themebrowser_draw(&tb, font);
        vita2d_end_drawing();
        vita2d_swap_buffers();
        prev_pad = pad;
    }

    bgpicker_free(&bp);
    vita2d_free_pgf(font);
    vita2d_fini();
    sceKernelExitProcess(0);
    return 0;
}
