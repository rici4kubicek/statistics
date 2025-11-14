/*
 * statistics.c
 *
 * Simple, minimalistic statistics helper for fixed-size samples.
 *
 * This module provides:
 *  - Initialization and deinitialization of a samples buffer
 *  - Appending a new sample into the buffer
 *  - Computing average for specific, supported types via macro-generated functions
 *
 * Notes:
 *  - Memory functions (malloc/free/memcpy/calloc) are abstracted by the port layer
 *    in statistics_config.h so the project can replace them if needed.
 *  - The storage is a raw byte buffer; itemSize defines the size of a single sample.
 *  - Sample indexing/rotation policy is project-specific; current code writes to
 *    the position indicated by sampleIdx.
 */

#include "statistics.h"

#include <stddef.h>
#include <stdlib.h>
#include "statistics_config.h"

#include <stdint.h>

static inline float fsqrt(float x) {
    if (x <= 0.0f) {
        return x == 0.0f ? 0.0f : (0.0f/0.0f);
    }

    union { uint32_t i; float f; } u = { .f = x };
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
static void oneLoad(Statistics * stat, uint8_t idx, void * data)
{
    uint8_t * field = &stat->samples[idx * stat->itemSize];

    statPortMemcpy(data, field, stat->itemSize);
}

/*
 * Initialize statistics instance.
 *  - itemSize: size of one sample in bytes
 *  - samplesCount: number of samples to allocate space for
 */
void Statistics_Init(Statistics * stat, uint8_t itemSize, uint32_t samplesCount)
{
    stat->itemSize = itemSize;
    stat->samplesCnt = samplesCount;
    stat->sampleIdx = 0;
    stat->samples = statPortMalloc(samplesCount * stat->itemSize);
    stat->enoughSamples = false;
}

/* Free resources associated with statistics instance. */
void Statistics_Free(Statistics * stat)
{
    statPortFree(stat->samples);
    stat->sampleIdx = 0;
    stat->enoughSamples = false;
}

/*
 * Append a sample to the buffer (writes to current index).
 * Caller provides a pointer to data of size itemSize.
 */
void Statistics_AddSample(Statistics * stat, const void * data)
{
    if (stat->samples) {
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

/*
 * Check whether there are enough samples for statistics calculations
 */
bool Statistics_HaveEnoughSamples(Statistics * stat)
{
    return stat->enoughSamples;
}

/*
 * Macro to generate typed functions.
 */
#define STAT_SUPPORT_TYPE(_type, _NameSuffix)    \
_type Statistics_Mean_##_NameSuffix(Statistics * stat)   \
{                   \
    float avg = 0;  \
    for (int idx = 0; idx < stat->samplesCnt; idx++) {  \
        _type value;    \
        oneLoad(stat, idx, &value); \
        avg += value;   \
    }   \
    return avg/stat->samplesCnt;    \
}   \
    \
_type Statistics_Max_##_NameSuffix(Statistics * stat)   \
{                   \
    _type max = 0;  \
    for (int idx = 0; idx < stat->samplesCnt; idx++) {  \
        _type value;    \
        oneLoad(stat, idx, &value); \
        if (value > max)   \
            max = value;    \
    }   \
    return max;    \
}   \
    \
_type Statistics_Min_##_NameSuffix(Statistics * stat)   \
{                   \
    _type min = (_type) 0xffffffffff;  \
    for (int idx = 0; idx < stat->samplesCnt; idx++) {  \
        _type value;    \
        oneLoad(stat, idx, &value); \
        if (value < min)   \
            min = value;    \
    }   \
    return min;    \
}   \
    \
float Statistics_Variance_##_NameSuffix(Statistics * stat)   \
{                   \
    float total = 0;  \
    float refVariance = 0;  \
    for (int idx = 0; idx < stat->samplesCnt; idx++) {  \
        _type value;    \
        oneLoad(stat, idx, &value); \
        total += value; \
        refVariance += value * value; \
    }   \
    float cv = (refVariance - total * total / stat->samplesCnt) / (stat->samplesCnt - 1);    \
    return cv;  \
}   \
    \
float Statistics_Stdev_##_NameSuffix(Statistics * stat)   \
{   \
    return fsqrt(Statistics_Variance_##_NameSuffix(stat));  \
}   \

/* Currently supported typed averages. Extend as needed. */
#if STATISTICS_U8_ENABLED == true
    STAT_SUPPORT_TYPE(uint8_t, U8);
#endif

#if STATISTICS_I8_ENABLED == true
    STAT_SUPPORT_TYPE(int8_t, I8);
#endif

#if STATISTICS_U16_ENABLED == true
    STAT_SUPPORT_TYPE(uint16_t, U16);
#endif

#if STATISTICS_I16_ENABLED == true
    STAT_SUPPORT_TYPE(int16_t, I16);
#endif

#if STATISTICS_U32_ENABLED == true
    STAT_SUPPORT_TYPE(uint32_t, U32);
#endif

#if STATISTICS_I32_ENABLED == true
    STAT_SUPPORT_TYPE(int32_t, I32);
#endif

#if STATISTICS_FLOAT_ENABLED == true
    STAT_SUPPORT_TYPE(float, F);
#endif