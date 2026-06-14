#ifndef SYSTEM_MONITOR_CPU_MONITOR_H
#define SYSTEM_MONITOR_CPU_MONITOR_H

#include "core/statistics_store.h"

/*
 * cpu_monitor_collect() - Collect CPU utilization and processor information from /proc/stat and /proc/cpuinfo.
 *
 * This function reads CPU statistics from Linux proc filesystem and calculates the overall CPU usage percentage
 * by analyzing the delta between consecutive samples. Processor counts (logical and physical) are also determined.
 *
 * @statisticsStore: Pointer to the shared statistics store where CPU metrics will be updated.
 *
 * Implementation details:
 * - CPU usage is calculated from /proc/stat jiffies deltas (idle vs total).
 * - Physical cores are determined from /proc/cpuinfo by analyzing physical-id/core-id pairs.
 * - First call returns 0% usage (no previous sample to compare); subsequent calls return accurate percentages.
 * - Gracefully handles missing /proc files by silently returning without updates.
 *
 * Thread safety: The function performs a thread-safe update to statisticsStore via mutex protection.
 * Update frequency: Called once per second by the collector thread.
 */
void cpu_monitor_collect(StatisticsStore* statisticsStore);

#endif
