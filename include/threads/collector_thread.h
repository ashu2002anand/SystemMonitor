#ifndef SYSTEM_MONITOR_COLLECTOR_THREAD_H
#define SYSTEM_MONITOR_COLLECTOR_THREAD_H

/* pthread entry point for metric collection. Expects an AppContext*. */
void* collector_thread_run(void* parameter);

#endif
