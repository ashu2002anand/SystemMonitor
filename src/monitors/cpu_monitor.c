#include "monitors/cpu_monitor.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct CpuSample
{
    unsigned long long idle;
    unsigned long long total;
} CpuSample;

typedef struct CoreIdentity
{
    int physicalId;
    int coreId;
} CoreIdentity;

static int read_cpu_sample(CpuSample* sample)
{
    FILE* file = fopen("/proc/stat", "r");
    unsigned long long user = 0;
    unsigned long long nice = 0;
    unsigned long long system = 0;
    unsigned long long idle = 0;
    unsigned long long iowait = 0;
    unsigned long long irq = 0;
    unsigned long long softirq = 0;
    unsigned long long steal = 0;

    if (file == NULL)
    {
        return 0;
    }

    if (fscanf(file,
               "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &user,
               &nice,
               &system,
               &idle,
               &iowait,
               &irq,
               &softirq,
               &steal) != 8)
    {
        fclose(file);
        return 0;
    }

    fclose(file);

    sample->idle = idle + iowait;
    sample->total = user + nice + system + idle + iowait + irq + softirq + steal;
    return 1;
}

static int parse_cpuinfo_value(const char* line, const char* key, int* value)
{
    const char* separator = NULL;

    if (strncmp(line, key, strlen(key)) != 0)
    {
        return 0;
    }

    separator = strchr(line, ':');
    if (separator == NULL)
    {
        return 0;
    }

    return sscanf(separator + 1, " %d", value) == 1;
}

static void add_unique_core(CoreIdentity* cores, unsigned int* coreCount, int physicalId, int coreId)
{
    for (unsigned int i = 0; i < *coreCount; ++i)
    {
        if (cores[i].physicalId == physicalId && cores[i].coreId == coreId)
        {
            return;
        }
    }

    if (*coreCount < 1024)
    {
        cores[*coreCount].physicalId = physicalId;
        cores[*coreCount].coreId = coreId;
        ++(*coreCount);
    }
}

static unsigned int count_logical_processors_from_cpuinfo(void)
{
    FILE* file = fopen("/proc/cpuinfo", "r");
    char line[256];
    unsigned int processors = 0;

    if (file == NULL)
    {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (strncmp(line, "processor", 9) == 0)
        {
            ++processors;
        }
    }

    fclose(file);
    return processors;
}

static unsigned int count_physical_cores(void)
{
    FILE* file = fopen("/proc/cpuinfo", "r");
    char line[256];
    CoreIdentity cores[1024];
    unsigned int coreCount = 0;
    int physicalId = -1;
    int coreId = -1;
    int fallbackCpuCores = 0;

    if (file == NULL)
    {
        return count_logical_processors_from_cpuinfo();
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (parse_cpuinfo_value(line, "physical id", &physicalId))
        {
            continue;
        }

        if (parse_cpuinfo_value(line, "core id", &coreId))
        {
            continue;
        }

        if (fallbackCpuCores == 0)
        {
            parse_cpuinfo_value(line, "cpu cores", &fallbackCpuCores);
        }

        if (line[0] == '\n')
        {
            if (physicalId >= 0 && coreId >= 0)
            {
                add_unique_core(cores, &coreCount, physicalId, coreId);
            }

            physicalId = -1;
            coreId = -1;
        }
    }

    if (physicalId >= 0 && coreId >= 0)
    {
        add_unique_core(cores, &coreCount, physicalId, coreId);
    }

    fclose(file);

    if (coreCount > 0)
    {
        return coreCount;
    }

    if (fallbackCpuCores > 0)
    {
        return (unsigned int)fallbackCpuCores;
    }

    return count_logical_processors_from_cpuinfo();
}

void cpu_monitor_collect(StatisticsStore* statisticsStore)
{
    static int hasPreviousSample = 0;
    static CpuSample previousSample = {0, 0};

    CpuSample currentSample;
    double usagePercent = 0.0;
    long logicalProcessors = sysconf(_SC_NPROCESSORS_ONLN);
    unsigned int physicalCores = count_physical_cores();

    if (!read_cpu_sample(&currentSample))
    {
        return;
    }

    if (hasPreviousSample)
    {
        unsigned long long totalDelta = currentSample.total - previousSample.total;
        unsigned long long idleDelta = currentSample.idle - previousSample.idle;
        usagePercent = totalDelta == 0 ? 0.0 : ((double)(totalDelta - idleDelta) * 100.0) / (double)totalDelta;
    }

    previousSample = currentSample;
    hasPreviousSample = 1;

    statistics_store_update_cpu(statisticsStore,
                                usagePercent,
                                logicalProcessors > 0 ? (uint32_t)logicalProcessors : 0,
                                physicalCores);
}
