#include "threads/gui_thread.h"

#include "core/app_context.h"

#include <stdatomic.h>
#include <unistd.h>

void* gui_thread_run(void* parameter)
{
    AppContext* context = (AppContext*)parameter;

    while (atomic_load(context->running))
    {
        /* Reserved for future GUI integration. */
        sleep(1);
    }

    return NULL;
}
