/**
 * @file window_regression_i32.c
 * @brief Implementation of the integer sliding-window regression.
 */

#include "window_regression_i32.h"

#include <stddef.h>

#include "statistics_int_math.h"

/** @cond INTERNAL */

#define FRAC_BITS 16
#define FRAC_ONE  ((int64_t) 1 << FRAC_BITS)
/* ages are scaled down until the oldest one is below this, keeps the fit sums inside int64_t */
#define X_LIMIT ((uint32_t) 1 << 13)
/* |slope| < 2^41 and |dx| < 2^21 give a product below 2^62 */
#define SLOPE_FAST  ((uint64_t) 1 << 41)
#define DX_FAST     ((uint64_t) 1 << 21)
#define PRODUCT_SAT ((uint64_t) 1 << 62)

static bool isValid(const WindowRegressionI32 * reg)
{
    return reg && reg->capacity != 0;
}

/* a - b as a signed difference in [-2^31, 2^31), wrap-safe */
static int64_t signedDiff(uint32_t a, uint32_t b)
{
    const uint32_t d = a - b;
    return d <= (uint32_t) INT32_MAX ? (int64_t) d : (int64_t) d - ((int64_t) 1 << 32);
}

/** @endcond */

bool WindowRegressionI32_Init(WindowRegressionI32 * reg, uint32_t * xStorage, int32_t * yStorage,
                              WindowRegressionI32Line * lines, uint16_t capacity, uint8_t channels)
{
    if (!reg) {
        return false;
    }

    reg->head = 0;
    reg->count = 0;
    reg->fitted = false;
    reg->fitX = 0;
    if (!xStorage || !yStorage || !lines || capacity < 2 || capacity > WINDOW_REGRESSION_I32_CAPACITY_MAX || channels == 0) {
        reg->x = NULL;
        reg->y = NULL;
        reg->lines = NULL;
        reg->capacity = 0;
        reg->channels = 0;
        return false;
    }

    reg->x = xStorage;
    reg->y = yStorage;
    reg->lines = lines;
    reg->capacity = capacity;
    reg->channels = channels;
    return true;
}

void WindowRegressionI32_Push(WindowRegressionI32 * reg, uint32_t x, const int32_t y[])
{
    if (!isValid(reg) || !y) {
        return;
    }

    reg->x[reg->head] = x;
    for (uint8_t c = 0; c < reg->channels; c++) {
        reg->y[(uint32_t) c * reg->capacity + reg->head] = y[c];
    }
    if (++reg->head == reg->capacity) {
        reg->head = 0;
    }
    if (reg->count < reg->capacity) {
        reg->count++;
    }
}

uint16_t WindowRegressionI32_Count(const WindowRegressionI32 * reg)
{
    return isValid(reg) ? reg->count : 0;
}

void WindowRegressionI32_Clear(WindowRegressionI32 * reg)
{
    if (!isValid(reg)) {
        return;
    }

    reg->head = 0;
    reg->count = 0;
    reg->fitted = false;
}

