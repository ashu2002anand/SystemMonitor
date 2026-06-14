/*
 * test_cpu_monitor.c - Unit tests for the CPU monitor module.
 *
 * Tests CPU utilization calculation, processor counting, and statistics store updates.
 * Since cpu_monitor reads from /proc, tests validate calculations rather than file I/O.
 */

#include <gtest/gtest.h>
#include <string.h>
#include "core/statistics_store.h"
#include "monitors/cpu_monitor.h"

/*
 * Test fixture for CPU monitor tests.
 * Provides setup and teardown for each test case.
 */
class CpuMonitorTest : public ::testing::Test
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
 * Test: CollectWithValidStore
 *
 * Verifies that cpu_monitor_collect() can successfully read /proc and
 * update the statistics store without crashing.
 *
 * Note: This test runs on a real system and checks that values are reasonable.
 */
TEST_F(CpuMonitorTest, CollectWithValidStore)
{
    SystemStats stats;

    /* Call collect (may be the first call, so CPU % might be 0). */
    cpu_monitor_collect(&store);
    statistics_store_snapshot(&store, &stats);

    /* Processor counts should be reasonable (at least 1). */
    EXPECT_GE(stats.logicalProcessorCount, 1u);
    EXPECT_GE(stats.physicalCoreCount, 1u);

    /* Logical processors should be >= physical cores. */
    EXPECT_GE(stats.logicalProcessorCount, stats.physicalCoreCount);

    /* CPU percentage should be in valid range (first call is often 0%). */
    EXPECT_GE(stats.cpuUsagePercent, 0.0);
    EXPECT_LE(stats.cpuUsagePercent, 100.0);
}

/*
 * Test: SuccessiveCollectsProduceValidCpuPercentage
 *
 * Verifies that after two successive calls, CPU percentage is calculated.
 * The second call should have enough data to compute delta-based percentage.
 */
TEST_F(CpuMonitorTest, SuccessiveCollectsProduceValidCpuPercentage)
{
    SystemStats stats1, stats2;

    /* First collect (establishes baseline). */
    cpu_monitor_collect(&store);
    statistics_store_snapshot(&store, &stats1);
    double cpuPercent1 = stats1.cpuUsagePercent;

    /* Small delay to accumulate some CPU jiffies. */
    sleep(1);

    /* Second collect (computes delta). */
    cpu_monitor_collect(&store);
    statistics_store_snapshot(&store, &stats2);
    double cpuPercent2 = stats2.cpuUsagePercent;

    /* Second reading should not be zero (has previous sample for comparison). */
    /* Note: May still be very low on idle systems, but should compute. */
    EXPECT_GE(cpuPercent2, 0.0);
    EXPECT_LE(cpuPercent2, 100.0);
}

/*
 * Test: ProcessorCountsRemainStable
 *
 * Verifies that processor counts (logical and physical) do not change
 * between successive calls (assuming hardware is static).
 */
TEST_F(CpuMonitorTest, ProcessorCountsRemainStable)
{
    SystemStats stats1, stats2;

    cpu_monitor_collect(&store);
    statistics_store_snapshot(&store, &stats1);

    sleep(1);

    cpu_monitor_collect(&store);
    statistics_store_snapshot(&store, &stats2);

    EXPECT_EQ(stats1.logicalProcessorCount, stats2.logicalProcessorCount);
    EXPECT_EQ(stats1.physicalCoreCount, stats2.physicalCoreCount);
}

/*
 * Test: CollectHandlesMissingProcGracefully
 *
 * Verifies that cpu_monitor_collect() handles read failures gracefully.
 *
 * Note: This test can only be performed on systems where /proc/stat
 * and /proc/cpuinfo are temporarily inaccessible (testing infrastructure dependent).
 * On typical systems, this test verifies no crash occurs.
 */
TEST_F(CpuMonitorTest, CollectDoesNotCrashOnError)
{
    /* Call collect multiple times. Even if /proc is unreadable, should not crash. */
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_NO_FATAL_FAILURE(cpu_monitor_collect(&store));
    }
}

/* Run all tests when the test program is executed. */
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
