package main

import (
	"fmt"
	"net"
	"os"
	"strconv"
	"strings"

	"fractureos/shared"
)

func printHelp() {
	fmt.Println("FractureOS CLI (fracture-cli)")
	fmt.Println("Usage: fracture-cli <command> [arguments]")
	fmt.Println()
	fmt.Println("Commands:")
	fmt.Println("  view <clock|launcher|telemetry|media|timer|server|settings>  Switch active view")
	fmt.Println("  brightness <0-100>                                           Adjust screen brightness")
	fmt.Println("  notify <title> <body>                                        Push toast notification")
	fmt.Println("  action <play_pause|next|prev|mute|sleep>                     Send remote action")
	fmt.Println("  night <on|off>                                               Toggle night theme")
}

func main() {
	if len(os.Args) < 2 {
		printHelp()
		return
	}

	cmd := os.Args[1]
	targetIP := "192.168.101.115"
	targetPort := shared.DefaultPort

	raddr, err := net.ResolveUDPAddr("udp", fmt.Sprintf("%s:%d", targetIP, targetPort))
	if err != nil {
		fmt.Printf("Resolve error: %v\n", err)
		return
	}

	conn, err := net.DialUDP("udp", nil, raddr)
	if err != nil {
		fmt.Printf("Dial error: %v\n", err)
		return
	}
	defer conn.Close()

	packet := &shared.TelemetryPacket{
		Flags: shared.FlagPCActive,
	}

	switch cmd {
	case "view":
		if len(os.Args) < 3 {
			fmt.Println("Specify view: clock, launcher, telemetry, media, timer, server, settings")
			return
		}
		name := strings.ToLower(os.Args[2])
		views := map[string]uint8{
			"clock":     shared.ViewClock,
			"launcher":  shared.ViewLauncher,
			"telemetry": shared.ViewTelemetry,
			"media":     shared.ViewMedia,
			"timer":     shared.ViewTimer,
			"server":    shared.ViewServer,
			"settings":  shared.ViewSettings,
		}
		if v, ok := views[name]; ok {
			packet.ActiveView = v
			fmt.Printf("Switching view to %s (ID %d)\n", name, v)
		} else {
			fmt.Println("Unknown view name")
			return
		}

	case "brightness":
		if len(os.Args) < 3 {
			fmt.Println("Specify brightness: 0-100")
			return
		}
		val, _ := strconv.Atoi(os.Args[2])
		packet.Brightness = uint8(val)
		fmt.Printf("Setting brightness to %d%%\n", val)

	case "notify":
		if len(os.Args) < 4 {
			fmt.Println("Usage: fracture-cli notify <title> <body>")
			return
		}
		title := os.Args[2]
		body := os.Args[3]
		packet.SetNotification(title, body, 6)
		fmt.Printf("Sending notification: [%s] %s\n", title, body)

	case "action":
		if len(os.Args) < 3 {
			fmt.Println("Specify action: play_pause, next, prev, mute, sleep")
			return
		}
		act := os.Args[2]
		switch act {
		case "play_pause":
			packet.Flags |= shared.FlagMediaPlaying
		case "sleep":
			packet.Brightness = 0
		}
		fmt.Printf("Action %s sent.\n", act)

	case "night":
		if len(os.Args) >= 3 && os.Args[2] == "on" {
			packet.Flags |= shared.FlagNightMode
		}
		fmt.Println("Night mode updated.")

	default:
		printHelp()
		return
	}

	data, err := packet.MarshalBinary()
	if err != nil {
		fmt.Printf("Marshal error: %v\n", err)
		return
	}

	_, err = conn.Write(data)
	if err != nil {
		fmt.Printf("Transmit error: %v\n", err)
		return
	}
	fmt.Println("Command delivered to FractureOS device successfully.")
}
