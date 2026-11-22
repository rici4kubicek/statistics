#include "exponential_smoothing_i32.h"
#include "mean_accumulator_i32.h"
#include "statistics.h"
#include "statistics_int_math.h"
#include "window_regression_i32.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

Statistics stat;

int main()
{
    printf("Hello, world!\n");
    fflush(stdout);

    Statistics_Init(&stat, sizeof(uint8_t), 4);
    if (!Statistics_IsValid(&stat)) {
        fprintf(stderr, "Statistics init failed: out of memory or invalid parameters.\n");
        return 1;
    }

    uint8_t v = 1;
    Statistics_AddSample(&stat, &v);
    v = 21;
    Statistics_AddSample(&stat, &v);
    v = 79;
    Statistics_AddSample(&stat, &v);
    v = 100;
    Statistics_AddSample(&stat, &v);
    v = 31;
    Statistics_AddSample(&stat, &v);
    v = 85;
    Statistics_AddSample(&stat, &v);

    printf("Max: %d\n", Statistics_Max_U8(&stat));
    printf("Min: %d\n", Statistics_Min_U8(&stat));

    int64_t mean = Statistics_Mean_U8(&stat);
    int64_t variance = Statistics_Variance_U8(&stat);
    int64_t stdev = Statistics_Stdev_U8(&stat);

    // Use llabs for fractional part to handle negative values correctly
    printf("Mean: %" PRId64 " (actual: %" PRId64 ".%03lld)\n", mean, mean / 1000, llabs(mean % 1000));
    printf("Variance: %" PRId64 " (actual: %" PRId64 ".%03lld)\n", variance, variance / 1000, llabs(variance % 1000));
    printf("Stdev: %" PRId64 " (actual: %" PRId64 ".%03lld)\n", stdev, stdev / 1000, llabs(stdev % 1000));

    printf("\n--- Testing Float type ---\n");
    Statistics statF;
    Statistics_Init(&statF, sizeof(float), 4);
    if (!Statistics_IsValid(&statF)) {
        fprintf(stderr, "Statistics init failed: out of memory or invalid parameters.\n");
        return 1;
    }
    float fv = 1.5f;
    Statistics_AddSample(&statF, &fv);
    fv = 21.3f;
    Statistics_AddSample(&statF, &fv);
    fv = 79.7f;
    Statistics_AddSample(&statF, &fv);
    fv = 100.2f;
    Statistics_AddSample(&statF, &fv);

    printf("Max: %.2f\n", Statistics_Max_F(&statF));
    printf("Min: %.2f\n", Statistics_Min_F(&statF));
    printf("Mean: %.2f\n", Statistics_Mean_F(&statF));
    printf("Variance: %.2f\n", Statistics_Variance_F(&statF));
    printf("Stdev: %.2f\n", Statistics_Stdev_F(&statF));

    Statistics_Free(&statF);
    Statistics_Free(&stat);

    printf("\n--- Integer modules (no float) ---\n");
    printf("StatInt_Map_I32(512, 0..1023 -> 0..255): %" PRId32 "\n", StatInt_Map_I32(512, 0, 1023, 0, 255));
    printf("StatInt_Clamp_I32(StatInt_Map_I32(1500, ...), 0, 255): %" PRId32 "\n", StatInt_Clamp_I32(StatInt_Map_I32(1500, 0, 1023, 0, 255), 0, 255));

    ExponentialSmoothingI32 ema;
    ExponentialSmoothingI32_Init(&ema, 2, EXPONENTIAL_SMOOTHING_I32_FRAC_DEFAULT, 0);
    for (int i = 0; i < 4; i++) {
        ExponentialSmoothingI32_Process(&ema, 100);
        printf("EMA step %d: %" PRId32 "\n", i + 1, ExponentialSmoothingI32_GetValue(&ema));
    }

    MeanAccumulatorI32 meanAcc;
    MeanAccumulatorI32_Reset(&meanAcc);
    MeanAccumulatorI32_Add(&meanAcc, -2);
    MeanAccumulatorI32_Add(&meanAcc, -3);
    printf("Mean of -2, -3: %" PRId32 "\n", MeanAccumulatorI32_Mean(&meanAcc));

    /* two channels sharing one time axis: y0 = 1000 + 3 t, y1 = -t */
    static uint32_t times[8];
    static int32_t values[8 * 2];
    static WindowRegressionI32Line lines[2];
    WindowRegressionI32 drift;
    WindowRegressionI32_Init(&drift, times, values, lines, 8, 2);
    for (uint32_t t = 0; t < 10; t++) {
        const int32_t y[2] = {1000 + 3 * (int32_t) t, -(int32_t) t};
        WindowRegressionI32_Push(&drift, t, y);
    }
    if (WindowRegressionI32_Fit(&drift, 4, 0)) {
        int32_t ahead;
        WindowRegressionI32_Evaluate(&drift, 0, 20, &ahead);
        printf("Drift slope ch0: %" PRId64 "/65536 per tick, value at t=20: %" PRId32 "\n", lines[0].slopeQ16, ahead);
    }
    return 0;
}