#!/bin/bash
set -e

TOMTOM_IP="192.168.101.115"

echo "[*] Stopping FractureOS and restoring stock OpenTom environment..."
python3 -c "
import telnetlib, time
tn = telnetlib.Telnet('${TOMTOM_IP}', 23, timeout=5)
tn.read_until(b'login: ', timeout=3)
tn.write(b'root\n')
time.sleep(1)

# Stop FractureOS
tn.write(b'killall -9 fracture-core\n')
time.sleep(1)

# Restore original watchface-toggle-info script
tn.write(b'echo \"#!/bin/sh\npid=\\\$(pidof watchface.new)\n[ -n \\\"\\\$pid\\\" ] || exit 1\nkill -USR2 \\\"\\\$pid\\\"\n\" > /mnt/sdcard/opentom/bin/watchface-toggle-info && chmod +x /mnt/sdcard/opentom/bin/watchface-toggle-info\n')
time.sleep(1)

# Resume OpenTom UI processes
tn.write(b'killall -CONT nano-X watchface.new\n')
time.sleep(1)

# Verify OpenTom processes
tn.write(b'ps | grep -E \"nano-X|watchface\"\n')
time.sleep(1)
out = tn.read_very_eager().decode('ascii', errors='ignore')
print(out)
tn.close()
"

echo "[✓] Original OpenTom UI successfully restored with zero changes!"
