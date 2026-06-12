#include "monitors/process_monitor.h"

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_numeric_name(const char* name)
{
    /* Only numeric /proc entries represent process IDs. */
    while (*name != '\0')
    {
        if (!isdigit((unsigned char)*name))
        {
            return 0;
        }
        ++name;
    }

    return 1;
}

static unsigned long long read_process_rss_kb(const char* pid)
{
    char path[256];
    char line[512];
    FILE* file = NULL;

    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    file = fopen(path, "r");

    if (file == NULL)
    {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        unsigned long long value = 0;

        /* VmRSS is resident memory, reported in kB by /proc/[pid]/status. */
        if (sscanf(line, "VmRSS: %llu kB", &value) == 1)
        {
            fclose(file);
            return value;
        }
    }

    fclose(file);
    return 0;
}

static unsigned long long read_process_cpu_ticks(const char* pid, char* processName, size_t processNameSize)
{
    char path[256];
    char line[4096];
    char* openParen = NULL;
    char* closeParen = NULL;
    char* fields = NULL;
    unsigned long long utime = 0;
    unsigned long long stime = 0;
    FILE* file = NULL;

    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    file = fopen(path, "r");

    if (file == NULL)
    {
        return 0;
    }

    if (fgets(line, sizeof(line), file) == NULL)
    {
        fclose(file);
        return 0;
    }

    fclose(file);

    /*
     * /proc/[pid]/stat wraps the command name in parentheses. The name may contain
     * spaces, so split around the final ')' before parsing numeric fields.
     */
    openParen = strchr(line, '(');
    closeParen = strrchr(line, ')');
    if (openParen == NULL || closeParen == NULL || closeParen <= openParen)
    {
        return 0;
    }

    *closeParen = '\0';
    snprintf(processName, processNameSize, "%s", openParen + 1);

    fields = closeParen + 2;

    /* utime and stime are accumulated user/system CPU ticks for this process. */
    if (sscanf(fields,
               "%*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %llu %llu",
               &utime,
               &stime) != 2)
    {
        return 0;
    }

    return utime + stime;
}

void process_monitor_collect(StatisticsStore* statisticsStore)
{
    DIR* proc = opendir("/proc");
    struct dirent* entry = NULL;
    uint32_t runningProcessCount = 0;
    unsigned long long highestCpuTicks = 0;
    unsigned long long highestMemoryKB = 0;
    char highestCpuProcess[SYSTEM_MONITOR_PROCESS_NAME_LENGTH] = "N/A";
    char highestMemoryProcess[SYSTEM_MONITOR_PROCESS_NAME_LENGTH] = "N/A";

    if (proc == NULL)
    {
        return;
    }

    while ((entry = readdir(proc)) != NULL)
    {
        char processName[SYSTEM_MONITOR_PROCESS_NAME_LENGTH] = "N/A";
        unsigned long long cpuTicks = 0;
        unsigned long long rssKB = 0;

        if (!is_numeric_name(entry->d_name))
        {
            continue;
        }

        ++runningProcessCount;

        /* Highest CPU process is based on accumulated CPU time, not instant CPU %. */
        cpuTicks = read_process_cpu_ticks(entry->d_name, processName, sizeof(processName));
        if (cpuTicks > highestCpuTicks)
        {
            highestCpuTicks = cpuTicks;
            snprintf(highestCpuProcess, sizeof(highestCpuProcess), "%s", processName);
        }

        /* Highest memory process is based on resident set size. */
        rssKB = read_process_rss_kb(entry->d_name);
        if (rssKB > highestMemoryKB)
        {
            highestMemoryKB = rssKB;
            snprintf(highestMemoryProcess, sizeof(highestMemoryProcess), "%s", processName);
        }
    }

    closedir(proc);
    statistics_store_update_processes(statisticsStore,
                                      runningProcessCount,
                                      highestCpuProcess,
                                      highestMemoryProcess);
}
