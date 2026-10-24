#include <stdio.h>

#include "view_manager.h"
#include "chrome.h"
#include "../core/framebuffer.h"
#include "views/view_clock.h"
#include "views/view_launcher.h"
#include "views/view_telemetry.h"
#include "views/view_media.h"
#include "views/view_timer.h"
#include "views/view_server.h"
#include "views/view_settings.h"

static int current_view = VIEW_CLOCK;

void view_manager_init(void) {
    view_clock_init();
    view_launcher_init();
    view_telemetry_init();
    view_media_init();
    view_timer_init();
    view_server_init();
    view_settings_init();
    current_view = VIEW_CLOCK;
}

void view_manager_switch(int view_id) {
    if (view_id >= 0 && view_id < VIEW_COUNT) {
        current_view = view_id;
    }
}

int view_manager_get_current_view(void) {
    return current_view;
}

void view_manager_render(const FractureTelemetryPacket *packet) {
    switch (current_view) {
        case VIEW_CLOCK:
            view_clock_render(packet);
            break;
        case VIEW_LAUNCHER:
            view_launcher_render(packet);
            break;
        case VIEW_TELEMETRY:
            view_telemetry_render(packet);
            break;
        case VIEW_MEDIA:
            view_media_render(packet);
            break;
        case VIEW_TIMER:
            view_timer_render(packet);
            break;
        case VIEW_SERVER:
            view_server_render(packet);
            break;
        case VIEW_SETTINGS:
            view_settings_render(packet);
            break;
        default:
            view_clock_render(packet);
            break;
    }

    /* Notification overlay if active */
    if (packet && packet->notif_timeout > 0 && packet->notif_title[0] != '\0') {
        chrome_render_toast(packet->notif_title, COLOR_AMBER);
    }
}

void view_manager_handle_input(InputEventType event) {
    if (event == INPUT_NONE) return;

    /* Global shortcut: medium power button press (400ms-1500ms) toggles launcher from anywhere */
    if (event == INPUT_BTN_MEDIUM) {
        if (current_view == VIEW_LAUNCHER) {
            view_manager_switch(VIEW_CLOCK);
        } else {
            view_manager_switch(VIEW_LAUNCHER);
        }
        return;
    }

    switch (current_view) {
        case VIEW_CLOCK:
            view_clock_handle_input(event);
            break;
        case VIEW_LAUNCHER:
            view_launcher_handle_input(event);
            break;
        case VIEW_TELEMETRY:
            view_telemetry_handle_input(event);
            break;
        case VIEW_MEDIA:
            view_media_handle_input(event);
            break;
        case VIEW_TIMER:
            view_timer_handle_input(event);
            break;
        case VIEW_SERVER:
            view_server_handle_input(event);
            break;
        case VIEW_SETTINGS:
            view_settings_handle_input(event);
            break;
        default:
            view_clock_handle_input(event);
            break;
    }
}
