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

    reg->n += 1;
    reg->mean_x = reg->mean_x + (x - reg->mean_x) / reg->n;
    reg->mean_xx = reg->mean_xx + (x * x - reg->mean_xx) / reg->n;
    reg->mean_y = reg->mean_y + (y - reg->mean_y) / reg->n;
    reg->mean_yy = reg->mean_yy + (y * y - reg->mean_yy) / reg->n;
    reg->mean_xy = reg->mean_xy + (x * y - reg->mean_xy) / reg->n;
}

bool linReg_hasResult(linReg_t * reg)
{
    if (!reg || reg->n <= 2) {
        return false;
    }

    return true;
}

bool linReg_result(linReg_t * reg, float * pM, float * pB, float * pR, float * pJitter)
{
    if (!linReg_hasResult(reg)) {
        return false;
    }

    float var_x = reg->mean_xx - (reg->mean_x * reg->mean_x);
    float covar_xy = reg->mean_xy - (reg->mean_x * reg->mean_y);

    float m = var_x != 0 ? covar_xy / var_x : 0;

    if (pM) {
        *pM = m;
    }
    if (pB) {
        *pB = reg->mean_y - m * reg->mean_x;
    }

    if (pR) {
        float var_y = reg->mean_yy - (reg->mean_y * reg->mean_y);

        if (var_x == 0 || var_y == 0) {
            *pR = 1;
        } else {
            float std_x = sqrtf(var_x);
            float std_y = sqrtf(var_y);
            *pR = covar_xy / (std_x * std_y);
        }
    }

    if (pJitter) {
        float s_yy = (reg->mean_yy - reg->mean_y * reg->mean_y) * reg->n;
        float s_xx = (reg->mean_xx - reg->mean_x * reg->mean_x) * reg->n;
        *pJitter = sqrtf((s_yy - m * m * s_xx) / (reg->n - 2));
    }

    return true;
}

float linReg_mUncertainty(linReg_t * reg, int32_t k, float resY)
{
    if (!reg || reg->n <= 1) {
        return INFINITY;
    }

    // u(resY)^2 - uncertainty caused by resolution (power of two)
    float u2_resY = resY * resY / 3;

    if (reg->n <= 2) {
        return k * sqrtf(u2_resY / reg->mean_xx);
    }

    // uncertainty of Y measurements (power of two)
    float sigma2 = reg->mean_yy / (reg->n - 2);

    // combined and extended uncertainty
    return k * sqrtf((sigma2 + u2_resY) / reg->mean_xx);
}
