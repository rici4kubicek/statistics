#include "unity.h"

#include <stdint.h>

#include "window_regression_i32.h"

#define CAP 16
#define CH  2

static uint32_t xs[CAP];
static int32_t ys[CAP * CH];
static WindowRegressionI32Line lines[CH];
static WindowRegressionI32 reg;

static void push2(uint32_t x, int32_t y0, int32_t y1)
{
    const int32_t y[CH] = {y0, y1};
    WindowRegressionI32_Push(&reg, x, y);
}

static int32_t eval(uint8_t channel, uint32_t x)
{
    int32_t value = 0x5A5A5A5A;
    TEST_ASSERT_TRUE(WindowRegressionI32_Evaluate(&reg, channel, x, &value));
    return value;
}

void test_init_invalid(void)
{
    WindowRegressionI32 r;
    TEST_ASSERT_FALSE(WindowRegressionI32_Init(&r, NULL, ys, lines, CAP, CH));
    TEST_ASSERT_FALSE(WindowRegressionI32_Init(&r, xs, NULL, lines, CAP, CH));
    TEST_ASSERT_FALSE(WindowRegressionI32_Init(&r, xs, ys, NULL, CAP, CH));
    TEST_ASSERT_FALSE(WindowRegressionI32_Init(&r, xs, ys, lines, 1, CH));
    TEST_ASSERT_FALSE(WindowRegressionI32_Init(&r, xs, ys, lines, WINDOW_REGRESSION_I32_CAPACITY_MAX + 1, CH));
    TEST_ASSERT_FALSE(WindowRegressionI32_Init(&r, xs, ys, lines, CAP, 0));
    TEST_ASSERT_FALSE(WindowRegressionI32_Init(NULL, xs, ys, lines, CAP, CH));

    /* invalid instance: all calls are no-ops */
    const int32_t y[CH] = {1, 2};
    WindowRegressionI32_Push(&r, 1, y);
    WindowRegressionI32_Push(&r, 2, y);
    TEST_ASSERT_EQUAL_UINT16(0, WindowRegressionI32_Count(&r));
    TEST_ASSERT_FALSE(WindowRegressionI32_Fit(&r, 2, 0));
    int32_t value;
    TEST_ASSERT_FALSE(WindowRegressionI32_Evaluate(&r, 0, 0, &value));
    TEST_ASSERT_EQUAL_UINT16(0, WindowRegressionI32_Count(NULL));
}

void test_linear_drift_exact(void)
{
    /* y0 = 1000 + 3 x, y1 = -500 - x / 4 (x step 8) */
    for (uint32_t x = 100; x < 100 + 8 * 10; x += 8) {
        push2(x, 1000 + 3 * (int32_t) x, -500 - (int32_t) x / 4);
    }
    TEST_ASSERT_EQUAL_UINT16(10, WindowRegressionI32_Count(&reg));
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&reg, 4, 0));
    TEST_ASSERT_TRUE(reg.fitted);
    TEST_ASSERT_EQUAL_UINT32(172, reg.fitX);

    TEST_ASSERT_EQUAL_INT64(3 * 65536, lines[0].slopeQ16);
    TEST_ASSERT_EQUAL_INT64((int64_t) (1000 + 3 * 172) * 65536, lines[0].levelQ16);
    TEST_ASSERT_EQUAL_INT64(-65536 / 4, lines[1].slopeQ16);
    TEST_ASSERT_EQUAL_INT64((int64_t) (-500 - 172 / 4) * 65536, lines[1].levelQ16);

    TEST_ASSERT_EQUAL_INT32(1000 + 3 * 172, eval(0, 172));
    TEST_ASSERT_EQUAL_INT32(1000 + 3 * 300, eval(0, 300)); /* extrapolated */
    TEST_ASSERT_EQUAL_INT32(1000 + 3 * 50, eval(0, 50)); /* before the window */
    TEST_ASSERT_EQUAL_INT32(-500 - 300 / 4, eval(1, 300));
    TEST_ASSERT_EQUAL_INT32(-525, eval(1, 102)); /* -525.5 rounds half up */
}

