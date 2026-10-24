#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "backlight.h"

static int current_brightness = 80;
static int current_power = 1;

int backlight_init(void) {
    int b = backlight_get_brightness();
    if (b > 0) current_brightness = b;
    return 0;
}

void backlight_set_brightness(int pct) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    current_brightness = pct;

    FILE *f = fopen("/sys/class/backlight/s3c/brightness", "w");
    if (f) {
        fprintf(f, "%d\n", pct);
        fclose(f);
    }
}

int backlight_get_brightness(void) {
    FILE *f = fopen("/sys/class/backlight/s3c/brightness", "r");
    if (f) {
        int val = 0;
        if (fscanf(f, "%d", &val) == 1) {
            fclose(f);
            return val;
        }
        fclose(f);
    }
    return current_brightness;
}

void backlight_set_power(int on) {
    current_power = on ? 1 : 0;
    FILE *f = fopen("/sys/class/backlight/s3c/power", "w");
    if (f) {
        /* power: 0 is ON, 1 is OFF in standard sysfs backlight */
        fprintf(f, "%d\n", on ? 0 : 1);
        fclose(f);
    }
}

int backlight_is_power_on(void) {
    return current_power;
}
