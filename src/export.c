#include "export.h"
#include <stdio.h>
#include <string.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/appmgr.h>
#include <psp2/kernel/threadmgr.h>

int copy_file(const char *src, const char *dst) {
    SceUID in = sceIoOpen(src, SCE_O_RDONLY, 0);
    if (in < 0) return -1;
    SceUID out = sceIoOpen(dst, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (out < 0) { sceIoClose(in); return -1; }
    char buf[4096];
    int bytes;
    while ((bytes = sceIoRead(in, buf, sizeof(buf))) > 0)
        sceIoWrite(out, buf, bytes);
    sceIoClose(in);
    sceIoClose(out);
    return 0;
}

static int update_cfg_key(const char *cfg_path, const char *key, const char *value) {
    SceUID f = sceIoOpen(cfg_path, SCE_O_RDONLY, 0);
    if (f < 0) return -1;
    static char content[131072];
    int len = sceIoRead(f, content, sizeof(content) - 1);
    sceIoClose(f);
    if (len < 0) return -1;
    content[len] = '\0';

    char newline[512];
    snprintf(newline, sizeof(newline), "%s = \"%s\"", key, value);

    static char result[131072];
    result[0] = '\0';
    int found = 0;

    char *line = content;
    while (*line) {
        char *end = strchr(line, '\n');
        char saved = 0;
        if (end) { saved = *end; *end = '\0'; }

        if (strncmp(line, key, strlen(key)) == 0 && line[strlen(key)] == ' ') {
            strncat(result, newline, sizeof(result) - strlen(result) - 1);
            found = 1;
        } else {
            strncat(result, line, sizeof(result) - strlen(result) - 1);
        }

        if (end) {
            *end = saved;
            strncat(result, "\n", sizeof(result) - strlen(result) - 1);
            line = end + 1;
        } else break;
    }

    if (!found) {
        strncat(result, newline, sizeof(result) - strlen(result) - 1);
        strncat(result, "\n",    sizeof(result) - strlen(result) - 1);
    }

    SceUID out = sceIoOpen(cfg_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (out < 0) return -1;
    sceIoWrite(out, result, strlen(result));
    sceIoClose(out);
    return 0;
}

static const char* pick_theme(unsigned int r, unsigned int g, unsigned int b) {
    if (r > 180 && g < 80  && b < 80)  return "0";
    if (r > 150 && g < 80  && b > 150) return "1";
    if (r < 80  && g < 80  && b > 180) return "2";
    if (r > 180 && g > 100 && b < 80)  return "3";
    if (r < 80  && g > 180 && b > 80)  return "4";
    if (r < 80  && g > 150 && b < 80)  return "5";
    if (r < 80  && g > 150 && b > 150) return "6";
    if (r > 200 && g > 150 && b < 80)  return "14";
    if (r < 60  && g < 60  && b < 60)  return "8";
    if (r > 200 && g > 200 && b > 200) return "9";
    return "3";
}
ExportResult export_theme(const char *name, const char *bg_path, unsigned int accent_color) {
    char theme_dir[256];
    snprintf(theme_dir, sizeof(theme_dir),
        "ux0:app/RETROVITA/assets/xmb/%s", name);

    sceIoMkdir("ux0:app/RETROVITA/assets", 0777);
    sceIoMkdir("ux0:app/RETROVITA/assets/xmb", 0777);
    if (sceIoMkdir(theme_dir, 0777) < 0 &&
        sceIoDopen(theme_dir) < 0)
        return EXPORT_ERR_MKDIR;

    if (bg_path && bg_path[0] != '\0') {
        char dst[256];
        int len = strlen(bg_path);
        const char *ext = (len > 4) ? bg_path + len - 4 : ".png";
        snprintf(dst, sizeof(dst), "%s/bg%s", theme_dir, ext);
        if (copy_file(bg_path, dst) < 0)
            return EXPORT_ERR_COPY;

        const char *cfg = "ux0:data/retroarch/retroarch.cfg";
        update_cfg_key(cfg, "menu_wallpaper", dst);
    }

    char hex[16];
    snprintf(hex, sizeof(hex), "%06x",
        ((accent_color >> 24) & 0xFF) << 16 |
        ((accent_color >> 16) & 0xFF) << 8  |
        ((accent_color >> 8)  & 0xFF));

    const char *cfg = "ux0:data/retroarch/retroarch.cfg";
    
    update_cfg_key(cfg, "menu_xmb_color_theme_custom_hex", hex);
    unsigned int r = (accent_color >> 0) & 0xFF;
    unsigned int g = (accent_color >> 8) & 0xFF;
    unsigned int b = (accent_color >> 16) & 0xFF;
    const char *theme_idx = pick_theme(r, g, b);
    update_cfg_key(cfg, "menu_xmb_color_theme", theme_idx);
    update_cfg_key(cfg, "xmb_menu_color_theme", theme_idx);
    update_cfg_key(cfg, "menu_xmb_icon_theme", "0");
    update_cfg_key(cfg, "menu_driver", "xmb");
    update_cfg_key(cfg, "savestate_auto_save", "false");
    update_cfg_key(cfg, "menu_wallpaper_opacity", "1.000000");
    char r_str[8], g_str[8], b_str[8];
    snprintf(r_str, sizeof(r_str), "%d", (accent_color >> 0) & 0xFF);
    snprintf(g_str, sizeof(g_str), "%d", (accent_color >> 8) & 0xFF);
    snprintf(b_str, sizeof(b_str), "%d", (accent_color >> 16) & 0xFF);
    update_cfg_key(cfg, "menu_font_color_red",   r_str);
    update_cfg_key(cfg, "menu_font_color_green", g_str);
    update_cfg_key(cfg, "menu_font_color_blue",  b_str);
    update_cfg_key(cfg, "menu_xmb_alpha_factor", "75");

    return EXPORT_OK;
}

void launch_retroarch(void) {
    sceKernelDelayThread(500000);
    const char *cfg2 = "ux0:data/retroarch/retroarch.cfg";
    update_cfg_key(cfg2, "menu_driver", "xmb");
    update_cfg_key(cfg2, "menu_wallpaper_opacity", "1.000000");
    sceKernelDelayThread(200000);
    sceAppMgrLaunchAppByUri(0xFFFFF, "psgm:play?titleid=RETROVITA");
}
