#include "unity.h"

#include <stdint.h>

#include "statistics.h"

#include <stdio.h>
#include <string.h>

static void fill_u8(Statistics * stat, const uint8_t * vals, uint8_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        Statistics_AddSample(stat, &vals[i]);
    }
}

static void fill_u16(Statistics * stat, const uint16_t * vals, uint8_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        Statistics_AddSample(stat, &vals[i]);
    }
}

static void fill_f(Statistics * stat, const float * vals)
{
    for (uint32_t i = 0; i < stat->samplesCnt; i++) {
        Statistics_AddSample(stat, &vals[i]);
    }
}

void test_Statistics_InitAndFree(void)
{
    Statistics st = {0};

    Statistics_Init(&st, sizeof(uint8_t), 4);

    TEST_ASSERT_EQUAL_UINT8(sizeof(uint8_t), st.itemSize);
    TEST_ASSERT_EQUAL_UINT32(4, st.samplesCnt);
    TEST_ASSERT_EQUAL_UINT32(0, st.sampleIdx);
    TEST_ASSERT_NOT_NULL(st.samples);

    // Free should not crash and should reset the index per implementation
    Statistics_Free(&st);
    TEST_ASSERT_EQUAL_UINT32(0, st.sampleIdx);
}

void test_Statistics_Mean_U8(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 5);

    const uint8_t data[5] = {10, 20, 30, 40, 50};
    fill_u8(&st, data, 5);

    int64_t avg = Statistics_Mean_U8(&st);
    // Mean is now scaled by 1000: (10+20+30+40+50)/5 * 1000 = 30000
    TEST_ASSERT_EQUAL_INT64(30000, avg);

    Statistics_Free(&st);
}

static void oneLoad(Statistics * stat, uint8_t idx, void * data)
{
    uint8_t * field = stat->samples + idx;

    memcpy(data, field, stat->itemSize);
}

void test_Statistics_Mean_U16(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint16_t), 4);

    const uint16_t data[4] = {1000, 2000, 3000, 4000};
    fill_u16(&st, data, 4);

    int64_t avg = Statistics_Mean_U16(&st);
    // Mean is now scaled by 1000: (1000+2000+3000+4000)/4 * 1000 = 2500000
    TEST_ASSERT_EQUAL_INT64(2500000, avg);

    Statistics_Free(&st);
}

void test_Statistics_Mean_Float(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(float), 4);

    const float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    fill_f(&st, data);

    float avg = Statistics_Mean_F(&st);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 2.5f, avg);

    Statistics_Free(&st);
}

void test_Statistics_Variance_U8(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 8);

    const uint8_t data[8] = {2, 4, 4, 4, 5, 5, 7, 9};
    fill_u8(&st, data, 8);

    // Sample variance = 32/7 = 4.571428..., scaled by 1000
    TEST_ASSERT_EQUAL_INT64(4571, Statistics_Variance_U8(&st));

    Statistics_Free(&st);
}

void test_Statistics_Variance_I16_NonIntegerMean(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(int16_t), 3);

    const int16_t data[3] = {-1, 0, 0};
    for (uint32_t i = 0; i < 3; i++) {
        Statistics_AddSample(&st, &data[i]);
    }

    // Mean = -1/3, sample variance = 1/3 = 0.3333..., scaled by 1000
    TEST_ASSERT_EQUAL_INT64(333, Statistics_Variance_I16(&st));

    Statistics_Free(&st);
}

void test_Statistics_Variance_U16_Large(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint16_t), 4);

    const uint16_t data[4] = {1000, 2000, 3000, 4000};
    fill_u16(&st, data, 4);

    // Sample variance = 5000000/3 = 1666666.666..., scaled by 1000 and rounded
    TEST_ASSERT_EQUAL_INT64(1666666667, Statistics_Variance_U16(&st));

    Statistics_Free(&st);
}

