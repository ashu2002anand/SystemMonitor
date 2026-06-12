# System Monitor

## 1. Overview

System Monitor is a lightweight Linux application designed to collect and display real-time system resource statistics.

The application shall periodically gather system information and present it to the user through a console interface initially and a graphical interface in future versions.

---

## 2. Objectives

The primary objectives are:

- Monitor CPU utilization
- Monitor RAM utilization
- Monitor Disk utilization
- Monitor Disk I/O activity
- Monitor Network activity
- Monitor running processes
- Provide periodic updates with minimal system overhead

---

## 3. Functional Requirements

### FR-001 CPU Monitoring

The application shall display:

- CPU utilization percentage
- Number of logical processors
- Number of physical cores

Update interval:
- 1 second

---

### FR-002 Memory Monitoring

The application shall display:

- Total physical memory
- Available physical memory
- Used physical memory
- Memory utilization percentage

Update interval:
- 1 second

---

### FR-003 Disk Monitoring

The application shall display:

- Total disk capacity
- Available disk space
- Used disk space
- Disk utilization percentage

Update interval:
- 5 seconds

---

### FR-004 Disk I/O Monitoring

The application shall display:

- Read throughput (MB/s)
- Write throughput (MB/s)

Update interval:
- 1 second

---

### FR-005 Network Monitoring

The application shall display:

- Download rate
- Upload rate
- Total bytes transmitted
- Total bytes received

Update interval:
- 1 second

---

### FR-006 Process Monitoring

The application shall display:

- Number of running processes
- Highest CPU-consuming process
- Highest memory-consuming process

Update interval:
- 5 seconds

---

### FR-007 Dashboard

The application shall provide a dashboard showing all monitored information.

Initial version:
- Console dashboard

Future version:
- GUI dashboard

---

### FR-008 Auto Refresh

The dashboard shall automatically refresh without user interaction.

Refresh interval:
- 1 second

---

## 4. Non-Functional Requirements

### NFR-001 Performance

The monitoring application shall consume:

- Less than 5% CPU usage
- Less than 100 MB RAM

under normal operating conditions.

---

### NFR-002 Reliability

The application shall continue operating even if one monitoring component fails.

---

### NFR-003 Scalability

Additional monitoring modules shall be added without modifying existing modules.

---

### NFR-004 Maintainability

Each monitoring component shall be implemented as an independent module.

---

## 5. Future Features

### Phase 2

- GUI Dashboard
- Historical graphs
- CSV export
- JSON export

### Phase 3

- SSD SMART monitoring
- NVMe health monitoring
- Temperature monitoring
- Power cycle count
- Power-on hours

### Phase 4

- Alert framework
- Threshold monitoring
- Event logging

---

## 6. Supported Platform

Operating System:

- Linux

Compiler:

- GCC
- Clang

Language:

- C for the initial monitoring core
- C++ may be introduced later for the GUI layer if needed
