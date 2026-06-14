#include "core/statistics_store.h"

#include <string.h>

/*
 * percent_used() - Calculate percentage of used resources.
 *
 * Safely computes percentage with protection against division by zero.
 *
 * @used: Amount of resource consumed.
 * @total: Total capacity of resource.
 *
 * Return: Percentage (0.0 to 100.0), or 0.0 if total is zero.
 *
 * Implementation notes:
 * - Returns 0.0 when total is zero to handle system files being unavailable.
 * - Formula: (used / total) * 100.0.
 * - Result is always in range [0.0, 100.0] for valid inputs where used <= total.
 */
static double percent_used(uint64_t used, uint64_t total)
{
    /* A zero total can happen when a collector fails or a system file is unavailable. */
    return total == 0 ? 0.0 : ((double)used * 100.0) / (double)total;
}

/*
 * copy_process_name() - Safely copy a process name with null-termination and "N/A" fallback.
 *
 * Copies the source process name into destination buffer with protection against null/empty source.
 * Ensures null-termination and limits the copy to SYSTEM_MONITOR_PROCESS_NAME_LENGTH - 1 bytes.
 *
 * @destination: Buffer to copy into (at least SYSTEM_MONITOR_PROCESS_NAME_LENGTH bytes).
 * @source: Process name string to copy, or NULL if unavailable.
 *
 * Behavior:
 * - If source is NULL or empty string, destination is set to "N/A".
 * - Otherwise, strncpy is used to safely copy up to SYSTEM_MONITOR_PROCESS_NAME_LENGTH - 1 bytes.
 * - Destination is always null-terminated after copying.
 *
 * Implementation notes:
 * - Ensures dashboard output remains stable even if a process name cannot be read.
 * - Prevents uninitialixed or garbage values from being displayed.
 */
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

/*
 * statistics_store_init() - Initialize the statistics store and mutex.
 *
 * Prepares the store for multi-threaded use by initializing the pthread mutex
 * and zeroing all metric fields. Process names are set to "N/A" as sensible defaults.
 *
 * @store: Pointer to uninitialized StatisticsStore to initialize.
 *
 * Preconditions:
 * - Must be called exactly once per store before any worker threads access it.
 * - Must be called from the main thread (no concurrent calls).
 *
 * Implementation notes:
 * - pthread_mutex_init() with NULL attributes uses default (non-recursive) mutex.
 * - memset() zeros all numeric fields (CPU, memory, disk, network, process metrics).
 * - Process names are initialized to "N/A" to provide stable output until first collection.
 * - Performs no validation; the caller ensures store is valid and uninitialized.
 */
void statistics_store_init(StatisticsStore* store)
{
    pthread_mutex_init(&store->lock, NULL);
    memset(&store->stats, 0, sizeof(store->stats));

    /* Process names are displayed as strings, so initialize them to useful defaults. */
    copy_process_name(store->stats.highestCpuProcess, "N/A");
    copy_process_name(store->stats.highestMemoryProcess, "N/A");
}

/*
 * statistics_store_destroy() - Release resources owned by the store.
 *
 * Destroys the pthread mutex. Must be called after all worker threads have stopped using the store.
 *
 * @store: Pointer to a StatisticsStore initialized by statistics_store_init().
 *
 * Preconditions:
 * - All worker threads must have exited and called pthread_join().
 * - No other thread may call store functions after this.
 *
 * Implementation notes:
 * - pthread_mutex_destroy() requires that the mutex is not held and no thread is blocked on it.
 * - Safe to call even if init failed (pthread_mutex_destroy ignores uninitialized mutexes gracefully on most systems).
 */
void statistics_store_destroy(StatisticsStore* store)
{
    pthread_mutex_destroy(&store->lock);
}

/*
 * statistics_store_snapshot() - Atomically read all metrics into a local copy.
 *
 * Acquires the mutex, copies the entire SystemStats struct, and releases the mutex.
 * Ensures readers never observe a partially-updated snapshot (metrics consistent at one point in time).
 *
 * @store: Pointer to the shared StatisticsStore.
 * @outStats: Pointer to SystemStats to populate with current values.
 *
 * Thread safety:
 * - Reader threads (terminal, GUI) call this to safely read while collector thread writes.
 * - Mutex lock prevents the collector from modifying the store while the copy is in progress.
 * - The entire struct is copied (140+ bytes); very fast operation.
 *
 * Implementation notes:
 * - Simple struct-to-struct assignment: outStats = store->stats.
 * - No validation of outStats pointer; caller ensures it is valid and writable.
 * - Predictable latency; always acquires and releases the lock once per call.
 */
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
