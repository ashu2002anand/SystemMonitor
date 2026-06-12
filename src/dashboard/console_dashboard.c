#include "dashboard/console_dashboard.h"

#include <stdio.h>

void console_dashboard_render(const SystemStats* stats)
{
    /* ANSI escape sequence: clear the terminal and move the cursor to the top-left. */
    printf("\033[2J\033[H");

    printf("System Monitor\n");
    printf("==============\n\n");
    printf("CPU: %.2f%% | Logical processors: %lu | Physical cores: %lu\n",
           stats->cpuUsagePercent,
           (unsigned long)stats->logicalProcessorCount,
           (unsigned long)stats->physicalCoreCount);
    printf("RAM: %llu MB / %llu MB (%.2f%%)\n",
           (unsigned long long)stats->usedRamMB,
           (unsigned long long)stats->totalRamMB,
           stats->memoryUsagePercent);
    printf("Disk: %llu GB / %llu GB (%.2f%%)\n",
           (unsigned long long)stats->usedDiskGB,
           (unsigned long long)stats->totalDiskGB,
           stats->diskUsagePercent);
    printf("Disk I/O: read %.2f MB/s | write %.2f MB/s\n", stats->diskReadMBps, stats->diskWriteMBps);
    printf("Network: down %.2f MB/s | up %.2f MB/s\n",
           stats->networkDownloadMBps,
           stats->networkUploadMBps);
    printf("Network totals: received %llu bytes | transmitted %llu bytes\n",
           (unsigned long long)stats->totalBytesReceived,
           (unsigned long long)stats->totalBytesTransmitted);
    printf("Processes: %lu running\n", (unsigned long)stats->runningProcessCount);
    printf("Top CPU process: %s\n", stats->highestCpuProcess);
    printf("Top memory process: %s\n", stats->highestMemoryProcess);
}
