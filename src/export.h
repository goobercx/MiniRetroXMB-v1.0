#ifndef EXPORT_H
#define EXPORT_H

typedef enum {
    EXPORT_OK,
    EXPORT_ERR_MKDIR,
    EXPORT_ERR_COPY,
    EXPORT_ERR_CFG
} ExportResult;

ExportResult export_theme(const char *name, const char *bg_path, unsigned int accent_color);
int copy_file(const char *src, const char *dst);

#endif
void launch_retroarch(void);
