#include <stdio.h>
#include <math.h>
#include <time.h>

#include "view_timer.h"
#include "../chrome.h"
#include "../view_manager.h"
#include "../../core/framebuffer.h"
#include "../../core/audio.h"

#define FOCUS_DURATION (25 * 60)
#define BREAK_DURATION (5 * 60)

static int timer_mode = 0; /* 0: Focus (25m), 1: Break (5m) */
static int is_running = 0;
static int seconds_left = FOCUS_DURATION;
static time_t last_tick = 0;
static int completed_pomodoros = 0;

void view_timer_init(void) {
    timer_mode = 0;
    is_running = 0;
    seconds_left = FOCUS_DURATION;
    last_tick = time(NULL);
    completed_pomodoros = 0;
}

void view_timer_render(const FractureTelemetryPacket *packet) {
    (void)packet;

    /* Update timer if running */
    time_t now = time(NULL);
    if (is_running && now != last_tick) {
        int diff = (int)(now - last_tick);
        last_tick = now;
        seconds_left -= diff;
        if (seconds_left <= 0) {
            seconds_left = 0;
            is_running = 0;
            audio_play(SOUND_ALARM);
            if (timer_mode == 0) {
                completed_pomodoros++;
                timer_mode = 1; /* Switch to break */
                seconds_left = BREAK_DURATION;
            } else {
                timer_mode = 0; /* Switch to focus */
                seconds_left = FOCUS_DURATION;
            }
        }
    } else {
        last_tick = now;
    }

    uint16_t theme_color = (timer_mode == 0) ? COLOR_AMBER : COLOR_EMERALD;
    const char *header_title = (timer_mode == 0) ? "POMODORO FOCUS" : "SHORT BREAK";
    chrome_render_header(header_title, theme_color);

    /* Circular / Radial Progress Dial (Center Left, x = 70, y = 115) */
    int cx = 75, cy = 115, r = 48;
    fb_draw_circle_fill(cx, cy, r, COLOR_PANEL_BG);
    fb_draw_circle_outline(cx, cy, r, COLOR_BORDER_MUTED);

    int total_dur = (timer_mode == 0) ? FOCUS_DURATION : BREAK_DURATION;
    int progress_pct = ((total_dur - seconds_left) * 100) / total_dur;

    /* Radial Arc Dots */
    for (int angle = 0; angle < 360; angle += 10) {
        double rad = (angle - 90) * 3.14159 / 180.0;
        int px = cx + (int)(cos(rad) * (r - 4));
        int py = cy + (int)(sin(rad) * (r - 4));
        int dot_pct = (angle * 100) / 360;
        uint16_t d_col = (dot_pct <= progress_pct) ? theme_color : COLOR_BORDER_MUTED;
        fb_draw_circle_fill(px, py, 2, d_col);
    }

    /* Inner Dial Status */
    fb_draw_icon(ICON_FOCUS, cx, cy - 8, theme_color);
    char pct_buf[16];
    snprintf(pct_buf, sizeof(pct_buf), "%d%%", progress_pct);
    int pw = fb_text_width_small(pct_buf);
    fb_draw_text_small(cx - pw / 2, cy + 10, pct_buf, COLOR_TEXT_MUTED);

    /* Big Countdown Digits (Center Right, x = 145..300) */
    int min = seconds_left / 60;
    int sec = seconds_left % 60;
    char time_str[32];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", min, sec);

    fb_draw_text_digits(150, 80, time_str, theme_color);

    /* Mode Pill Badge */
    int pill_y = 120;
    fb_draw_pill_badge(150, pill_y, 110, 22, is_running ? COLOR_PANEL_HOVER : COLOR_PANEL_BG, is_running ? theme_color : COLOR_BORDER_MUTED);
    const char *state_label = is_running ? "RUNNING..." : "PAUSED";
    fb_draw_text_small(160, pill_y + 6, state_label, is_running ? theme_color : COLOR_TEXT_MUTED);

    /* Completed Sessions tally */
    char tally_buf[32];
    snprintf(tally_buf, sizeof(tally_buf), "Completed: %d Pomo", completed_pomodoros);
    fb_draw_text_small(150, 155, tally_buf, COLOR_TEXT_MUTED);

    chrome_render_footer("TAP: Switch", "2x: Start/Stop", "PWR: Menu");
}

void view_timer_handle_input(InputEventType event) {
    if (event == INPUT_DOUBLE_TAP) {
        is_running = !is_running;
        audio_play(SOUND_CHIME);
    } else if (event == INPUT_SINGLE_TAP) {
        timer_mode = !timer_mode;
        seconds_left = (timer_mode == 0) ? FOCUS_DURATION : BREAK_DURATION;
        is_running = 0;
        audio_play(SOUND_CLICK);
    } else if (event == INPUT_LONG_TAP) {
        seconds_left = (timer_mode == 0) ? FOCUS_DURATION : BREAK_DURATION;
        is_running = 0;
        audio_play(SOUND_SWOOSH);
    } else if (event == INPUT_BTN_QUICK) {
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_LAUNCHER);
    }
}
