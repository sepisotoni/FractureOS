package main

import (
	"fmt"
	"net"
	"os"
	"os/signal"
	"strconv"
	"syscall"
	"time"

	"fractureos/shared"
)

func main() {
	fmt.Println("Starting FractureOS Linux Hub on stoni-room-serve...")

	targetIP := "192.168.101.115"
	targetPort := shared.DefaultPort

	raddr, err := net.ResolveUDPAddr("udp", fmt.Sprintf("%s:%d", targetIP, targetPort))
	if err != nil {
		fmt.Printf("Error resolving target: %v\n", err)
		return
	}

	conn, err := net.DialUDP("udp", nil, raddr)
	if err != nil {
		fmt.Printf("Error dialing UDP: %v\n", err)
		return
	}
	defer conn.Close()

	packet := &shared.TelemetryPacket{
		Flags:      shared.FlagPCActive,
		ActiveView: shared.ViewClock,
		Brightness: 80,
	}
	packet.SetMedia("Waiting for Spotify", "Windows Workstation")

	// Start Web Portal
	startWebDashboard(8080, func(cmd, arg string) {
		switch cmd {
		case "view":
			v, _ := strconv.Atoi(arg)
			packet.ActiveView = uint8(v)
		case "brightness":
			b, _ := strconv.Atoi(arg)
			packet.Brightness = uint8(b)
		case "night_mode":
			packet.Flags ^= shared.FlagNightMode
		case "pc_active":
			packet.Flags ^= shared.FlagPCActive
		case "notify":
			if arg != "" {
				packet.SetNotification("PC Alert", arg, 5)
			}
		}
	})

	// Ticker for periodic broadcast (every 1 second)
	ticker := time.NewTicker(1 * time.Second)
	defer ticker.Stop()

	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	fmt.Printf("FractureOS Hub transmitting telemetry to %s:%d...\n", targetIP, targetPort)

	for {
		select {
		case <-sigChan:
			fmt.Println("Hub shutting down...")
			return
		case <-ticker.C:
			// 1. Collect Linux Server metrics
			srv := CollectServerMetrics()
			packet.SrvCPUUsage = srv.CPUUsage
			packet.SrvRAMUsage = srv.RAMUsage
			packet.SrvDiskUsage = srv.DiskUsage
			packet.SrvTemp = srv.TempC

			// 2. Fetch local weather
			weather := FetchWeather(-23.83, 30.16)
			packet.WeatherTemp = weather.TempTenths
			packet.WeatherHigh = weather.HighTenths
			packet.WeatherLow = weather.LowTenths
			packet.WeatherHum = weather.Humidity
			packet.WeatherCode = weather.Code

			packet.Sequence++

			// 3. Serialize and transmit
			data, err := packet.MarshalBinary()
			if err == nil {
				conn.Write(data)
			}
		}
	}
}
