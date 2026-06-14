#include "monitors/cpu_monitor.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct CpuSample
{
    /* Idle and total jiffies from /proc/stat. Usage is calculated from sample deltas. */
    unsigned long long idle;
    unsigned long long total;
} CpuSample;

typedef struct CoreIdentity
{
    /* Unique physical-id/core-id pairs represent physical cores on most Linux systems. */
    int physicalId;
    int coreId;
} CoreIdentity;

/*
 * read_cpu_sample() - Read CPU jiffies from /proc/stat aggregate line.
 *
 * Parses the first "cpu" line from /proc/stat (aggregate time across all processors).
 * Extracts jiffies for user, nice, system, idle, iowait, irq, softirq, and steal times.
 * Computes total jiffies and idle+iowait (treated as idle for user-visible utilization).
 *
 * @sample: Pointer to CpuSample struct to populate with idle and total jiffies.
 *
 * Return: 1 on success, 0 on failure (missing file or parse error).
 *
 * Implementation notes:
 * - Iowait time is included in the idle calculation since users don't perceive waiting
 *   for I/O as "busy" CPU time (the CPU is not executing user/system code).
 * - Total is the sum of all eight time slices and represents jiffies since boot.
 * - The caller uses sample deltas to compute CPU usage percentage between samples.
 */
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

    /* The first "cpu" line is aggregate CPU time across all logical processors. */
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

    /* Treat iowait as idle time for user-visible CPU utilization. */
    sample->idle = idle + iowait;
    sample->total = user + nice + system + idle + iowait + irq + softirq + steal;
    return 1;
}

/*
 * parse_cpuinfo_value() - Extract an integer value from a /proc/cpuinfo key:value line.
 *
 * Parses lines like "physical id    : 0" to extract the integer value after the colon.
 * Used to extract physical_id, core_id, and cpu_cores fields from cpuinfo.
 *
 * @line: A single line from /proc/cpuinfo.
 * @key: The field name to match (e.g., "physical id", "core id").
 * @value: Pointer to int to populate with the parsed value.
 *
 * Return: 1 if the line contains the key and a valid integer was parsed, 0 otherwise.
 *
 * Implementation notes:
 * - String comparison uses strncmp() to match the key up to its length.
 * - Looks for a ':' separator and scans for an integer after it.
 * - Returns 0 if the key is not found or if parsing fails, allowing caller to continue.
 */
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

/*
 * add_unique_core() - Add a physical core (physical_id, core_id pair) if not already tracked.
 *
 * Maintains a list of unique physical cores to avoid counting hyperthreads twice.
 * On systems with physical_id and core_id fields, each unique pair represents one physical core.
 *
 * @cores: Array of CoreIdentity structures to store unique cores.
 * @coreCount: Pointer to current count of unique cores in the array.
 * @physicalId: Physical CPU ID from /proc/cpuinfo.
 * @coreId: Core ID within the physical CPU from /proc/cpuinfo.
 *
 * Side effects: Increments coreCount if the core is new and array space is available.
 *
 * Implementation notes:
 * - Checks if the (physicalId, coreId) pair already exists before adding.
 * - Array size is limited to 1024 cores (sufficient for virtually all systems).
 * - Duplicate pairs are silently ignored (returned without modifying the array).
 */
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

/*
 * count_logical_processors_from_cpuinfo() - Count "processor" entries in /proc/cpuinfo.
 *
 * Scans /proc/cpuinfo and counts lines starting with "processor" to determine
 * the number of logical processors (including hyperthreads).
 *
 * Return: Number of logical processors, or 0 if /proc/cpuinfo cannot be opened.
 *
 * Implementation notes:
 * - Each logical processor gets its own section in /proc/cpuinfo with a "processor" line.
 * - This count includes hyperthreads (logical cores that share physical resources).
 * - Used as a fallback if physical core detection fails or as input to other functions.
 */
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

/*
 * count_physical_cores() - Determine the number of physical CPU cores.
 *
 * Attempts to count unique (physical_id, core_id) pairs from /proc/cpuinfo.
 * Falls back to "cpu cores" field if pairs are unavailable, then logical processor count.
 *
 * Return: Number of physical cores (not hyperthreads), or 0 if all methods fail.
 *
 * Fallback strategy (priority order):
 *   1. Unique (physical_id, core_id) pairs from /proc/cpuinfo (most accurate on modern systems).
 *   2. "cpu cores" field per processor in /proc/cpuinfo (works on some systems).
 *   3. Logical processor count (assumes no hyperthreading).
 *
 * Implementation notes:
 * - Logical processors are grouped by blank lines in /proc/cpuinfo.
 * - Tracks the most recent physical_id and core_id before each blank line.
 * - Adds unique pairs to the cores array, excluding duplicates (from hyperthreads).
 * - Many systems do not report physical IDs; fallback ensures coverage.
 */
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

        /* A blank line ends one logical processor section in /proc/cpuinfo. */
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

    /* Prefer unique physical/core IDs, then cpu cores, then logical processor count. */
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

/*
 * cpu_monitor_collect() - Main entry point. Collect CPU metrics and update the shared store.
 *
 * Reads the current CPU sample, calculates usage percentage if a previous sample exists,
 * and updates the statistics store with CPU utilization and processor counts.
 *
 * @statisticsStore: Pointer to the shared statistics store to update.
 *
 * Implementation notes:
 * - Static variables preserve the previous sample between calls for delta calculation.
 * - First call sets hasPreviousSample flag; subsequent calls calculate actual percentage.
 * - Initial CPU usage is shown as 0% because we have no baseline to compare against.
 * - CPU usage % = (totalDelta - idleDelta) / totalDelta * 100.
 * - Processor counts are retrieved fresh each call (relatively cheap operations).
 * - Gracefully handles missing /proc files by returning early without updates.
 * - Thread-safe update via statistics_store_update_cpu() which holds the mutex.
 */
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

        /* First real percentage appears on the second sample because CPU is delta-based. */
        usagePercent = totalDelta == 0 ? 0.0 : ((double)(totalDelta - idleDelta) * 100.0) / (double)totalDelta;
    }

    previousSample = currentSample;
    hasPreviousSample = 1;

    statistics_store_update_cpu(statisticsStore,
                                usagePercent,
                                logicalProcessors > 0 ? (uint32_t)logicalProcessors : 0,
                                physicalCores);
}
