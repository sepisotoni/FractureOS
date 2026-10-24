#!/bin/bash
set -e

TOMTOM_IP="192.168.101.115"
HOST_IP="192.168.101.114"
HTTP_PORT=8894
PROJ_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "=== FractureOS Zero-Touch OTA Deployment ==="
echo "Building ARM binary..."
make -C "$PROJ_DIR/device"

echo "Deploying to TomTom ($TOMTOM_IP)..."
python3 -c "
import http.server, socketserver, threading, socket, time

PORT = $HTTP_PORT
class ReusableTCPServer(socketserver.TCPServer):
    allow_reuse_address = True

class Handler(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        return '$PROJ_DIR/device/bin/fracture-core'
    def log_message(self, *args): pass

httpd = ReusableTCPServer(('0.0.0.0', PORT), Handler)
t = threading.Thread(target=httpd.serve_forever, daemon=True)
t.start()

s = socket.socket()
s.connect(('$TOMTOM_IP', 23))
time.sleep(0.3); s.recv(1024)
s.sendall(b'root\n')
time.sleep(0.3); s.recv(1024)
s.sendall(b'stty -echo\n')
time.sleep(0.2); s.recv(1024)

cmd = 'mkdir -p /mnt/sdcard/fracture/bin && wget -O /mnt/sdcard/fracture/bin/fracture-core http://$HOST_IP:$HTTP_PORT/fracture-core && chmod +x /mnt/sdcard/fracture/bin/fracture-core\necho __DEPLOY_DONE__\n'
s.sendall(cmd.encode())

buf = b''
while b'__DEPLOY_DONE__' not in buf:
    chunk = s.recv(1024)
    if not chunk: break
    buf += chunk

print('TomTom output:\n', buf.decode('latin1', errors='replace'))
s.close()
httpd.shutdown()
"

echo "=== Deployment Complete! ==="
