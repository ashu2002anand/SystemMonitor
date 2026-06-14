/*
 * test_statistics_store.c - Unit tests for the statistics store module.
 *
 * Tests the thread-safe storage and retrieval of system metrics.
 * Validates mutex behavior, data initialization, and safe snapshots.
 */

#include <gtest/gtest.h>
#include <pthread.h>
#include <string.h>
#include "core/statistics_store.h"
#include "core/system_stats.h"

/*
 * Test fixture for statistics store tests.
 * Provides setup and teardown for each test case.
 */
class StatisticsStoreTest : public ::testing::Test
{
protected:
    StatisticsStore store;

    /*
     * SetUp() - Called before each test to initialize the store.
     *
     * Initializes a fresh StatisticsStore instance for isolated test execution.
     */
    void SetUp() override
    {
        statistics_store_init(&store);
    }

    /*
     * TearDown() - Called after each test to clean up resources.
     *
     * Destroys the mutex and releases store resources.
     */
    void TearDown() override
    {
        statistics_store_destroy(&store);
    }
};

/*
 * Test: InitializeCreatesValidStore
 *
 * Verifies that statistics_store_init() properly initializes all fields.
 * Process names should default to "N/A" and all numeric metrics to 0.
 */
TEST_F(StatisticsStoreTest, InitializeCreatesValidStore)
{
    SystemStats stats;

    /* Take a snapshot immediately after initialization. */
    statistics_store_snapshot(&store, &stats);

    /* All numeric fields should be zero. */
    EXPECT_EQ(stats.cpuUsagePercent, 0.0);
    EXPECT_EQ(stats.logicalProcessorCount, 0u);
    EXPECT_EQ(stats.physicalCoreCount, 0u);
    EXPECT_EQ(stats.totalRamMB, 0u);
    EXPECT_EQ(stats.availableRamMB, 0u);
    EXPECT_EQ(stats.usedRamMB, 0u);
    EXPECT_EQ(stats.memoryUsagePercent, 0.0);
    EXPECT_EQ(stats.diskReadMBps, 0.0);
    EXPECT_EQ(stats.diskWriteMBps, 0.0);
    EXPECT_EQ(stats.networkDownloadMBps, 0.0);
    EXPECT_EQ(stats.networkUploadMBps, 0.0);
    EXPECT_EQ(stats.totalBytesReceived, 0u);
    EXPECT_EQ(stats.totalBytesTransmitted, 0u);
    EXPECT_EQ(stats.runningProcessCount, 0u);

    /* Process names should default to "N/A". */
    EXPECT_STREQ(stats.highestCpuProcess, "N/A");
    EXPECT_STREQ(stats.highestMemoryProcess, "N/A");
}

/*
 * Test: SnapshotReturnsConsistentData
 *
 * Verifies that snapshot() atomically copies the entire store without tearing.
 * Multiple snapshots of the same store should return consistent data.
 */
TEST_F(StatisticsStoreTest, SnapshotReturnsConsistentData)
{
    SystemStats snapshot1, snapshot2;

    /* Update CPU metrics. */
    statistics_store_update_cpu(&store, 45.5, 8, 4);

    /* Take two snapshots and verify they match. */
    statistics_store_snapshot(&store, &snapshot1);
    statistics_store_snapshot(&store, &snapshot2);

    EXPECT_EQ(snapshot1.cpuUsagePercent, snapshot2.cpuUsagePercent);
    EXPECT_EQ(snapshot1.logicalProcessorCount, snapshot2.logicalProcessorCount);
    EXPECT_EQ(snapshot1.physicalCoreCount, snapshot2.physicalCoreCount);

    /* Verify the values match what was set. */
    EXPECT_EQ(snapshot1.cpuUsagePercent, 45.5);
    EXPECT_EQ(snapshot1.logicalProcessorCount, 8u);
    EXPECT_EQ(snapshot1.physicalCoreCount, 4u);
}

/*
 * Test: UpdateCpuStoresCorrectValues
 *
 * Verifies that statistics_store_update_cpu() correctly stores CPU metrics.
 */
