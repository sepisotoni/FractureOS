#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>

#include "framebuffer.h"
#include "../ui/font_data.h"

static int fbfd = -1;
static uint16_t *fbp = NULL;
static uint16_t backbuffer[SCREEN_H][SCREEN_W];

int fb_init(const char *fb_path) {
    if (!fb_path) fb_path = "/dev/fb";
    fbfd = open(fb_path, O_RDWR);
    if (fbfd < 0) {
        fbfd = open("/dev/fb0", O_RDWR);
    }
    if (fbfd < 0) {
        perror("open fb");
        return -1;
    }

    fbp = (uint16_t *)mmap(0, SCREEN_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (fbp == MAP_FAILED) {
        perror("mmap fb");
        close(fbfd);
        fbfd = -1;
        return -1;
    }

    fb_clear(COLOR_OBSIDIAN);
    fb_present();
    return 0;
}

void fb_close(void) {
    if (fbp && fbp != MAP_FAILED) {
        munmap(fbp, SCREEN_BYTES);
        fbp = NULL;
    }
    if (fbfd >= 0) {
        close(fbfd);
        fbfd = -1;
    }
}

void fb_present(void) {
    if (fbp) {
        memcpy(fbp, backbuffer, SCREEN_BYTES);
    }
}

void fb_present_region(int x, int y, int w, int h) {
    if (!fbp) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_W) w = SCREEN_W - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;
    if (w <= 0 || h <= 0) return;

    for (int j = y; j < y + h; j++) {
        memcpy(&fbp[j * SCREEN_W + x], &backbuffer[j][x], w * 2);
    }
}

void fb_clear(uint16_t color) {
    for (int j = 0; j < SCREEN_H; j++) {
        for (int i = 0; i < SCREEN_W; i++) {
            backbuffer[j][i] = color;
        }
    }
}

void fb_put_pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        backbuffer[y][x] = color;
    }
}

uint16_t fb_get_pixel(int x, int y) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        return backbuffer[y][x];
    }
    return 0;
}

void fb_draw_rect_fill(int x, int y, int w, int h, uint16_t color) {
    int x2 = x + w, y2 = y + h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x2 > SCREEN_W) x2 = SCREEN_W;
    if (y2 > SCREEN_H) y2 = SCREEN_H;

    for (int j = y; j < y2; j++) {
        for (int i = x; i < x2; i++) {
            backbuffer[j][i] = color;
        }
    }
}

void fb_draw_rect_outline(int x, int y, int w, int h, uint16_t color) {
    for (int i = x; i < x + w; i++) {
        fb_put_pixel(i, y, color);
        fb_put_pixel(i, y + h - 1, color);
    }
    for (int j = y; j < y + h; j++) {
        fb_put_pixel(x, j, color);
        fb_put_pixel(x + w - 1, j, color);
    }
}

void fb_draw_rounded_rect_fill(int x, int y, int w, int h, int r, uint16_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int in_corner = 0;
            if (i < r && j < r) {
                if ((r - i)*(r - i) + (r - j)*(r - j) > r*r) in_corner = 1;
            } else if (i >= w - r && j < r) {
                int dx = i - (w - r - 1);
                if (dx*dx + (r - j)*(r - j) > r*r) in_corner = 1;
            } else if (i < r && j >= h - r) {
                int dy = j - (h - r - 1);
                if ((r - i)*(r - i) + dy*dy > r*r) in_corner = 1;
            } else if (i >= w - r && j >= h - r) {
                int dx = i - (w - r - 1);
                int dy = j - (h - r - 1);
                if (dx*dx + dy*dy > r*r) in_corner = 1;
            }
            if (!in_corner) {
                fb_put_pixel(x + i, y + j, color);
            }
        }
    }
}

