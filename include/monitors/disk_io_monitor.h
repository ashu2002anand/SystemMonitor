#ifndef SYSTEM_MONITOR_DISK_IO_MONITOR_H
#define SYSTEM_MONITOR_DISK_IO_MONITOR_H

#include "core/statistics_store.h"

/*
 * disk_io_monitor_collect() - Collect aggregate disk I/O throughput from /proc/diskstats.
 *
 * Reads sector counters from /proc/diskstats, excludes loop and ram disks, and calculates
 * read/write throughput in MB/s by comparing with the previous sample.
 *
 * @statisticsStore: Pointer to the shared statistics store where disk I/O metrics will be updated.
 *
 * Implementation details:
 * - Aggregates statistics from all block devices (excluding loop and ram disks).
 * - Linux diskstats sectors are 512 bytes each; converted to MB/s in calculations.
 * - Throughput is calculated from sector deltas using CLOCK_MONOTONIC for accuracy.
 * - First call returns 0 MB/s (no previous sample); subsequent calls return accurate rates.
 * - Gracefully handles missing /proc file or parsing errors by returning without updates.
 * - Loop/ram disks are excluded to focus on actual physical storage I/O.
 *
 * Thread safety: The function performs a thread-safe update to statisticsStore via mutex protection.
 * Update frequency: Called once per second by the collector thread.
 */
void disk_io_monitor_collect(StatisticsStore* statisticsStore);

#endif
