#include "monitors/disk_io_monitor.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct DiskIoSample
{
    unsigned long long sectorsRead;
    unsigned long long sectorsWritten;
    struct timespec timestamp;
} DiskIoSample;

static int should_skip_device(const char* deviceName)
{
    return strncmp(deviceName, "loop", 4) == 0 || strncmp(deviceName, "ram", 3) == 0;
}

static int read_disk_io_sample(DiskIoSample* sample)
{
    FILE* file = fopen("/proc/diskstats", "r");
    char line[512];
    unsigned long long sectorsRead = 0;
    unsigned long long sectorsWritten = 0;

    if (file == NULL)
    {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        char deviceName[64];
        unsigned long long deviceSectorsRead = 0;
        unsigned long long deviceSectorsWritten = 0;

        if (sscanf(line,
                   " %*u %*u %63s %*llu %*llu %llu %*llu %*llu %*llu %llu",
                   deviceName,
                   &deviceSectorsRead,
                   &deviceSectorsWritten) == 3 &&
            !should_skip_device(deviceName))
        {
            sectorsRead += deviceSectorsRead;
            sectorsWritten += deviceSectorsWritten;
        }
    }

    fclose(file);

    sample->sectorsRead = sectorsRead;
    sample->sectorsWritten = sectorsWritten;
    clock_gettime(CLOCK_MONOTONIC, &sample->timestamp);
    return 1;
}

void disk_io_monitor_collect(StatisticsStore* statisticsStore)
{
    static int hasPreviousSample = 0;
    static DiskIoSample previousSample;

    DiskIoSample currentSample;
    double elapsedSeconds = 0.0;
    double readMBps = 0.0;
    double writeMBps = 0.0;

    if (!read_disk_io_sample(&currentSample))
    {
        return;
    }

    if (hasPreviousSample)
    {
        elapsedSeconds = (double)(currentSample.timestamp.tv_sec - previousSample.timestamp.tv_sec) +
                         (double)(currentSample.timestamp.tv_nsec - previousSample.timestamp.tv_nsec) / 1000000000.0;

        if (elapsedSeconds > 0.0)
        {
            readMBps = ((double)(currentSample.sectorsRead - previousSample.sectorsRead) * 512.0) /
                       (1024.0 * 1024.0 * elapsedSeconds);
            writeMBps = ((double)(currentSample.sectorsWritten - previousSample.sectorsWritten) * 512.0) /
                        (1024.0 * 1024.0 * elapsedSeconds);
        }
    }

    previousSample = currentSample;
    hasPreviousSample = 1;

    statistics_store_update_disk_io(statisticsStore, readMBps, writeMBps);
}
