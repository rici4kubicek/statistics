#include "unity.h"

#include <stdint.h>

#include "statistics_int_math.h"

void test_floorShift(void)
{
    TEST_ASSERT_EQUAL_INT64(2, StatInt_FloorShift(5, 1));
    TEST_ASSERT_EQUAL_INT64(-3, StatInt_FloorShift(-5, 1));
    TEST_ASSERT_EQUAL_INT64(-1, StatInt_FloorShift(-1, 4));
    TEST_ASSERT_EQUAL_INT64(-1, StatInt_FloorShift(-16, 4));
    TEST_ASSERT_EQUAL_INT64(-2, StatInt_FloorShift(-17, 4));
    TEST_ASSERT_EQUAL_INT64(0, StatInt_FloorShift(15, 4));
    TEST_ASSERT_EQUAL_INT64(7, StatInt_FloorShift(7, 0));
    TEST_ASSERT_EQUAL_INT64(-7, StatInt_FloorShift(-7, 0));
    TEST_ASSERT_EQUAL_INT64(INT64_MIN / 2, StatInt_FloorShift(INT64_MIN, 1));
    TEST_ASSERT_EQUAL_INT64(INT64_MAX / 2, StatInt_FloorShift(INT64_MAX, 1));
    TEST_ASSERT_EQUAL_INT64(-1, StatInt_FloorShift(INT64_MIN, 63));
    TEST_ASSERT_EQUAL_INT64(0, StatInt_FloorShift(INT64_MAX, 63));
    TEST_ASSERT_EQUAL_INT64(-1, StatInt_FloorShift(-1, 200));
    TEST_ASSERT_EQUAL_INT64(0, StatInt_FloorShift(12345, 200));
}

void test_divRound(void)
{
    TEST_ASSERT_EQUAL_INT64(3, StatInt_DivRound(5, 2));
    TEST_ASSERT_EQUAL_INT64(-3, StatInt_DivRound(-5, 2));
    TEST_ASSERT_EQUAL_INT64(1, StatInt_DivRound(4, 3));
    TEST_ASSERT_EQUAL_INT64(-1, StatInt_DivRound(-4, 3));
    TEST_ASSERT_EQUAL_INT64(2, StatInt_DivRound(5, 3));
    TEST_ASSERT_EQUAL_INT64(-2, StatInt_DivRound(-5, 3));
    TEST_ASSERT_EQUAL_INT64(0, StatInt_DivRound(-1, 4));
    TEST_ASSERT_EQUAL_INT64(-1, StatInt_DivRound(-2, 4));
    TEST_ASSERT_EQUAL_INT64(0, StatInt_DivRound(0, 7));
    TEST_ASSERT_EQUAL_INT64(INT64_MAX, StatInt_DivRound(INT64_MAX, 1));
    TEST_ASSERT_EQUAL_INT64(INT64_MIN, StatInt_DivRound(INT64_MIN, 1));
    TEST_ASSERT_EQUAL_INT64((int64_t) 1 << 62, StatInt_DivRound(INT64_MAX, 2));
    TEST_ASSERT_EQUAL_INT64(-((int64_t) 1 << 62), StatInt_DivRound(INT64_MIN, 2));
    TEST_ASSERT_EQUAL_INT64(1, StatInt_DivRound(INT64_MAX, INT64_MAX));
    TEST_ASSERT_EQUAL_INT64(-1, StatInt_DivRound(INT64_MIN, INT64_MAX));
    /* invalid divisor */
    TEST_ASSERT_EQUAL_INT64(0, StatInt_DivRound(10, 0));
    TEST_ASSERT_EQUAL_INT64(0, StatInt_DivRound(10, -2));
}

void test_isqrt64(void)
{
    TEST_ASSERT_EQUAL_UINT32(0, StatInt_Isqrt64(0));
    TEST_ASSERT_EQUAL_UINT32(1, StatInt_Isqrt64(1));
    TEST_ASSERT_EQUAL_UINT32(1, StatInt_Isqrt64(3));
    TEST_ASSERT_EQUAL_UINT32(2, StatInt_Isqrt64(4));
    TEST_ASSERT_EQUAL_UINT32(9, StatInt_Isqrt64(99));
    TEST_ASSERT_EQUAL_UINT32(10, StatInt_Isqrt64(100));
    TEST_ASSERT_EQUAL_UINT32(65535, StatInt_Isqrt64(4294967295ULL));
    TEST_ASSERT_EQUAL_UINT32(65536, StatInt_Isqrt64(4294967296ULL));
    TEST_ASSERT_EQUAL_UINT32(3037000499UL, StatInt_Isqrt64((uint64_t) INT64_MAX));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, StatInt_Isqrt64(UINT64_MAX));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX - 1, StatInt_Isqrt64((uint64_t) UINT32_MAX * UINT32_MAX - 1));
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, StatInt_Isqrt64((uint64_t) UINT32_MAX * UINT32_MAX));
}

