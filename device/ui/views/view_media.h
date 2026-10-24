#ifndef FRACTURE_VIEW_MEDIA_H
#define FRACTURE_VIEW_MEDIA_H

#include "../core/protocol.h"
#include "../core/input.h"

void view_media_init(void);
void view_media_render(const FractureTelemetryPacket *packet);
void view_media_handle_input(InputEventType event);

#endif /* FRACTURE_VIEW_MEDIA_H */
