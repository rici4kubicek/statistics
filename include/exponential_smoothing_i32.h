#ifndef EXPONENTIAL_SMOOTHING_I32_H
#define EXPONENTIAL_SMOOTHING_I32_H

/**
 * @file exponential_smoothing_i32.h
 * @brief Public API for a fixed-point exponential smoothing (EMA) filter
 *        without floating point.
 *
 * The filter keeps the value in an int32_t accumulator in Q@p frac fixed point
 * and updates it with a shift instead of a division:
 *
 *     acc += StatInt_FloorShift((sample << frac) - acc, shift)
 *
 * which is an EMA with alpha = 1 / 2^shift. The accumulator always stays
 * between its previous value and the new scaled sample, so it never overflows.
 *
 * See @ref exponential_smoothing_i32.c for the implementation details.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup exponential_smoothing_i32_core Fixed-point exponential smoothing filter
 * @brief Structures and functions for the integer exponential smoothing filter.
 * @{ */

/** Default number of fractional bits of the accumulator. */
#define EXPONENTIAL_SMOOTHING_I32_FRAC_DEFAULT 8

/** Largest accepted @p shift. */
#define EXPONENTIAL_SMOOTHING_I32_SHIFT_MAX 31

/** Largest accepted @p frac. */
#define EXPONENTIAL_SMOOTHING_I32_FRAC_MAX 30

/**
 * @brief Fixed-point exponential smoothing filter state.
 */
typedef struct {
    /** Filtered value in Q@ref frac fixed point. */
    int32_t acc;
    /** alpha = 1 / 2^shift: larger values smooth more but react slower. */
    uint8_t shift;
    /** Fractional bits of @ref acc. */
    uint8_t frac;
    /** false after an Init with invalid parameters. */
    bool valid;
} ExponentialSmoothingI32;

/**
 * @brief Initialize the filter.
 *
 * Valid input range for samples and @p initValue is
 * [INT32_MIN / 2^frac, INT32_MAX / 2^frac] (frac = 8: +-8388607); values
 * outside are clamped to it.
 *
 * An invalid parameter (@p shift > @ref EXPONENTIAL_SMOOTHING_I32_SHIFT_MAX,
 * @p frac > @ref EXPONENTIAL_SMOOTHING_I32_FRAC_MAX) leaves the filter in an
 * invalid state where @ref ExponentialSmoothingI32_Process and
 * @ref ExponentialSmoothingI32_Reset are no-ops and
 * @ref ExponentialSmoothingI32_GetValue returns 0.
 *
 * @param[out] filter    Pointer to the filter instance to initialize.
 * @param      shift     alpha = 1 / 2^shift, 0 = no smoothing.
 * @param      frac      Fractional bits of the accumulator, usually
 *                       @ref EXPONENTIAL_SMOOTHING_I32_FRAC_DEFAULT.
 * @param      initValue Initial filtered value.
 */
void ExponentialSmoothingI32_Init(ExponentialSmoothingI32 * filter, unsigned shift, unsigned frac, int32_t initValue);

/**
 * @brief Feed a new sample into the filter.
 *
 * @param filter Pointer to the filter instance.
 * @param sample New sample, clamped to the input range (see @ref ExponentialSmoothingI32_Init).
 */
void ExponentialSmoothingI32_Process(ExponentialSmoothingI32 * filter, int32_t sample);

/**
 * @brief Get the current filtered value rounded to an integer (half up, toward +infinity).
 *
 * @param filter Pointer to the filter instance.
 * @return Filtered value, or 0 if @p filter is NULL or invalid.
 */
int32_t ExponentialSmoothingI32_GetValue(const ExponentialSmoothingI32 * filter);

/**
 * @brief Set the filtered value, keeping shift and frac.
 *
 * @param filter Pointer to the filter instance.
 * @param value  New filtered value, clamped to the input range.
 */
void ExponentialSmoothingI32_Reset(ExponentialSmoothingI32 * filter, int32_t value);

/**
 * @brief Change the smoothing factor of a running filter.
 *
 * Keeps the accumulator and @ref ExponentialSmoothingI32::frac unchanged, so
 * the filtered value does not jump. The new alpha = 1 / 2^shift applies from
 * the next @ref ExponentialSmoothingI32_Process; the time constant is about
 * 2^shift samples. Example: track fast with shift 2, then switch to shift 7.
 *
 * @param filter Pointer to the filter instance.
 * @param shift  New shift, <= @ref EXPONENTIAL_SMOOTHING_I32_SHIFT_MAX.
 * @return true on success; false if @p filter is NULL or invalid or @p shift
 *         is out of range (the filter is then unchanged).
 */
bool ExponentialSmoothingI32_SetShift(ExponentialSmoothingI32 * filter, unsigned shift);

/**
 * @brief Current smoothing shift (alpha = 1 / 2^shift).
 *
 * @param filter Pointer to the filter instance.
 * @return Shift, 0 if @p filter is NULL or invalid.
 */
unsigned ExponentialSmoothingI32_GetShift(const ExponentialSmoothingI32 * filter);

/** @} */ /* end of exponential_smoothing_i32_core */

#ifdef __cplusplus
}
#endif

#endif /* EXPONENTIAL_SMOOTHING_I32_H */
