#include <stdio.h>
#include <string.h>

#include "view_media.h"
#include "../chrome.h"
#include "../view_manager.h"
#include "../../core/framebuffer.h"
#include "../../core/network.h"
#include "../../core/audio.h"

static int anim_tick = 0;
static int marquee_offset = 0;

void view_media_init(void) {
    anim_tick = 0;
    marquee_offset = 0;
}

void view_media_render(const FractureTelemetryPacket *packet) {
    chrome_render_header("NOW PLAYING", COLOR_GREEN_SPOTIFY);

    int is_playing = packet && (packet->media_state == 1 || (packet->flags & FRACTURE_FLAG_MEDIA_PLAYING));
    const char *title = (packet && packet->media_title[0]) ? packet->media_title : "No Track Playing";
    const char *artist = (packet && packet->media_artist[0]) ? packet->media_artist : "Spotify / Windows Audio";
    int pos = packet ? packet->media_pos : 115;
    int dur = packet ? packet->media_dur : 240;

    anim_tick++;

    /* Album Art / Audio Visualizer Disc (Center Left, x = 20, y = 35) */
    int disc_cx = 65, disc_cy = 95;
    int disc_r = 38;

    fb_draw_circle_fill(disc_cx, disc_cy, disc_r, COLOR_PANEL_BG);
    fb_draw_circle_outline(disc_cx, disc_cy, disc_r, is_playing ? COLOR_GREEN_SPOTIFY : COLOR_BORDER_MUTED);

    /* Grooves */
    fb_draw_circle_outline(disc_cx, disc_cy, 28, COLOR_BORDER_MUTED);
    fb_draw_circle_outline(disc_cx, disc_cy, 18, COLOR_BORDER_MUTED);
    fb_draw_circle_fill(disc_cx, disc_cy, 10, is_playing ? COLOR_GREEN_SPOTIFY : COLOR_PANEL_HOVER);
    fb_draw_circle_fill(disc_cx, disc_cy, 3, COLOR_OBSIDIAN);

    /* Equalizer Waveform Bars (Center Right, x = 125..300) */
    int eq_x = 120, eq_y = 40;
    int num_bars = 16;
    int bar_spacing = 11;

    for (int i = 0; i < num_bars; i++) {
        int max_h = 42;
        int bar_h = 6;
        if (is_playing) {
            /* Animated bouncing equalizer waveform */
            int phase = (anim_tick * 4 + i * 25) % 100;
            bar_h = 6 + (phase * (max_h - 6)) / 100;
            if (bar_h > max_h) bar_h = max_h;
        }
        int bx = eq_x + i * bar_spacing;
        int by = eq_y + max_h - bar_h;
        uint16_t b_col = (i % 2 == 0) ? COLOR_GREEN_SPOTIFY : COLOR_CYAN;
        fb_draw_rounded_rect_fill(bx, by, 7, bar_h, 2, is_playing ? b_col : COLOR_PANEL_HOVER);
    }

    /* Track Metadata Card (y = 100..165) */
    int card_y = 105;
    fb_draw_rounded_rect_fill(14, card_y, SCREEN_W - 28, 62, 8, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(14, card_y, SCREEN_W - 28, 62, 8, 1, COLOR_BORDER_MUTED);

    /* Status badge pill */
    fb_draw_pill_badge(24, card_y + 8, 60, 16, is_playing ? COLOR_PANEL_HOVER : COLOR_PANEL_BG, is_playing ? COLOR_GREEN_SPOTIFY : COLOR_BORDER_MUTED);
    fb_draw_text_small(30, card_y + 11, is_playing ? "PLAYING" : "PAUSED", is_playing ? COLOR_GREEN_SPOTIFY : COLOR_TEXT_MUTED);

    /* Title */
    int title_w = fb_text_width_normal(title);
    int title_box_w = SCREEN_W - 120;
    int tx = 92;
    if (title_w > title_box_w) {
        /* Marquee scroll */
        marquee_offset = (anim_tick * 2) % (title_w + 30);
        tx = 92 - marquee_offset;
    }
    fb_draw_text_normal(tx, card_y + 9, title, COLOR_TEXT_WHITE);

    /* Artist */
    fb_draw_text_small(24, card_y + 36, artist, COLOR_TEXT_MUTED);

    /* Progress Bar (y = 175..205) */
    int prog_y = 175;
    int bar_w = SCREEN_W - 48;
    int bar_x = 24;

    fb_draw_rect_fill(bar_x, prog_y + 6, bar_w, 4, COLOR_PANEL_BG);
    int fill_w = dur > 0 ? (bar_w * pos) / dur : 0;
    if (fill_w > bar_w) fill_w = bar_w;
    fb_draw_rect_fill(bar_x, prog_y + 6, fill_w, 4, COLOR_GREEN_SPOTIFY);
    /* Indicator scrub head */
    fb_draw_circle_fill(bar_x + fill_w, prog_y + 8, 5, COLOR_TEXT_WHITE);

    /* Timestamps */
    char pos_str[16], dur_str[16];
    snprintf(pos_str, sizeof(pos_str), "%02d:%02d", pos / 60, pos % 60);
    snprintf(dur_str, sizeof(dur_str), "%02d:%02d", dur / 60, dur % 60);

    fb_draw_text_small(bar_x, prog_y + 16, pos_str, COLOR_TEXT_MUTED);
    int dw = fb_text_width_small(dur_str);
    fb_draw_text_small(bar_x + bar_w - dw, prog_y + 16, dur_str, COLOR_TEXT_MUTED);

    chrome_render_footer("TAP: Next", "2x: Play/Pause", "PWR: Menu");
}

void view_media_handle_input(InputEventType event) {
    if (event == INPUT_DOUBLE_TAP) {
        /* Send remote Play/Pause action (ID = 1) */
        audio_play(SOUND_CHIME);
        network_send_action(1);
    } else if (event == INPUT_SINGLE_TAP) {
        /* Send remote Next Track action (ID = 2) */
        audio_play(SOUND_CLICK);
        network_send_action(2);
    } else if (event == INPUT_BTN_QUICK) {
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_LAUNCHER);
    } else if (event == INPUT_LONG_TAP) {
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_CLOCK);
    }
}
