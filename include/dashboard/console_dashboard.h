#ifndef SYSTEM_MONITOR_CONSOLE_DASHBOARD_H
#define SYSTEM_MONITOR_CONSOLE_DASHBOARD_H

#include "core/system_stats.h"

/* Render one terminal frame from a previously captured SystemStats snapshot. */
void console_dashboard_render(const SystemStats* stats);

#endif
