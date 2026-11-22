#ifndef MEAN_ACCUMULATOR_I32_H
#define MEAN_ACCUMULATOR_I32_H

/**
 * @file mean_accumulator_i32.h
 * @brief Public API for an integer running mean (sum + count) without
 *        floating point or stored samples.
 *
 * The int64_t sum cannot overflow: |sum| <= 2^31 * (2^32 - 1) < 2^63 for
 * any int32_t samples up to the maximum count of UINT32_MAX.
 *
 * See @ref mean_accumulator_i32.c for the implementation details.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup mean_accumulator_i32_core Integer mean accumulator
 * @brief Structures and functions for the integer mean accumulator.
 * @{ */

/**
 * @brief Integer mean accumulator state.
 */
typedef struct {
    /** Sum of the added samples. */
    int64_t sum;
    /** Number of added samples. */
    uint32_t count;
} MeanAccumulatorI32;

/**
 * @brief Clear the accumulator (sum and count = 0). Also serves as init.
 *
 * @param acc Pointer to the accumulator instance.
 */
void MeanAccumulatorI32_Reset(MeanAccumulatorI32 * acc);

/**
 * @brief Add one sample.
 *
 * @param acc    Pointer to the accumulator instance.
 * @param sample Sample value.
 * @return true if added, false if @p acc is NULL or already holds UINT32_MAX samples.
 */
bool MeanAccumulatorI32_Add(MeanAccumulatorI32 * acc, int32_t sample);

/**
 * @brief Number of added samples.
 *
 * @param acc Pointer to the accumulator instance.
 * @return Sample count, 0 if @p acc is NULL.
 */
uint32_t MeanAccumulatorI32_Count(const MeanAccumulatorI32 * acc);

/**
 * @brief Mean of the added samples, rounded half away from zero.
 *
 * @param acc Pointer to the accumulator instance.
 * @return Rounded mean, 0 if @p acc is NULL or empty.
 */
int32_t MeanAccumulatorI32_Mean(const MeanAccumulatorI32 * acc);

/** @} */ /* end of mean_accumulator_i32_core */

#ifdef __cplusplus
}
#endif

#endif /* MEAN_ACCUMULATOR_I32_H */
