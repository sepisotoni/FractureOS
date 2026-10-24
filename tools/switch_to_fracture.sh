#!/bin/bash
set -e

TOMTOM_IP="192.168.101.115"
HOST_IP="192.168.101.114"
HTTP_PORT=8894
PROJ_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Building FractureOS..."
make -C "$PROJ_DIR/device"

echo "Deploying to TomTom..."
python3 -c "
import http.server, socketserver, threading, socket, time

class ReusableTCPServer(socketserver.TCPServer):
    allow_reuse_address = True

class Handler(http.server.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        if 'fracture-core' in path: return '$PROJ_DIR/device/bin/fracture-core'
        if 'start_fracture' in path: return '$PROJ_DIR/tools/start_fracture.sh'
        if 'restore_opentom' in path: return '$PROJ_DIR/tools/restore_opentom.sh'
        return '/dev/null'
    def log_message(self, *args): pass

httpd = ReusableTCPServer(('0.0.0.0', $HTTP_PORT), Handler)
t = threading.Thread(target=httpd.serve_forever, daemon=True)
t.start()

s = socket.socket()
s.connect(('$TOMTOM_IP', 23))
time.sleep(0.3); s.recv(1024)
s.sendall(b'root\n'); time.sleep(0.3); s.recv(1024)
s.sendall(b'stty -echo\n'); time.sleep(0.2); s.recv(1024)

cmd = '''
killall -9 fracture-core 2>/dev/null || true
mkdir -p /mnt/sdcard/fracture/bin
wget -O /mnt/sdcard/fracture/bin/fracture-core http://$HOST_IP:$HTTP_PORT/fracture-core
wget -O /mnt/sdcard/fracture/start_fracture.sh http://$HOST_IP:$HTTP_PORT/start_fracture.sh
wget -O /mnt/sdcard/fracture/restore_opentom.sh http://$HOST_IP:$HTTP_PORT/restore_opentom.sh
chmod +x /mnt/sdcard/fracture/bin/fracture-core /mnt/sdcard/fracture/*.sh

# Run start script
sh /mnt/sdcard/fracture/start_fracture.sh
echo __SWITCH_DONE__
'''
s.sendall(cmd.encode())

buf = b''
while b'__SWITCH_DONE__' not in buf:
    chunk = s.recv(1024)
    if not chunk: break
    buf += chunk

print('Device response:\n', buf.decode('latin1', errors='replace'))
s.close()
httpd.shutdown()
"

echo "=== FractureOS is now deployed and running on your TomTom! ==="
