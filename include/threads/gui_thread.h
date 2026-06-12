#ifndef SYSTEM_MONITOR_GUI_THREAD_H
#define SYSTEM_MONITOR_GUI_THREAD_H

/* pthread entry point reserved for future GUI integration. Expects an AppContext*. */
void* gui_thread_run(void* parameter);

#endif
