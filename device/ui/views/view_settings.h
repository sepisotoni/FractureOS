#ifndef FRACTURE_VIEW_SETTINGS_H
#define FRACTURE_VIEW_SETTINGS_H

#include "../core/protocol.h"
#include "../core/input.h"

void view_settings_init(void);
void view_settings_render(const FractureTelemetryPacket *packet);
void view_settings_handle_input(InputEventType event);

#endif /* FRACTURE_VIEW_SETTINGS_H */
