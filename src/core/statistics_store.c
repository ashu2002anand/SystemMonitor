#include "core/statistics_store.h"

#include <string.h>

static double percent_used(uint64_t used, uint64_t total)
{
    /* A zero total can happen when a collector fails or a system file is unavailable. */
    return total == 0 ? 0.0 : ((double)used * 100.0) / (double)total;
}

static void copy_process_name(char destination[SYSTEM_MONITOR_PROCESS_NAME_LENGTH], const char* source)
{
    /* Keep dashboard output stable even when a process name cannot be read. */
    if (source == NULL || source[0] == '\0')
    {
        source = "N/A";
    }

    strncpy(destination, source, SYSTEM_MONITOR_PROCESS_NAME_LENGTH - 1);
    destination[SYSTEM_MONITOR_PROCESS_NAME_LENGTH - 1] = '\0';
}

void statistics_store_init(StatisticsStore* store)
{
    pthread_mutex_init(&store->lock, NULL);
    memset(&store->stats, 0, sizeof(store->stats));

    /* Process names are displayed as strings, so initialize them to useful defaults. */
    copy_process_name(store->stats.highestCpuProcess, "N/A");
    copy_process_name(store->stats.highestMemoryProcess, "N/A");
}

void statistics_store_destroy(StatisticsStore* store)
{
    pthread_mutex_destroy(&store->lock);
}

void statistics_store_snapshot(StatisticsStore* store, SystemStats* outStats)
{
    /* Copy the whole struct while locked so readers never see a partially updated snapshot. */
    pthread_mutex_lock(&store->lock);
    *outStats = store->stats;
    pthread_mutex_unlock(&store->lock);
}

void statistics_store_update_cpu(StatisticsStore* store,
                                 double usagePercent,
                                 uint32_t logicalProcessors,
                                 uint32_t physicalCores)
{
    pthread_mutex_lock(&store->lock);
    store->stats.cpuUsagePercent = usagePercent;
    store->stats.logicalProcessorCount = logicalProcessors;
    store->stats.physicalCoreCount = physicalCores;
    pthread_mutex_unlock(&store->lock);
}

void statistics_store_update_memory(StatisticsStore* store, uint64_t totalMB, uint64_t availableMB)
{
    pthread_mutex_lock(&store->lock);
    store->stats.totalRamMB = totalMB;
    store->stats.availableRamMB = availableMB;

    /* Clamp to zero to protect against bad or partial collector input. */
    store->stats.usedRamMB = totalMB > availableMB ? totalMB - availableMB : 0;
    store->stats.memoryUsagePercent = percent_used(store->stats.usedRamMB, totalMB);
    pthread_mutex_unlock(&store->lock);
}

void statistics_store_update_disk(StatisticsStore* store, uint64_t totalGB, uint64_t availableGB)
{
    pthread_mutex_lock(&store->lock);
    store->stats.totalDiskGB = totalGB;
    store->stats.availableDiskGB = availableGB;

    /* statvfs values should be sane, but keep this defensive for tests and odd filesystems. */
    store->stats.usedDiskGB = totalGB > availableGB ? totalGB - availableGB : 0;
    store->stats.diskUsagePercent = percent_used(store->stats.usedDiskGB, totalGB);
    pthread_mutex_unlock(&store->lock);
}

void statistics_store_update_disk_io(StatisticsStore* store, double readMBps, double writeMBps)
{
    pthread_mutex_lock(&store->lock);
    store->stats.diskReadMBps = readMBps;
    store->stats.diskWriteMBps = writeMBps;
    pthread_mutex_unlock(&store->lock);
}

void statistics_store_update_network(StatisticsStore* store,
                                     double downloadMBps,
                                     double uploadMBps,
                                     uint64_t totalBytesReceived,
                                     uint64_t totalBytesTransmitted)
{
    pthread_mutex_lock(&store->lock);
    store->stats.networkDownloadMBps = downloadMBps;
    store->stats.networkUploadMBps = uploadMBps;
    store->stats.totalBytesReceived = totalBytesReceived;
    store->stats.totalBytesTransmitted = totalBytesTransmitted;
    pthread_mutex_unlock(&store->lock);
}

void statistics_store_update_processes(StatisticsStore* store,
                                       uint32_t runningProcessCount,
                                       const char* highestCpuProcess,
                                       const char* highestMemoryProcess)
{
    pthread_mutex_lock(&store->lock);
    store->stats.runningProcessCount = runningProcessCount;
    copy_process_name(store->stats.highestCpuProcess, highestCpuProcess);
    copy_process_name(store->stats.highestMemoryProcess, highestMemoryProcess);
    pthread_mutex_unlock(&store->lock);
}
