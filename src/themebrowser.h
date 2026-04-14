#ifndef THEMEBROWSER_H
#define THEMEBROWSER_H

#include <vita2d.h>
#include <psp2/io/dirent.h>
#include <psp2/ctrl.h>

#define MAX_THEMES 32

typedef struct {
    int count;
    int selected;
    int scroll;
    char names[MAX_THEMES][32];
} ThemeBrowser;

void themebrowser_init(ThemeBrowser *tb);
void themebrowser_scan(ThemeBrowser *tb);
void themebrowser_draw(ThemeBrowser *tb, vita2d_pgf *font);
int  themebrowser_update(ThemeBrowser *tb, unsigned int pressed);
void themebrowser_delete_selected(ThemeBrowser *tb);

#endif
