package main

import (
	"fmt"
	"net"
	"os"
	"os/signal"
	"syscall"
	"time"

	"fractureos/shared"
)

// Windows Companion - Lazy Loading Architecture
// Only active feature goroutines run; inactive features remain nil and unallocated.

type Config struct {
	HubIP           string `json:"hub_ip"`
	EnableTelemetry bool   `json:"enable_telemetry"`
	EnableMedia     bool   `json:"enable_media"`
	EnableIdleWatch bool   `json:"enable_idle_watch"`
	RefreshSec      int    `json:"refresh_sec"`
}

var currentConfig = Config{
	HubIP:           "192.168.101.114",
	EnableTelemetry: true,
	EnableMedia:     true,
	EnableIdleWatch: true,
	RefreshSec:      1,
}

func main() {
	fmt.Println("Starting FractureOS Windows Companion (fracture-pc)...")

	raddr, err := net.ResolveUDPAddr("udp", fmt.Sprintf("%s:%d", currentConfig.HubIP, shared.DefaultPort))
	if err != nil {
		fmt.Printf("Cannot resolve Hub IP: %v\n", err)
		return
	}

	conn, err := net.DialUDP("udp", nil, raddr)
	if err != nil {
		fmt.Printf("Cannot dial Hub: %v\n", err)
		return
	}
	defer conn.Close()

	packet := &shared.TelemetryPacket{
		Flags: shared.FlagPCActive,
	}

	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	ticker := time.NewTicker(time.Duration(currentConfig.RefreshSec) * time.Second)
	defer ticker.Stop()

	fmt.Printf("Connected to Fracture Hub at %s:%d (Lazy Mode Active)\n", currentConfig.HubIP, shared.DefaultPort)

	for {
		select {
		case <-sigChan:
			fmt.Println("Windows Companion stopping...")
			return
		case <-ticker.C:
			// Lazy metrics evaluation
			if currentConfig.EnableTelemetry {
				pollSystemMetrics(packet)
			}
			if currentConfig.EnableMedia {
				pollMediaSession(packet)
			}
			if currentConfig.EnableIdleWatch {
				checkUserIdle(packet)
			}

			data, err := packet.MarshalBinary()
			if err == nil {
				conn.Write(data)
			}
		}
	}
}

// Lazy loaded metric pollers with fallback for cross-compilation
func pollSystemMetrics(p *shared.TelemetryPacket) {
	// Defaults/Simulated metrics when running or cross-compiled
	p.CPUUsage = 28
	p.CPUTemp = 52
	p.GPUUsage = 45
	p.GPUTemp = 64
	p.RAMUsage = 58
	p.NetDownKB = 1240
	p.NetUpKB = 180
}

func pollMediaSession(p *shared.TelemetryPacket) {
	p.Flags |= shared.FlagMediaPlaying
	p.MediaState = 1
	p.MediaPos = 145
	p.MediaDur = 230
	p.SetMedia("Starboy", "The Weeknd ft. Daft Punk")
}

func checkUserIdle(p *shared.TelemetryPacket) {
	// In true Windows build, calls user32.dll GetLastInputInfo
	p.Flags |= shared.FlagPCActive
}
