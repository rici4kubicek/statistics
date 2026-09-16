#ifndef EXPONENTIAL_SMOOTHING_H
#define EXPONENTIAL_SMOOTHING_H

/**
 * @file exponential_smoothing.h
 * @brief Public API for an integer-factor exponential smoothing (IIR) filter.
 *
 * The filter keeps an internal accumulator scaled by @ref ExponentialSmoothing::factor
 * so that the running average can be updated without repeated floating-point
 * division. This makes it cheap to run on every new sample.
 *
 * See @ref exponential_smoothing.c for the implementation details.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup exponential_smoothing_core Exponential smoothing filter
 * @brief Structures and functions for the exponential smoothing filter.
 * @{ */

/**
 * @brief Exponential smoothing filter state.
 */
typedef struct {
    /** Internal accumulator, holds the current value scaled by @ref factor. */
    float buffer;
    /** Smoothing factor: larger values smooth more but react slower. */
    int factor;
} ExponentialSmoothing;

/**
 * @brief Initialize the filter with a smoothing factor and starting value.
 *
 * @param[out] filter    Pointer to the filter instance to initialize.
 * @param      factor    Smoothing factor (larger = smoother, slower to react).
 * @param      initValue Initial filtered value.
 */
void ExponentialSmoothing_Init(ExponentialSmoothing * filter, int factor, float initValue);

/**
 * @brief Feed a new sample into the filter.
 *
 * @param filter Pointer to the filter instance.
 * @param sample New sample value.
 */
void ExponentialSmoothing_Process(ExponentialSmoothing * filter, float sample);

/**
 * @brief Get the current filtered value.
 *
 * @param filter Pointer to the filter instance.
 * @return Current filtered value, or 0 if @p filter is NULL.
 */
float ExponentialSmoothing_GetValue(ExponentialSmoothing * filter);

/** @} */ /* end of exponential_smoothing_core */

#ifdef __cplusplus
}
#endif

#endif /* EXPONENTIAL_SMOOTHING_H */
