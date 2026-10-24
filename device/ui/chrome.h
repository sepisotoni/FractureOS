#ifndef FRACTURE_CHROME_H
#define FRACTURE_CHROME_H

#include "../core/protocol.h"

void chrome_render_header(const char *view_title, uint16_t accent_color);
void chrome_render_footer(const char *left_hint, const char *center_hint, const char *right_hint);
void chrome_render_toast(const char *message, uint16_t color);

#endif /* FRACTURE_CHROME_H */
