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
static inline float fsqrt(float x)
{
    if (x <= 0.0f) {
        return x == 0.0f ? 0.0f : (0.0f / 0.0f);
    }

    union {
        uint32_t i;
        float f;
    } u = {.f = x};
    u.i = (u.i >> 1) + 0x1FC00000u;

    float y = u.f;

    y = 0.5f * (y + x / y);
    y = 0.5f * (y + x / y);
    y = 0.5f * (y + x / y);

    return y;
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
/** @endcond */

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
        stat->valid = true;
    } else {
        // Allocation failed: leave fields in a safe state so callers can detect invalid instance
        stat->samplesCnt = 0;
        stat->sampleIdx = 0;
        stat->enoughSamples = false;
        stat->valid = false;
    }
}

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
 * Macro to generate typed functions.
 */
#define STAT_SUPPORT_TYPE(_type, _NameSuffix) \
    _type Statistics_Mean_##_NameSuffix(Statistics * stat) \
    { \
        if (!(stat && stat->valid && stat->samples && stat->samplesCnt > 0)) { \
            return (_type) 0; \
        } \
        float avg = 0.0f; \
        for (uint32_t idx = 0; idx < stat->samplesCnt; idx++) { \
            _type value; \
            oneLoad(stat, idx, &value); \
            avg += (float) value; \
        } \
        return (_type) (avg / (float) stat->samplesCnt); \
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
    float Statistics_Variance_##_NameSuffix(Statistics * stat) \
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
    float Statistics_Stdev_##_NameSuffix(Statistics * stat) \
    { \
        float v = Statistics_Variance_##_NameSuffix(stat); \
        return fsqrt(v); \
    }

/**
 * @name Generated typed functions
 * @brief Definitions of type-specific statistic functions declared in statistics.h.
 * @see _STAT_SUPPORT_TYPE
 * @{ */
#if STATISTICS_U8_ENABLED
STAT_SUPPORT_TYPE(uint8_t, U8);
#endif

#if STATISTICS_I8_ENABLED
STAT_SUPPORT_TYPE(int8_t, I8);
#endif

#if STATISTICS_U16_ENABLED
STAT_SUPPORT_TYPE(uint16_t, U16);
#endif

#if STATISTICS_I16_ENABLED
STAT_SUPPORT_TYPE(int16_t, I16);
#endif

#if STATISTICS_U32_ENABLED
STAT_SUPPORT_TYPE(uint32_t, U32);
#endif

#if STATISTICS_I32_ENABLED
STAT_SUPPORT_TYPE(int32_t, I32);
#endif

#if STATISTICS_FLOAT_ENABLED
STAT_SUPPORT_TYPE(float, F);
#endif
/** @} */