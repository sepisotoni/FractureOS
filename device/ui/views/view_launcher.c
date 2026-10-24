#include <stdio.h>
#include <string.h>

#include "view_launcher.h"
#include "../chrome.h"
#include "../view_manager.h"
#include "../../core/framebuffer.h"
#include "../../core/audio.h"

typedef struct {
    int target_view;
    const char *title;
    const char *subtitle;
    uint16_t color;
    int icon_id;
} AppTile;

static AppTile TILES[6] = {
    { VIEW_CLOCK,     "CLOCK",     "Zen & Desk",  COLOR_CYAN,          ICON_CLOCK },
    { VIEW_TELEMETRY, "TELEMETRY", "PC Monitor",  COLOR_EMERALD,       ICON_TELEMETRY },
    { VIEW_MEDIA,     "SPOTIFY",   "Media Hub",   COLOR_GREEN_SPOTIFY, ICON_SPOTIFY },
    { VIEW_TIMER,     "POMODORO",  "Focus Timer", COLOR_AMBER,         ICON_FOCUS },
    { VIEW_SERVER,    "SERVER",    "Ubuntu NAS",  COLOR_PURPLE,        ICON_SERVER },
    { VIEW_SETTINGS,  "SETTINGS",  "Display Lux", COLOR_ROSE,          ICON_DISPLAY }
};

static int selected_index = 1; /* Default to TELEMETRY */
static int toast_countdown = 0;

void view_launcher_init(void) {
    selected_index = 1;
    toast_countdown = 0;
}

void view_launcher_render(const FractureTelemetryPacket *packet) {
    (void)packet;

    /* Render standard top header */
    chrome_render_header("APP MENU", COLOR_CYAN);

    /* 6 Tiles Grid: 3 columns x 2 rows */
    int card_w = 94, card_h = 76;
    int start_x = 11, gap_x = 9;
    int start_y = 33, gap_y = 9;

    for (int i = 0; i < 6; i++) {
        int col = i % 3;
        int row = i / 3;
        int x = start_x + col * (card_w + gap_x);
        int y = start_y + row * (card_h + gap_y);
        int is_sel = (i == selected_index);

        uint16_t border_col = is_sel ? TILES[i].color : COLOR_BORDER_MUTED;
        uint16_t card_bg = is_sel ? COLOR_PANEL_HOVER : COLOR_PANEL_BG;

        fb_draw_rounded_rect_fill(x, y, card_w, card_h, 8, card_bg);
        fb_draw_rounded_rect_outline(x, y, card_w, card_h, 8, is_sel ? 2 : 1, border_col);

        /* Icon circular plate */
        int ic_cx = x + card_w / 2;
        int ic_cy = y + 23;
        fb_draw_circle_fill(ic_cx, ic_cy, 14, is_sel ? COLOR_OBSIDIAN : COLOR_PANEL_HOVER);
        fb_draw_icon(TILES[i].icon_id, ic_cx, ic_cy, TILES[i].color);

        /* Title */
        int tw = fb_text_width_small(TILES[i].title);
        fb_draw_text_small(x + (card_w - tw) / 2, y + 43, TILES[i].title, is_sel ? COLOR_TEXT_WHITE : COLOR_TEXT_MUTED);

        /* Subtitle */
        int sw = fb_text_width_small(TILES[i].subtitle);
        fb_draw_text_small(x + (card_w - sw) / 2, y + 57, TILES[i].subtitle, is_sel ? TILES[i].color : COLOR_TEXT_DARK);

        /* Corner glowing indicator */
        if (is_sel) {
            fb_draw_circle_fill(x + card_w - 9, y + 9, 3, TILES[i].color);
        }
    }

    /* Footer Navigation Prompts (Zero collision!) */
    if (toast_countdown > 0) {
        toast_countdown--;
        char toast[48];
        snprintf(toast, sizeof(toast), "Opening %s...", TILES[selected_index].title);
        chrome_render_toast(toast, TILES[selected_index].color);
    } else {
        chrome_render_footer("TAP: Next", "2x: Launch", "PWR: Open");
    }
}

void view_launcher_handle_input(InputEventType event) {
    if (event == INPUT_SINGLE_TAP) {
        selected_index = (selected_index + 1) % 6;
        audio_play(SOUND_CLICK);
    } else if (event == INPUT_DOUBLE_TAP || event == INPUT_BTN_QUICK) {
        audio_play(SOUND_CHIME);
        toast_countdown = 8;
        view_manager_switch(TILES[selected_index].target_view);
    } else if (event == INPUT_LONG_TAP) {
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_CLOCK);
    }
}

int view_launcher_get_selected_app(void) {
    return selected_index;
}
