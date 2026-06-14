#ifndef SYSTEM_MONITOR_GUI_THREAD_H
#define SYSTEM_MONITOR_GUI_THREAD_H

/*
 * gui_thread_run() - Placeholder thread for future GUI integration.
 *
 * Pthread entry point reserved for future graphical user interface rendering.
 * Currently implements a no-op loop that respects the shutdown flag.
 *
 * @parameter: Pointer to an AppContext structure containing the statistics store and running flag.
 *
 * Return value: Always returns NULL (standard pthread return).
 *
 * Current behavior:
 * - This is a placeholder thread with minimal functionality.
 * - Thread loop continues while context->running is true (atomic flag).
 * - No actual GUI rendering is performed; the thread simply checks the running flag.
 *
 * Future implementation:
 * - This thread will handle GUI window lifecycle and event processing.
 * - It will read snapshots from the statistics store and render graphics.
 * - Separate from collector and terminal rendering threads for clean architecture.
 *
 * Thread safety: Will receive an AppContext containing the shared statistics store and atomic running flag.
 */
void* gui_thread_run(void* parameter);

#endif
