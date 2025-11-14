#ifndef STATISTICS_H
#define STATISTICS_H

/*
 * statistics.h
 *
 * Public types for the lightweight statistics helper.
 *
 * The API is intentionally minimal and uses a generic byte buffer to store
 * fixed-size samples. See statistics.c for function implementations.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <statistics_config.h>

/*
 * Opaque-like structure describing a statistics buffer.
 * - samplesCnt: number of samples allocated in the buffer
 * - sampleIdx: index of the current sample position (write pointer policy is
 *              project-specific)
 * - itemSize: size of a single sample in bytes
 * - samples: pointer to contiguous storage of samplesCnt * itemSize bytes
 * - enoughSamples: flag for necessary samples count indication
 */
typedef struct {
    uint32_t samplesCnt;
    uint32_t sampleIdx;
    uint8_t itemSize;
    uint8_t * samples;
    bool enoughSamples;
} Statistics;

void Statistics_Init(Statistics * stat, uint8_t itemSize, uint32_t samplesCount);
void Statistics_Free(Statistics * stat);
void Statistics_AddSample(Statistics * stat, const void * sample);
bool Statistics_HaveEnoughSamples(Statistics * stat);

#define _STAT_SUPPORT_TYPE(_type, _NameSuffix)    \
_type Statistics_Mean_##_NameSuffix(Statistics * stat);   \
_type Statistics_Max_##_NameSuffix(Statistics * stat);  \
_type Statistics_Min_##_NameSuffix(Statistics * stat);  \
float Statistics_Variance_##_NameSuffix(Statistics * stat); \
float Statistics_Stdev_##_NameSuffix(Statistics * stat);    \

#if STATISTICS_U8_ENABLED
    _STAT_SUPPORT_TYPE(uint8_t, U8);
#endif

#if STATISTICS_I8_ENABLED
    _STAT_SUPPORT_TYPE(int8_t, I8);
#endif

#if STATISTICS_U16_ENABLED
    _STAT_SUPPORT_TYPE(uint16_t, U16);
#endif

#if STATISTICS_I16_ENABLED
    _STAT_SUPPORT_TYPE(int16_t, I16);
#endif

#if STATISTICS_U32_ENABLED
    _STAT_SUPPORT_TYPE(uint32_t, U32);
#endif

#if STATISTICS_I32_ENABLED
    _STAT_SUPPORT_TYPE(int32_t, I32);
#endif

#if STATISTICS_FLOAT_ENABLED
    _STAT_SUPPORT_TYPE(float, F);
#endif

#ifdef __cplusplus
}
#endif

#endif //STATISTICS_H