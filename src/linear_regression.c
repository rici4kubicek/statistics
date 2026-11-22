/**
 * @file linear_regression.c
 * @brief Implementation of the online linear regression.
 */

#include "linear_regression.h"
#include <math.h>
#include <string.h>

void linReg_clear(linReg_t * reg)
{
    if (!reg) {
        return;
    }

    memset(reg, 0, sizeof(*reg));
}

void linReg_process(linReg_t * reg, float x, float y)
{
    if (!reg) {
        return;
    }

    // Welford update of the means and the centered sums
    reg->n += 1;
    float dx = x - reg->mean_x;
    float dy = y - reg->mean_y;
    reg->mean_x += dx / reg->n;
    reg->mean_y += dy / reg->n;
    float dy2 = y - reg->mean_y;
    reg->s_xx += dx * (x - reg->mean_x);
    reg->s_yy += dy * dy2;
    reg->s_xy += dx * dy2;
}

bool linReg_hasResult(linReg_t * reg)
{
    if (!reg || reg->n <= 2) {
        return false;
    }

    return true;
}

/* residual sum of squares of the fitted line, rounding of a perfect fit can make it slightly negative */
static float residualSumOfSquares(const linReg_t * reg)
{
    float sse = reg->s_yy - reg->s_xy * reg->s_xy / reg->s_xx;
    return sse > 0 ? sse : 0;
}

bool linReg_result(linReg_t * reg, float * pM, float * pB, float * pR, float * pJitter)
{
    if (!linReg_hasResult(reg)) {
        return false;
    }

    float m = reg->s_xx != 0 ? reg->s_xy / reg->s_xx : 0;

    if (pM) {
        *pM = m;
    }
    if (pB) {
        *pB = reg->mean_y - m * reg->mean_x;
    }

    if (pR) {
        if (reg->s_xx == 0 || reg->s_yy == 0) {
            *pR = 1;
        } else {
            *pR = reg->s_xy / sqrtf(reg->s_xx * reg->s_yy);
        }
    }

    if (pJitter) {
        float sse = reg->s_xx != 0 ? residualSumOfSquares(reg) : reg->s_yy;
        *pJitter = sqrtf(sse / (reg->n - 2));
    }

    return true;
}

float linReg_mUncertainty(linReg_t * reg, int32_t k, float resY)
{
    if (!reg || reg->n <= 1 || reg->s_xx <= 0) {
        return INFINITY;
    }

    // u(resY)^2 - uncertainty caused by resolution (power of two)
    float u2_resY = resY * resY / 3;

    if (reg->n <= 2) {
        return k * sqrtf(u2_resY / reg->s_xx);
    }

    // residual variance of Y around the fitted line
    float sigma2 = residualSumOfSquares(reg) / (reg->n - 2);

    // combined and extended uncertainty
    return k * sqrtf((sigma2 + u2_resY) / reg->s_xx);
}