void test_Statistics_Variance_U8_RoundHalfUp(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 2);

    const uint8_t data[2] = {0, 1};
    fill_u8(&st, data, 2);

    // Sample variance = 0.5 exactly, scaled by 1000
    TEST_ASSERT_EQUAL_INT64(500, Statistics_Variance_U8(&st));

    Statistics_Free(&st);
}

void test_Statistics_Variance_TooFewSamples(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 1);

    const uint8_t data[1] = {7};
    fill_u8(&st, data, 1);

    TEST_ASSERT_EQUAL_INT64(-1, Statistics_Variance_U8(&st));

    Statistics_Free(&st);
}

void test_Statistics_AddSample_WritesToCurrentIndex(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 5);

    // Initialize all samples to 0 first
    uint8_t zeros[5] = {0};
    fill_u8(&st, zeros, 5);

    // Now write a non-zero value to a specific index
    uint8_t v = 77;
    st.sampleIdx = 2;
    Statistics_AddSample(&st, &v);

    // Mean is scaled by 1000: 77/5 * 1000 = 15400
    int64_t avg = Statistics_Mean_U8(&st);
    TEST_ASSERT_EQUAL_INT64(15400, avg);

    Statistics_Free(&st);
}

void test_Statistics_Mean_U8_AllZero(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 3);

    uint8_t zeros[3] = {0};
    fill_u8(&st, zeros, 3);

    int64_t avg = Statistics_Mean_U8(&st);
    TEST_ASSERT_EQUAL_INT64(0, avg);

    Statistics_Free(&st);
}

void test_Statistics_Rotation_IndexWrapsToZero(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 4);

    uint8_t v = 1;
    for (uint32_t i = 0; i < st.samplesCnt; i++) {
        Statistics_AddSample(&st, &v);
    }

    TEST_ASSERT_EQUAL_UINT32(0, st.sampleIdx);

    Statistics_Free(&st);
}

void test_Statistics_Mean_U8_Rotation(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 4);

    const uint8_t first[5] = {10, 20, 30, 40, 50};
    fill_u8(&st, first, 5);

    // After rotation, buffer contains [50, 20, 30, 40]
    // Mean: (50+20+30+40)/4 * 1000 = 35000
    int64_t avg = Statistics_Mean_U8(&st);
    TEST_ASSERT_EQUAL_INT64(35000, avg);

    Statistics_Free(&st);
}

void test_Statistics_Mean_U16_Rotation(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint16_t), 4);

    const uint16_t data[7] = {1000, 2000, 3000, 4000, 3123, 1234, 8457};
    fill_u16(&st, data, 7);

    // After rotation, buffer contains [3123, 1234, 8457, 4000]
    // Mean: (3123+1234+8457+4000)/4 * 1000 = 4203500
    int64_t avg = Statistics_Mean_U16(&st);
    TEST_ASSERT_EQUAL_INT64(4203500, avg);

    Statistics_Free(&st);
}

void test_Statistics_Max_U8(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 5);

    const uint8_t data[5] = {10, 250, 30, 40, 50};
    fill_u8(&st, data, 5);

    uint8_t maxv = Statistics_Max_U8(&st);
    TEST_ASSERT_EQUAL_UINT8(250, maxv);

    Statistics_Free(&st);
}

void test_Statistics_Max_U16(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint16_t), 4);

    const uint16_t data[4] = {1000, 65500, 3000, 4000};
    fill_u16(&st, data, 4);

    uint16_t maxv = Statistics_Max_U16(&st);
    TEST_ASSERT_EQUAL_UINT16(65500, maxv);

    Statistics_Free(&st);
}

void test_Statistics_Max_Float(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(float), 4);

    const float data[4] = {1.5f, -2.0f, 3.25f, 3.24f};
    fill_f(&st, data);

    float maxv = Statistics_Max_F(&st);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, 3.25f, maxv);

    Statistics_Free(&st);
}

