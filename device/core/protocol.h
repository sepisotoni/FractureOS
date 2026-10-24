#ifndef FRACTURE_PROTOCOL_H
#define FRACTURE_PROTOCOL_H

#include <stdint.h>

#define FRACTURE_MAGIC        0x43415246 /* 'FRAC' in little-endian */
#define FRACTURE_VERSION      2
#define FRACTURE_DEFAULT_PORT 45872

/* Protocol Flags */
#define FRACTURE_FLAG_PC_ACTIVE     (1 << 0) /* User active on PC (not idle) */
#define FRACTURE_FLAG_MEDIA_PLAYING (1 << 1) /* Spotify/media actively playing */
#define FRACTURE_FLAG_NIGHT_MODE    (1 << 2) /* Deep night / stealth theme */
#define FRACTURE_FLAG_NOTIF_ACTIVE  (1 << 3) /* Toast notification pending */
#define FRACTURE_FLAG_AUDIO_MUTED   (1 << 4) /* Tap sounds muted */
#define FRACTURE_FLAG_ALERT_THERMAL (1 << 5) /* Thermal warning alert */

/* Complication Slot Choices */
enum ComplicationSlot {
    SLOT_NONE        = 0,
    SLOT_WEATHER     = 1,
    SLOT_BATTERY     = 2,
    SLOT_CPU_LOAD    = 3,
    SLOT_RAM_LOAD    = 4,
    SLOT_GPU_LOAD    = 5,
    SLOT_NET_SPEED   = 6,
    SLOT_DATE        = 7,
    SLOT_MEDIA_TITLE = 8,
    SLOT_SPARKLINE   = 9
};

/* Clock Style */
enum ClockStyle {
    CLOCK_STYLE_DUO     = 0, /* Modern bold numerals */
    CLOCK_STYLE_SEGMENT = 1, /* Crisp 7-segment display */
    CLOCK_STYLE_STACKED = 2, /* Apple-style stacked Hour/Min */
    CLOCK_STYLE_ANALOG  = 3  /* Minimalist circular dial */
};

/* View IDs */
enum FractureViewID {
    VIEW_CLOCK     = 0,
    VIEW_LAUNCHER  = 1,
    VIEW_TELEMETRY = 2,
    VIEW_MEDIA     = 3,
    VIEW_TIMER     = 4,
    VIEW_SERVER    = 5,
    VIEW_SETTINGS  = 6,
    VIEW_WEATHER   = 7,
    VIEW_COUNT     = 8
};

typedef struct {
    uint32_t magic;          /* FRACTURE_MAGIC */
    uint16_t version;        /* FRACTURE_VERSION */
    uint16_t flags;          /* Bitwise flags */
    uint32_t sequence;       /* Packet sequence counter */
    
    /* Active View & Complication Config */
    uint8_t  active_view;    /* Desired or active view ID */
    uint8_t  clock_style;    /* Clock style enum */
    uint8_t  slot_tl;        /* Top-Left slot */
    uint8_t  slot_tr;        /* Top-Right slot */
    uint8_t  slot_bottom;    /* Bottom bar slot */
    uint8_t  brightness;     /* 0..100 brightness (255 = no change) */

    /* PC Telemetry (from Windows companion) */
    uint8_t  cpu_usage;      /* 0..100 % */
    uint8_t  cpu_temp;       /* °C */
    uint8_t  gpu_usage;      /* 0..100 % */
    uint8_t  gpu_temp;       /* °C */
    uint8_t  ram_usage;      /* 0..100 % */
    uint8_t  disk_usage;     /* 0..100 % */
    uint16_t net_up_kb;      /* Upload KB/s */
    uint16_t net_down_kb;    /* Download KB/s */

    /* Linux Server Telemetry (from stoni-room-serve) */
    uint8_t  srv_cpu_usage;  /* 0..100 % */
    uint8_t  srv_ram_usage;  /* 0..100 % */
    uint8_t  srv_disk_usage; /* 0..100 % */
    uint8_t  srv_temp;       /* °C */

    /* Media Playback (Spotify / WinRT GSMTC) */
    uint8_t  media_state;    /* 0: Stopped, 1: Playing, 2: Paused */
    uint16_t media_pos;      /* Playback position (seconds) */
    uint16_t media_dur;      /* Total duration (seconds) */
    char     media_title[40];
    char     media_artist[28];

    /* Weather */
    int16_t  weather_temp;   /* Tenths of °C (e.g. 215 = 21.5°C) */
    int16_t  weather_high;   /* Tenths of °C */
    int16_t  weather_low;    /* Tenths of °C */
    uint8_t  weather_code;   /* WMO weather code */
    uint8_t  weather_humid;  /* Relative humidity % */

    /* Notification Overlay */
    char     notif_title[24];
    char     notif_body[48];
    uint8_t  notif_timeout;  /* Seconds to display (0 = none) */

    /* Padding to exactly 256 bytes for predictable alignment */
    uint8_t  reserved[38];
} __attribute__((packed)) FractureTelemetryPacket;

/* Upstream Response Packet (TomTom -> Host/PC) */
typedef struct {
    uint32_t magic;          /* FRACTURE_MAGIC */
    uint16_t version;
    uint8_t  active_view;    /* Currently visible view */
    uint8_t  action_event;   /* 1: Play/Pause, 2: Next, 3: Prev, 4: Mute PC, 5: Sleep */
    uint8_t  battery_pct;    /* TomTom battery % */
    uint8_t  battery_charging;
    uint32_t uptime_sec;
    uint8_t  reserved[16];
} __attribute__((packed)) FractureDeviceResponse;

#endif /* FRACTURE_PROTOCOL_H */
