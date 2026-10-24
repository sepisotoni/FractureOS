#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/input.h>

#include "input.h"

#ifndef EVIOCGRAB
#define EVIOCGRAB _IOW('E', 0x90, int)
#endif

typedef struct {
    uint32_t sequence;
    uint32_t duration_ms;
} GPIO_BUTTON_EVENT;

#define IOR_BUTTON_EVENT _IOR('U', 30, GPIO_BUTTON_EVENT)

static int ts_fd = -1;
static int hw_fd = -1;
static volatile int signal_quick_btn = 0;

static uint32_t last_btn_sequence = 0;
static long long last_touch_down = 0;
static long long last_tap_up = 0;
static int touch_is_down = 0;
static int pending_single_tap = 0;
static long long single_tap_deadline = 0;

static long long get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + (tv.tv_usec / 1000);
}

void input_trigger_quick_button(void) {
    signal_quick_btn = 1;
}

int input_init(void) {
    ts_fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (ts_fd < 0) {
        perror("open /dev/input/event0");
    } else {
        /* Grab event device exclusively so nano-X or background daemons don't eat events */
        if (ioctl(ts_fd, EVIOCGRAB, 1) < 0) {
            perror("EVIOCGRAB /dev/input/event0");
        }
    }

    hw_fd = open("/dev/hwstatus", O_RDONLY | O_NONBLOCK);
    if (hw_fd < 0) {
        perror("open /dev/hwstatus");
    }

    last_btn_sequence = 0;
    last_touch_down = 0;
    last_tap_up = 0;
    touch_is_down = 0;
    pending_single_tap = 0;
    signal_quick_btn = 0;
    return 0;
}

void input_close(void) {
    if (ts_fd >= 0) {
        ioctl(ts_fd, EVIOCGRAB, 0);
        close(ts_fd);
        ts_fd = -1;
    }
    if (hw_fd >= 0) {
        close(hw_fd);
        hw_fd = -1;
    }
}

InputEventType input_poll(void) {
    long long now = get_time_ms();
    InputEventType result = INPUT_NONE;

    /* 0. Check USR1 signal triggered button */
    if (signal_quick_btn) {
        signal_quick_btn = 0;
        return INPUT_BTN_QUICK;
    }

    /* 1. Poll Touchscreen (/dev/input/event0) */
    if (ts_fd >= 0) {
        struct input_event ev;
        while (read(ts_fd, &ev, sizeof(ev)) == sizeof(ev)) {
            int is_press = 0;
            int is_release = 0;

            if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
                if (ev.value == 1) is_press = 1;
                else if (ev.value == 0) is_release = 1;
            } else if (ev.type == EV_ABS && ev.code == ABS_PRESSURE) {
                if (ev.value > 0) is_press = 1;
                else is_release = 1;
            } else if (ev.type == EV_ABS && (ev.code == ABS_X || ev.code == ABS_Y)) {
                /* Active coordinate motion while down counts as active press */
                if (!touch_is_down) is_press = 1;
            }

            if (is_press && !touch_is_down) {
                /* Finger Down */
                touch_is_down = 1;
                last_touch_down = now;
            } else if (is_release && touch_is_down) {
                /* Finger Up */
                long long press_duration = now - last_touch_down;
                touch_is_down = 0;

                if (press_duration >= 600) {
                    /* Long Tap */
                    pending_single_tap = 0;
                    return INPUT_LONG_TAP;
                } else {
                    /* Short tap */
                    if (pending_single_tap && (now - last_tap_up) <= 350) {
                        /* Double Tap! */
                        pending_single_tap = 0;
                        return INPUT_DOUBLE_TAP;
                    } else {
                        /* First tap: wait up to 250ms to see if second tap follows */
                        pending_single_tap = 1;
                        last_tap_up = now;
                        single_tap_deadline = now + 250;
                    }
                }
            }
        }
    }

    /* Check if pending single tap timed out (no second tap -> single tap confirmed) */
    if (pending_single_tap && now >= single_tap_deadline) {
        pending_single_tap = 0;
        return INPUT_SINGLE_TAP;
    }

    /* 2. Poll Power Button directly from (/dev/hwstatus) */
    if (hw_fd >= 0) {
        GPIO_BUTTON_EVENT btn_event;
        if (ioctl(hw_fd, IOR_BUTTON_EVENT, &btn_event) == 0) {
            if (btn_event.sequence != last_btn_sequence && btn_event.sequence > 0) {
                last_btn_sequence = btn_event.sequence;
                uint32_t ms = btn_event.duration_ms;

                if (ms <= 300) {
                    return INPUT_BTN_QUICK;
                } else if (ms < 1500) {
                    return INPUT_BTN_MEDIUM;
                } else {
                    return INPUT_BTN_LONG;
                }
            }
        }
    }

    return result;
}