void test_Statistics_Max_U8_Rotation(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 4);

    const uint8_t first[4] = {10, 20, 80, 40};
    fill_u8(&st, first, 4);

    // Overwrite index 0 with rotation by adding one more sample
    uint8_t extra = 50;
    Statistics_AddSample(&st, &extra); // buffer now [50,20,80,40], max = 80
    TEST_ASSERT_EQUAL_UINT8(80, Statistics_Max_U8(&st));

    // Add another to overwrite index 1
    extra = 110; // new max
    Statistics_AddSample(&st, &extra); // buffer now [50,110,80,40], max = 110
    TEST_ASSERT_EQUAL_UINT8(110, Statistics_Max_U8(&st));

    Statistics_Free(&st);
}

void test_Statistics_Max_U16_Rotation(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint16_t), 4);

    const uint16_t first[4] = {1000, 40000, 30000, 20000};
    fill_u16(&st, first, 4); // max = 40000

    // Overwrite index 0 with rotation by adding one more sample (new max)
    uint16_t extra = 45000;
    Statistics_AddSample(&st, &extra); // buffer now [45000,40000,30000,20000]
    TEST_ASSERT_EQUAL_UINT16(45000, Statistics_Max_U16(&st));

    // Overwrite index 1 with a smaller value; max should remain 45000
    extra = 42000;
    Statistics_AddSample(&st, &extra); // buffer now [45000,42000,30000,20000]
    TEST_ASSERT_EQUAL_UINT16(45000, Statistics_Max_U16(&st));

    // Overwrite index 2 with a new global maximum
    extra = 65535;
    Statistics_AddSample(&st, &extra); // buffer now [45000,42000,65535,20000]
    TEST_ASSERT_EQUAL_UINT16(65535, Statistics_Max_U16(&st));

    Statistics_Free(&st);
}

void test_Statistics_Min_U8(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 5);

    const uint8_t data[5] = {10, 250, 5, 40, 50};
    fill_u8(&st, data, 5);

    uint8_t minv = Statistics_Min_U8(&st);
    TEST_ASSERT_EQUAL_UINT8(5, minv);

    Statistics_Free(&st);
}

void test_Statistics_Min_U16(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint16_t), 4);

    const uint16_t data[4] = {1000, 65500, 3000, 40};
    fill_u16(&st, data, 4);

    uint16_t minv = Statistics_Min_U16(&st);
    TEST_ASSERT_EQUAL_UINT16(40, minv);

    Statistics_Free(&st);
}

void test_Statistics_Min_Float(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(float), 4);

    const float data[4] = {1.5f, -2.0f, 3.25f, -3.24f};
    fill_f(&st, data);

    float minv = Statistics_Min_F(&st);
    TEST_ASSERT_FLOAT_WITHIN(1e-6f, -3.24f, minv);

    Statistics_Free(&st);
}

void test_Statistics_Min_U8_Rotation(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 4);

    const uint8_t first[4] = {10, 20, 80, 40};
    fill_u8(&st, first, 4); // min = 10

    // Overwrite index 0 with rotation by adding one more sample (new min)
    uint8_t extra = 5;
    Statistics_AddSample(&st, &extra); // buffer now [5,20,80,40]
    TEST_ASSERT_EQUAL_UINT8(5, Statistics_Min_U8(&st));

    // Overwrite index 1 with a larger value; min should remain 5
    extra = 30;
    Statistics_AddSample(&st, &extra); // buffer now [5,30,80,40]
    TEST_ASSERT_EQUAL_UINT8(5, Statistics_Min_U8(&st));

    Statistics_Free(&st);
}

void test_Statistics_Stdev_U8(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 8);

    const uint8_t data[8] = {2, 4, 4, 4, 5, 5, 7, 9};
    fill_u8(&st, data, 8);

    // Sample stdev = 2.13809..., scaled by 1000
    TEST_ASSERT_EQUAL_INT64(2138, Statistics_Stdev_U8(&st));

    Statistics_Free(&st);
}

