/**
 * @file exponential_smoothing.c
 * @brief Implementation of the exponential smoothing filter.
 */

#include "exponential_smoothing.h"

void ExponentialSmoothing_Init(ExponentialSmoothing * filter, int factor, float initValue)
{
    if (!filter) {
        return;
    }

    if (factor <= 0) {
        filter->factor = 0;
        filter->buffer = 0;
        return;
    }

    filter->factor = factor;
    filter->buffer = initValue * factor;
}

void ExponentialSmoothing_Process(ExponentialSmoothing * filter, float sample)
{
    if (!filter || filter->factor <= 0) {
        return;
    }

    filter->buffer = filter->buffer - filter->buffer / filter->factor + sample;
}

float ExponentialSmoothing_GetValue(ExponentialSmoothing * filter)
{
    if (!filter || filter->factor <= 0) {
        return 0;
    }

    return filter->buffer / filter->factor;
}
