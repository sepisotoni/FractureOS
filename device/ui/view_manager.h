#ifndef FRACTURE_VIEW_MANAGER_H
#define FRACTURE_VIEW_MANAGER_H

#include "../core/protocol.h"
#include "../core/input.h"

void view_manager_init(void);
void view_manager_switch(int view_id);
int  view_manager_get_current_view(void);
void view_manager_render(const FractureTelemetryPacket *packet);
void view_manager_handle_input(InputEventType event);

#endif /* FRACTURE_VIEW_MANAGER_H */