void fb_draw_rounded_rect_outline(int x, int y, int w, int h, int r, int thickness, uint16_t color) {
    for (int t = 0; t < thickness; t++) {
        int cx = x + t, cy = y + t;
        int cw = w - 2*t, ch = h - 2*t;
        for (int i = r; i < cw - r; i++) {
            fb_put_pixel(cx + i, cy, color);
            fb_put_pixel(cx + i, cy + ch - 1, color);
        }
        for (int j = r; j < ch - r; j++) {
            fb_put_pixel(cx, cy + j, color);
            fb_put_pixel(cx + cw - 1, cy + j, color);
        }
        for (int angle = 0; angle <= 90; angle += 4) {
            double rad = angle * 3.14159 / 180.0;
            int px = (int)(cos(rad) * (r - t) + 0.5);
            int py = (int)(sin(rad) * (r - t) + 0.5);
            fb_put_pixel(cx + cw - r + px, cy + r - py, color);
            fb_put_pixel(cx + r - px, cy + r - py, color);
            fb_put_pixel(cx + r - px, cy + ch - r + py, color);
            fb_put_pixel(cx + cw - r + px, cy + ch - r + py, color);
        }
    }
}

void fb_draw_pill_badge(int x, int y, int w, int h, uint16_t bg, uint16_t border) {
    int r = h / 2;
    fb_draw_rounded_rect_fill(x, y, w, h, r, bg);
    fb_draw_rounded_rect_outline(x, y, w, h, r, 1, border);
}

void fb_draw_line(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        fb_put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void fb_draw_circle_fill(int cx, int cy, int r, uint16_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                fb_put_pixel(cx + x, cy + y, color);
            }
        }
    }
}

void fb_draw_circle_outline(int cx, int cy, int r, uint16_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            int d = x*x + y*y;
            if (d >= (r-1)*(r-1) && d <= r*r) {
                fb_put_pixel(cx + x, cy + y, color);
            }
        }
    }
}

/* Typography Rendering */
void fb_draw_text_small(int x, int y, const char *text, uint16_t color) {
    if (!text) return;
    int cur_x = x;
    int bytes_per_row = (FONT_SMALL_W + 7) / 8;

    while (*text) {
        char c = *text;
        if (c >= 32 && c <= 126) {
            int idx = c - 32;
            for (int row = 0; row < FONT_SMALL_H; row++) {
                for (int b = 0; b < bytes_per_row; b++) {
                    unsigned char val = font_small_glyphs[idx][row * bytes_per_row + b];
                    for (int bit = 0; bit < 8; bit++) {
                        int col = b * 8 + bit;
                        if (col < FONT_SMALL_W && (val & (1 << (7 - bit)))) {
                            fb_put_pixel(cur_x + col, y + row, color);
                        }
                    }
                }
            }
        }
        cur_x += (FONT_SMALL_W - 2);
        text++;
    }
}

int fb_text_width_small(const char *text) {
    if (!text) return 0;
    return strlen(text) * (FONT_SMALL_W - 2);
}

void fb_draw_text_normal(int x, int y, const char *text, uint16_t color) {
    if (!text) return;
    int cur_x = x;
    int bytes_per_row = (FONT_NORMAL_W + 7) / 8;

    while (*text) {
        char c = *text;
        if (c >= 32 && c <= 126) {
            int idx = c - 32;
            for (int row = 0; row < FONT_NORMAL_H; row++) {
                for (int b = 0; b < bytes_per_row; b++) {
                    unsigned char val = font_normal_glyphs[idx][row * bytes_per_row + b];
                    for (int bit = 0; bit < 8; bit++) {
                        int col = b * 8 + bit;
                        if (col < FONT_NORMAL_W && (val & (1 << (7 - bit)))) {
                            fb_put_pixel(cur_x + col, y + row, color);
                        }
                    }
                }
            }
        }
        cur_x += (FONT_NORMAL_W - 3);
        text++;
    }
}

int fb_text_width_normal(const char *text) {
    if (!text) return 0;
    return strlen(text) * (FONT_NORMAL_W - 3);
}

void fb_draw_text_digits(int x, int y, const char *text, uint16_t color) {
    if (!text) return;
    int cur_x = x;
    int bytes_per_row = (FONT_DIGITS_W + 7) / 8;

    while (*text) {
        char c = *text;
        if (c >= 32 && c <= 126) {
            int idx = c - 32;
            for (int row = 0; row < FONT_DIGITS_H; row++) {
                for (int b = 0; b < bytes_per_row; b++) {
                    unsigned char val = font_digits_glyphs[idx][row * bytes_per_row + b];
                    for (int bit = 0; bit < 8; bit++) {
                        int col = b * 8 + bit;
                        if (col < FONT_DIGITS_W && (val & (1 << (7 - bit)))) {
                            fb_put_pixel(cur_x + col, y + row, color);
                        }
                    }
                }
            }
        }
        cur_x += (FONT_DIGITS_W - 1);
        text++;
    }
}

