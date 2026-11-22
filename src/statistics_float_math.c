/**
 * @file statistics_float_math.c
 * @brief Float variants of the helpers in statistics_int_math.h.
 *
 * Kept in a separate translation unit so that the integer helpers never pull
 * soft-float code into builds for MCUs without FPU.
 */

#include "statistics_int_math.h"

#if STATISTICS_FLOAT_ENABLED

float StatFloat_Clamp(float value, float lo, float hi)
{
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

float StatFloat_Map(float value, float inMin, float inMax, float outMin, float outMax)
{
    if (inMin == inMax) {
        return outMin;
    }
    return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}

#endif /* STATISTICS_FLOAT_ENABLED */