TEST_F(StatisticsStoreTest, UpdateCpuStoresCorrectValues)
{
    SystemStats stats;

    statistics_store_update_cpu(&store, 62.3, 16, 8);
    statistics_store_snapshot(&store, &stats);

    EXPECT_EQ(stats.cpuUsagePercent, 62.3);
    EXPECT_EQ(stats.logicalProcessorCount, 16u);
    EXPECT_EQ(stats.physicalCoreCount, 8u);
}

/*
 * Test: UpdateMemoryCalculatesUsedAndPercentage
 *
 * Verifies that statistics_store_update_memory() correctly:
 * - Stores total and available memory
 * - Calculates used memory (total - available)
 * - Calculates memory utilization percentage
 */
TEST_F(StatisticsStoreTest, UpdateMemoryCalculatesUsedAndPercentage)
{
    SystemStats stats;
    uint64_t totalMB = 16384;
    uint64_t availableMB = 12288;

    statistics_store_update_memory(&store, totalMB, availableMB);
    statistics_store_snapshot(&store, &stats);

    EXPECT_EQ(stats.totalRamMB, totalMB);
    EXPECT_EQ(stats.availableRamMB, availableMB);
    EXPECT_EQ(stats.usedRamMB, totalMB - availableMB); /* 4096 MB */

    /* Verify percentage: (4096 / 16384) * 100 = 25% */
    EXPECT_NEAR(stats.memoryUsagePercent, 25.0, 0.01);
}

/*
 * Test: UpdateDiskCalculatesUsedAndPercentage
 *
 * Verifies that statistics_store_update_disk() correctly:
 * - Stores total and available disk space
 * - Calculates used disk space
 * - Calculates disk utilization percentage
 */
TEST_F(StatisticsStoreTest, UpdateDiskCalculatesUsedAndPercentage)
{
    SystemStats stats;
    uint64_t totalGB = 500;
    uint64_t availableGB = 250;

    statistics_store_update_disk(&store, totalGB, availableGB);
    statistics_store_snapshot(&store, &stats);

    EXPECT_EQ(stats.totalDiskGB, totalGB);
    EXPECT_EQ(stats.availableDiskGB, availableGB);
    EXPECT_EQ(stats.usedDiskGB, totalGB - availableGB); /* 250 GB */

    /* Verify percentage: (250 / 500) * 100 = 50% */
    EXPECT_NEAR(stats.diskUsagePercent, 50.0, 0.01);
}

/*
 * Test: UpdateDiskIoStoresRates
 *
 * Verifies that statistics_store_update_disk_io() correctly stores throughput rates.
 */
TEST_F(StatisticsStoreTest, UpdateDiskIoStoresRates)
{
    SystemStats stats;
    double readMBps = 123.45;
    double writeMBps = 98.76;

    statistics_store_update_disk_io(&store, readMBps, writeMBps);
    statistics_store_snapshot(&store, &stats);

    EXPECT_NEAR(stats.diskReadMBps, readMBps, 0.01);
    EXPECT_NEAR(stats.diskWriteMBps, writeMBps, 0.01);
}

/*
 * Test: UpdateNetworkStoresRatesAndTotals
 *
 * Verifies that statistics_store_update_network() correctly stores:
 * - Download/upload rates (MB/s)
 * - Total bytes received/transmitted (cumulative counters)
 */
TEST_F(StatisticsStoreTest, UpdateNetworkStoresRatesAndTotals)
{
    SystemStats stats;
    double downloadMBps = 10.5;
    double uploadMBps = 5.3;
    uint64_t bytesReceived = 1000000000ULL;
    uint64_t bytesTransmitted = 500000000ULL;

    statistics_store_update_network(&store, downloadMBps, uploadMBps,
                                    bytesReceived, bytesTransmitted);
    statistics_store_snapshot(&store, &stats);

    EXPECT_NEAR(stats.networkDownloadMBps, downloadMBps, 0.01);
    EXPECT_NEAR(stats.networkUploadMBps, uploadMBps, 0.01);
    EXPECT_EQ(stats.totalBytesReceived, bytesReceived);
    EXPECT_EQ(stats.totalBytesTransmitted, bytesTransmitted);
}

