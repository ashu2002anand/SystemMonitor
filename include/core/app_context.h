#ifndef SYSTEM_MONITOR_APP_CONTEXT_H
#define SYSTEM_MONITOR_APP_CONTEXT_H

#include "core/statistics_store.h"

#include <stdatomic.h>

/*
 * Shared application context passed to worker threads.
 *
 * statisticsStore points to the central metric store.
 * running is an atomic shutdown flag controlled by main signal handling.
 */
typedef struct AppContext
{
    StatisticsStore* statisticsStore;
    atomic_bool* running;
} AppContext;

#endif
