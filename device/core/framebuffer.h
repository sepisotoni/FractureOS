#ifndef FRACTURE_FRAMEBUFFER_H
#define FRACTURE_FRAMEBUFFER_H

#include <stdint.h>

#define SCREEN_W 320
#define SCREEN_H 240
#define SCREEN_BYTES (SCREEN_W * SCREEN_H * 2)

#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3))

/* Cyberpunk & Obsidian Dark Palette */
#define COLOR_OBSIDIAN       RGB565(10, 14, 20)
#define COLOR_PANEL_BG       RGB565(18, 24, 34)
#define COLOR_PANEL_HOVER    RGB565(26, 35, 48)
#define COLOR_BORDER_MUTED   RGB565(40, 48, 62)
#define COLOR_BORDER_ACTIVE  RGB565(56, 189, 248)

#define COLOR_TEXT_WHITE     RGB565(248, 250, 252)
#define COLOR_TEXT_MUTED     RGB565(148, 163, 184)
#define COLOR_TEXT_DARK      RGB565(100, 116, 139)

/* Accent Colors */
#define COLOR_CYAN           RGB565(56, 189, 248)   /* #38BDF8 */
#define COLOR_EMERALD        RGB565(16, 185, 129)   /* #10B981 */
#define COLOR_GREEN_SPOTIFY  RGB565(34, 197, 94)    /* #22C55E */
#define COLOR_AMBER          RGB565(245, 158, 11)   /* #F59E0B */
#define COLOR_ROSE           RGB565(244, 63, 94)    /* #F43F5E */
#define COLOR_PURPLE         RGB565(168, 85, 247)   /* #A855F7 */
#define COLOR_BLUE           RGB565(59, 130, 246)   /* #3B82F6 */

/* Icons */
enum IconID {
    ICON_CLOCK = 0,
    ICON_TELEMETRY,
    ICON_SPOTIFY,
    ICON_FOCUS,
    ICON_SERVER,
    ICON_DISPLAY,
    ICON_WEATHER_SUN,
    ICON_WEATHER_CLOUD,
    ICON_WEATHER_RAIN,
    ICON_BATTERY,
    ICON_WIFI,
    ICON_CHECK,
    ICON_BELL
};

/* Core Lifecycle */
int  fb_init(const char *fb_path);
void fb_close(void);
void fb_present(void);
void fb_present_region(int x, int y, int w, int h);
void fb_clear(uint16_t color);

/* Primitives */
void fb_put_pixel(int x, int y, uint16_t color);
uint16_t fb_get_pixel(int x, int y);
void fb_draw_rect_fill(int x, int y, int w, int h, uint16_t color);
void fb_draw_rect_outline(int x, int y, int w, int h, uint16_t color);
void fb_draw_rounded_rect_fill(int x, int y, int w, int h, int r, uint16_t color);
void fb_draw_rounded_rect_outline(int x, int y, int w, int h, int r, int thickness, uint16_t color);
void fb_draw_line(int x0, int y0, int x1, int y1, uint16_t color);
void fb_draw_circle_fill(int cx, int cy, int r, uint16_t color);
void fb_draw_circle_outline(int cx, int cy, int r, uint16_t color);
void fb_draw_pill_badge(int x, int y, int w, int h, uint16_t bg, uint16_t border);

/* Typography */
void fb_draw_text_small(int x, int y, const char *text, uint16_t color);
void fb_draw_text_normal(int x, int y, const char *text, uint16_t color);
void fb_draw_text_digits(int x, int y, const char *text, uint16_t color);
int  fb_text_width_small(const char *text);
int  fb_text_width_normal(const char *text);
int  fb_text_width_digits(const char *text);

/* Icon Rendering */
void fb_draw_icon(int icon_id, int cx, int cy, uint16_t color);

#endif /* FRACTURE_FRAMEBUFFER_H */
