# System Monitor Unit Tests

This directory contains unit tests for the System Monitor application using Google Test (GTest) framework.

## Test Files

### test_statistics_store.c
Unit tests for the thread-safe statistics store module.

**Tests included:**
- Store initialization and default values
- Snapshot functionality and consistency
- CPU metrics storage and retrieval
- Memory metrics with used calculation and percentage
- Disk metrics with capacity calculation
- Disk I/O throughput rates
- Network metrics (rates and totals)
- Process metrics (count and top processes)
- Defensive handling of anomalous values
- NULL/empty input handling

### test_cpu_monitor.c
Unit tests for the CPU monitor module.

**Tests included:**
- Collection with valid statistics store
- Successive collections produce valid CPU percentages
- Processor counts remain stable across calls
- Graceful handling of missing /proc files

## Building Tests

### Prerequisites
- GCC or Clang compiler
- CMake 3.14+ (for FetchContent)
- pthread library
- Internet connection (to download Google Test)

### Build Instructions

1. Create a build directory:
```bash
cd d:\study\projects\SystemMonitor
mkdir -p build
cd build
```

2. Configure CMake:
```bash
cmake ..
```

3. Build the test executables:
```bash
cmake --build . --target test_statistics_store test_cpu_monitor
```

Or build all targets including tests:
```bash
cmake --build .
```

## Running Tests

### Run all tests:
```bash
ctest -V
```

### Run specific test executable:
```bash
./test/test_statistics_store
./test/test_cpu_monitor
```

### Run tests with more verbose output:
```bash
ctest -V --output-on-failure
```

### Run specific test:
```bash
./test/test_statistics_store --gtest_filter=StatisticsStoreTest.InitializeCreatesValidStore
```

## Test Coverage

The test suite covers:

1. **Initialization**: Proper setup of data structures and mutexes
2. **Atomicity**: Thread-safe snapshots and updates
3. **Calculations**: Percentage and derived value calculations
4. **Edge Cases**: NULL inputs, anomalous values (available > total)
5. **Integration**: Updates flowing through to snapshots
6. **Graceful Degradation**: Handling of missing system files

## Adding New Tests

To add new tests:

1. Create a new test file (e.g., `test_memory_monitor.c`)
2. Add it to CMakeLists.txt:
```cmake
add_executable(test_memory_monitor
    test_memory_monitor.c
    ../src/core/statistics_store.c
    ../src/monitors/memory_monitor.c
)
# ... configure target ...
add_test(NAME MemoryMonitorTests COMMAND test_memory_monitor)
```
3. Rebuild with CMake
4. Tests are automatically discovered and run

## Continuous Integration

To integrate with CI/CD:

1. Run CMake configuration
2. Build test targets
3. Execute `ctest` to run all tests
4. Parse XML output with `--output-junit` for CI integration:
```bash
ctest --output-junit test-results.xml
```

## Test Architecture

- **Fixture-based**: Each test class has SetUp/TearDown for initialization
- **Isolated**: Tests do not depend on system state or /proc availability
- **Defensive**: Tests verify graceful handling of anomalies
- **Platform-aware**: Tests account for variations in /proc on different Linux systems

## Notes

- Tests that read /proc (cpu_monitor) run on real system data; results depend on current load
- Memory allocation tests do not mock malloc; they test actual behavior
- Mutex tests validate thread safety mechanisms
- Some tests (e.g., process count) may have system-dependent results

## Future Test Expansion

Planned test coverage:
- Memory monitor unit tests
- Disk monitor unit tests
- Network monitor unit tests
- Process monitor unit tests
- Terminal/dashboard render tests
- Thread lifecycle and synchronization tests
- Performance/load tests
- Integration tests combining multiple components