/*
 * Test: UpdateProcessesStoresCountAndTopProcesses
 *
 * Verifies that statistics_store_update_processes() correctly stores:
 * - Total process count
 * - Highest CPU-consuming process name
 * - Highest memory-consuming process name
 */
TEST_F(StatisticsStoreTest, UpdateProcessesStoresCountAndTopProcesses)
{
    SystemStats stats;
    uint32_t processCount = 256;
    const char* topCpuProcess = "firefox";
    const char* topMemoryProcess = "chromium";

    statistics_store_update_processes(&store, processCount,
                                      topCpuProcess, topMemoryProcess);
    statistics_store_snapshot(&store, &stats);

    EXPECT_EQ(stats.runningProcessCount, processCount);
    EXPECT_STREQ(stats.highestCpuProcess, topCpuProcess);
    EXPECT_STREQ(stats.highestMemoryProcess, topMemoryProcess);
}

/*
 * Test: UpdateProcessesHandlesNullProcessNames
 *
 * Verifies that statistics_store_update_processes() gracefully handles
 * NULL process names by converting them to "N/A".
 */
TEST_F(StatisticsStoreTest, UpdateProcessesHandlesNullProcessNames)
{
    SystemStats stats;

    /* Pass NULL for process names. */
    statistics_store_update_processes(&store, 100, NULL, NULL);
    statistics_store_snapshot(&store, &stats);

    EXPECT_STREQ(stats.highestCpuProcess, "N/A");
    EXPECT_STREQ(stats.highestMemoryProcess, "N/A");
}

/*
 * Test: MultipleUpdatesOverwritePreviousValues
 *
 * Verifies that successive updates correctly overwrite previous metrics.
 */
TEST_F(StatisticsStoreTest, MultipleUpdatesOverwritePreviousValues)
{
    SystemStats stats;

    /* First update. */
    statistics_store_update_cpu(&store, 25.0, 4, 2);
    statistics_store_snapshot(&store, &stats);
    EXPECT_EQ(stats.cpuUsagePercent, 25.0);

    /* Second update should overwrite. */
    statistics_store_update_cpu(&store, 75.0, 8, 4);
    statistics_store_snapshot(&store, &stats);
    EXPECT_EQ(stats.cpuUsagePercent, 75.0);
    EXPECT_EQ(stats.logicalProcessorCount, 8u);
    EXPECT_EQ(stats.physicalCoreCount, 4u);
}

/*
 * Test: MemoryClampedWhenAvailableGreaterThanTotal
 *
 * Verifies defensive programming: if available > total (system anomaly),
 * used memory is clamped to 0 instead of underflowing.
 */
TEST_F(StatisticsStoreTest, MemoryClampedWhenAvailableGreaterThanTotal)
{
    SystemStats stats;

    /* Anomalous case: available > total (should be prevented but we're defensive). */
    statistics_store_update_memory(&store, 1000, 1500);
    statistics_store_snapshot(&store, &stats);

    EXPECT_EQ(stats.totalRamMB, 1000u);
    EXPECT_EQ(stats.availableRamMB, 1500u);
    EXPECT_EQ(stats.usedRamMB, 0u); /* Clamped to zero, not negative. */
    EXPECT_EQ(stats.memoryUsagePercent, 0.0);
}

/*
 * Test: DiskClampedWhenAvailableGreaterThanTotal
 *
 * Verifies defensive programming: if available > total (filesystem anomaly),
 * used disk is clamped to 0 instead of underflowing.
 */
TEST_F(StatisticsStoreTest, DiskClampedWhenAvailableGreaterThanTotal)
{
    SystemStats stats;

    /* Anomalous case: available > total. */
    statistics_store_update_disk(&store, 100, 150);
    statistics_store_snapshot(&store, &stats);

    EXPECT_EQ(stats.totalDiskGB, 100u);
    EXPECT_EQ(stats.availableDiskGB, 150u);
    EXPECT_EQ(stats.usedDiskGB, 0u); /* Clamped to zero. */
    EXPECT_EQ(stats.diskUsagePercent, 0.0);
}

/* Run all tests when the test program is executed. */
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
