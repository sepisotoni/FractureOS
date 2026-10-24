#!/bin/sh
# Start FractureOS on TomTom without breaking stock OpenTom

# 1. Stop stock watchface and nano-X cleanly so they release /dev/input/event0 and /dev/fb
killall -STOP watchface.new 2>/dev/null || true
killall -STOP nano-X 2>/dev/null || true
killall -9 fracture-core 2>/dev/null || true

# 2. Launch FractureOS Core daemon
/mnt/sdcard/fracture/bin/fracture-core >/mnt/sdcard/fracture/fracture.log 2>&1 &
echo $! > /mnt/sdcard/fracture/fracture.pid

echo "FractureOS started (PID $(cat /mnt/sdcard/fracture/fracture.pid))"
