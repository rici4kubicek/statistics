/* Sliding-window line fit over two channels sharing one time axis */
#include "window_regression_i32.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
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
