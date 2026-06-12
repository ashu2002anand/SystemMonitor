#include "core/statistics_store.h"
#include "core/app_context.h"
#include "threads/collector_thread.h"
#include "threads/gui_thread.h"
#include "threads/terminal_thread.h"

#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>

static atomic_bool gRunning = ATOMIC_VAR_INIT(1);

static void handle_signal(int signalNumber)
{
    (void)signalNumber;

    /* Signal handlers should do minimal work; worker threads observe this flag and exit. */
    atomic_store(&gRunning, 0);
}

int main(void)
{
    StatisticsStore statisticsStore;
    AppContext context;
    pthread_t collectorThread;
    pthread_t terminalThread;
    pthread_t guiThread;
    int collectorStarted = 0;
    int terminalStarted = 0;
    int guiStarted = 0;

    statistics_store_init(&statisticsStore);
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    /* All worker threads share the same store and shutdown flag. */
    context.statisticsStore = &statisticsStore;
    context.running = &gRunning;

    /* Start each worker independently so startup failures can shut the app down cleanly. */
    if (pthread_create(&collectorThread, NULL, collector_thread_run, &context) == 0)
    {
        collectorStarted = 1;
    }
    else
    {
        fprintf(stderr, "Failed to start collector thread\n");
        atomic_store(&gRunning, 0);
    }

    if (pthread_create(&terminalThread, NULL, terminal_thread_run, &context) == 0)
    {
        terminalStarted = 1;
    }
    else
    {
        fprintf(stderr, "Failed to start terminal thread\n");
        atomic_store(&gRunning, 0);
    }

    if (pthread_create(&guiThread, NULL, gui_thread_run, &context) == 0)
    {
        guiStarted = 1;
    }
    else
    {
        fprintf(stderr, "Failed to start GUI placeholder thread\n");
        atomic_store(&gRunning, 0);
    }

    if (collectorStarted)
    {
        pthread_join(collectorThread, NULL);
    }

    if (terminalStarted)
    {
        pthread_join(terminalThread, NULL);
    }

    if (guiStarted)
    {
        pthread_join(guiThread, NULL);
    }

    /* Destroy the mutex after all threads have stopped using the statistics store. */
    statistics_store_destroy(&statisticsStore);
    return 0;
}