bool WindowRegressionI32_Fit(WindowRegressionI32 * reg, uint16_t minCount, uint32_t slopeMaxQ16)
{
    if (!isValid(reg)) {
        return false;
    }

    reg->fitted = false;
    const uint16_t n = reg->count;
    if (n < 2 || n < minCount) {
        return false;
    }

    const uint16_t cap = reg->capacity;
    const uint16_t oldest = (uint16_t) (reg->head >= n ? reg->head - n : reg->head + cap - n);
    const uint16_t newest = (uint16_t) (reg->head == 0 ? cap - 1 : reg->head - 1);
    const uint32_t xNewest = reg->x[newest];
    const uint32_t oldestAge = xNewest - reg->x[oldest];

    unsigned shift = 0;
    while ((oldestAge >> shift) >= X_LIMIT) {
        shift++;
    }

    /* x sums, x = -(age >> shift) in (-2^13, 0] */
    int64_t sumAge = 0;
    int32_t sx = 0;
    int64_t sxx = 0;
    uint16_t slot = oldest;
    for (uint16_t i = 0; i < n; i++) {
        uint32_t age = xNewest - reg->x[slot];
        if (age > oldestAge) {
            age = oldestAge;
        }
        const int32_t x = -(int32_t) (age >> shift);
        sumAge += age;
        sx += x;
        sxx += x * x;
        if (++slot == cap) {
            slot = 0;
        }
    }

    const int64_t n64 = n;
    const int64_t den = n64 * sxx - (int64_t) sx * sx; /* n^2 var(x) <= 2^38 */
    if (den <= 0) {
        return false;
    }
    const int64_t denPerUnit = den * ((int64_t) 1 << shift); /* <= 2^57 */

    for (uint8_t c = 0; c < reg->channels; c++) {
        const int32_t * ring = &reg->y[(uint32_t) c * cap];
        const int32_t yNewest = ring[newest];
        int32_t sy = 0; /* |sy| <= n 2^20 <= 2^27 */
        int64_t sxy = 0;

        slot = oldest;
        for (uint16_t i = 0; i < n; i++) {
            uint32_t age = xNewest - reg->x[slot];
            if (age > oldestAge) {
                age = oldestAge;
            }
            const int32_t x = -(int32_t) (age >> shift);
            const int32_t dy = (int32_t) StatInt_Clamp_I64((int64_t) ring[slot] - yNewest,
                                                           -WINDOW_REGRESSION_I32_Y_DELTA_MAX, WINDOW_REGRESSION_I32_Y_DELTA_MAX);
            sy += dy;
            sxy += (int64_t) x * dy;
            if (++slot == cap) {
                slot = 0;
            }
        }

        int64_t slope = StatInt_DivRound((n64 * sxy - (int64_t) sx * sy) * FRAC_ONE, denPerUnit);
        if (slopeMaxQ16 != 0) {
            slope = StatInt_Clamp_I64(slope, -(int64_t) slopeMaxQ16, slopeMaxQ16);
        }

        /* line through the centroid: level(newest) = yNewest + mean(dy) + slope * mean(age) */
        reg->lines[c].slopeQ16 = slope;
        reg->lines[c].levelQ16 = (int64_t) yNewest * FRAC_ONE + StatInt_DivRound((int64_t) sy * FRAC_ONE + slope * sumAge, n64);
    }

    reg->fitted = true;
    reg->fitX = xNewest;
    return true;
}

bool WindowRegressionI32_Evaluate(const WindowRegressionI32 * reg, uint8_t channel, uint32_t x, int32_t * value)
{
    if (!isValid(reg) || !value || !reg->fitted || channel >= reg->channels) {
        return false;
    }

    const WindowRegressionI32Line * line = &reg->lines[channel];
    const int64_t dx = signedDiff(x, reg->fitX);
    const uint64_t uSlope = line->slopeQ16 < 0 ? (uint64_t) -line->slopeQ16 : (uint64_t) line->slopeQ16;
    const uint64_t uDx = dx < 0 ? (uint64_t) -dx : (uint64_t) dx;

    int64_t product;
    if ((uSlope < SLOPE_FAST && uDx < DX_FAST) || uSlope == 0 || uDx <= PRODUCT_SAT / uSlope) {
        product = line->slopeQ16 * dx;
    } else { /* far outside int32_t after the shift, saturate */
        product = ((line->slopeQ16 < 0) != (dx < 0)) ? -(int64_t) PRODUCT_SAT : (int64_t) PRODUCT_SAT;
    }

    const int64_t q = StatInt_FloorShift(line->levelQ16 + product + FRAC_ONE / 2, FRAC_BITS);
    *value = (int32_t) StatInt_Clamp_I64(q, INT32_MIN, INT32_MAX);
    return true;
}
