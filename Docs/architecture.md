# System Monitor Architecture

## 1. Architecture Overview

The application follows a modular Linux architecture with three long-running threads.

```text
+----------------------------------+
|          Collector Thread         |
| CPU / RAM / Disk / I/O / Network |
| Process collectors                |
+----------------+-----------------+
                 |
                 v
+----------------------------------+
|       Shared Statistics Store     |
+----------------+-----------------+
                 |
     +-----------+-----------+
     |                       |
     v                       v
+------------+        +--------------+
|  Terminal  |        | GUI Thread   |
| Dashboard  |        | Placeholder  |
+------------+        +--------------+
```

---

## 2. Major Components

### 2.1 Collector Thread

Responsibilities:

- Poll Linux system sources
- Update the shared statistics store
- Keep fast metrics on a 1 second interval
- Keep slower metrics on a 5 second interval

Linux sources:

```text
/proc/stat
/proc/cpuinfo
/proc/meminfo
/proc/diskstats
/proc/net/dev
/proc/[pid]/stat
/proc/[pid]/status
statvfs("/")
```

---

### 2.2 Terminal Dashboard Thread

Responsibilities:

- Read snapshots from the shared statistics store
- Refresh the terminal once per second
- Present user-friendly values

---

### 2.3 GUI Thread

Responsibilities:

- Reserved for future GUI integration
- Keep GUI lifecycle separate from collection and terminal rendering

Current behavior:

```text
No-op placeholder thread
```

---

### 2.4 Shared Statistics Store

Responsibilities:

- Centralized storage of collected metrics
- Thread-safe access with `pthread_mutex_t`
- Snapshot reads for dashboard rendering

Structure:

```c
typedef struct SystemStats
{
    double cpuUsagePercent;
    uint32_t logicalProcessorCount;
    uint32_t physicalCoreCount;

    uint64_t totalRamMB;
    uint64_t availableRamMB;
    uint64_t usedRamMB;
    double memoryUsagePercent;

    uint64_t totalDiskGB;
    uint64_t availableDiskGB;
    uint64_t usedDiskGB;
    double diskUsagePercent;

    double diskReadMBps;
    double diskWriteMBps;

    double networkDownloadMBps;
    double networkUploadMBps;
    uint64_t totalBytesReceived;
    uint64_t totalBytesTransmitted;

    uint32_t runningProcessCount;
    char highestCpuProcess[260];
    char highestMemoryProcess[260];
} SystemStats;
```

---

## 3. Thread Model

```text
Main Thread
    |
    +-- Collector Thread
    |
    +-- Terminal Dashboard Thread
    |
    +-- GUI Placeholder Thread
```

The main thread handles startup, signal handling, and joins worker threads during shutdown.

---

## 4. Synchronization

Synchronization Method:

```c
pthread_mutex_t
```

Shared Resource:

```c
SystemStats
```

Access Pattern:

```text
Collector Thread
      |
      v
Shared Statistics Store
      ^
      |
Terminal / GUI Threads
```

---

## 5. Future Architecture

Future releases may include:

- GUI dashboard implementation
- Historical data storage
- CSV export
- JSON export
- Plugin-based monitoring modules
- NVMe monitoring subsystem
- Alert manager
- Event logger
