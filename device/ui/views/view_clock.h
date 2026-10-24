#ifndef FRACTURE_VIEW_CLOCK_H
#define FRACTURE_VIEW_CLOCK_H

#include "../core/protocol.h"
#include "../core/input.h"

void view_clock_init(void);
void view_clock_render(const FractureTelemetryPacket *packet);
void view_clock_handle_input(InputEventType event);

#endif /* FRACTURE_VIEW_CLOCK_H */
