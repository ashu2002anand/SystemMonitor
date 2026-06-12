#include "threads/gui_thread.h"

#include "core/app_context.h"

#include <stdatomic.h>
#include <unistd.h>

void* gui_thread_run(void* parameter)
{
    AppContext* context = (AppContext*)parameter;

    while (atomic_load(context->running))
    {
        /* Future GUI event loop will live here and read snapshots from the shared store. */
        sleep(1);
    }

    return NULL;
}
