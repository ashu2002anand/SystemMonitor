#ifndef SYSTEM_MONITOR_SYSTEM_STATS_H
#define SYSTEM_MONITOR_SYSTEM_STATS_H

#include <stdint.h>

/* Linux process names from /proc are short, but this leaves room for future full paths. */
#define SYSTEM_MONITOR_PROCESS_NAME_LENGTH 260

/*
 * Snapshot of all metrics displayed by the system monitor.
 *
 * Units:
 * - Percent values are stored as 0.0 to 100.0.
 * - Memory values are stored in MB.
 * - Disk capacity values are stored in GB.
 * - Disk and network rates are stored in MB/s.
 * - Network totals are raw byte counters from Linux.
 */
typedef struct SystemStats
{
    /* CPU metrics collected from /proc/stat and /proc/cpuinfo. */
    double cpuUsagePercent;
    uint32_t logicalProcessorCount;
    uint32_t physicalCoreCount;

    /* Memory metrics collected from /proc/meminfo. */
    uint64_t totalRamMB;
    uint64_t availableRamMB;
    uint64_t usedRamMB;
    double memoryUsagePercent;

    /* Root filesystem capacity collected with statvfs("/"). */
    uint64_t totalDiskGB;
    uint64_t availableDiskGB;
    uint64_t usedDiskGB;
    double diskUsagePercent;

    /* Aggregate block-device throughput calculated from /proc/diskstats deltas. */
    double diskReadMBps;
    double diskWriteMBps;

    /* Aggregate non-loopback network counters calculated from /proc/net/dev. */
    double networkDownloadMBps;
    double networkUploadMBps;
    uint64_t totalBytesReceived;
    uint64_t totalBytesTransmitted;

    /* Process summary collected from numeric /proc/[pid] entries. */
    uint32_t runningProcessCount;
    char highestCpuProcess[SYSTEM_MONITOR_PROCESS_NAME_LENGTH];
    char highestMemoryProcess[SYSTEM_MONITOR_PROCESS_NAME_LENGTH];
} SystemStats;

#endif
