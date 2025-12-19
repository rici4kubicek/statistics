/**
 * @file statistics.c
 * @brief Implementation of the lightweight statistics helper.
 *
 * This module provides:
 *  - Initialization and deinitialization of a samples buffer
 *  - Appending a new sample into the buffer
 *  - Computing basic statistics for specific, supported types via
 *    macro-generated functions
 *
 * Notes:
 *  - Memory functions (malloc/free/memcpy/calloc) are abstracted by the port layer
 *    in @ref statistics_config.h so the project can replace them if needed.
 *  - The storage is a raw byte buffer; @ref Statistics::itemSize defines the size of a single sample.
 *  - Sample indexing/rotation policy is intentionally minimal and write position is
 *    controlled by @ref Statistics::sampleIdx.
 */

#include "statistics.h"

#include "statistics_config.h"
#include <stddef.h>
#include <stdlib.h>

#include <stdint.h>

/** @cond INTERNAL */

/* Integer square root using Newton's method for 64-bit integers */
static inline int64_t isqrt64(int64_t x)
{
    if (x <= 0) {
        return 0;
    }

    /* Initial estimate using bit manipulation */
    int64_t res = x;
    int64_t bit = (int64_t) 1 << 62;

    /* Find the highest set bit */
    while (bit > res) {
        bit >>= 2;
    }

    /* Newton-Raphson iterations */
    while (bit != 0) {
        if (res >= bit) {
            res -= bit;
            res >>= 1;
            res += bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }

    return res;
}

/* Return pointer to the current sample slot. */
static inline void * fieldPtr(Statistics * stat)
{
    return &stat->samples[stat->sampleIdx * stat->itemSize];
}

/* Store a single sample of itemSize bytes into the current slot. */
static void oneStore(Statistics * stat, const void * data)
{
    uint8_t * field = fieldPtr(stat);

    statPortMemcpy(field, data, stat->itemSize);
}

/* Load a single sample at index idx into user-provided buffer. */
static void oneLoad(Statistics * stat, uint32_t idx, void * data)
{
    uint8_t * field = &stat->samples[idx * stat->itemSize];

    statPortMemcpy(data, field, stat->itemSize);
}
/**
 * Initialize a Statistics object and allocate a zero-filled sample buffer.
 *
 * Sets the object's item size and capacity, resets internal indices and flags,
 * attempts to allocate a contiguous buffer for samples, and marks the object
 * valid only if allocation succeeds. If allocation fails the object is left
 * in a safe, invalid state (samples pointer NULL, samplesCnt set to 0,
 * sampleIdx reset to 0, enoughSamples false, valid false).
 *
 * Passing a NULL `stat` is a no-op.
 *
 * @param stat Pointer to the Statistics instance to initialize.
 * @param itemSize Size in bytes of a single sample.
 * @param samplesCount Number of samples to allocate space for (buffer capacity).
 */

void Statistics_Init(Statistics * stat, uint8_t itemSize, uint32_t samplesCount)
{
    if (!stat) {
        return;
    }

    stat->itemSize = itemSize;
    stat->samplesCnt = samplesCount;
    stat->sampleIdx = 0;
    stat->enoughSamples = false;
    stat->valid = false;

    stat->samples = statPortMalloc((size_t) samplesCount * (size_t) stat->itemSize);
    if (stat->samples) {
        statPortMemset(stat->samples, 0, (size_t) samplesCount * (size_t) stat->itemSize);
        stat->valid = true;
    } else {
        // Allocation failed: leave fields in a safe state so callers can detect invalid instance
        stat->samplesCnt = 0;
        stat->sampleIdx = 0;
        stat->enoughSamples = false;
        stat->valid = false;
    }
}

/**
 * Reset the statistics instance to an initial empty state.
 *
 * If `stat` is non-NULL and has an allocated samples buffer, zeroes the buffer,
 * sets the write index to 0, and clears the enough-samples flag.
 *
 * @param stat Pointer to the Statistics instance to reset. If NULL, the function does nothing.
 */
void Statistics_Reset(Statistics * stat)
{
    if (!stat) {
        return;
    }
    if (stat->samples) {
        statPortMemset(stat->samples, 0, (size_t) stat->samplesCnt * (size_t) stat->itemSize);
        stat->sampleIdx = 0;
        stat->enoughSamples = false;
    }
}

/**
 * Release internal resources of a Statistics instance and mark it invalid.
 *
 * Frees the internal samples buffer if present, resets buffer pointer, counts, item size, write index, and the enoughSamples flag, and marks the instance as invalid. Safe to call with a NULL pointer.
 *
 * @param stat Pointer to the Statistics instance to free; no action is taken if NULL.
 */
void Statistics_Free(Statistics * stat)
{
    if (!stat) {
        return;
    }
    if (stat->samples) {
        statPortFree(stat->samples);
        stat->samples = NULL;
    }
    stat->samplesCnt = 0;
    stat->sampleIdx = 0;
    stat->enoughSamples = false;
    stat->itemSize = 0;
    stat->valid = false;
}

void Statistics_AddSample(Statistics * stat, const void * data)
{
    if (stat && stat->valid && stat->samples && stat->itemSize > 0) {
        oneStore(stat, data);
        // Advance write index with rotation when reaching capacity
        if (stat->samplesCnt) {
            stat->sampleIdx++;
            if (stat->sampleIdx >= stat->samplesCnt) {
                stat->sampleIdx = 0;
                stat->enoughSamples = true;
            }
        }
    }
}

bool Statistics_HaveEnoughSamples(Statistics * stat)
{
    return (stat && stat->valid) ? stat->enoughSamples : false;
}

bool Statistics_IsValid(const Statistics * stat)
{
    return (stat != NULL) && stat->valid && (stat->samples != NULL) && (stat->samplesCnt > 0) && (stat->itemSize > 0);
}

/*
 * Macro to generate typed functions for integer types (returns scaled by 1000).
 */
#define STAT_SUPPORT_TYPE_INT(_type, _NameSuffix) \
    int64_t Statistics_Mean_##_NameSuffix(Statistics * stat) \
    { \
        if (!(stat && stat->valid && stat->samples && stat->samplesCnt > 0)) { \
            return 0; \
        } \
        int64_t sum = 0; \
        for (uint32_t idx = 0; idx < stat->samplesCnt; idx++) { \
            _type value; \
            oneLoad(stat, idx, &value); \
            sum += (int64_t) value; \
        } \
        /* Multiply by 1000 for fixed-point, then divide with rounding */ \
        int64_t numerator = sum * 1000; \
        int64_t denominator = (int64_t) stat->samplesCnt; \
        int64_t halfDenom = denominator / 2; \
        if (numerator >= 0) { \
            return (numerator + halfDenom) / denominator; \
        } else { \
            return (numerator - halfDenom) / denominator; \
        } \
    } \
\
    _type Statistics_Max_##_NameSuffix(Statistics * stat) \
    { \
        if (!(stat && stat->valid && stat->samples && stat->samplesCnt > 0)) { \
            return (_type) 0; \
        } \
        _type max; \
        oneLoad(stat, 0, &max); \
        for (uint32_t idx = 1; idx < stat->samplesCnt; idx++) { \
            _type value; \
            oneLoad(stat, idx, &value); \
            if (value > max) \
                max = value; \
        } \
        return max; \
    } \
\
    _type Statistics_Min_##_NameSuffix(Statistics * stat) \
    { \
        if (!(stat && stat->valid && stat->samples && stat->samplesCnt > 0)) { \
            return (_type) 0; \
        } \
        _type min; \
        oneLoad(stat, 0, &min); \
        for (uint32_t idx = 1; idx < stat->samplesCnt; idx++) { \
            _type value; \
            oneLoad(stat, idx, &value); \
            if (value < min) \
                min = value; \
        } \
        return min; \
    } \
\
    int64_t Statistics_Variance_##_NameSuffix(Statistics * stat) \
    { \
        if (!(stat && stat->valid && stat->samples && stat->samplesCnt > 1)) { \
            return -1; /* Error indicator */ \
        } \
        /* Calculate sum and sum of squares using integer arithmetic */ \
        int64_t sum = 0; \
        int64_t sumSquares = 0; \
        for (uint32_t idx = 0; idx < stat->samplesCnt; idx++) { \
            _type value; \
            oneLoad(stat, idx, &value); \
            int64_t val64 = (int64_t) value; \
            sum += val64; \
            sumSquares += val64 * val64; \
        } \
        /* Variance formula: (sumSquares - sum^2/n) / (n-1) */ \
        /* Multiply by 1000 for fixed-point representation before division */ \
        int64_t n = (int64_t) stat->samplesCnt; \
        int64_t numerator = (sumSquares * n - sum * sum) * 1000; \
        int64_t denominator = n * (n - 1); \
        /* Handle rounding for division */ \
        int64_t halfDenom = denominator / 2; \
        if (numerator >= 0) { \
            return (numerator + halfDenom) / denominator; \
        } else { \
            return (numerator - halfDenom) / denominator; \
        } \
    } \
\
    int64_t Statistics_Stdev_##_NameSuffix(Statistics * stat) \
    { \
        int64_t variance = Statistics_Variance_##_NameSuffix(stat); \
        if (variance < 0) { \
            return -1; /* Error indicator */ \
        } \
        /* Variance is scaled by 1000, so we need sqrt(variance * 1000) */ \
        /* This gives us stdev * sqrt(1000) ≈ stdev * 31.62 */ \
        /* To get stdev * 1000, we calculate: sqrt(variance) * sqrt(1000) */ \
        int64_t sqrtVar = isqrt64(variance); \
        /* sqrt(1000) ≈ 31.622776... ≈ 31623/1000 */ \
        return (sqrtVar * 31623 + 500) / 1000; \
    }

/*
 * Macro to generate typed functions for float type (returns float for all functions).
 */
#define STAT_SUPPORT_TYPE_FLOAT(_type, _NameSuffix) \
    _type Statistics_Mean_##_NameSuffix(Statistics * stat) \
    { \
        if (!(stat && stat->valid && stat->samples && stat->samplesCnt > 0)) { \
            return (_type) 0; \
        } \
        float sum = 0.0f; \
        for (uint32_t idx = 0; idx < stat->samplesCnt; idx++) { \
            _type value; \
            oneLoad(stat, idx, &value); \
            sum += value; \
        } \
        return sum / (float) stat->samplesCnt; \
    } \
\
    _type Statistics_Max_##_NameSuffix(Statistics * stat) \
    { \
        if (!(stat && stat->valid && stat->samples && stat->samplesCnt > 0)) { \
            return (_type) 0; \
        } \
        _type max; \
        oneLoad(stat, 0, &max); \
        for (uint32_t idx = 1; idx < stat->samplesCnt; idx++) { \
            _type value; \
            oneLoad(stat, idx, &value); \
            if (value > max) \
                max = value; \
        } \
        return max; \
    } \
\
    _type Statistics_Min_##_NameSuffix(Statistics * stat) \
    { \
        if (!(stat && stat->valid && stat->samples && stat->samplesCnt > 0)) { \
            return (_type) 0; \
        } \
        _type min; \
        oneLoad(stat, 0, &min); \
        for (uint32_t idx = 1; idx < stat->samplesCnt; idx++) { \
            _type value; \
            oneLoad(stat, idx, &value); \
            if (value < min) \
                min = value; \
        } \
        return min; \
    } \
\
    _type Statistics_Variance_##_NameSuffix(Statistics * stat) \
    { \
        if (!(stat && stat->valid && stat->samples && stat->samplesCnt > 1)) { \
            return 0.0f / 0.0f; /* NaN */ \
        } \
        float total = 0.0f; \
        float refVariance = 0.0f; \
        for (uint32_t idx = 0; idx < stat->samplesCnt; idx++) { \
            _type value; \
            oneLoad(stat, idx, &value); \
            float fv = (float) value; \
            total += fv; \
            refVariance += fv * fv; \
        } \
        float n = (float) stat->samplesCnt; \
        float cv = (refVariance - (total * total) / n) / (n - 1.0f); \
        return cv; \
    } \
\
    _type Statistics_Stdev_##_NameSuffix(Statistics * stat) \
    { \
        _type variance = Statistics_Variance_##_NameSuffix(stat); \
        if (variance != variance) { /* Check for NaN */ \
            return variance; \
        } \
        /* For float, we can use a simple sqrt approximation or keep using integer sqrt */ \
        /* Using the existing isqrt64 scaled up for better precision */ \
        int64_t scaledVar = (int64_t) (variance * 1000000.0f); \
        if (scaledVar < 0) \
            scaledVar = 0; \
        int64_t scaledStd = isqrt64(scaledVar); \
        return (float) scaledStd / 1000.0f; \
    }

/**
 * @name Generated typed functions
 * @brief Definitions of type-specific statistic functions declared in statistics.h.
 * @see STAT_SUPPORT_TYPE_INT, STAT_SUPPORT_TYPE_FLOAT
 * @{ */
#if STATISTICS_U8_ENABLED
STAT_SUPPORT_TYPE_INT(uint8_t, U8);
#endif

#if STATISTICS_I8_ENABLED
STAT_SUPPORT_TYPE_INT(int8_t, I8);
#endif

#if STATISTICS_U16_ENABLED
STAT_SUPPORT_TYPE_INT(uint16_t, U16);
#endif

#if STATISTICS_I16_ENABLED
STAT_SUPPORT_TYPE_INT(int16_t, I16);
#endif

#if STATISTICS_U32_ENABLED
STAT_SUPPORT_TYPE_INT(uint32_t, U32);
#endif

#if STATISTICS_I32_ENABLED
STAT_SUPPORT_TYPE_INT(int32_t, I32);
#endif

#if STATISTICS_FLOAT_ENABLED
STAT_SUPPORT_TYPE_FLOAT(float, F);
#endif
/** @} */