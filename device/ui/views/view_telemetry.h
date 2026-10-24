#ifndef FRACTURE_VIEW_TELEMETRY_H
#define FRACTURE_VIEW_TELEMETRY_H

#include "../core/protocol.h"
#include "../core/input.h"

void view_telemetry_init(void);
void view_telemetry_render(const FractureTelemetryPacket *packet);
void view_telemetry_handle_input(InputEventType event);

#endif /* FRACTURE_VIEW_TELEMETRY_H */
