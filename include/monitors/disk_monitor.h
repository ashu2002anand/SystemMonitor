#ifndef SYSTEM_MONITOR_DISK_MONITOR_H
#define SYSTEM_MONITOR_DISK_MONITOR_H

#include "core/statistics_store.h"

/*
 * disk_monitor_collect() - Collect root filesystem capacity information using statvfs().
 *
 * Calls statvfs("/") to retrieve filesystem capacity statistics on the root filesystem.
 * Converts byte counts to gigabytes and updates the statistics store with total, available,
 * used, and utilization percentage values.
 *
 * @statisticsStore: Pointer to the shared statistics store where disk metrics will be updated.
 *
 * Implementation details:
 * - Monitors only the root filesystem (/) via statvfs().
 * - f_bavail is used for available space (respects reserved blocks for unprivileged users).
 * - Used disk space is calculated as total - available and clamped to prevent underflow.
 * - Gracefully handles statvfs() failures by returning without updates.
 * - This is an expensive operation compared to CPU/memory/network monitoring.
 *
 * Thread safety: The function performs a thread-safe update to statisticsStore via mutex protection.
 * Update frequency: Called once every 5 seconds by the collector thread.
 */
void disk_monitor_collect(StatisticsStore* statisticsStore);

#endif
