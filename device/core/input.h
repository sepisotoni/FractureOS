#ifndef FRACTURE_INPUT_H
#define FRACTURE_INPUT_H

#include <stdint.h>

typedef enum {
    INPUT_NONE = 0,
    INPUT_SINGLE_TAP,
    INPUT_DOUBLE_TAP,
    INPUT_LONG_TAP,
    INPUT_BTN_QUICK,   /* < 250ms: toggle menu / confirm */
    INPUT_BTN_MEDIUM,  /* 400ms - 1500ms: night mode toggle */
    INPUT_BTN_LONG     /* > 3000ms: sleep / standby */
} InputEventType;

int  input_init(void);
void input_close(void);
InputEventType input_poll(void);

#endif /* FRACTURE_INPUT_H */
