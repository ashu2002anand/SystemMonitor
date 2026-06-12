#include "monitors/memory_monitor.h"

#include <stdio.h>
#include <string.h>

void memory_monitor_collect(StatisticsStore* statisticsStore)
{
    FILE* file = fopen("/proc/meminfo", "r");
    char label[64];
    unsigned long long value = 0;
    uint64_t totalKB = 0;
    uint64_t availableKB = 0;

    if (file == NULL)
    {
        return;
    }

    /* MemAvailable is the kernel's best estimate of memory available without swapping. */
    while (fscanf(file, "%63s %llu %*15s", label, &value) == 2)
    {
        if (strcmp(label, "MemTotal:") == 0)
        {
            totalKB = value;
        }
        else if (strcmp(label, "MemAvailable:") == 0)
        {
            availableKB = value;
        }

        if (totalKB != 0 && availableKB != 0)
        {
            break;
        }
    }

    fclose(file);

    /* /proc/meminfo reports kB; the dashboard stores and prints MB. */
    statistics_store_update_memory(statisticsStore, totalKB / 1024, availableKB / 1024);
}
