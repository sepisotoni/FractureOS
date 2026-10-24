#ifndef FRACTURE_VIEW_LAUNCHER_H
#define FRACTURE_VIEW_LAUNCHER_H

#include "../core/protocol.h"
#include "../core/input.h"

void view_launcher_init(void);
void view_launcher_render(const FractureTelemetryPacket *packet);
void view_launcher_handle_input(InputEventType event);
int  view_launcher_get_selected_app(void);

#endif /* FRACTURE_VIEW_LAUNCHER_H */
