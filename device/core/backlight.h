#ifndef FRACTURE_BACKLIGHT_H
#define FRACTURE_BACKLIGHT_H

int  backlight_init(void);
void backlight_set_brightness(int pct);
int  backlight_get_brightness(void);
void backlight_set_power(int on);
int  backlight_is_power_on(void);

#endif /* FRACTURE_BACKLIGHT_H */
