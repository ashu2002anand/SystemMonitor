# Google Test Framework Setup for System Monitor Tests
# This file configures the unit testing infrastructure using Google Test (GTest)

# Enable testing
enable_testing()

# Fetch Google Test from repository
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# Set Google Test build options
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add test executable for statistics store
add_executable(test_statistics_store
    test_statistics_store.c
    ../src/core/statistics_store.c
)

target_include_directories(test_statistics_store PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}/include
)

target_link_libraries(test_statistics_store
    gtest_main
    pthread
)

# Enable CMake testing
add_test(
    NAME StatisticsStoreTests
    COMMAND test_statistics_store
)

# Add test executable for CPU monitor (if statistics store is available)
add_executable(test_cpu_monitor
    test_cpu_monitor.c
    ../src/core/statistics_store.c
    ../src/monitors/cpu_monitor.c
)

target_include_directories(test_cpu_monitor PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}/include
)

target_link_libraries(test_cpu_monitor
    gtest_main
    pthread
)

add_test(
    NAME CpuMonitorTests
    COMMAND test_cpu_monitor
)

# Summary output
message(STATUS "Test executables configured:")
message(STATUS "  - test_statistics_store: Core statistics store functionality")
message(STATUS "  - test_cpu_monitor: CPU monitor metric collection")
message(STATUS "")
message(STATUS "To build tests: cmake --build . --target test_statistics_store test_cpu_monitor")
message(STATUS "To run tests: ctest -V")
