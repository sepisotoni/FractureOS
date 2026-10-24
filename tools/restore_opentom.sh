#!/bin/sh
# 1-Click safe restore: stops FractureOS and brings back original OpenTom watchface

killall -9 fracture-core 2>/dev/null || true
rm -f /mnt/sdcard/fracture/fracture.pid

# Resume original OpenTom display and touch
killall -CONT nano-X 2>/dev/null || true
killall -CONT watchface.new 2>/dev/null || true
pidof watchface.new >/dev/null 2>&1 || ( /mnt/sdcard/opentom/bin/watchface-main & )

echo "Restored OpenTom cleanly. FractureOS stopped."
