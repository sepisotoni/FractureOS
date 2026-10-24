#include <stdio.h>
#include <string.h>

#include "view_telemetry.h"
#include "../chrome.h"
#include "../view_manager.h"
#include "../../core/framebuffer.h"
#include "../../core/audio.h"

#define HISTORY_LEN 40
static uint8_t cpu_history[HISTORY_LEN];
static uint8_t gpu_history[HISTORY_LEN];
static int hist_idx = 0;
static int show_gpu_graph = 0;

void view_telemetry_init(void) {
    memset(cpu_history, 0, sizeof(cpu_history));
    memset(gpu_history, 0, sizeof(gpu_history));
    hist_idx = 0;
    show_gpu_graph = 0;
}

static void draw_stat_card(int x, int y, int w, int h, const char *label, int pct, int temp, uint16_t color) {
    fb_draw_rounded_rect_fill(x, y, w, h, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(x, y, w, h, 6, 1, COLOR_BORDER_MUTED);

    /* Label */
    fb_draw_text_small(x + 8, y + 6, label, COLOR_TEXT_MUTED);

    /* Pct / Value */
    char val_str[16];
    snprintf(val_str, sizeof(val_str), "%d%%", pct);
    fb_draw_text_normal(x + 8, y + 18, val_str, color);

    /* Temp if available */
    if (temp > 0) {
        char temp_str[16];
        snprintf(temp_str, sizeof(temp_str), "%dC", temp);
        int tw = fb_text_width_small(temp_str);
        fb_draw_text_small(x + w - tw - 8, y + 19, temp_str, temp > 75 ? COLOR_ROSE : COLOR_TEXT_MUTED);
    }

    /* Progress bar */
    int bar_x = x + 8;
    int bar_y = y + h - 8;
    int bar_w = w - 16;
    fb_draw_rect_fill(bar_x, bar_y, bar_w, 3, COLOR_PANEL_HOVER);

    int fill_w = (bar_w * pct) / 100;
    if (fill_w > bar_w) fill_w = bar_w;
    fb_draw_rect_fill(bar_x, bar_y, fill_w, 3, color);
}

void view_telemetry_render(const FractureTelemetryPacket *packet) {
    chrome_render_header("TELEMETRY", COLOR_EMERALD);

    uint8_t cpu = packet ? packet->cpu_usage : 34;
    uint8_t gpu = packet ? packet->gpu_usage : 62;
    uint8_t ram = packet ? packet->ram_usage : 48;
    uint8_t cpu_t = packet ? packet->cpu_temp : 54;
    uint8_t gpu_t = packet ? packet->gpu_temp : 68;
    uint16_t down = packet ? packet->net_down_kb : 1420;
    uint16_t up = packet ? packet->net_up_kb : 310;

    /* Push history */
    cpu_history[hist_idx] = cpu;
    gpu_history[hist_idx] = gpu;
    hist_idx = (hist_idx + 1) % HISTORY_LEN;

    /* 4 Primary Metric Tiles (2x2) */
    int card_w = 144, card_h = 42;
    draw_stat_card(12, 28, card_w, card_h, "CPU LOAD", cpu, cpu_t, cpu > 85 ? COLOR_ROSE : COLOR_EMERALD);
    draw_stat_card(164, 28, card_w, card_h, "GPU LOAD", gpu, gpu_t, gpu > 85 ? COLOR_ROSE : COLOR_CYAN);
    draw_stat_card(12, 74, card_w, card_h, "RAM USED", ram, 0, COLOR_PURPLE);

    /* Net Card */
    int net_x = 164, net_y = 74;
    fb_draw_rounded_rect_fill(net_x, net_y, card_w, card_h, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(net_x, net_y, card_w, card_h, 6, 1, COLOR_BORDER_MUTED);
    fb_draw_text_small(net_x + 8, net_y + 6, "NETWORK SPEED", COLOR_TEXT_MUTED);

    char down_str[32], up_str[32];
    if (down > 1024) snprintf(down_str, sizeof(down_str), "DL %.1fMB", down / 1024.0);
    else snprintf(down_str, sizeof(down_str), "DL %dKB", down);

    if (up > 1024) snprintf(up_str, sizeof(up_str), "UP %.1fMB", up / 1024.0);
    else snprintf(up_str, sizeof(up_str), "UP %dKB", up);

    fb_draw_text_small(net_x + 8, net_y + 22, down_str, COLOR_CYAN);
    fb_draw_text_small(net_x + 78, net_y + 22, up_str, COLOR_EMERALD);

    /* Historical Sparkline Graph Area (y = 122..208) */
    int graph_x = 12, graph_y = 122;
    int graph_w = SCREEN_W - 24, graph_h = 86;
    fb_draw_rounded_rect_fill(graph_x, graph_y, graph_w, graph_h, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(graph_x, graph_y, graph_w, graph_h, 6, 1, COLOR_BORDER_MUTED);

    const char *graph_title = show_gpu_graph ? "GPU LOAD HISTORY (40s)" : "CPU LOAD HISTORY (40s)";
    uint16_t graph_color = show_gpu_graph ? COLOR_CYAN : COLOR_EMERALD;
    fb_draw_text_small(graph_x + 8, graph_y + 6, graph_title, graph_color);

    /* Draw grid lines */
    for (int g = 25; g <= 75; g += 25) {
        int gy = graph_y + graph_h - 10 - (g * (graph_h - 26) / 100);
        for (int gx = graph_x + 8; gx < graph_x + graph_w - 8; gx += 8) {
            fb_put_pixel(gx, gy, COLOR_BORDER_MUTED);
        }
    }

    /* Plot sparkline */
    uint8_t *src_hist = show_gpu_graph ? gpu_history : cpu_history;
    int step_x = (graph_w - 24) / (HISTORY_LEN - 1);
    int prev_px = -1, prev_py = -1;

    for (int i = 0; i < HISTORY_LEN; i++) {
        int idx = (hist_idx + i) % HISTORY_LEN;
        int val = src_hist[idx];
        int px = graph_x + 12 + i * step_x;
        int py = graph_y + graph_h - 10 - (val * (graph_h - 26) / 100);

        if (prev_px >= 0) {
            fb_draw_line(prev_px, prev_py, px, py, graph_color);
        }
        prev_px = px;
        prev_py = py;
    }

    chrome_render_footer("TAP: Graph", "2x: Reset", "PWR: Menu");
}

void view_telemetry_handle_input(InputEventType event) {
    if (event == INPUT_SINGLE_TAP) {
        show_gpu_graph = !show_gpu_graph;
        audio_play(SOUND_CLICK);
    } else if (event == INPUT_DOUBLE_TAP) {
        memset(cpu_history, 0, sizeof(cpu_history));
        memset(gpu_history, 0, sizeof(gpu_history));
        audio_play(SOUND_CHIME);
    } else if (event == INPUT_BTN_QUICK) {
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_LAUNCHER);
    } else if (event == INPUT_LONG_TAP) {
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_CLOCK);
    }
}
