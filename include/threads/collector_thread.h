#ifndef SYSTEM_MONITOR_COLLECTOR_THREAD_H
#define SYSTEM_MONITOR_COLLECTOR_THREAD_H

/*
 * collector_thread_run() - Main loop for periodic system metric collection.
 *
 * Pthread entry point that runs in a dedicated thread. Calls monitor functions at different intervals
 * (1 second for fast metrics, 5 seconds for expensive scans) and updates the shared statistics store.
 *
 * @parameter: Pointer to an AppContext structure containing the statistics store and running flag.
 *
 * Return value: Always returns NULL (standard pthread return).
 *
 * Thread behavior:
 * - Fast metrics (CPU, memory, disk I/O, network) are collected every 1 second.
 * - Slow metrics (disk capacity, process list) are collected every 5 seconds using a counter.
 * - Thread continues running while context->running is true (atomic flag).
 * - Graceful shutdown occurs when the flag is set to false by signal handler.
 * - Each monitor call internally handles missing /proc files without terminating the thread.
 *
 * Thread safety: Receives an AppContext containing the shared statistics store and atomic running flag.
 * All metric updates are thread-safe via the statistics store's internal mutex.
 */
void* collector_thread_run(void* parameter);

#endif
