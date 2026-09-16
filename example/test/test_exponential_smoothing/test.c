#include "unity.h"

#include "exponential_smoothing.h"

void test_ExponentialSmoothing(void)
{
    ExponentialSmoothing filter;

    ExponentialSmoothing_Init(&filter, 10, 5.5);
    TEST_ASSERT_EQUAL_FLOAT(5.5, ExponentialSmoothing_GetValue(&filter));

    ExponentialSmoothing_Process(&filter, 10);
    TEST_ASSERT_EQUAL_FLOAT(5.95, ExponentialSmoothing_GetValue(&filter));
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
    RUN_TEST(test_ExponentialSmoothing);
    return UNITY_END();
}

int main(void)
{
    return runUnityTests();
}
