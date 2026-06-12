#ifndef SYSTEM_MONITOR_STATISTICS_STORE_H
#define SYSTEM_MONITOR_STATISTICS_STORE_H

#include "core/system_stats.h"

#include <pthread.h>
#include <stdint.h>

/*
 * Thread-safe storage for the latest SystemStats snapshot.
 *
 * The collector thread writes metric groups through the update functions.
 * Dashboard/GUI threads read a consistent copy through statistics_store_snapshot().
 */
typedef struct StatisticsStore
{
    pthread_mutex_t lock;
    SystemStats stats;
} StatisticsStore;

/* Initialize the mutex and default metric values before sharing the store with threads. */
void statistics_store_init(StatisticsStore* store);

/* Release resources owned by the store after all worker threads have stopped. */
void statistics_store_destroy(StatisticsStore* store);

/* Copy the current metrics into outStats while holding the mutex. */
void statistics_store_snapshot(StatisticsStore* store, SystemStats* outStats);

/* Update CPU utilization and processor counts. */
void statistics_store_update_cpu(StatisticsStore* store,
                                 double usagePercent,
                                 uint32_t logicalProcessors,
                                 uint32_t physicalCores);

/* Update memory values and derive used memory plus utilization percentage. */
void statistics_store_update_memory(StatisticsStore* store, uint64_t totalMB, uint64_t availableMB);

/* Update disk capacity values and derive used disk plus utilization percentage. */
void statistics_store_update_disk(StatisticsStore* store, uint64_t totalGB, uint64_t availableGB);

/* Update aggregate disk read/write throughput in MB/s. */
void statistics_store_update_disk_io(StatisticsStore* store, double readMBps, double writeMBps);

/* Update aggregate network rates and total byte counters. */
void statistics_store_update_network(StatisticsStore* store,
                                     double downloadMBps,
                                     double uploadMBps,
                                     uint64_t totalBytesReceived,
                                     uint64_t totalBytesTransmitted);

/* Update process count and current top process names. */
void statistics_store_update_processes(StatisticsStore* store,
                                       uint32_t runningProcessCount,
                                       const char* highestCpuProcess,
                                       const char* highestMemoryProcess);

#endif
