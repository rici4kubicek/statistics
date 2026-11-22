#include "unity.h"

#include <stdint.h>

#include "exponential_smoothing_i32.h"

void test_init_and_get(void)
{
    ExponentialSmoothingI32 filter;

    ExponentialSmoothingI32_Init(&filter, 3, EXPONENTIAL_SMOOTHING_I32_FRAC_DEFAULT, 42);
    TEST_ASSERT_TRUE(filter.valid);
    TEST_ASSERT_EQUAL_INT32(42 << 8, filter.acc);
    TEST_ASSERT_EQUAL_INT32(42, ExponentialSmoothingI32_GetValue(&filter));

    ExponentialSmoothingI32_Init(&filter, 3, 8, -42);
    TEST_ASSERT_EQUAL_INT32(-42, ExponentialSmoothingI32_GetValue(&filter));
}

void test_step_response(void)
{
    ExponentialSmoothingI32 filter;
    ExponentialSmoothingI32_Init(&filter, 2, 8, 0);

    /* alpha = 1/4: 25, 43.75, 57.81, 68.36 */
    ExponentialSmoothingI32_Process(&filter, 100);
    TEST_ASSERT_EQUAL_INT32(25, ExponentialSmoothingI32_GetValue(&filter));
    ExponentialSmoothingI32_Process(&filter, 100);
    TEST_ASSERT_EQUAL_INT32(44, ExponentialSmoothingI32_GetValue(&filter));
    ExponentialSmoothingI32_Process(&filter, 100);
    TEST_ASSERT_EQUAL_INT32(58, ExponentialSmoothingI32_GetValue(&filter));
    ExponentialSmoothingI32_Process(&filter, 100);
    TEST_ASSERT_EQUAL_INT32(68, ExponentialSmoothingI32_GetValue(&filter));

    for (int i = 0; i < 100; i++) {
        ExponentialSmoothingI32_Process(&filter, 100);
    }
    TEST_ASSERT_EQUAL_INT32(100, ExponentialSmoothingI32_GetValue(&filter));
}

void test_negative_step_response(void)
{
    ExponentialSmoothingI32 filter;
    ExponentialSmoothingI32_Init(&filter, 2, 8, 0);

    /* -25, -43.75, -57.81 */
    ExponentialSmoothingI32_Process(&filter, -100);
    TEST_ASSERT_EQUAL_INT32(-25, ExponentialSmoothingI32_GetValue(&filter));
    ExponentialSmoothingI32_Process(&filter, -100);
    TEST_ASSERT_EQUAL_INT32(-44, ExponentialSmoothingI32_GetValue(&filter));
    ExponentialSmoothingI32_Process(&filter, -100);
    TEST_ASSERT_EQUAL_INT32(-58, ExponentialSmoothingI32_GetValue(&filter));

    for (int i = 0; i < 100; i++) {
        ExponentialSmoothingI32_Process(&filter, -100);
    }
    TEST_ASSERT_EQUAL_INT32(-100, ExponentialSmoothingI32_GetValue(&filter));
    /* floor steps reach a negative target exactly */
    TEST_ASSERT_EQUAL_INT32(-100 * 256, filter.acc);

    /* crossing zero back to a positive value */
    for (int i = 0; i < 100; i++) {
        ExponentialSmoothingI32_Process(&filter, 7);
    }
    TEST_ASSERT_EQUAL_INT32(7, ExponentialSmoothingI32_GetValue(&filter));
}

void test_no_smoothing(void)
{
    ExponentialSmoothingI32 filter;
    ExponentialSmoothingI32_Init(&filter, 0, 8, 10);

    ExponentialSmoothingI32_Process(&filter, -1234);
    TEST_ASSERT_EQUAL_INT32(-1234, ExponentialSmoothingI32_GetValue(&filter));
}

