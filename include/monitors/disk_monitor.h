#ifndef SYSTEM_MONITOR_DISK_MONITOR_H
#define SYSTEM_MONITOR_DISK_MONITOR_H

#include "core/statistics_store.h"

/* Collect root filesystem capacity with statvfs("/"), then update the shared store. */
void disk_monitor_collect(StatisticsStore* statisticsStore);

#endif
