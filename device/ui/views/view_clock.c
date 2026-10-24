#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "view_clock.h"
#include "../chrome.h"
#include "../view_manager.h"
#include "../../core/framebuffer.h"
#include "../../core/audio.h"

static int show_seconds = 1;
static int force_zen_face = 0;

void view_clock_init(void) {
    show_seconds = 1;
    force_zen_face = 0;
}

void view_clock_render(const FractureTelemetryPacket *packet) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    int is_pc_active = (packet && (packet->flags & FRACTURE_FLAG_PC_ACTIVE));
    int is_night_mode = (packet && (packet->flags & FRACTURE_FLAG_NIGHT_MODE));

    uint16_t time_color = is_night_mode ? COLOR_AMBER : COLOR_CYAN;
    uint16_t accent_color = is_night_mode ? COLOR_AMBER : COLOR_BLUE;

    /* Top Status / Complication slots */
    fb_draw_rect_fill(0, 0, SCREEN_W, 22, COLOR_PANEL_BG);
    fb_draw_rect_fill(0, 22, SCREEN_W, 1, COLOR_BORDER_MUTED);

    if (is_pc_active && !force_zen_face && packet) {
        /* Active Desk Mode: Show complications in Top-Left and Top-Right */
        char cpu_str[32];
        snprintf(cpu_str, sizeof(cpu_str), "CPU %d%%", packet->cpu_usage);
        fb_draw_text_small(12, 6, cpu_str, packet->cpu_usage > 85 ? COLOR_ROSE : COLOR_EMERALD);

        char ram_str[32];
        snprintf(ram_str, sizeof(ram_str), "RAM %d%%", packet->ram_usage);
        int rw = fb_text_width_small(ram_str);
        fb_draw_text_small(SCREEN_W - rw - 12, 6, ram_str, COLOR_TEXT_MUTED);

        /* Center indicator */
        fb_draw_text_small(125, 6, "DESK LIVE", COLOR_CYAN);
    } else {
        /* Zen / Ambient Mode: Clean brand and subtle status */
        fb_draw_circle_fill(10, 11, 3, time_color);
        fb_draw_text_normal(18, 4, "FRACTURE", COLOR_TEXT_WHITE);

        if (packet && packet->weather_temp != 0) {
            char temp_str[16];
            snprintf(temp_str, sizeof(temp_str), "%d.%dC", packet->weather_temp / 10, abs(packet->weather_temp % 10));
            int tw = fb_text_width_small(temp_str);
            fb_draw_icon(ICON_WEATHER_SUN, SCREEN_W - tw - 24, 11, COLOR_AMBER);
            fb_draw_text_small(SCREEN_W - tw - 10, 6, temp_str, COLOR_TEXT_WHITE);
        }
    }

    /* Hero Clock Face in Center */
    char hour_str[8], min_str[8], sec_str[8];
    if (tm) {
        snprintf(hour_str, sizeof(hour_str), "%02d", tm->tm_hour);
        snprintf(min_str, sizeof(min_str), "%02d", tm->tm_min);
        snprintf(sec_str, sizeof(sec_str), "%02d", tm->tm_sec);
    } else {
        strcpy(hour_str, "12");
        strcpy(min_str, "00");
        strcpy(sec_str, "00");
    }

    int center_y = 65;

    /* Big Hero Time Display */
    char full_time[32];
    snprintf(full_time, sizeof(full_time), "%s:%s", hour_str, min_str);
    int tw = fb_text_width_digits(full_time);
    int tx = (SCREEN_W - tw) / 2;

    /* Draw time digits in large crisp typography */
    fb_draw_text_digits(tx, center_y, full_time, time_color);

    /* Seconds pill indicator */
    if (show_seconds && tm) {
        int sec_x = tx + tw + 6;
        int sec_y = center_y + 12;
        fb_draw_text_small(sec_x, sec_y, sec_str, COLOR_TEXT_MUTED);

        /* Linear progress seconds bar */
        int bar_w = 160;
        int bar_x = (SCREEN_W - bar_w) / 2;
        int bar_y = center_y + 44;
        fb_draw_rect_fill(bar_x, bar_y, bar_w, 3, COLOR_PANEL_BG);
        int prog_w = (bar_w * tm->tm_sec) / 59;
        if (prog_w > bar_w) prog_w = bar_w;
        fb_draw_rect_fill(bar_x, bar_y, prog_w, 3, accent_color);
    }

    /* Date line */
    char date_buf[64];
    if (tm) {
        strftime(date_buf, sizeof(date_buf), "%A, %d %B", tm);
    } else {
        strcpy(date_buf, "Saturday, 24 October");
    }
    int dw = fb_text_width_normal(date_buf);
    fb_draw_text_normal((SCREEN_W - dw) / 2, center_y + 56, date_buf, COLOR_TEXT_WHITE);

    /* Context Bar: Now playing ticker or system info */
    if (packet && (packet->flags & FRACTURE_FLAG_MEDIA_PLAYING) && packet->media_title[0] != '\0') {
        int bar_y = 176;
        fb_draw_rounded_rect_fill(14, bar_y, SCREEN_W - 28, 26, 6, COLOR_PANEL_BG);
        fb_draw_icon(ICON_SPOTIFY, 26, bar_y + 13, COLOR_GREEN_SPOTIFY);

        char media_str[64];
        snprintf(media_str, sizeof(media_str), "%s - %s", packet->media_title, packet->media_artist);
        fb_draw_text_small(40, bar_y + 7, media_str, COLOR_TEXT_WHITE);
    } else {
        /* Subtle Uptime / Status pill */
        int bar_y = 176;
        fb_draw_rounded_rect_fill(20, bar_y, SCREEN_W - 40, 26, 6, COLOR_PANEL_BG);
        fb_draw_text_small(30, bar_y + 7, "FRACTURE OS", COLOR_CYAN);
        fb_draw_text_small(130, bar_y + 7, is_pc_active ? "Workstation Active" : "Desk Idle / Zen", COLOR_TEXT_MUTED);
    }

    /* Bottom Navigation Prompts */
    chrome_render_footer("TAP: Style", "2x: Face", "PWR: Menu");
}

void view_clock_handle_input(InputEventType event) {
    if (event == INPUT_SINGLE_TAP) {
        show_seconds = !show_seconds;
        audio_play(SOUND_CLICK);
    } else if (event == INPUT_DOUBLE_TAP) {
        force_zen_face = !force_zen_face;
        audio_play(SOUND_CHIME);
    } else if (event == INPUT_BTN_QUICK) {
        /* In Clock: Quick power button opens Launcher */
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_LAUNCHER);
    } else if (event == INPUT_BTN_MEDIUM) {
        /* Hold power button toggles zen vs complications directly */
        force_zen_face = !force_zen_face;
        audio_play(SOUND_CHIME);
    } else if (event == INPUT_LONG_TAP) {
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_TELEMETRY);
    }
}

