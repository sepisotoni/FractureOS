#ifndef FRACTURE_VIEW_SERVER_H
#define FRACTURE_VIEW_SERVER_H

#include "../core/protocol.h"
#include "../core/input.h"

void view_server_init(void);
void view_server_render(const FractureTelemetryPacket *packet);
void view_server_handle_input(InputEventType event);

#endif /* FRACTURE_VIEW_SERVER_H */
