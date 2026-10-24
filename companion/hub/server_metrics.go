package main

import (
	"bufio"
	"os"
	"strconv"
	"strings"
	"syscall"
)

type ServerMetrics struct {
	CPUUsage  uint8
	RAMUsage  uint8
	DiskUsage uint8
	TempC     uint8
}

var lastIdle, lastTotal uint64

func CollectServerMetrics() ServerMetrics {
	m := ServerMetrics{
		CPUUsage:  readCPU(),
		RAMUsage:  readRAM(),
		DiskUsage: readDisk(),
		TempC:     readTemp(),
	}
	return m
}

func readCPU() uint8 {
	f, err := os.Open("/proc/stat")
	if err != nil {
		return 10
	}
	defer f.Close()

	scanner := bufio.NewScanner(f)
	if scanner.Scan() {
		fields := strings.Fields(scanner.Text())
		if len(fields) >= 5 && fields[0] == "cpu" {
			var total, idle uint64
			for i := 1; i < len(fields); i++ {
				val, _ := strconv.ParseUint(fields[i], 10, 64)
				total += val
				if i == 4 { // idle
					idle = val
				}
			}
			if lastTotal > 0 && total > lastTotal {
				totalDiff := total - lastTotal
				idleDiff := idle - lastIdle
				usage := float64(totalDiff-idleDiff) / float64(totalDiff) * 100.0
				lastTotal = total
				lastIdle = idle
				if usage < 0 {
					usage = 0
				}
				if usage > 100 {
					usage = 100
				}
				return uint8(usage)
			}
			lastTotal = total
			lastIdle = idle
		}
	}
	return 12
}

func readRAM() uint8 {
	f, err := os.Open("/proc/meminfo")
	if err != nil {
		return 35
	}
	defer f.Close()

	var total, available uint64
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		line := scanner.Text()
		if strings.HasPrefix(line, "MemTotal:") {
			fields := strings.Fields(line)
			if len(fields) >= 2 {
				total, _ = strconv.ParseUint(fields[1], 10, 64)
			}
		} else if strings.HasPrefix(line, "MemAvailable:") {
			fields := strings.Fields(line)
			if len(fields) >= 2 {
				available, _ = strconv.ParseUint(fields[1], 10, 64)
			}
		}
		if total > 0 && available > 0 {
			break
		}
	}
	if total > 0 && available <= total {
		used := total - available
		return uint8((float64(used) / float64(total)) * 100.0)
	}
	return 30
}

func readDisk() uint8 {
	var stat syscall.Statfs_t
	err := syscall.Statfs("/", &stat)
	if err != nil {
		return 25
	}
	total := stat.Blocks * uint64(stat.Bsize)
	free := stat.Bfree * uint64(stat.Bsize)
	if total > 0 && free <= total {
		used := total - free
		return uint8((float64(used) / float64(total)) * 100.0)
	}
	return 25
}

func readTemp() uint8 {
	files := []string{
		"/sys/class/thermal/thermal_zone0/temp",
		"/sys/class/hwmon/hwmon0/temp1_input",
	}
	for _, path := range files {
		data, err := os.ReadFile(path)
		if err == nil {
			val, err := strconv.Atoi(strings.TrimSpace(string(data)))
			if err == nil && val > 0 {
				if val > 1000 {
					val /= 1000 // millidegrees to degrees
				}
				return uint8(val)
			}
		}
	}
	return 42
}
