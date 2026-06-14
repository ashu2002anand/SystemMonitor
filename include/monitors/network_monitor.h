#ifndef SYSTEM_MONITOR_NETWORK_MONITOR_H
#define SYSTEM_MONITOR_NETWORK_MONITOR_H

#include "core/statistics_store.h"

/*
 * network_monitor_collect() - Collect aggregate non-loopback network statistics from /proc/net/dev.
 *
 * Reads byte counters from /proc/net/dev for all network interfaces (excluding loopback),
 * and calculates download/upload throughput in MB/s by comparing with the previous sample.
 *
 * @statisticsStore: Pointer to the shared statistics store where network metrics will be updated.
 *
 * Implementation details:
 * - Aggregates statistics from all non-loopback network interfaces.
 * - Loopback interface (lo) is excluded to focus on actual external network activity.
 * - Throughput is calculated from byte deltas using CLOCK_MONOTONIC for accurate timing.
 * - First call returns 0 MB/s (no previous sample); subsequent calls return accurate rates.
 * - Total bytes received/transmitted are raw counters from /proc/net/dev (not rates).
 * - Gracefully handles missing /proc file or parsing errors by returning without updates.
 *
 * Thread safety: The function performs a thread-safe update to statisticsStore via mutex protection.
 * Update frequency: Called once per second by the collector thread.
 */
void network_monitor_collect(StatisticsStore* statisticsStore);

#endif
