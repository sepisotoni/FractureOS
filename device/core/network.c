#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "network.h"

static int sock_fd = -1;
static struct sockaddr_in last_peer_addr;
static socklen_t last_peer_len = sizeof(struct sockaddr_in);
static time_t last_packet_time = 0;

int network_init(int port) {
    if (port <= 0) port = FRACTURE_DEFAULT_PORT;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("network socket");
        return -1;
    }

    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /* Set non-blocking */
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("network bind");
        close(sock_fd);
        sock_fd = -1;
        return -1;
    }

    last_packet_time = 0;
    return 0;
}

void network_close(void) {
    if (sock_fd >= 0) {
        close(sock_fd);
        sock_fd = -1;
    }
}

int network_poll_packet(FractureTelemetryPacket *out_packet) {
    if (sock_fd < 0 || !out_packet) return 0;

    struct sockaddr_in peer;
    socklen_t peer_len = sizeof(peer);
    uint8_t buffer[sizeof(FractureTelemetryPacket) + 32];

    int n = recvfrom(sock_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&peer, &peer_len);
    if (n >= (int)sizeof(FractureTelemetryPacket)) {
        FractureTelemetryPacket *p = (FractureTelemetryPacket *)buffer;
        if (p->magic == FRACTURE_MAGIC) {
            memcpy(out_packet, p, sizeof(FractureTelemetryPacket));
            memcpy(&last_peer_addr, &peer, sizeof(peer));
            last_peer_len = peer_len;
            last_packet_time = time(NULL);
            return 1;
        }
    }
    return 0;
}

void network_send_action(uint8_t action_id) {
    if (sock_fd < 0 || last_packet_time == 0) return;

    FractureDeviceResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.magic = FRACTURE_MAGIC;
    resp.version = FRACTURE_VERSION;
    resp.action_event = action_id;
    resp.uptime_sec = (uint32_t)time(NULL);

    sendto(sock_fd, &resp, sizeof(resp), 0, (struct sockaddr *)&last_peer_addr, last_peer_len);
}

int network_is_companion_connected(void) {
    if (last_packet_time == 0) return 0;
    return (time(NULL) - last_packet_time) <= 10;
}
