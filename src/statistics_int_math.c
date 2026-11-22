/**
 * @file statistics_int_math.c
 * @brief Implementation of the integer math helpers.
 */

#include "statistics_int_math.h"

int64_t StatInt_FloorShift(int64_t value, unsigned shift)
{
    if (shift > 63) {
        shift = 63;
    }
    if (value >= 0) {
        return value >> shift;
    }
    /* ~value == -value - 1 >= 0, so floor(value / 2^s) == ~(~value >> s) without overflow */
    return ~(~value >> shift);
}

int64_t StatInt_DivRound(int64_t value, int64_t divisor)
{
    if (divisor <= 0) {
        return 0;
    }

    const uint64_t d = (uint64_t) divisor;
    /* magnitude in unsigned arithmetic, also valid for INT64_MIN */
    const uint64_t mag = value < 0 ? (uint64_t) 0 - (uint64_t) value : (uint64_t) value;
    uint64_t q = mag / d;
    const uint64_t r = mag - q * d;

    if (r >= d - r) { /* r >= d / 2 rounded up, i.e. half away from zero */
        q++;
    }
    if (value >= 0 || q == 0) {
        return (int64_t) q;
    }
    /* 0 < q <= 2^63 here, -(q - 1) - 1 avoids negating 2^63 in int64_t */
    return -(int64_t) (q - 1) - 1;
}

uint32_t StatInt_Isqrt64(uint64_t value)
{
    uint64_t result = 0;
    uint64_t bit = (uint64_t) 1 << 62;

    /* start at the highest power of four <= value */
    while (bit > value) {
        bit >>= 2;
    }
    while (bit != 0) {
        if (value >= result + bit) {
            value -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    return (uint32_t) result;
}

int32_t StatInt_Clamp_I32(int32_t value, int32_t lo, int32_t hi)
{
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

int64_t StatInt_Clamp_I64(int64_t value, int64_t lo, int64_t hi)
{
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

uint32_t StatInt_Clamp_U32(uint32_t value, uint32_t lo, uint32_t hi)
{
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

int32_t StatInt_Map_I32(int32_t value, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax)
{
    int64_t a = (int64_t) value - inMin;
    int64_t d = (int64_t) inMax - inMin;
    const int64_t b = (int64_t) outMax - outMin;

    if (d == 0 || a == 0 || b == 0) {
        return outMin;
    }
    if (d < 0) {
        d = -d;
        a = -a;
    }

    /* |a|, |b|, d < 2^32: work on magnitudes in uint64_t, a * b / d = q * |b| + r * |b| / d */
    const int negative = (a < 0) != (b < 0);
    const uint64_t ua = a < 0 ? (uint64_t) -a : (uint64_t) a;
    const uint64_t ub = b < 0 ? (uint64_t) -b : (uint64_t) b;
    const uint64_t ud = (uint64_t) d;

    uint64_t q = 0;
    uint64_t r = ua;
    if (ua >= ud) { /* only when value is at or beyond inMax: skips a division in range */
        q = ua / ud;
        r = ua - q * ud;
    }
    /* r < d and |b| < 2^32, so r * |b| < 2^64 */
    const uint64_t rb = r * ub;
    uint64_t f = rb / ud;
    const uint64_t rem = rb - f * ud;
    if (rem >= ud - rem) {
        f++;
    }
    /* q * |b| <= (2^32 - 1)^2 and f <= |b|, the sum fits uint64_t */
    uint64_t mag = q * ub + f;

    /* any magnitude above 2^33 is outside int32_t whatever outMin is */
    if (mag > ((uint64_t) 1 << 33)) {
        mag = (uint64_t) 1 << 33;
    }
    const int64_t result = (int64_t) outMin + (negative ? -(int64_t) mag : (int64_t) mag);
    return (int32_t) StatInt_Clamp_I64(result, INT32_MIN, INT32_MAX);
}
