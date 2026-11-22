#ifndef LINEAR_REGRESSION_H
#define LINEAR_REGRESSION_H

/**
 * @file linear_regression.h
 * @brief Public API for an online (streaming) linear regression.
 *
 * The regression accumulates the running means of x and y and the centered
 * sums Sxx, Syy and Sxy (Welford's online algorithm) so that slope,
 * intercept, correlation coefficient, jitter and slope uncertainty can be
 * derived at any point without storing individual samples. The centered sums
 * keep the results stable when the data has a large constant offset.
 *
 * See @ref linear_regression.c for the implementation details.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup linear_regression_core Linear regression
 * @brief Structures and functions for online linear regression.
 * @{ */

/**
 * @brief Online linear regression state.
 */
typedef struct {
    /** Running mean of x. */
    float mean_x;
    /** Running mean of y. */
    float mean_y;
    /** Centered sum of squares of x: sum (x - mean_x)^2. */
    float s_xx;
    /** Centered sum of squares of y: sum (y - mean_y)^2. */
    float s_yy;
    /** Centered sum of products: sum (x - mean_x)(y - mean_y). */
    float s_xy;
    /** Number of processed points. */
    float n;
} linReg_t;

/**
 * @brief Clear intermediate calculations and start a new regression.
 * @param reg Pointer to the regression instance.
 */
void linReg_clear(linReg_t * reg);

/**
 * @brief Add one X/Y point to the linear regression.
 * @param reg Pointer to the regression instance.
 * @param x   X value.
 * @param y   Y value.
 */
void linReg_process(linReg_t * reg, float x, float y);

/**
 * @brief Check whether enough samples have been added to compute a result.
 * @param reg Pointer to the regression instance.
 * @return true if sufficient samples are present.
 */
bool linReg_hasResult(linReg_t * reg);

/**
 * @brief Calculate current regression parameters.
 *
 * y = m*x + b
 *
 * @param reg     Pointer to the regression instance.
 * @param pM      Pointer to m - slope (can be NULL).
 * @param pB      Pointer to b - intercept (can be NULL).
 * @param pR      Pointer to r - correlation coefficient (can be NULL).
 * @param pJitter Pointer to regression jitter (can be NULL).
 * @return true if sufficient number of samples are present to calculate m, b and jitter.
 */
bool linReg_result(linReg_t * reg, float * pM, float * pB, float * pR, float * pJitter);

/**
 * @brief Calculate uncertainty of the slope.
 *
 * Uses the centered statistics of the samples, so the result does not change
 * when all X or all Y values are shifted by a constant:
 * Sxx = sum (x - mean_x)^2, SSE = Syy - Sxy^2 / Sxx (clamped to >= 0),
 * U = k * sqrt((SSE / (n - 2) + resY^2 / 3) / Sxx) for n > 2 and
 * U = k * sqrt((resY^2 / 3) / Sxx) for n == 2.
 *
 * @param reg  Pointer to the regression instance.
 * @param k    Extended uncertainty factor.
 * @param resY Resolution of Y values.
 * @return Uncertainty of the slope, INFINITY for n <= 1 or when all X values
 *         are equal (Sxx == 0).
 */
float linReg_mUncertainty(linReg_t * reg, int32_t k, float resY);

/** @} */ /* end of linear_regression_core */

#ifdef __cplusplus
}
#endif

#endif /* LINEAR_REGRESSION_H */