void test_gap_in_x(void)
{
    /* samples, a pause, more samples on the same line y = 2 x - 7 */
    const uint32_t x[] = {0, 1, 2, 3, 1000, 1001, 1002};
    for (unsigned i = 0; i < sizeof x / sizeof x[0]; i++) {
        push2(x[i], 2 * (int32_t) x[i] - 7, 5);
    }
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&reg, 2, 0));
    TEST_ASSERT_EQUAL_INT64(2 * 65536, lines[0].slopeQ16);
    TEST_ASSERT_EQUAL_INT32(2 * 1002 - 7, eval(0, 1002));
    TEST_ASSERT_EQUAL_INT32(2 * 500 - 7, eval(0, 500));
    TEST_ASSERT_EQUAL_INT64(0, lines[1].slopeQ16);
    TEST_ASSERT_EQUAL_INT32(5, eval(1, 5000));
}

void test_large_span_scaled(void)
{
    /* span 150000 > 2^13: ages are scaled by 2^5, slope 0.01 per x unit */
    for (int32_t i = 0; i < 16; i++) {
        const uint32_t x = (uint32_t) i * 10000;
        push2(x, 20000 + (int32_t) (x / 100), -(int32_t) (x / 50));
    }
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&reg, 2, 0));
    TEST_ASSERT_INT64_WITHIN(1, 655, lines[0].slopeQ16); /* 0.01 * 65536 = 655.36 */
    TEST_ASSERT_INT64_WITHIN(1, -1311, lines[1].slopeQ16); /* -0.02 * 65536 = -1310.72 */
    TEST_ASSERT_INT32_WITHIN(1, 20000 + 1500, eval(0, 150000));
    TEST_ASSERT_INT32_WITHIN(1, 20000 + 750, eval(0, 75000));
    TEST_ASSERT_INT32_WITHIN(1, -3000, eval(1, 150000));
    TEST_ASSERT_INT32_WITHIN(1, -1500, eval(1, 75000));
}

void test_slope_clamp_keeps_mean_level(void)
{
    /* y = 10 x around x in [0, 9], clamp the slope to 1 per x unit */
    int64_t sumY = 0;
    for (int32_t x = 0; x < 10; x++) {
        push2((uint32_t) x, 10 * x, -10 * x);
        sumY += 10 * x;
    }
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&reg, 2, 65536));
    TEST_ASSERT_EQUAL_INT64(65536, lines[0].slopeQ16);
    TEST_ASSERT_EQUAL_INT64(-65536, lines[1].slopeQ16);
    /* centroid (4.5, 45): the clamped line goes through it */
    TEST_ASSERT_EQUAL_INT64(sumY * 65536 / 10 + (int64_t) 45 * 65536 / 10, lines[0].levelQ16); /* 49.5 at x = 9 */
    TEST_ASSERT_EQUAL_INT32(41, eval(0, 0)); /* 45 - 4.5 = 40.5 -> 41 */
    TEST_ASSERT_EQUAL_INT32(50, eval(0, 9)); /* 45 + 4.5 = 49.5 -> 50 */
    TEST_ASSERT_EQUAL_INT32(-40, eval(1, 0)); /* -45 + 4.5 = -40.5 -> -40 (half up) */
    TEST_ASSERT_EQUAL_INT32(-49, eval(1, 9)); /* -49.5 -> -49 */

    /* no clamp when the limit is above the slope */
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&reg, 2, 20 * 65536));
    TEST_ASSERT_EQUAL_INT64(10 * 65536, lines[0].slopeQ16);
    TEST_ASSERT_EQUAL_INT32(90, eval(0, 9));
}

