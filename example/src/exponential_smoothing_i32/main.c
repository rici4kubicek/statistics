/* Fixed-point exponential smoothing: step response with alpha = 1/4 */
#include "exponential_smoothing_i32.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    ExponentialSmoothingI32 ema;
    ExponentialSmoothingI32_Init(&ema, 2, EXPONENTIAL_SMOOTHING_I32_FRAC_DEFAULT, 0);
    for (int i = 0; i < 4; i++) {
        ExponentialSmoothingI32_Process(&ema, 100);
        printf("EMA step %d: %" PRId32 "\n", i + 1, ExponentialSmoothingI32_GetValue(&ema));
    }

    return 0;
}
