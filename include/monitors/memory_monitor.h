#ifndef SYSTEM_MONITOR_MEMORY_MONITOR_H
#define SYSTEM_MONITOR_MEMORY_MONITOR_H

#include "core/statistics_store.h"

/* Collect memory totals from /proc/meminfo, then update the shared statistics store. */
void memory_monitor_collect(StatisticsStore* statisticsStore);

#endif