void test_Statistics_Stdev_I8_Negative(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(int8_t), 8);

    const int8_t data[8] = {-2, -4, -4, -4, -5, -5, -7, -9};
    for (uint32_t i = 0; i < 8; i++) {
        Statistics_AddSample(&st, &data[i]);
    }

    TEST_ASSERT_EQUAL_INT64(2138, Statistics_Stdev_I8(&st));

    Statistics_Free(&st);
}

void test_Statistics_Stdev_U16_Large(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint16_t), 4);

    const uint16_t data[4] = {1000, 2000, 3000, 4000};
    fill_u16(&st, data, 4);

    // Sample stdev = 1290.9944..., scaled by 1000
    TEST_ASSERT_EQUAL_INT64(1290994, Statistics_Stdev_U16(&st));

    Statistics_Free(&st);
}

void test_Statistics_Stdev_U8_Constant(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 4);

    const uint8_t data[4] = {42, 42, 42, 42};
    fill_u8(&st, data, 4);

    TEST_ASSERT_EQUAL_INT64(0, Statistics_Stdev_U8(&st));

    Statistics_Free(&st);
}

void test_Statistics_Stdev_Float(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(float), 8);

    const float data[8] = {2.0f, 4.0f, 4.0f, 4.0f, 5.0f, 5.0f, 7.0f, 9.0f};
    fill_f(&st, data);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 2.13809f, Statistics_Stdev_F(&st));

    Statistics_Free(&st);
}

void test_Statistics_Stdev_U8_RoundsToNearest(void)
{
    Statistics st = {0};
    Statistics_Init(&st, sizeof(uint8_t), 4);

    const uint8_t data[4] = {0, 1, 2, 3};
    fill_u8(&st, data, 4);

    // Sample stdev = sqrt(5/3) = 1.290994..., scaled by 1000 and rounded
    TEST_ASSERT_EQUAL_INT64(1291, Statistics_Stdev_U8(&st));

    Statistics_Free(&st);
}

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

int runUnityTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Statistics_InitAndFree);
    RUN_TEST(test_Statistics_Mean_U8);
    RUN_TEST(test_Statistics_Mean_U16);
    RUN_TEST(test_Statistics_Mean_Float);
    RUN_TEST(test_Statistics_Variance_U8);
    RUN_TEST(test_Statistics_Variance_I16_NonIntegerMean);
    RUN_TEST(test_Statistics_Variance_U16_Large);
    RUN_TEST(test_Statistics_Variance_U8_RoundHalfUp);
    RUN_TEST(test_Statistics_Variance_TooFewSamples);
    RUN_TEST(test_Statistics_AddSample_WritesToCurrentIndex);
    RUN_TEST(test_Statistics_Mean_U8_AllZero);
    RUN_TEST(test_Statistics_Rotation_IndexWrapsToZero);
    RUN_TEST(test_Statistics_Mean_U8_Rotation);
    RUN_TEST(test_Statistics_Mean_U16_Rotation);
    RUN_TEST(test_Statistics_Max_U8);
    RUN_TEST(test_Statistics_Max_U16);
    RUN_TEST(test_Statistics_Max_Float);
    RUN_TEST(test_Statistics_Max_U8_Rotation);
    RUN_TEST(test_Statistics_Max_U16_Rotation);
    RUN_TEST(test_Statistics_Min_U8);
    RUN_TEST(test_Statistics_Min_U16);
    RUN_TEST(test_Statistics_Min_Float);
    RUN_TEST(test_Statistics_Min_U8_Rotation);
    RUN_TEST(test_Statistics_Stdev_U8);
    RUN_TEST(test_Statistics_Stdev_I8_Negative);
    RUN_TEST(test_Statistics_Stdev_U16_Large);
    RUN_TEST(test_Statistics_Stdev_U8_Constant);
    RUN_TEST(test_Statistics_Stdev_Float);
    RUN_TEST(test_Statistics_Stdev_U8_RoundsToNearest);
    return UNITY_END();
}

int main(void)
{
    return runUnityTests();
}
