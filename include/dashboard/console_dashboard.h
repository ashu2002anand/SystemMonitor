#ifndef SYSTEM_MONITOR_CONSOLE_DASHBOARD_H
#define SYSTEM_MONITOR_CONSOLE_DASHBOARD_H

#include "core/system_stats.h"

/*
 * console_dashboard_render() - Render a single terminal frame with system statistics.
 *
 * Displays a formatted view of CPU, memory, disk, network, and process statistics
 * to the terminal using ANSI escape sequences for clearing and formatting.
 *
 * @stats: Pointer to a SystemStats snapshot containing current metric values.
 *
 * Output format:
 * - Clears the terminal and positions cursor at top-left.
 * - Displays CPU usage percentage with logical processor and physical core counts.
 * - Shows memory usage (used/total MB and percentage).
 * - Shows disk usage (used/total GB and percentage).
 * - Displays disk I/O rates (read/write MB/s).
 * - Shows network rates (down/up MB/s) and total byte counters.
 * - Lists running process count and top CPU/memory consuming process names.
 *
 * Thread safety: This function only reads from the snapshot and writes to stdout.
 * It is safe to call from any thread as long as the stats pointer is not freed.
 *
 * Note: Uses ANSI escape codes \033[2J\033[H to clear screen. Compatible with Linux terminals.
 */
void console_dashboard_render(const SystemStats* stats);

#endif
