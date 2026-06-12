CC ?= gcc

TARGET := build/system_monitor

CFLAGS := -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pthread -Iinclude
LDFLAGS := -pthread

SOURCES := \
	src/main.c \
	src/core/statistics_store.c \
	src/dashboard/console_dashboard.c \
	src/threads/collector_thread.c \
	src/threads/terminal_thread.c \
	src/threads/gui_thread.c \
	src/monitors/cpu_monitor.c \
	src/monitors/memory_monitor.c \
	src/monitors/disk_monitor.c \
	src/monitors/disk_io_monitor.c \
	src/monitors/network_monitor.c \
	src/monitors/process_monitor.c

OBJECTS := $(patsubst %.c,build/obj/%.o,$(SOURCES))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

build/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(TARGET)

clean:
	rm -rf build
