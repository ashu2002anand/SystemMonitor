#ifndef SYSTEM_MONITOR_MEMORY_MONITOR_H
#define SYSTEM_MONITOR_MEMORY_MONITOR_H

#include "core/statistics_store.h"

/*
 * memory_monitor_collect() - Collect physical memory usage information from /proc/meminfo.
 *
 * Reads MemTotal and MemAvailable values from /proc/meminfo and updates the statistics store.
 * The used memory is calculated as the difference, and memory utilization percentage is derived.
 *
 * @statisticsStore: Pointer to the shared statistics store where memory metrics will be updated.
 *
 * Implementation details:
 * - Values are read from /proc/meminfo in kilobytes and converted to megabytes for storage.
 * - MemAvailable is the kernel's estimate of available memory without swapping (preferred over MemFree).
 * - Gracefully handles missing /proc file or parsing errors by returning without updates.
 * - Used memory is clamped to prevent underflow if available > total.
 *
 * Thread safety: The function performs a thread-safe update to statisticsStore via mutex protection.
 * Update frequency: Called once per second by the collector thread.
 */
void memory_monitor_collect(StatisticsStore* statisticsStore);

#endif
