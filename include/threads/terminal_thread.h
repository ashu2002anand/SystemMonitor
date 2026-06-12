#ifndef SYSTEM_MONITOR_TERMINAL_THREAD_H
#define SYSTEM_MONITOR_TERMINAL_THREAD_H

/* pthread entry point for terminal rendering. Expects an AppContext*. */
void* terminal_thread_run(void* parameter);

#endif