void test_clamp(void)
{
    TEST_ASSERT_EQUAL_INT32(5, StatInt_Clamp_I32(5, 0, 10));
    TEST_ASSERT_EQUAL_INT32(0, StatInt_Clamp_I32(-5, 0, 10));
    TEST_ASSERT_EQUAL_INT32(10, StatInt_Clamp_I32(15, 0, 10));
    TEST_ASSERT_EQUAL_INT32(-10, StatInt_Clamp_I32(-20, -10, -5));
    TEST_ASSERT_EQUAL_INT32(-5, StatInt_Clamp_I32(0, -10, -5));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, StatInt_Clamp_I32(INT32_MIN, INT32_MIN, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, StatInt_Clamp_I32(INT32_MAX, INT32_MIN, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(3, StatInt_Clamp_I32(INT32_MIN, 3, 3));

    TEST_ASSERT_EQUAL_INT64(INT64_MIN + 1, StatInt_Clamp_I64(INT64_MIN, INT64_MIN + 1, 0));
    TEST_ASSERT_EQUAL_INT64(INT64_MAX - 1, StatInt_Clamp_I64(INT64_MAX, 0, INT64_MAX - 1));
    TEST_ASSERT_EQUAL_INT64(-7, StatInt_Clamp_I64(-7, -10, 10));

    TEST_ASSERT_EQUAL_UINT32(10, StatInt_Clamp_U32(5, 10, 20));
    TEST_ASSERT_EQUAL_UINT32(20, StatInt_Clamp_U32(UINT32_MAX, 10, 20));
    TEST_ASSERT_EQUAL_UINT32(15, StatInt_Clamp_U32(15, 10, 20));
    TEST_ASSERT_EQUAL_UINT32(0, StatInt_Clamp_U32(0, 0, UINT32_MAX));
}

void test_mapI32_basic(void)
{
    TEST_ASSERT_EQUAL_INT32(0, StatInt_Map_I32(0, 0, 1023, 0, 255));
    TEST_ASSERT_EQUAL_INT32(255, StatInt_Map_I32(1023, 0, 1023, 0, 255));
    TEST_ASSERT_EQUAL_INT32(128, StatInt_Map_I32(512, 0, 1023, 0, 255)); /* 127.62 */
    TEST_ASSERT_EQUAL_INT32(50, StatInt_Map_I32(5, 0, 10, 0, 100));
    /* rounding half away from zero */
    TEST_ASSERT_EQUAL_INT32(1, StatInt_Map_I32(1, 0, 2, 0, 1)); /* 0.5 */
    TEST_ASSERT_EQUAL_INT32(-1, StatInt_Map_I32(1, 0, 2, 0, -1)); /* -0.5 */
    TEST_ASSERT_EQUAL_INT32(0, StatInt_Map_I32(1, 0, 3, 0, 1)); /* 0.33 */
    TEST_ASSERT_EQUAL_INT32(1, StatInt_Map_I32(2, 0, 3, 0, 1)); /* 0.67 */
    TEST_ASSERT_EQUAL_INT32(-1, StatInt_Map_I32(2, 0, 3, 0, -1)); /* -0.67 */
    TEST_ASSERT_EQUAL_INT32(0, StatInt_Map_I32(1, 0, 3, 0, -1)); /* -0.33 */
    TEST_ASSERT_EQUAL_INT32(4, StatInt_Map_I32(3, 0, 4, 1, 5)); /* 1 + 3 */
}

void test_mapI32_reversed_and_negative(void)
{
    /* reversed output */
    TEST_ASSERT_EQUAL_INT32(100, StatInt_Map_I32(0, 0, 10, 100, 0));
    TEST_ASSERT_EQUAL_INT32(0, StatInt_Map_I32(10, 0, 10, 100, 0));
    TEST_ASSERT_EQUAL_INT32(70, StatInt_Map_I32(3, 0, 10, 100, 0));
    /* reversed input */
    TEST_ASSERT_EQUAL_INT32(0, StatInt_Map_I32(10, 10, 0, 0, 100));
    TEST_ASSERT_EQUAL_INT32(100, StatInt_Map_I32(0, 10, 0, 0, 100));
    TEST_ASSERT_EQUAL_INT32(70, StatInt_Map_I32(3, 10, 0, 0, 100));
    /* both reversed */
    TEST_ASSERT_EQUAL_INT32(30, StatInt_Map_I32(3, 10, 0, 100, 0));
    /* negative ranges */
    TEST_ASSERT_EQUAL_INT32(0, StatInt_Map_I32(-50, -100, 0, -10, 10));
    TEST_ASSERT_EQUAL_INT32(-10, StatInt_Map_I32(-100, -100, 0, -10, 10));
    TEST_ASSERT_EQUAL_INT32(-5, StatInt_Map_I32(-75, -100, 0, -10, 10));
    TEST_ASSERT_EQUAL_INT32(-1000, StatInt_Map_I32(-1, -1, 1, -1000, 1000));
    /* inMin == inMax */
    TEST_ASSERT_EQUAL_INT32(42, StatInt_Map_I32(7, 5, 5, 42, 99));
    TEST_ASSERT_EQUAL_INT32(42, StatInt_Map_I32(5, 5, 5, 42, 99));
}

void test_mapI32_extrapolation(void)
{
    /* no output clamping */
    TEST_ASSERT_EQUAL_INT32(200, StatInt_Map_I32(20, 0, 10, 0, 100));
    TEST_ASSERT_EQUAL_INT32(-100, StatInt_Map_I32(-10, 0, 10, 0, 100));
    TEST_ASSERT_EQUAL_INT32(-100, StatInt_Map_I32(20, 0, 10, 100, 0));
    TEST_ASSERT_EQUAL_INT32(150, StatInt_Clamp_I32(StatInt_Map_I32(20, 0, 10, 0, 100), 0, 150));
}

void test_mapI32_extremes(void)
{
    /* full range identity and inversion */
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, StatInt_Map_I32(INT32_MIN, INT32_MIN, INT32_MAX, INT32_MIN, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, StatInt_Map_I32(INT32_MAX, INT32_MIN, INT32_MAX, INT32_MIN, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(12345, StatInt_Map_I32(12345, INT32_MIN, INT32_MAX, INT32_MIN, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(-12345, StatInt_Map_I32(-12345, INT32_MIN, INT32_MAX, INT32_MIN, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, StatInt_Map_I32(INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX, INT32_MIN));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, StatInt_Map_I32(INT32_MAX, INT32_MIN, INT32_MAX, INT32_MAX, INT32_MIN));
    /* full range to small range: (0 - INT32_MIN) * 255 / (2^32 - 1) = 127.50000003 */
    TEST_ASSERT_EQUAL_INT32(128, StatInt_Map_I32(0, INT32_MIN, INT32_MAX, 0, 255));
    TEST_ASSERT_EQUAL_INT32(-1, StatInt_Map_I32(-1, INT32_MIN, INT32_MAX, -128, 127)); /* -128 + round(127.49999997) */
    /* small range to full range */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, StatInt_Map_I32(1, 0, 1, INT32_MIN, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(0, StatInt_Map_I32(1, 0, 2, INT32_MIN, INT32_MAX)); /* INT32_MIN + round(2147483647.5) */
    /* saturation of extrapolated results */
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, StatInt_Map_I32(INT32_MAX, 0, 1, 0, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, StatInt_Map_I32(INT32_MIN, 0, 1, 0, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, StatInt_Map_I32(INT32_MAX, 0, 1, 0, INT32_MIN));
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, StatInt_Map_I32(INT32_MAX, INT32_MAX - 1, INT32_MAX, INT32_MIN, INT32_MAX));
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, StatInt_Map_I32(INT32_MIN, INT32_MAX - 1, INT32_MAX, INT32_MIN, INT32_MAX));
}

#if STATISTICS_FLOAT_ENABLED
void test_float_variants(void)
{
    TEST_ASSERT_EQUAL_FLOAT(1.5f, StatFloat_Clamp(1.5f, 0.0f, 2.0f));
    TEST_ASSERT_EQUAL_FLOAT(0.0f, StatFloat_Clamp(-1.5f, 0.0f, 2.0f));
    TEST_ASSERT_EQUAL_FLOAT(2.0f, StatFloat_Clamp(3.0f, 0.0f, 2.0f));
    TEST_ASSERT_EQUAL_FLOAT(127.5f, StatFloat_Map(0.5f, 0.0f, 1.0f, 0.0f, 255.0f));
    TEST_ASSERT_EQUAL_FLOAT(75.0f, StatFloat_Map(2.5f, 0.0f, 10.0f, 100.0f, 0.0f));
    TEST_ASSERT_EQUAL_FLOAT(-50.0f, StatFloat_Map(-5.0f, 0.0f, 10.0f, 0.0f, 100.0f));
    TEST_ASSERT_EQUAL_FLOAT(3.0f, StatFloat_Map(1.0f, 2.0f, 2.0f, 3.0f, 4.0f));
}
#endif

void setUp(void)
{
}

void tearDown(void)
{
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_floorShift);
    RUN_TEST(test_divRound);
    RUN_TEST(test_isqrt64);
    RUN_TEST(test_clamp);
    RUN_TEST(test_mapI32_basic);
    RUN_TEST(test_mapI32_reversed_and_negative);
    RUN_TEST(test_mapI32_extrapolation);
    RUN_TEST(test_mapI32_extremes);
#if STATISTICS_FLOAT_ENABLED
    RUN_TEST(test_float_variants);
#endif
    return UNITY_END();
}