void test_window_overwrite(void)
{
    WindowRegressionI32 small;
    uint32_t sx[4];
    int32_t sy[4];
    WindowRegressionI32Line sl[1];
    TEST_ASSERT_TRUE(WindowRegressionI32_Init(&small, sx, sy, sl, 4, 1));

    /* outliers that must drop out of the window */
    int32_t y = 100000;
    WindowRegressionI32_Push(&small, 0, &y);
    y = -100000;
    WindowRegressionI32_Push(&small, 1, &y);
    for (uint32_t x = 2; x < 8; x++) {
        y = 5 * (int32_t) x + 1;
        WindowRegressionI32_Push(&small, x, &y);
    }
    TEST_ASSERT_EQUAL_UINT16(4, WindowRegressionI32_Count(&small));
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&small, 4, 0));
    TEST_ASSERT_EQUAL_INT64(5 * 65536, sl[0].slopeQ16);
    TEST_ASSERT_EQUAL_INT64((int64_t) 36 * 65536, sl[0].levelQ16);
    TEST_ASSERT_EQUAL_UINT32(7, small.fitX);

    WindowRegressionI32_Clear(&small);
    TEST_ASSERT_EQUAL_UINT16(0, WindowRegressionI32_Count(&small));
    TEST_ASSERT_FALSE(small.fitted);
    int32_t value;
    TEST_ASSERT_FALSE(WindowRegressionI32_Evaluate(&small, 0, 7, &value));
}

void test_below_min_count(void)
{
    int32_t value;
    TEST_ASSERT_FALSE(WindowRegressionI32_Fit(&reg, 0, 0)); /* empty */
    push2(0, 1, 1);
    TEST_ASSERT_FALSE(WindowRegressionI32_Fit(&reg, 0, 0)); /* one sample, minCount < 2 acts as 2 */
    push2(1, 2, 2);
    push2(2, 3, 3);
    TEST_ASSERT_FALSE(WindowRegressionI32_Fit(&reg, 4, 0));
    TEST_ASSERT_FALSE(reg.fitted);
    TEST_ASSERT_FALSE(WindowRegressionI32_Evaluate(&reg, 0, 2, &value));
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&reg, 3, 0));

    /* a failed fit drops the previous result */
    TEST_ASSERT_FALSE(WindowRegressionI32_Fit(&reg, 4, 0));
    TEST_ASSERT_FALSE(WindowRegressionI32_Evaluate(&reg, 0, 2, &value));
}

void test_equal_x_no_result(void)
{
    push2(5, 1, 1);
    push2(5, 2, 2);
    push2(5, 3, 3);
    TEST_ASSERT_FALSE(WindowRegressionI32_Fit(&reg, 2, 0));
}

void test_evaluate_args(void)
{
    push2(0, 0, 0);
    push2(1, 1, 1);
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&reg, 2, 0));
    int32_t value;
    TEST_ASSERT_FALSE(WindowRegressionI32_Evaluate(&reg, CH, 0, &value));
    TEST_ASSERT_FALSE(WindowRegressionI32_Evaluate(&reg, 0, 0, NULL));
    TEST_ASSERT_FALSE(WindowRegressionI32_Evaluate(NULL, 0, 0, &value));
}

void test_time_wrap(void)
{
    /* x crosses 2^32 inside the window, y = 3 (x - x0) + 7 */
    const uint32_t x0 = UINT32_MAX - 10;
    for (uint32_t i = 0; i < 12; i++) {
        const uint32_t x = x0 + 2 * i; /* wraps after i = 5 */
        push2(x, 3 * (int32_t) (2 * i) + 7, -(int32_t) i);
    }
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&reg, 2, 0));
    TEST_ASSERT_EQUAL_UINT32(x0 + 22, reg.fitX);
    TEST_ASSERT_EQUAL_INT64(3 * 65536, lines[0].slopeQ16);
    TEST_ASSERT_EQUAL_INT64(-65536 / 2, lines[1].slopeQ16);
    TEST_ASSERT_EQUAL_INT32(3 * 22 + 7, eval(0, x0 + 22));
    TEST_ASSERT_EQUAL_INT32(3 * 30 + 7, eval(0, x0 + 30)); /* after the wrap */
    TEST_ASSERT_EQUAL_INT32(7, eval(0, x0)); /* before the wrap */
    TEST_ASSERT_EQUAL_INT32(-15, eval(1, x0 + 30));
}

