#include "threads/terminal_thread.h"

#include "core/app_context.h"
#include "dashboard/console_dashboard.h"

#include <stdatomic.h>
#include <unistd.h>

void* terminal_thread_run(void* parameter)
{
    AppContext* context = (AppContext*)parameter;

    while (atomic_load(context->running))
    {
        SystemStats stats;
        statistics_store_snapshot(context->statisticsStore, &stats);
        console_dashboard_render(&stats);
        sleep(1);
    }

    return NULL;
}
