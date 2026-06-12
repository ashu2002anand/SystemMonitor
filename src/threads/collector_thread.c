#include "threads/collector_thread.h"

#include "core/app_context.h"
#include "monitors/cpu_monitor.h"
#include "monitors/disk_io_monitor.h"
#include "monitors/disk_monitor.h"
#include "monitors/memory_monitor.h"
#include "monitors/network_monitor.h"
#include "monitors/process_monitor.h"

#include <stdatomic.h>
#include <unistd.h>

void* collector_thread_run(void* parameter)
{
    AppContext* context = (AppContext*)parameter;
    unsigned int refreshCount = 0;

    while (atomic_load(context->running))
    {
        cpu_monitor_collect(context->statisticsStore);
        memory_monitor_collect(context->statisticsStore);
        disk_io_monitor_collect(context->statisticsStore);
        network_monitor_collect(context->statisticsStore);

        if (refreshCount % 5 == 0)
        {
            disk_monitor_collect(context->statisticsStore);
            process_monitor_collect(context->statisticsStore);
        }

        ++refreshCount;
        sleep(1);
    }

    return NULL;
}
