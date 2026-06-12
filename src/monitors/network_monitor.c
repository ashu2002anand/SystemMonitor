#include "monitors/network_monitor.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct NetworkSample
{
    /* Raw byte counters from /proc/net/dev plus a monotonic timestamp for rate math. */
    unsigned long long bytesReceived;
    unsigned long long bytesTransmitted;
    struct timespec timestamp;
} NetworkSample;

static int read_network_sample(NetworkSample* sample)
{
    FILE* file = fopen("/proc/net/dev", "r");
    char line[512];
    unsigned int lineNumber = 0;
    unsigned long long bytesReceived = 0;
    unsigned long long bytesTransmitted = 0;

    if (file == NULL)
    {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        char interfaceName[64];
        unsigned long long rxBytes = 0;
        unsigned long long txBytes = 0;

        if (++lineNumber <= 2)
        {
            /* The first two lines are headers, not interface rows. */
            continue;
        }

        /* Ignore loopback so local IPC does not inflate user-visible network rates. */
        if (sscanf(line,
                   " %63[^:]: %llu %*llu %*llu %*llu %*llu %*llu %*llu %*llu %llu",
                   interfaceName,
                   &rxBytes,
                   &txBytes) == 3 &&
            strcmp(interfaceName, "lo") != 0)
        {
            bytesReceived += rxBytes;
            bytesTransmitted += txBytes;
        }
    }

    fclose(file);

    sample->bytesReceived = bytesReceived;
    sample->bytesTransmitted = bytesTransmitted;
    clock_gettime(CLOCK_MONOTONIC, &sample->timestamp);
    return 1;
}

void network_monitor_collect(StatisticsStore* statisticsStore)
{
    static int hasPreviousSample = 0;
    static NetworkSample previousSample;

    NetworkSample currentSample;
    double elapsedSeconds = 0.0;
    double downloadMBps = 0.0;
    double uploadMBps = 0.0;

    if (!read_network_sample(&currentSample))
    {
        return;
    }

    if (hasPreviousSample)
    {
        elapsedSeconds = (double)(currentSample.timestamp.tv_sec - previousSample.timestamp.tv_sec) +
                         (double)(currentSample.timestamp.tv_nsec - previousSample.timestamp.tv_nsec) / 1000000000.0;

        if (elapsedSeconds > 0.0)
        {
            /* Convert byte deltas to MB/s over the elapsed sample window. */
            downloadMBps = ((double)(currentSample.bytesReceived - previousSample.bytesReceived)) /
                           (1024.0 * 1024.0 * elapsedSeconds);
            uploadMBps = ((double)(currentSample.bytesTransmitted - previousSample.bytesTransmitted)) /
                         (1024.0 * 1024.0 * elapsedSeconds);
        }
    }

    previousSample = currentSample;
    hasPreviousSample = 1;

    statistics_store_update_network(statisticsStore,
                                    downloadMBps,
                                    uploadMBps,
                                    currentSample.bytesReceived,
                                    currentSample.bytesTransmitted);
}
