#ifndef SYSTEM_MONITOR_SYSTEM_STATS_H
#define SYSTEM_MONITOR_SYSTEM_STATS_H

#include <stdint.h>

#define SYSTEM_MONITOR_PROCESS_NAME_LENGTH 260

typedef struct SystemStats
{
    double cpuUsagePercent;
    uint32_t logicalProcessorCount;
    uint32_t physicalCoreCount;

    uint64_t totalRamMB;
    uint64_t availableRamMB;
    uint64_t usedRamMB;
    double memoryUsagePercent;

    uint64_t totalDiskGB;
    uint64_t availableDiskGB;
    uint64_t usedDiskGB;
    double diskUsagePercent;

    double diskReadMBps;
    double diskWriteMBps;

    double networkDownloadMBps;
    double networkUploadMBps;
    uint64_t totalBytesReceived;
    uint64_t totalBytesTransmitted;

    uint32_t runningProcessCount;
    char highestCpuProcess[SYSTEM_MONITOR_PROCESS_NAME_LENGTH];
    char highestMemoryProcess[SYSTEM_MONITOR_PROCESS_NAME_LENGTH];
} SystemStats;

#endif