void test_extreme_inputs_saturate(void)
{
    /* |y - yNewest| beyond 2^20 is clamped, no overflow; Evaluate far away saturates */
    push2(0, INT32_MIN, 0);
    push2(1, INT32_MAX, INT32_MAX);
    push2(2, INT32_MIN, INT32_MIN);
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&reg, 2, 0));
    int32_t v = eval(0, 2);
    (void) v;
    /* channel 1: negative slope, dx = 2^31 - 3 and dx = -2^31 */
    TEST_ASSERT_TRUE(lines[1].slopeQ16 < 0);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, eval(1, 0x7FFFFFFF));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, eval(1, 0x80000002));

    /* full-span window: oldest age 2^32 - 1, slope 2^20 / (2^32 - 1) = 16.0000004 in Q16 */
    WindowRegressionI32_Clear(&reg);
    push2(0, 0, 2000);
    push2(UINT32_MAX / 2, 1 << 19, 1000);
    push2(UINT32_MAX, 1 << 20, 0);
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&reg, 2, 0));
    TEST_ASSERT_EQUAL_INT64(16, lines[0].slopeQ16);
    TEST_ASSERT_INT32_WITHIN(1, 1 << 20, eval(0, UINT32_MAX));
    TEST_ASSERT_INT32_WITHIN(1, 1 << 19, eval(0, UINT32_MAX / 2));
    /* slope -2000 / 2^32 per x unit is below the Q16 resolution: rounds to 0, the line is the mean */
    TEST_ASSERT_EQUAL_INT64(0, lines[1].slopeQ16);
    TEST_ASSERT_EQUAL_INT32(1000, eval(1, UINT32_MAX));
    TEST_ASSERT_EQUAL_INT32(1000, eval(1, 0));
}

void test_full_capacity_max_values(void)
{
    /* 128 samples, |x| up to 2^13 - 1 and |dy| = 2^20: the bound case of the fit sums */
    static uint32_t bx[WINDOW_REGRESSION_I32_CAPACITY_MAX];
    static int32_t by[WINDOW_REGRESSION_I32_CAPACITY_MAX];
    WindowRegressionI32Line bl[1];
    WindowRegressionI32 big;
    TEST_ASSERT_TRUE(WindowRegressionI32_Init(&big, bx, by, bl, WINDOW_REGRESSION_I32_CAPACITY_MAX, 1));
    for (uint32_t i = 0; i < WINDOW_REGRESSION_I32_CAPACITY_MAX; i++) {
        int32_t y = (i < 64) ? (1 << 20) : 0;
        WindowRegressionI32_Push(&big, i < 64 ? 0 : 8191, &y);
    }
    TEST_ASSERT_TRUE(WindowRegressionI32_Fit(&big, 2, 0));
    /* two clusters: slope = -2^20 / 8191 per x unit */
    TEST_ASSERT_INT64_WITHIN(1, -(((int64_t) 1 << 36) + 4095) / 8191, bl[0].slopeQ16);
    int32_t value;
    TEST_ASSERT_TRUE(WindowRegressionI32_Evaluate(&big, 0, 8191, &value));
    TEST_ASSERT_EQUAL_INT32(0, value);
    TEST_ASSERT_TRUE(WindowRegressionI32_Evaluate(&big, 0, 0, &value));
    TEST_ASSERT_INT32_WITHIN(1, 1 << 20, value);
}

void setUp(void)
{
    TEST_ASSERT_TRUE(WindowRegressionI32_Init(&reg, xs, ys, lines, CAP, CH));
}

void tearDown(void)
{
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init_invalid);
    RUN_TEST(test_linear_drift_exact);
    RUN_TEST(test_gap_in_x);
    RUN_TEST(test_large_span_scaled);
    RUN_TEST(test_slope_clamp_keeps_mean_level);
    RUN_TEST(test_window_overwrite);
    RUN_TEST(test_below_min_count);
    RUN_TEST(test_equal_x_no_result);
    RUN_TEST(test_evaluate_args);
    RUN_TEST(test_time_wrap);
    RUN_TEST(test_extreme_inputs_saturate);
    RUN_TEST(test_full_capacity_max_values);
    return UNITY_END();
}