int fb_text_width_digits(const char *text) {
    if (!text) return 0;
    return strlen(text) * (FONT_DIGITS_W - 1);
}

/* Vector Glyph Rendering */
void fb_draw_icon(int icon_id, int cx, int cy, uint16_t color) {
    switch (icon_id) {
        case ICON_CLOCK:
            fb_draw_circle_outline(cx, cy, 9, color);
            for (int i = 0; i <= 5; i++) fb_put_pixel(cx, cy - i, color);
            for (int i = 0; i <= 6; i++) fb_put_pixel(cx + i, cy, color);
            fb_draw_circle_fill(cx, cy, 1, color);
            break;
        case ICON_TELEMETRY:
            fb_draw_circle_outline(cx, cy, 9, color);
            for (int i = 0; i <= 6; i++) fb_put_pixel(cx - 2 + i, cy + 2 - i, color);
            fb_draw_circle_fill(cx, cy + 3, 2, color);
            break;
        case ICON_SPOTIFY:
            fb_draw_circle_fill(cx, cy, 9, color);
            for (int a = -40; a <= 40; a += 6) {
                double rad = (a - 90) * 3.14159 / 180.0;
                for (int arc = 3; arc <= 7; arc += 2) {
                    int px = cx + (int)(cos(rad) * arc);
                    int py = cy + 2 + (int)(sin(rad) * arc);
                    fb_put_pixel(px, py, COLOR_PANEL_BG);
                }
            }
            break;
        case ICON_FOCUS:
            fb_draw_circle_fill(cx, cy + 1, 8, color);
            fb_draw_rect_fill(cx - 2, cy - 10, 4, 3, color);
            for (int i = 0; i < 4; i++) fb_put_pixel(cx, cy - i, COLOR_PANEL_BG);
            for (int i = 0; i < 4; i++) fb_put_pixel(cx + i, cy, COLOR_PANEL_BG);
            break;
        case ICON_SERVER:
            fb_draw_rounded_rect_fill(cx - 9, cy - 8, 18, 4, 1, color);
            fb_draw_rounded_rect_fill(cx - 9, cy - 2, 18, 4, 1, color);
            fb_draw_rounded_rect_fill(cx - 9, cy + 4, 18, 4, 1, color);
            fb_put_pixel(cx - 6, cy - 6, COLOR_PANEL_BG);
            fb_put_pixel(cx - 6, cy, COLOR_PANEL_BG);
            fb_put_pixel(cx - 6, cy + 6, COLOR_PANEL_BG);
            break;
        case ICON_DISPLAY:
            fb_draw_circle_fill(cx, cy, 4, color);
            for (int d = 6; d <= 9; d++) {
                fb_put_pixel(cx, cy - d, color);
                fb_put_pixel(cx, cy + d, color);
                fb_put_pixel(cx - d, cy, color);
                fb_put_pixel(cx + d, cy, color);
            }
            break;
        case ICON_WEATHER_SUN:
            fb_draw_circle_fill(cx, cy, 5, color);
            for (int d = 7; d <= 9; d++) {
                fb_put_pixel(cx, cy - d, color);
                fb_put_pixel(cx, cy + d, color);
                fb_put_pixel(cx - d, cy, color);
                fb_put_pixel(cx + d, cy, color);
            }
            break;
        case ICON_BATTERY:
            fb_draw_rect_outline(cx - 8, cy - 4, 14, 8, color);
            fb_draw_rect_fill(cx + 6, cy - 2, 2, 4, color);
            fb_draw_rect_fill(cx - 6, cy - 2, 8, 4, color);
            break;
        case ICON_WIFI:
            for (int arc = 2; arc <= 8; arc += 3) {
                for (int a = -45; a <= 45; a += 5) {
                    double rad = (a - 90) * 3.14159 / 180.0;
                    fb_put_pixel(cx + (int)(cos(rad) * arc), cy + 4 + (int)(sin(rad) * arc), color);
                }
            }
            fb_put_pixel(cx, cy + 5, color);
            break;
        default:
            fb_draw_circle_fill(cx, cy, 4, color);
            break;
    }
}
