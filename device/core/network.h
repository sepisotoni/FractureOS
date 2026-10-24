#ifndef FRACTURE_NETWORK_H
#define FRACTURE_NETWORK_H

#include "protocol.h"

int  network_init(int port);
void network_close(void);
int  network_poll_packet(FractureTelemetryPacket *out_packet);
void network_send_action(uint8_t action_id);
int  network_is_companion_connected(void);

#endif /* FRACTURE_NETWORK_H */
