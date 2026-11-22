#ifndef WINDOW_REGRESSION_I32_H
#define WINDOW_REGRESSION_I32_H

/**
 * @file window_regression_i32.h
 * @brief Public API for an integer sliding-window least-squares line fit
 *        over several y channels that share one x (time) axis.
 *
 * Samples are kept in caller-owned ring buffers (no malloc). When the window
 * is full a push overwrites the oldest sample. A fit computes, per channel, the
 * slope (Q16 per x unit) and the level (Q16 at the newest x) of the
 * least-squares line; @ref WindowRegressionI32_Evaluate gives the line value
 * at any x.
 *
 * Method and bounds (all sums are int64_t, no overflow):
 * - x is taken relative to the newest x as a wrap-safe uint32_t difference
 *   (age = xNewest - x), so x may wrap around 2^32. Precondition: x is pushed
 *   in non-decreasing order (mod 2^32); an age above the oldest age is clamped to it.
 * - Ages are scaled down by 2^shift until the oldest one is < 2^13, so the
 *   window may span up to 2^32 - 1 x units.
 * - y is taken relative to the newest y of its channel. Input range:
 *   |y - yNewest| <= 2^20 (@ref WINDOW_REGRESSION_I32_Y_DELTA_MAX); larger
 *   differences are clamped to it.
 * - With n <= @ref WINDOW_REGRESSION_I32_CAPACITY_MAX = 128:
 *   |n Sxy - Sx Sy| <= n^2 2^32 <= 2^46, so the Q16 numerator stays <= 2^62.
 * - The fit uses two int64_t divisions per channel and none inside the
 *   per-sample loops.
 * - With a clamped slope the line still goes through the centroid
 *   (mean x, mean y), so the mean level stays right.
 * - Precision: the slope resolution is 2^-16 y units per x unit; a smaller
 *   slope rounds to 0 and the line becomes the mean level. With a span below
 *   2^13 the fit is exact up to the Q16 rounding. With a longer span the
 *   scaled-down ages are truncated; random tests over the full input range
 *   showed level and slope * span errors up to about 2^-13 of the y range.
 *
 * See @ref window_regression_i32.c for the implementation details.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup window_regression_i32_core Integer sliding-window regression
 * @brief Structures and functions for the integer sliding-window line fit.
 * @{ */

/** Largest supported window capacity (keeps the fit sums inside int64_t). */
#define WINDOW_REGRESSION_I32_CAPACITY_MAX 128

/** Largest accepted |y - yNewest|, larger differences are clamped. */
#define WINDOW_REGRESSION_I32_Y_DELTA_MAX ((int32_t) 1 << 20)

/**
 * @brief Fitted line of one channel: y(x) = level + slope * (x - fitX).
 */
typedef struct {
    /** Slope in Q16 per x unit (y units * 2^16 / x unit). */
    int64_t slopeQ16;
    /** Line value at @ref WindowRegressionI32::fitX in Q16. */
    int64_t levelQ16;
} WindowRegressionI32Line;

/**
 * @brief Sliding-window regression state. Treat the fields as read-only.
 */
typedef struct {
    /** x ring, @ref capacity elements. */
    uint32_t * x;
    /** y rings, @ref capacity * @ref channels elements, channel c slot i at y[c * capacity + i]. */
    int32_t * y;
    /** Fit result per channel, @ref channels elements. */
    WindowRegressionI32Line * lines;
    /** Window capacity, 0 = invalid instance. */
    uint16_t capacity;
    /** Next slot to write. */
    uint16_t head;
    /** Stored samples. */
    uint16_t count;
    /** Number of y channels. */
    uint8_t channels;
    /** true when @ref lines hold the result of the last successful fit. */
    bool fitted;
    /** Newest x at the last successful fit, the x of @ref WindowRegressionI32Line::levelQ16. */
    uint32_t fitX;
} WindowRegressionI32;

/**
 * @brief Initialize the regression with caller-owned storage.
 *
 * @param[out] reg       Pointer to the instance to initialize.
 * @param      xStorage  x ring, @p capacity elements.
 * @param      yStorage  y rings, @p capacity * @p channels elements.
 * @param      lines     Fit results, @p channels elements.
 * @param      capacity  Window size, 2..@ref WINDOW_REGRESSION_I32_CAPACITY_MAX.
 * @param      channels  Number of y channels, >= 1.
 * @return true on success; false on NULL storage or an out-of-range
 *         parameter (the instance is then invalid and all calls are no-ops).
 */
bool WindowRegressionI32_Init(WindowRegressionI32 * reg, uint32_t * xStorage, int32_t * yStorage,
                              WindowRegressionI32Line * lines, uint16_t capacity, uint8_t channels);

/**
 * @brief Add one sample: x and one y per channel. Overwrites the oldest
 *        sample when the window is full. Does not change the last fit result.
 *
 * @param reg Pointer to the instance.
 * @param x   Sample x (time), non-decreasing mod 2^32.
 * @param y   @ref WindowRegressionI32::channels y values.
 */
void WindowRegressionI32_Push(WindowRegressionI32 * reg, uint32_t x, const int32_t y[]);

/**
 * @brief Number of stored samples.
 *
 * @param reg Pointer to the instance.
 * @return Stored samples, 0 if @p reg is NULL or invalid.
 */
uint16_t WindowRegressionI32_Count(const WindowRegressionI32 * reg);

/**
 * @brief Drop all samples and the fit result.
 *
 * @param reg Pointer to the instance.
 */
void WindowRegressionI32_Clear(WindowRegressionI32 * reg);

/**
 * @brief Fit a least-squares line per channel over the stored samples.
 *
 * On success the result is in @ref WindowRegressionI32::lines and
 * @ref WindowRegressionI32::fitX is the newest x.
 *
 * @param reg         Pointer to the instance.
 * @param minCount    Minimum stored samples for a result (values < 2 act as 2).
 * @param slopeMaxQ16 |slope| limit in Q16 per x unit, 0 = no limit. A clamped
 *                    line still goes through the centroid of the samples.
 * @return true on success; false when fewer than @p minCount samples are
 *         stored or all stored x are equal (no result, fitted = false).
 */
bool WindowRegressionI32_Fit(WindowRegressionI32 * reg, uint16_t minCount, uint32_t slopeMaxQ16);

/**
 * @brief Value of the last fitted line of a channel at @p x.
 *
 * @p x is taken relative to @ref WindowRegressionI32::fitX as a wrap-safe
 * signed difference, so it must lie within +-2^31 of it. The result is
 * rounded half up and saturates to the int32_t range.
 *
 * @param reg     Pointer to the instance.
 * @param channel Channel index.
 * @param x       x to evaluate the line at.
 * @param[out] value Line value at @p x.
 * @return true on success; false if there is no fit result, @p channel is out
 *         of range or a pointer is NULL (@p value is not written).
 */
bool WindowRegressionI32_Evaluate(const WindowRegressionI32 * reg, uint8_t channel, uint32_t x, int32_t * value);

/** @} */ /* end of window_regression_i32_core */

#ifdef __cplusplus
}
#endif

#endif /* WINDOW_REGRESSION_I32_H */
