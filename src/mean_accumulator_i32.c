/**
 * @file mean_accumulator_i32.c
 * @brief Implementation of the integer mean accumulator.
 */

#include "mean_accumulator_i32.h"

#include <stddef.h>

#include "statistics_int_math.h"

void MeanAccumulatorI32_Reset(MeanAccumulatorI32 * acc)
{
    if (!acc) {
        return;
    }

    acc->sum = 0;
    acc->count = 0;
}

bool MeanAccumulatorI32_Add(MeanAccumulatorI32 * acc, int32_t sample)
{
    if (!acc || acc->count == UINT32_MAX) {
        return false;
    }

    acc->sum += sample;
    acc->count++;
    return true;
}

uint32_t MeanAccumulatorI32_Count(const MeanAccumulatorI32 * acc)
{
    return acc ? acc->count : 0;
}

int32_t MeanAccumulatorI32_Mean(const MeanAccumulatorI32 * acc)
{
    if (!acc || acc->count == 0) {
        return 0;
    }

    /* the mean of int32_t samples always fits int32_t */
    return (int32_t) StatInt_DivRound(acc->sum, acc->count);
}
