CC = /home/sepisotoni/projects/LeddaZ-OpenTom/gcc-3.3.4_glibc-2.3.2/bin/arm-linux-gcc
STRIP = /home/sepisotoni/projects/LeddaZ-OpenTom/gcc-3.3.4_glibc-2.3.2/bin/arm-linux-strip

CFLAGS = -std=gnu99 -march=armv5te -mtune=arm9tdmi -O2 -Wall -W -I. -Icore -Iui
LDFLAGS = -lm

SRCS = \
	core/main.c \
	core/framebuffer.c \
	core/input.c \
	core/audio.c \
	core/backlight.c \
	core/network.c \
	ui/chrome.c \
	ui/view_manager.c \
	ui/views/view_clock.c \
	ui/views/view_launcher.c \
	ui/views/view_telemetry.c \
	ui/views/view_media.c \
	ui/views/view_timer.c \
	ui/views/view_server.c \
	ui/views/view_settings.c

OBJS = $(SRCS:.c=.o)
TARGET = bin/fracture-core

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p bin
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)
	$(STRIP) --strip-unneeded $(TARGET)
	@echo "Build successful: $(TARGET) ($$(ls -lh $(TARGET) | awk '{print $$5}'))"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
