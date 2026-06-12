#include "monitors/disk_monitor.h"

#include <stdint.h>
#include <sys/statvfs.h>

void disk_monitor_collect(StatisticsStore* statisticsStore)
{
    struct statvfs fileSystem;
    uint64_t totalBytes = 0;
    uint64_t availableBytes = 0;
    const uint64_t bytesPerGB = 1024ULL * 1024ULL * 1024ULL;

    if (statvfs("/", &fileSystem) != 0)
    {
        return;
    }

    totalBytes = (uint64_t)fileSystem.f_blocks * (uint64_t)fileSystem.f_frsize;
    availableBytes = (uint64_t)fileSystem.f_bavail * (uint64_t)fileSystem.f_frsize;

    statistics_store_update_disk(statisticsStore, totalBytes / bytesPerGB, availableBytes / bytesPerGB);
}
