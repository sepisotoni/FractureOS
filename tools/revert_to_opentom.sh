#!/bin/bash
set -e

TOMTOM_IP="192.168.101.115"

echo "Reverting TomTom back to original OpenTom..."
python3 -c "
import socket, time

s = socket.socket()
s.connect(('$TOMTOM_IP', 23))
time.sleep(0.3); s.recv(1024)
s.sendall(b'root\n'); time.sleep(0.3); s.recv(1024)
s.sendall(b'stty -echo\n'); time.sleep(0.2); s.recv(1024)

cmd = '''
if [ -x /mnt/sdcard/fracture/restore_opentom.sh ]; then
    sh /mnt/sdcard/fracture/restore_opentom.sh
else
    killall -9 fracture-core 2>/dev/null || true
    killall -CONT watchface.new 2>/dev/null || true
    pidof watchface.new >/dev/null 2>&1 || ( /mnt/sdcard/opentom/bin/watchface-main & )
fi
echo __REVERT_DONE__
'''
s.sendall(cmd.encode())

buf = b''
while b'__REVERT_DONE__' not in buf:
    chunk = s.recv(1024)
    if not chunk: break
    buf += chunk

print('Device response:\n', buf.decode('latin1', errors='replace'))
s.close()
"
echo "=== Restored to OpenTom! FractureOS has been safely stopped. ==="
