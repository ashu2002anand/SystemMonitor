#ifndef SYSTEM_MONITOR_STATISTICS_STORE_H
#define SYSTEM_MONITOR_STATISTICS_STORE_H

#include "core/system_stats.h"

#include <pthread.h>
#include <stdint.h>

typedef struct StatisticsStore
{
    pthread_mutex_t lock;
    SystemStats stats;
} StatisticsStore;

void statistics_store_init(StatisticsStore* store);
void statistics_store_destroy(StatisticsStore* store);
void statistics_store_snapshot(StatisticsStore* store, SystemStats* outStats);

void statistics_store_update_cpu(StatisticsStore* store,
                                 double usagePercent,
                                 uint32_t logicalProcessors,
                                 uint32_t physicalCores);
void statistics_store_update_memory(StatisticsStore* store, uint64_t totalMB, uint64_t availableMB);
void statistics_store_update_disk(StatisticsStore* store, uint64_t totalGB, uint64_t availableGB);
void statistics_store_update_disk_io(StatisticsStore* store, double readMBps, double writeMBps);
void statistics_store_update_network(StatisticsStore* store,
                                     double downloadMBps,
                                     double uploadMBps,
                                     uint64_t totalBytesReceived,
                                     uint64_t totalBytesTransmitted);
void statistics_store_update_processes(StatisticsStore* store,
                                       uint32_t runningProcessCount,
                                       const char* highestCpuProcess,
                                       const char* highestMemoryProcess);

#endif
