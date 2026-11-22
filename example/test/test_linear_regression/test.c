#include "unity.h"

#include <math.h>

#include "linear_regression.h"


static void test_linReg_clear(void)
{
    linReg_t linreg = {1, 2, 3, 4, 5, 6};

    linReg_clear(&linreg);

    TEST_ASSERT_EQUAL(0, linreg.mean_x);
    TEST_ASSERT_EQUAL(0, linreg.mean_y);
    TEST_ASSERT_EQUAL(0, linreg.s_xx);
    TEST_ASSERT_EQUAL(0, linreg.s_yy);
    TEST_ASSERT_EQUAL(0, linreg.s_xy);
    TEST_ASSERT_EQUAL(0, linreg.n);
}

static void test_linReg_process(void)
{
    linReg_t linreg;
    linReg_clear(&linreg);

    linReg_process(&linreg, 1, 2);
    linReg_process(&linreg, 2, 3);

    TEST_ASSERT_EQUAL(2, linreg.n);
    TEST_ASSERT_EQUAL_FLOAT(1.5, linreg.mean_x);
    TEST_ASSERT_EQUAL_FLOAT(2.5, linreg.mean_y);
}

static void test_linReg_result(void)
{
    linReg_t linreg;

    float m;
    float b;
    float r;
    float jitter;

    linReg_clear(&linreg);
    TEST_ASSERT_FALSE(linReg_hasResult(&linreg));
    TEST_ASSERT_FALSE(linReg_result(&linreg, &m, &b, &r, &jitter));

    linReg_process(&linreg, 1, 3);
    TEST_ASSERT_FALSE(linReg_hasResult(&linreg));
    TEST_ASSERT_FALSE(linReg_result(&linreg, &m, &b, &r, &jitter));

    linReg_process(&linreg, 2, 5);
    TEST_ASSERT_FALSE(linReg_hasResult(&linreg));
    TEST_ASSERT_FALSE(linReg_result(&linreg, &m, &b, &r, &jitter));

    linReg_process(&linreg, 3, 7);
    TEST_ASSERT_TRUE(linReg_hasResult(&linreg));
    TEST_ASSERT_TRUE(linReg_result(&linreg, &m, &b, &r, &jitter));

    TEST_ASSERT_EQUAL_FLOAT(2, m);
    TEST_ASSERT_EQUAL_FLOAT(1, b);
    TEST_ASSERT_EQUAL_FLOAT(1, r);
    TEST_ASSERT_EQUAL_FLOAT(0, jitter);

    linReg_process(&linreg, 4, 10);
    TEST_ASSERT_TRUE(linReg_hasResult(&linreg));
    TEST_ASSERT_TRUE(linReg_result(&linreg, &m, &b, &r, &jitter));

    TEST_ASSERT_EQUAL_FLOAT(2.3, m);
    TEST_ASSERT_EQUAL_FLOAT(0.5, b);
    TEST_ASSERT_EQUAL_FLOAT(0.9943768, r);
    TEST_ASSERT_EQUAL_FLOAT(0.3872978, jitter);
}

static void test_linReg_mUncertainty(void)
{
    linReg_t linreg;

    linReg_clear(&linreg);
    TEST_ASSERT(isinf(linReg_mUncertainty(&linreg, 1, 0.1)));

    linReg_process(&linreg, 1, 1);
    TEST_ASSERT(isinf(linReg_mUncertainty(&linreg, 1, 0.1)));

    /* reference values from centered statistics in double precision */
    linReg_process(&linreg, 1.1, 2);
    TEST_ASSERT_FLOAT_WITHIN(1e-4, 0.8164966, linReg_mUncertainty(&linreg, 1, 0.1));

    linReg_process(&linreg, 1.2, 3.1);
    TEST_ASSERT_FLOAT_WITHIN(1e-4, 0.5, linReg_mUncertainty(&linreg, 1, 0.1));

    linReg_process(&linreg, 1.3, 4);
    TEST_ASSERT_FLOAT_WITHIN(1e-4, 0.3696846, linReg_mUncertainty(&linreg, 1, 0.1));
    TEST_ASSERT_FLOAT_WITHIN(1e-4, 2 * 0.3696846, linReg_mUncertainty(&linreg, 2, 0.1));
}

static float uncertaintyOfShifted(float dx, float dy)
{
    const float x[] = {1, 1.1f, 1.2f, 1.3f};
    const float y[] = {1, 2, 3.1f, 4};
    linReg_t linreg;

    linReg_clear(&linreg);
    for (int i = 0; i < 4; i++) {
        linReg_process(&linreg, x[i] + dx, y[i] + dy);
    }
    return linReg_mUncertainty(&linreg, 1, 0.1);
}

static void test_linReg_mUncertainty_translation_invariant(void)
{
    const float base = uncertaintyOfShifted(0, 0);

    TEST_ASSERT_FLOAT_WITHIN(1e-3, base, uncertaintyOfShifted(10, 0));
    TEST_ASSERT_FLOAT_WITHIN(1e-3, base, uncertaintyOfShifted(-5, 0));
    TEST_ASSERT_FLOAT_WITHIN(1e-3, base, uncertaintyOfShifted(0, 50));
    TEST_ASSERT_FLOAT_WITHIN(1e-3, base, uncertaintyOfShifted(0, -20));
    TEST_ASSERT_FLOAT_WITHIN(1e-3, base, uncertaintyOfShifted(10, 50));
    TEST_ASSERT_FLOAT_WITHIN(1e-3, base, uncertaintyOfShifted(100, -100));
}

static void test_linReg_mUncertainty_degenerate_x(void)
{
    linReg_t linreg;

    linReg_clear(&linreg);
    linReg_process(&linreg, 2, 1);
    linReg_process(&linreg, 2, 5);
    TEST_ASSERT(isinf(linReg_mUncertainty(&linreg, 1, 0.1)));
    linReg_process(&linreg, 2, 3);
    TEST_ASSERT(isinf(linReg_mUncertainty(&linreg, 1, 0.1)));

    TEST_ASSERT(isinf(linReg_mUncertainty(NULL, 1, 0.1)));
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
    RUN_TEST(test_linReg_clear);
    RUN_TEST(test_linReg_process);
    RUN_TEST(test_linReg_result);
    RUN_TEST(test_linReg_mUncertainty);
    RUN_TEST(test_linReg_mUncertainty_translation_invariant);
    RUN_TEST(test_linReg_mUncertainty_degenerate_x);
    return UNITY_END();
}

int main(void)
{
    return runUnityTests();
}
