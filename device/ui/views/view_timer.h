#ifndef FRACTURE_VIEW_TIMER_H
#define FRACTURE_VIEW_TIMER_H

#include "../core/protocol.h"
#include "../core/input.h"

void view_timer_init(void);
void view_timer_render(const FractureTelemetryPacket *packet);
void view_timer_handle_input(InputEventType event);

#endif /* FRACTURE_VIEW_TIMER_H */
