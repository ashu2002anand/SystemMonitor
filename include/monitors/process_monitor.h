#ifndef SYSTEM_MONITOR_PROCESS_MONITOR_H
#define SYSTEM_MONITOR_PROCESS_MONITOR_H

#include "core/statistics_store.h"

/*
 * process_monitor_collect() - Collect process count and identify top CPU/memory consuming processes.
 *
 * Scans /proc directory for numeric entries (process IDs), counts total processes,
 * and identifies the process with the highest accumulated CPU time and highest resident memory.
 *
 * @statisticsStore: Pointer to the shared statistics store where process metrics will be updated.
 *
 * Implementation details:
 * - Counts all numeric /proc directory entries (each represents an active process).
 * - Highest CPU process is selected based on accumulated CPU ticks (user + system time) from /proc/[pid]/stat.
 * - Highest memory process is selected based on resident set size (VmRSS) from /proc/[pid]/status.
 * - Gracefully handles missing /proc entries or unreadable process files (displays "N/A" instead).
 * - This is an expensive operation due to scanning hundreds of /proc/[pid] files.
 * - Both CPU and memory selections use accumulated metrics, not instant rates.
 *
 * Thread safety: The function performs a thread-safe update to statisticsStore via mutex protection.
 * Update frequency: Called once every 5 seconds by the collector thread (less frequent due to cost).
 */
void process_monitor_collect(StatisticsStore* statisticsStore);

#endif
