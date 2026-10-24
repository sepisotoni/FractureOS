package shared

import (
	"bytes"
	"encoding/binary"
	"fmt"
)

const (
	Magic       uint32 = 0x43415246 // 'FRAC' in little-endian
	Version     uint16 = 2
	DefaultPort int    = 45872

	FlagPCActive     uint16 = 1 << 0
	FlagMediaPlaying uint16 = 1 << 1
	FlagNightMode    uint16 = 1 << 2
	FlagNotifActive  uint16 = 1 << 3
	FlagAudioMuted   uint16 = 1 << 4
	FlagAlertThermal uint16 = 1 << 5

	ViewClock     uint8 = 0
	ViewLauncher  uint8 = 1
	ViewTelemetry uint8 = 2
	ViewMedia     uint8 = 3
	ViewTimer     uint8 = 4
	ViewServer    uint8 = 5
	ViewSettings  uint8 = 6
	ViewWeather   uint8 = 7

	ActionPlayPause uint8 = 1
	ActionNext      uint8 = 2
	ActionPrev      uint8 = 3
	ActionMute      uint8 = 4
	ActionSleep     uint8 = 5
)

// TelemetryPacket matches device/core/protocol.h exactly (256 bytes packed)
type TelemetryPacket struct {
	Magic       uint32
	Version     uint16
	Flags       uint16
	Sequence    uint32
	ActiveView  uint8
	ClockStyle  uint8
	SlotTL      uint8
	SlotTR      uint8
	SlotBottom  uint8
	Brightness  uint8
	CPUUsage    uint8
	CPUTemp     uint8
	GPUUsage    uint8
	GPUTemp     uint8
	RAMUsage    uint8
	DiskUsage   uint8
	NetUpKB     uint16
	NetDownKB   uint16
	SrvCPUUsage uint8
	SrvRAMUsage uint8
	SrvDiskUsage uint8
	SrvTemp     uint8
	MediaState  uint8
	MediaPos    uint16
	MediaDur    uint16
	MediaTitle  [40]byte
	MediaArtist [28]byte
	WeatherTemp int16
	WeatherHigh int16
	WeatherLow  int16
	WeatherCode uint8
	WeatherHum  uint8
	NotifTitle  [24]byte
	NotifBody   [48]byte
	NotifTimeout uint8
	Reserved    [38]byte
}

func (p *TelemetryPacket) SetMedia(title, artist string) {
	copy(p.MediaTitle[:], []byte(title))
	copy(p.MediaArtist[:], []byte(artist))
}

func (p *TelemetryPacket) SetNotification(title, body string, timeoutSec uint8) {
	copy(p.NotifTitle[:], []byte(title))
	copy(p.NotifBody[:], []byte(body))
	p.NotifTimeout = timeoutSec
	if timeoutSec > 0 {
		p.Flags |= FlagNotifActive
	}
}

func (p *TelemetryPacket) MarshalBinary() ([]byte, error) {
	p.Magic = Magic
	p.Version = Version
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, p)
	if err != nil {
		return nil, err
	}
	data := buf.Bytes()
	if len(data) != 256 {
		return nil, fmt.Errorf("packet size mismatch: got %d, want 256", len(data))
	}
	return data, nil
}

func UnmarshalTelemetryPacket(data []byte) (*TelemetryPacket, error) {
	if len(data) < 256 {
		return nil, fmt.Errorf("data too short: got %d, want 256", len(data))
	}
	p := &TelemetryPacket{}
	r := bytes.NewReader(data[:256])
	err := binary.Read(r, binary.LittleEndian, p)
	if err != nil {
		return nil, err
	}
	if p.Magic != Magic {
		return nil, fmt.Errorf("invalid magic: 0x%X", p.Magic)
	}
	return p, nil
}

type DeviceResponse struct {
	Magic           uint32
	Version         uint16
	ActiveView      uint8
	ActionEvent     uint8
	BatteryPct      uint8
	BatteryCharging uint8
	UptimeSec       uint32
	Reserved        [16]byte
}

func UnmarshalDeviceResponse(data []byte) (*DeviceResponse, error) {
	if len(data) < 26 {
		return nil, fmt.Errorf("response data too short: %d", len(data))
	}
	resp := &DeviceResponse{}
	r := bytes.NewReader(data)
	err := binary.Read(r, binary.LittleEndian, resp)
	if err != nil {
		return nil, err
	}
	return resp, nil
}