void test_input_range_no_overflow(void)
{
    ExponentialSmoothingI32 filter;
    /* frac 8: input range [-8388608, 8388607], values outside are clamped */
    ExponentialSmoothingI32_Init(&filter, 1, 8, INT32_MAX);
    TEST_ASSERT_EQUAL_INT32(8388607, ExponentialSmoothingI32_GetValue(&filter));

    for (int i = 0; i < 64; i++) {
        ExponentialSmoothingI32_Process(&filter, INT32_MIN);
    }
    TEST_ASSERT_EQUAL_INT32(-8388608, ExponentialSmoothingI32_GetValue(&filter));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, filter.acc);

    for (int i = 0; i < 64; i++) {
        ExponentialSmoothingI32_Process(&filter, INT32_MAX);
    }
    TEST_ASSERT_EQUAL_INT32(8388607, ExponentialSmoothingI32_GetValue(&filter));

    /* frac 0, shift 31: full int32_t range, smallest alpha */
    ExponentialSmoothingI32_Init(&filter, 31, 0, INT32_MIN);
    ExponentialSmoothingI32_Process(&filter, INT32_MAX);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN + 1, ExponentialSmoothingI32_GetValue(&filter));
    ExponentialSmoothingI32_Init(&filter, 0, 0, INT32_MIN);
    ExponentialSmoothingI32_Process(&filter, INT32_MAX);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, ExponentialSmoothingI32_GetValue(&filter));

    /* frac 30: input range [-2, 1] */
    ExponentialSmoothingI32_Init(&filter, 1, 30, 5);
    TEST_ASSERT_EQUAL_INT32(1, ExponentialSmoothingI32_GetValue(&filter));
    ExponentialSmoothingI32_Reset(&filter, -5);
    TEST_ASSERT_EQUAL_INT32(-2, ExponentialSmoothingI32_GetValue(&filter));
}

void test_rounding(void)
{
    ExponentialSmoothingI32 filter;
    ExponentialSmoothingI32_Init(&filter, 1, 8, 0);

    ExponentialSmoothingI32_Process(&filter, 1); /* 0.5 -> 1 (half up) */
    TEST_ASSERT_EQUAL_INT32(128, filter.acc);
    TEST_ASSERT_EQUAL_INT32(1, ExponentialSmoothingI32_GetValue(&filter));

    ExponentialSmoothingI32_Reset(&filter, 0);
    ExponentialSmoothingI32_Process(&filter, -1); /* -0.5 -> 0 (half up) */
    TEST_ASSERT_EQUAL_INT32(-128, filter.acc);
    TEST_ASSERT_EQUAL_INT32(0, ExponentialSmoothingI32_GetValue(&filter));
    ExponentialSmoothingI32_Process(&filter, -1); /* -0.75 -> -1 */
    TEST_ASSERT_EQUAL_INT32(-1, ExponentialSmoothingI32_GetValue(&filter));
}

void test_reset(void)
{
    ExponentialSmoothingI32 filter;
    ExponentialSmoothingI32_Init(&filter, 4, 8, 0);
    ExponentialSmoothingI32_Process(&filter, 1000);
    ExponentialSmoothingI32_Reset(&filter, -77);
    TEST_ASSERT_EQUAL_INT32(-77, ExponentialSmoothingI32_GetValue(&filter));
    TEST_ASSERT_EQUAL_UINT8(4, filter.shift);
    TEST_ASSERT_EQUAL_UINT8(8, filter.frac);
}

void test_invalid(void)
{
    ExponentialSmoothingI32 filter;

    ExponentialSmoothingI32_Init(&filter, 32, 8, 10);
    TEST_ASSERT_FALSE(filter.valid);
    ExponentialSmoothingI32_Process(&filter, 10);
    ExponentialSmoothingI32_Reset(&filter, 10);
    TEST_ASSERT_EQUAL_INT32(0, ExponentialSmoothingI32_GetValue(&filter));

    ExponentialSmoothingI32_Init(&filter, 1, 31, 10);
    TEST_ASSERT_FALSE(filter.valid);
    TEST_ASSERT_EQUAL_INT32(0, ExponentialSmoothingI32_GetValue(&filter));

    ExponentialSmoothingI32_Init(NULL, 1, 8, 10);
    ExponentialSmoothingI32_Process(NULL, 10);
    ExponentialSmoothingI32_Reset(NULL, 10);
    TEST_ASSERT_EQUAL_INT32(0, ExponentialSmoothingI32_GetValue(NULL));
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
    RUN_TEST(test_init_and_get);
    RUN_TEST(test_step_response);
    RUN_TEST(test_negative_step_response);
    RUN_TEST(test_no_smoothing);
    RUN_TEST(test_input_range_no_overflow);
    RUN_TEST(test_rounding);
    RUN_TEST(test_reset);
    RUN_TEST(test_invalid);
    return UNITY_END();
}
