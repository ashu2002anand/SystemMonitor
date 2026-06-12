CC ?= gcc

# Final Linux executable location.
TARGET := build/system_monitor

# _POSIX_C_SOURCE exposes POSIX APIs such as clock_gettime with strict C11.
# -pthread is needed during both compilation and linking for pthread support.
CFLAGS := -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -pthread -Iinclude
LDFLAGS := -pthread

# Application sources grouped by layer: entry point, core, dashboard, threads, collectors.
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

# Default target: compile the system monitor binary.
all: $(TARGET)

# Link all compiled objects into the final executable.
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile each C source into a matching object path under build/obj.
build/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Build and launch the terminal dashboard.
run: $(TARGET)
	$(TARGET)

# Remove all generated build artifacts.
clean:
	rm -rf build
