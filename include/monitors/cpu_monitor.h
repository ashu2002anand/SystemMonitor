#ifndef SYSTEM_MONITOR_CPU_MONITOR_H
#define SYSTEM_MONITOR_CPU_MONITOR_H

#include "core/statistics_store.h"

/* Collect CPU usage and processor counts, then update the shared statistics store. */
void cpu_monitor_collect(StatisticsStore* statisticsStore);

#endif
