#include <stdio.h>
#include <string.h>

#include "view_server.h"
#include "../chrome.h"
#include "../view_manager.h"
#include "../../core/framebuffer.h"
#include "../../core/audio.h"

void view_server_init(void) {
}

void view_server_render(const FractureTelemetryPacket *packet) {
    chrome_render_header("LINUX SERVER", COLOR_PURPLE);

    uint8_t cpu = packet ? packet->srv_cpu_usage : 8;
    uint8_t ram = packet ? packet->srv_ram_usage : 32;
    uint8_t disk = packet ? packet->srv_disk_usage : 24;
    uint8_t temp = packet ? packet->srv_temp : 42;

    /* Main Host Header Card */
    int card_y = 30;
    fb_draw_rounded_rect_fill(14, card_y, SCREEN_W - 28, 48, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(14, card_y, SCREEN_W - 28, 48, 6, 1, COLOR_BORDER_MUTED);

    fb_draw_icon(ICON_SERVER, 32, card_y + 24, COLOR_PURPLE);
    fb_draw_text_normal(52, card_y + 10, "stoni-room-serve", COLOR_TEXT_WHITE);
    fb_draw_text_small(52, card_y + 28, "Ubuntu 24.04 LTS (x86_64)", COLOR_TEXT_MUTED);

    /* LAN Pill Badge */
    fb_draw_pill_badge(SCREEN_W - 96, card_y + 14, 70, 20, COLOR_PANEL_HOVER, COLOR_BORDER_MUTED);
    fb_draw_text_small(SCREEN_W - 88, card_y + 19, "ONLINE", COLOR_EMERALD);

    /* 3 Performance Metric Cards (Row 2, y = 86..150) */
    int stat_w = 90, stat_h = 56;
    int gap = 11;
    int start_x = 14;

    /* CPU */
    int c1_x = start_x;
    fb_draw_rounded_rect_fill(c1_x, 86, stat_w, stat_h, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(c1_x, 86, stat_w, stat_h, 6, 1, COLOR_BORDER_MUTED);
    fb_draw_text_small(c1_x + 8, 92, "SRV CPU", COLOR_TEXT_MUTED);
    char cpu_s[16]; snprintf(cpu_s, sizeof(cpu_s), "%d%%", cpu);
    fb_draw_text_normal(c1_x + 8, 108, cpu_s, COLOR_CYAN);
    if (temp > 0) {
        char temp_s[16]; snprintf(temp_s, sizeof(temp_s), "%dC", temp);
        fb_draw_text_small(c1_x + 8, 126, temp_s, COLOR_TEXT_DARK);
    }

    /* RAM */
    int c2_x = start_x + stat_w + gap;
    fb_draw_rounded_rect_fill(c2_x, 86, stat_w, stat_h, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(c2_x, 86, stat_w, stat_h, 6, 1, COLOR_BORDER_MUTED);
    fb_draw_text_small(c2_x + 8, 92, "SRV RAM", COLOR_TEXT_MUTED);
    char ram_s[16]; snprintf(ram_s, sizeof(ram_s), "%d%%", ram);
    fb_draw_text_normal(c2_x + 8, 108, ram_s, COLOR_PURPLE);
    fb_draw_text_small(c2_x + 8, 126, "16GB Pool", COLOR_TEXT_DARK);

    /* Disk */
    int c3_x = start_x + 2 * (stat_w + gap);
    fb_draw_rounded_rect_fill(c3_x, 86, stat_w, stat_h, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(c3_x, 86, stat_w, stat_h, 6, 1, COLOR_BORDER_MUTED);
    fb_draw_text_small(c3_x + 8, 92, "STORAGE", COLOR_TEXT_MUTED);
    char disk_s[16]; snprintf(disk_s, sizeof(disk_s), "%d%%", disk);
    fb_draw_text_normal(c3_x + 8, 108, disk_s, COLOR_EMERALD);
    fb_draw_text_small(c3_x + 8, 126, "ZFS / NVMe", COLOR_TEXT_DARK);

    /* Network / Connectivity Status Card (y = 150..205) */
    int net_y = 150;
    fb_draw_rounded_rect_fill(14, net_y, SCREEN_W - 28, 54, 6, COLOR_PANEL_BG);
    fb_draw_rounded_rect_outline(14, net_y, SCREEN_W - 28, 54, 6, 1, COLOR_BORDER_MUTED);

    fb_draw_text_small(24, net_y + 8, "IP Host: 192.168.101.114  |  TomTom: 192.168.101.115", COLOR_TEXT_MUTED);
    fb_draw_text_small(24, net_y + 24, "Tailscale: 100.77.125.59 (Connected)", COLOR_CYAN);
    fb_draw_text_small(24, net_y + 38, "Link: USB Gadget Ethernet (0.65ms RTT)", COLOR_TEXT_DARK);

    chrome_render_footer("TAP: Refresh", "2x: Ping", "PWR: Menu");
}

void view_server_handle_input(InputEventType event) {
    if (event == INPUT_SINGLE_TAP || event == INPUT_DOUBLE_TAP) {
        audio_play(SOUND_CLICK);
    } else if (event == INPUT_BTN_QUICK) {
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_LAUNCHER);
    } else if (event == INPUT_LONG_TAP) {
        audio_play(SOUND_SWOOSH);
        view_manager_switch(VIEW_CLOCK);
    }
}
