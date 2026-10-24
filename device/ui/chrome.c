#include <stdio.h>
#include <string.h>
#include <time.h>

#include "chrome.h"
#include "../core/framebuffer.h"
#include "../core/network.h"

void chrome_render_header(const char *view_title, uint16_t accent_color) {
    /* Header background */
    fb_draw_rect_fill(0, 0, SCREEN_W, 22, COLOR_PANEL_BG);
    fb_draw_rect_fill(0, 22, SCREEN_W, 1, COLOR_BORDER_MUTED);

    /* Brand Dot and Logo */
    fb_draw_circle_fill(10, 11, 3, COLOR_CYAN);
    fb_draw_text_normal(18, 4, "FRACTURE", COLOR_TEXT_WHITE);

    /* View Title in Center */
    if (view_title) {
        int tw = fb_text_width_small(view_title);
        fb_draw_text_small((SCREEN_W - tw) / 2, 6, view_title, accent_color);
    }

    /* Right side: PC connection badge, battery, clock */
    int conn = network_is_companion_connected();
    fb_draw_circle_fill(238, 11, 3, conn ? COLOR_EMERALD : COLOR_TEXT_DARK);

    /* Battery icon */
    fb_draw_icon(ICON_BATTERY, 256, 11, COLOR_TEXT_MUTED);

    /* Clock */
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char time_buf[16];
    if (tm) {
        snprintf(time_buf, sizeof(time_buf), "%02d:%02d", tm->tm_hour, tm->tm_min);
    } else {
        strcpy(time_buf, "21:00");
    }
    fb_draw_text_normal(275, 4, time_buf, COLOR_TEXT_WHITE);
}

void chrome_render_footer(const char *left_hint, const char *center_hint, const char *right_hint) {
    /* Footer background */
    fb_draw_rect_fill(0, 216, SCREEN_W, 24, COLOR_PANEL_BG);
    fb_draw_rect_fill(0, 215, SCREEN_W, 1, COLOR_BORDER_MUTED);

    /* Left pill (x = 8, w = 78) */
    if (left_hint) {
        fb_draw_pill_badge(8, 218, 78, 19, COLOR_PANEL_HOVER, COLOR_BORDER_MUTED);
        int tw = fb_text_width_small(left_hint);
        fb_draw_text_small(8 + (78 - tw)/2, 223, left_hint, COLOR_CYAN);
    }

    /* Center pill (x = 114, w = 92) */
    if (center_hint) {
        fb_draw_pill_badge(114, 218, 92, 19, COLOR_PANEL_HOVER, COLOR_BORDER_MUTED);
        int tw = fb_text_width_small(center_hint);
        fb_draw_text_small(114 + (92 - tw)/2, 223, center_hint, COLOR_GREEN_SPOTIFY);
    }

    /* Right pill (x = 232, w = 80) */
    if (right_hint) {
        fb_draw_pill_badge(232, 218, 80, 19, COLOR_PANEL_HOVER, COLOR_BORDER_MUTED);
        int tw = fb_text_width_small(right_hint);
        fb_draw_text_small(232 + (80 - tw)/2, 223, right_hint, COLOR_AMBER);
    }
}

void chrome_render_toast(const char *message, uint16_t color) {
    if (!message) return;
    int tw = fb_text_width_normal(message) + 24;
    if (tw > 280) tw = 280;
    int th = 26;
    int tx = (SCREEN_W - tw) / 2;
    int ty = 100;

    fb_draw_rounded_rect_fill(tx, ty, tw, th, 8, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(tx, ty, tw, th, 8, 2, color);

    int text_w = fb_text_width_normal(message);
    fb_draw_text_normal((SCREEN_W - text_w) / 2, ty + 6, message, COLOR_TEXT_WHITE);
}
