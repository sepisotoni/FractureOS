#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "protocol.h"
#include "framebuffer.h"
#include "input.h"
#include "audio.h"
#include "backlight.h"
#include "network.h"
#include "../ui/view_manager.h"

static volatile int running = 1;

static void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        input_trigger_quick_button();
        return;
    }
    running = 0;
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGUSR1, handle_signal);

    printf("Starting FractureOS Core Daemon...\n");

    if (fb_init(NULL) != 0) {
        fprintf(stderr, "Failed to initialize framebuffer\n");
        return 1;
    }

    input_init();
    audio_init();
    backlight_init();
    network_init(FRACTURE_DEFAULT_PORT);
    view_manager_init();

    FractureTelemetryPacket telemetry;
    memset(&telemetry, 0, sizeof(telemetry));

    time_t last_clock_tick = 0;
    int needs_redraw = 1;

    /* Play subtle power-on chime */
    audio_play(SOUND_CHIME);

    while (running) {
        time_t now = time(NULL);

        /* 1. Poll incoming network telemetry packets */
        if (network_poll_packet(&telemetry)) {
            needs_redraw = 1;

            /* Check remote view switch command */
            if (telemetry.active_view > 0 && telemetry.active_view < VIEW_COUNT) {
                view_manager_switch(telemetry.active_view);
            }

            /* Check remote brightness adjustment */
            if (telemetry.brightness <= 100) {
                backlight_set_brightness(telemetry.brightness);
            }
        }

        /* 2. Poll hardware input (touchscreen taps & power button) */
        InputEventType input_ev = input_poll();
        if (input_ev != INPUT_NONE) {
            view_manager_handle_input(input_ev);
            needs_redraw = 1;
        }

        /* 3. 1-second system clock tick */
        if (now != last_clock_tick) {
            last_clock_tick = now;
            needs_redraw = 1;

            /* Decrement notification timeout if active */
            if (telemetry.notif_timeout > 0) {
                telemetry.notif_timeout--;
            }
        }

        /* 4. Render and present if state changed */
        if (needs_redraw) {
            fb_clear(COLOR_OBSIDIAN);
            view_manager_render(&telemetry);
            fb_present();
            needs_redraw = 0;
        }

        /* 40ms sleep for ~25 FPS responsiveness with minimal CPU */
        usleep(40000);
    }

    printf("FractureOS shutting down gracefully...\n");
    audio_play(SOUND_SWOOSH);

    input_close();
    network_close();
    audio_close();
    fb_close();

    printf("FractureOS shutdown complete.\n");
    return 0;
}
