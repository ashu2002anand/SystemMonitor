#ifndef SYSTEM_MONITOR_APP_CONTEXT_H
#define SYSTEM_MONITOR_APP_CONTEXT_H

#include "core/statistics_store.h"

#include <stdatomic.h>

typedef struct AppContext
{
    StatisticsStore* statisticsStore;
    atomic_bool* running;
} AppContext;

#endif
