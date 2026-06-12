#ifndef SYSTEM_MONITOR_PROCESS_MONITOR_H
#define SYSTEM_MONITOR_PROCESS_MONITOR_H

#include "core/statistics_store.h"

/* Collect process count plus highest CPU-time and memory consumers from /proc. */
void process_monitor_collect(StatisticsStore* statisticsStore);

#endif
