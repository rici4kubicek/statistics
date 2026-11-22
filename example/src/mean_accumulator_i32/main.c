/* Integer mean with rounding half away from zero */
#include "mean_accumulator_i32.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    MeanAccumulatorI32 meanAcc;
    MeanAccumulatorI32_Reset(&meanAcc);
    MeanAccumulatorI32_Add(&meanAcc, -2);
    MeanAccumulatorI32_Add(&meanAcc, -3);
    printf("Mean of -2, -3: %" PRId32 "\n", MeanAccumulatorI32_Mean(&meanAcc));

    return 0;
}
