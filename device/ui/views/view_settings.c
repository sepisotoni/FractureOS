#include <stdio.h>
#include <string.h>

#include "view_settings.h"
#include "../chrome.h"
#include "../view_manager.h"
#include "../../core/framebuffer.h"
#include "../../core/backlight.h"
#include "../../core/audio.h"

static int brightness_level = 80;
static int night_mode = 0;

void view_settings_init(void) {
    brightness_level = backlight_get_brightness();
    if (brightness_level <= 0) brightness_level = 80;
    night_mode = 0;
}

void view_settings_render(const FractureTelemetryPacket *packet) {
    (void)packet;

    chrome_render_header("SETTINGS", COLOR_ROSE);

    /* Setting 1: Brightness Level (y = 28..72) */
    int c1_y = 28;
    fb_draw_rounded_rect_fill(14, c1_y, SCREEN_W - 28, 44, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(14, c1_y, SCREEN_W - 28, 44, 6, 1, COLOR_BORDER_MUTED);

    fb_draw_icon(ICON_DISPLAY, 30, c1_y + 22, COLOR_ROSE);
    fb_draw_text_normal(48, c1_y + 8, "Backlight Brightness", COLOR_TEXT_WHITE);

    char bright_str[16];
    snprintf(bright_str, sizeof(bright_str), "%d%%", brightness_level);
    fb_draw_text_small(240, c1_y + 10, bright_str, COLOR_ROSE);

    /* Brightness progress bar */
    int bar_x = 48, bar_y = c1_y + 26, bar_w = 230;
    fb_draw_rect_fill(bar_x, bar_y, bar_w, 4, COLOR_PANEL_HOVER);
    int fill_w = (bar_w * brightness_level) / 100;
    fb_draw_rect_fill(bar_x, bar_y, fill_w, 4, COLOR_ROSE);
    fb_draw_circle_fill(bar_x + fill_w, bar_y + 2, 4, COLOR_TEXT_WHITE);

    /* Setting 2: Night Mode (y = 78..122) */
    int c2_y = 78;
    fb_draw_rounded_rect_fill(14, c2_y, SCREEN_W - 28, 40, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(14, c2_y, SCREEN_W - 28, 40, 6, 1, COLOR_BORDER_MUTED);

    fb_draw_text_normal(24, c2_y + 12, "Night / Stealth Theme", COLOR_TEXT_WHITE);
    fb_draw_pill_badge(230, c2_y + 10, 60, 20, night_mode ? COLOR_PANEL_HOVER : COLOR_PANEL_BG, night_mode ? COLOR_AMBER : COLOR_BORDER_MUTED);
    fb_draw_text_small(242, c2_y + 14, night_mode ? "ACTIVE" : "OFF", night_mode ? COLOR_AMBER : COLOR_TEXT_MUTED);

    /* Setting 3: Audio Clicks (y = 124..168) */
    int c3_y = 124;
    fb_draw_rounded_rect_fill(14, c3_y, SCREEN_W - 28, 40, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(14, c3_y, SCREEN_W - 28, 40, 6, 1, COLOR_BORDER_MUTED);

    fb_draw_text_normal(24, c3_y + 12, "Tap Audio Clicks", COLOR_TEXT_WHITE);
    int muted = audio_is_muted();
    fb_draw_pill_badge(230, c3_y + 10, 60, 20, !muted ? COLOR_PANEL_HOVER : COLOR_PANEL_BG, !muted ? COLOR_EMERALD : COLOR_BORDER_MUTED);
    fb_draw_text_small(242, c3_y + 14, !muted ? "ON" : "MUTED", !muted ? COLOR_EMERALD : COLOR_TEXT_MUTED);

    /* Device Info Box (y = 170..208) */
    int c4_y = 170;
    fb_draw_rounded_rect_fill(14, c4_y, SCREEN_W - 28, 38, 6, COLOR_PANEL_BG);
    fb_draw_text_small(24, c4_y + 6, "Device: TomTom ONE v6 (S3C24xx ARM9 @ 264MHz)", COLOR_TEXT_MUTED);
    fb_draw_text_small(24, c4_y + 20, "RAM: 30MB Total  |  FractureOS Native C v2.0", COLOR_CYAN);

    chrome_render_footer("TAP: Bright", "2x: Night", "PWR: Menu");
}

void view_settings_handle_input(InputEventType event) {
    if (event == INPUT_SINGLE_TAP) {
        /* Cycle brightness: 20 -> 40 -> 60 -> 80 -> 100 -> 20 */
        brightness_level += 20;
        if (brightness_level > 100) brightness_level = 20;
        backlight_set_brightness(brightness_level);
        audio_play(SOUND_CLICK);
    } else if (event == INPUT_DOUBLE_TAP) {
        night_mode = !night_mode;
        audio_play(SOUND_CHIME);
    } else if (event == INPUT_LONG_TAP) {
        audio_set_muted(!audio_is_muted());
        audio_play(SOUND_CHIME);
    } else if (event == INPUT_BTN_QUICK) {
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_LAUNCHER);
    }
}
