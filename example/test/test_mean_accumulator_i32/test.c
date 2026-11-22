#include "unity.h"

#include <stdint.h>

#include "mean_accumulator_i32.h"

void test_mean(void)
{
    MeanAccumulatorI32 acc;
    MeanAccumulatorI32_Reset(&acc);
    TEST_ASSERT_EQUAL_UINT32(0, MeanAccumulatorI32_Count(&acc));
    TEST_ASSERT_EQUAL_INT32(0, MeanAccumulatorI32_Mean(&acc));

    TEST_ASSERT_TRUE(MeanAccumulatorI32_Add(&acc, 1));
    TEST_ASSERT_TRUE(MeanAccumulatorI32_Add(&acc, 2));
    TEST_ASSERT_TRUE(MeanAccumulatorI32_Add(&acc, 4));
    TEST_ASSERT_EQUAL_UINT32(3, MeanAccumulatorI32_Count(&acc));
    TEST_ASSERT_EQUAL_INT32(2, MeanAccumulatorI32_Mean(&acc)); /* 2.33 */

    MeanAccumulatorI32_Add(&acc, 3);
    TEST_ASSERT_EQUAL_INT32(3, MeanAccumulatorI32_Mean(&acc)); /* 2.5 -> 3 */

    MeanAccumulatorI32_Reset(&acc);
    TEST_ASSERT_EQUAL_UINT32(0, MeanAccumulatorI32_Count(&acc));
    TEST_ASSERT_EQUAL_INT32(0, MeanAccumulatorI32_Mean(&acc));
}

void test_negative_rounding(void)
{
    MeanAccumulatorI32 acc;
    MeanAccumulatorI32_Reset(&acc);

    MeanAccumulatorI32_Add(&acc, -2);
    MeanAccumulatorI32_Add(&acc, -3);
    TEST_ASSERT_EQUAL_INT32(-3, MeanAccumulatorI32_Mean(&acc)); /* -2.5 -> -3 */

    MeanAccumulatorI32_Add(&acc, -2);
    TEST_ASSERT_EQUAL_INT32(-2, MeanAccumulatorI32_Mean(&acc)); /* -2.33 */

    MeanAccumulatorI32_Reset(&acc);
    MeanAccumulatorI32_Add(&acc, -1);
    MeanAccumulatorI32_Add(&acc, 0);
    MeanAccumulatorI32_Add(&acc, 0);
    TEST_ASSERT_EQUAL_INT32(0, MeanAccumulatorI32_Mean(&acc)); /* -0.33 */

    MeanAccumulatorI32_Add(&acc, -1);
    MeanAccumulatorI32_Add(&acc, -1);
    MeanAccumulatorI32_Add(&acc, 0);
    TEST_ASSERT_EQUAL_INT32(-1, MeanAccumulatorI32_Mean(&acc)); /* -0.5 -> -1 */
}

void test_large_counts(void)
{
    MeanAccumulatorI32 acc;
    MeanAccumulatorI32_Reset(&acc);

    for (uint32_t i = 0; i < 1000000; i++) {
        MeanAccumulatorI32_Add(&acc, INT32_MAX);
    }
    TEST_ASSERT_EQUAL_UINT32(1000000, MeanAccumulatorI32_Count(&acc));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, MeanAccumulatorI32_Mean(&acc));

    /* extremes of the sum at the maximum count, set directly (2^32 adds take too long) */
    acc.sum = (int64_t) INT32_MIN * (UINT32_MAX - 1);
    acc.count = UINT32_MAX - 1;
    TEST_ASSERT_TRUE(MeanAccumulatorI32_Add(&acc, INT32_MIN));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, MeanAccumulatorI32_Count(&acc));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, MeanAccumulatorI32_Mean(&acc));
    TEST_ASSERT_FALSE(MeanAccumulatorI32_Add(&acc, INT32_MIN));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, MeanAccumulatorI32_Count(&acc));

    acc.sum = (int64_t) INT32_MAX * (UINT32_MAX - 1);
    acc.count = UINT32_MAX - 1;
    TEST_ASSERT_TRUE(MeanAccumulatorI32_Add(&acc, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, MeanAccumulatorI32_Mean(&acc));

    /* mixed extremes */
    MeanAccumulatorI32_Reset(&acc);
    MeanAccumulatorI32_Add(&acc, INT32_MIN);
    MeanAccumulatorI32_Add(&acc, INT32_MAX);
    TEST_ASSERT_EQUAL_INT32(-1, MeanAccumulatorI32_Mean(&acc)); /* -0.5 -> -1 */
}

void test_null(void)
{
    MeanAccumulatorI32_Reset(NULL);
    TEST_ASSERT_FALSE(MeanAccumulatorI32_Add(NULL, 1));
    TEST_ASSERT_EQUAL_UINT32(0, MeanAccumulatorI32_Count(NULL));
    TEST_ASSERT_EQUAL_INT32(0, MeanAccumulatorI32_Mean(NULL));
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_mean);
    RUN_TEST(test_negative_rounding);
    RUN_TEST(test_large_counts);
    RUN_TEST(test_null);
    return UNITY_END();
}
