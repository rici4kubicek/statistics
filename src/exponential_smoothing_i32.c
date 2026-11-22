/**
 * @file exponential_smoothing_i32.c
 * @brief Implementation of the fixed-point exponential smoothing filter.
 */

#include "exponential_smoothing_i32.h"

#include <stddef.h>

#include "statistics_int_math.h"

/* sample << frac, with the sample clamped so that the result fits int32_t */
static int32_t toFixed(const ExponentialSmoothingI32 * filter, int32_t value)
{
    const int32_t lo = (int32_t) StatInt_FloorShift(INT32_MIN, filter->frac);
    const int32_t hi = (int32_t) StatInt_FloorShift(INT32_MAX, filter->frac);
    return (int32_t) ((int64_t) StatInt_Clamp_I32(value, lo, hi) * ((int64_t) 1 << filter->frac));
}

void ExponentialSmoothingI32_Init(ExponentialSmoothingI32 * filter, unsigned shift, unsigned frac, int32_t initValue)
{
    if (!filter) {
        return;
    }

    filter->acc = 0;
    filter->valid = shift <= EXPONENTIAL_SMOOTHING_I32_SHIFT_MAX && frac <= EXPONENTIAL_SMOOTHING_I32_FRAC_MAX;
    if (!filter->valid) {
        filter->shift = 0;
        filter->frac = 0;
        return;
    }

    filter->shift = (uint8_t) shift;
    filter->frac = (uint8_t) frac;
    filter->acc = toFixed(filter, initValue);
}

void ExponentialSmoothingI32_Process(ExponentialSmoothingI32 * filter, int32_t sample)
{
    if (!filter || !filter->valid) {
        return;
    }

    /* the step (up to 2^32 - 1 for shift 0) never overshoots the target, so the sum fits int32_t */
    filter->acc = (int32_t) (filter->acc + StatInt_FloorShift((int64_t) toFixed(filter, sample) - filter->acc, filter->shift));
}

int32_t ExponentialSmoothingI32_GetValue(const ExponentialSmoothingI32 * filter)
{
    if (!filter || !filter->valid) {
        return 0;
    }

    const int64_t half = filter->frac ? (int64_t) 1 << (filter->frac - 1) : 0;
    return (int32_t) StatInt_FloorShift((int64_t) filter->acc + half, filter->frac);
}

void ExponentialSmoothingI32_Reset(ExponentialSmoothingI32 * filter, int32_t value)
{
    if (!filter || !filter->valid) {
        return;
    }

    filter->acc = toFixed(filter, value);
}

bool ExponentialSmoothingI32_SetShift(ExponentialSmoothingI32 * filter, unsigned shift)
{
    if (!filter || !filter->valid || shift > EXPONENTIAL_SMOOTHING_I32_SHIFT_MAX) {
        return false;
    }

    filter->shift = (uint8_t) shift;
    return true;
}

unsigned ExponentialSmoothingI32_GetShift(const ExponentialSmoothingI32 * filter)
{
    return (filter && filter->valid) ? filter->shift : 0;
}
