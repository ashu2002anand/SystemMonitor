#ifndef SYSTEM_MONITOR_NETWORK_MONITOR_H
#define SYSTEM_MONITOR_NETWORK_MONITOR_H

#include "core/statistics_store.h"

/* Collect aggregate non-loopback network counters from /proc/net/dev. */
void network_monitor_collect(StatisticsStore* statisticsStore);

#endif
