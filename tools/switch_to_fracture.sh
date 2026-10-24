#!/bin/bash
set -e

HOST_IP="192.168.101.114"
TOMTOM_IP="192.168.101.115"
PORT=8000

echo "[*] Compiling FractureOS device core..."
make -C /home/sepisotoni/projects/FractureOS/device clean
make -C /home/sepisotoni/projects/FractureOS/device

echo "[*] Serving binary over HTTP..."
python3 -m http.server $PORT --directory /home/sepisotoni/projects/FractureOS/device/bin &
HTTP_PID=$!
sleep 1


echo "[*] Deploying and switching to FractureOS on TomTom..."
python3 -c "
import telnetlib, time
tn = telnetlib.Telnet('${TOMTOM_IP}', 23, timeout=5)
tn.read_until(b'login: ', timeout=3)
tn.write(b'root\n')
time.sleep(1)

# Ensure fracture dir exists
tn.write(b'mkdir -p /mnt/sdcard/fracture/bin /mnt/sdcard/fracture/config /mnt/sdcard/fracture/logs\n')
time.sleep(1)

# Pause OpenTom UI to release framebuffer and avoid CPU usage
tn.write(b'killall -STOP nano-X watchface.new\n')
time.sleep(1)

# Stop any previous fracture instance
tn.write(b'killall -9 fracture-core\n')
time.sleep(1)

# Fetch latest binary
tn.write(b'wget http://${HOST_IP}:${PORT}/fracture-core -O /mnt/sdcard/fracture/bin/fracture-core\n')
time.sleep(2)
tn.write(b'chmod +x /mnt/sdcard/fracture/bin/fracture-core\n')
time.sleep(1)

# Setup power button hook: watchface-toggle-info sends USR1 to fracture-core
tn.write(b'echo \"#!/bin/sh\npid=\\\$(pidof fracture-core)\n[ -n \\\"\\\$pid\\\" ] && kill -USR1 \\\"\\\$pid\\\"\n\" > /mnt/sdcard/opentom/bin/watchface-toggle-info && chmod +x /mnt/sdcard/opentom/bin/watchface-toggle-info\n')
time.sleep(1)

# Launch FractureOS core daemon
tn.write(b'/mnt/sdcard/fracture/bin/fracture-core >> /mnt/sdcard/fracture/logs/fracture.log 2>&1 &\n')
time.sleep(1)

# Check running processes
tn.write(b'ps | grep fracture\n')
time.sleep(1)
out = tn.read_very_eager().decode('ascii', errors='ignore')
print(out)
tn.close()
"

kill $HTTP_PID
echo "[✓] FractureOS deployed and active!"
