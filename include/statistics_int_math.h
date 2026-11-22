#ifndef STATISTICS_INT_MATH_H
#define STATISTICS_INT_MATH_H

/**
 * @file statistics_int_math.h
 * @brief Integer helpers for MCUs without FPU: shifts, rounding division,
 *        square root, clamping and linear range mapping.
 *
 * All integer helpers are defined for the full range of their argument types
 * (no undefined behavior, no overflow) unless a precondition below says
 * otherwise. They use no floating point and no dynamic memory.
 *
 * The float variants (`StatFloat_Clamp`, `StatFloat_Map`) are only declared when
 * `STATISTICS_FLOAT_ENABLED` is non-zero and live in a separate translation
 * unit, so linking the integer helpers never pulls in soft-float code.
 *
 * See @ref statistics_int_math.c for the implementation details.
 */

#include <stdint.h>

#include "statistics_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup statistics_int_math Integer math helpers
 * @brief Fixed-point friendly helpers without float or malloc.
 * @{ */

/**
 * @brief Arithmetic shift right with floor rounding: floor(value / 2^shift).
 *
 * Unlike `value >> shift`, the result does not depend on the
 * implementation-defined right shift of negative values.
 * Example: `StatInt_FloorShift(-1, 4) == -1`, `StatInt_FloorShift(-16, 4) == -1`,
 * `StatInt_FloorShift(-17, 4) == -2`.
 *
 * @param value Value to shift (full int64_t range).
 * @param shift Shift count; values above 63 behave like 63
 *              (result 0 for value >= 0, -1 for value < 0).
 * @return floor(value / 2^shift).
 */
int64_t StatInt_FloorShift(int64_t value, unsigned shift);

/**
 * @brief Integer division rounded half away from zero.
 *
 * Example: `StatInt_DivRound(5, 2) == 3`, `StatInt_DivRound(-5, 2) == -3`, `StatInt_DivRound(4, 3) == 1`.
 * Works for the full int64_t range of @p value. On Cortex-M0+ one call costs
 * one `__aeabi_uldivmod`.
 *
 * @param value   Dividend (full int64_t range).
 * @param divisor Divisor, precondition: divisor > 0. Returns 0 when violated.
 * @return round(value / divisor).
 */
int64_t StatInt_DivRound(int64_t value, int64_t divisor);

/**
 * @brief Integer square root: floor(sqrt(value)).
 *
 * @param value Radicand (full uint64_t range).
 * @return floor(sqrt(value)), always fits uint32_t.
 */
uint32_t StatInt_Isqrt64(uint64_t value);

/**
 * @brief Clamp a value into [lo, hi].
 *
 * Precondition: lo <= hi. When violated the result is lo for value < lo,
 * otherwise hi for value > hi, otherwise value (no undefined behavior).
 *
 * @param value Value to clamp.
 * @param lo    Lower bound.
 * @param hi    Upper bound.
 * @return @p value limited to [lo, hi].
 */
int32_t StatInt_Clamp_I32(int32_t value, int32_t lo, int32_t hi);

/** @copydoc StatInt_Clamp_I32 */
int64_t StatInt_Clamp_I64(int64_t value, int64_t lo, int64_t hi);

/** @copydoc StatInt_Clamp_I32 */
uint32_t StatInt_Clamp_U32(uint32_t value, uint32_t lo, uint32_t hi);

/**
 * @brief Linear range mapping (Arduino `map()` style) with rounding.
 *
 * Returns outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin),
 * with the division rounded half away from zero (see @ref StatInt_DivRound).
 *
 * - Reversed ranges are allowed (inMin > inMax and/or outMin > outMax).
 * - inMin == inMax returns outMin.
 * - The output is NOT clamped to [outMin, outMax]: a value outside the input
 *   range is extrapolated. Combine with @ref StatInt_Clamp_I32 when needed.
 * - A result outside the int32_t range saturates to INT32_MIN / INT32_MAX.
 * - No overflow for any int32_t arguments (64-bit intermediates; at most two
 *   unsigned 64-bit divisions per call).
 *
 * @param value  Input value.
 * @param inMin  Input value mapped to @p outMin.
 * @param inMax  Input value mapped to @p outMax.
 * @param outMin Output for @p inMin.
 * @param outMax Output for @p inMax.
 * @return Mapped value.
 */
int32_t StatInt_Map_I32(int32_t value, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax);

#if STATISTICS_FLOAT_ENABLED
/**
 * @brief Clamp a float into [lo, hi]. Same rules as @ref StatInt_Clamp_I32.
 *
 * A NaN @p value is returned unchanged.
 *
 * @param value Value to clamp.
 * @param lo    Lower bound.
 * @param hi    Upper bound.
 * @return @p value limited to [lo, hi].
 */
float StatFloat_Clamp(float value, float lo, float hi);

/**
 * @brief Linear range mapping for floats. Same rules as @ref StatInt_Map_I32
 *        (inMin == inMax returns outMin, no output clamping), no rounding.
 *
 * @param value  Input value.
 * @param inMin  Input value mapped to @p outMin.
 * @param inMax  Input value mapped to @p outMax.
 * @param outMin Output for @p inMin.
 * @param outMax Output for @p inMax.
 * @return Mapped value.
 */
float StatFloat_Map(float value, float inMin, float inMax, float outMin, float outMax);
#endif

/** @} */ /* end of statistics_int_math */

#ifdef __cplusplus
}
#endif

#endif /* STATISTICS_INT_MATH_H */
