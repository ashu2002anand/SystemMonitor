#ifndef SYSTEM_MONITOR_TERMINAL_THREAD_H
#define SYSTEM_MONITOR_TERMINAL_THREAD_H

/*
 * terminal_thread_run() - Main loop for terminal dashboard rendering.
 *
 * Pthread entry point that runs in a dedicated thread. Periodically reads a snapshot from the shared
 * statistics store and renders it to the terminal via ANSI escape sequences.
 *
 * @parameter: Pointer to an AppContext structure containing the statistics store and running flag.
 *
 * Return value: Always returns NULL (standard pthread return).
 *
 * Thread behavior:
 * - Wakes every 1 second to refresh the terminal display.
 * - Reads a consistent snapshot from the statistics store (avoiding partial reads during collector updates).
 * - Clears the terminal and renders CPU, memory, disk, network, and process information.
 * - Thread continues running while context->running is true (atomic flag).
 * - Graceful shutdown occurs when the flag is set to false by signal handler.
 *
 * Thread safety: Uses snapshot() to safely copy the statistics store while the collector may be writing.
 * This prevents the terminal from rendering partially updated metrics.
 *
 * Update frequency: Terminal refreshes once per second.
 */
void* terminal_thread_run(void* parameter);

#endif
