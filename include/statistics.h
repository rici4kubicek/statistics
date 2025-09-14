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

#include <stdint.h>

/*
 * Opaque-like structure describing a statistics buffer.
 * - samplesCnt: number of samples allocated in the buffer
 * - sampleIdx: index of the current sample position (write pointer policy is
 *              project-specific)
 * - itemSize: size of a single sample in bytes
 * - samples: pointer to contiguous storage of samplesCnt * itemSize bytes
 */
typedef struct {
    uint32_t samplesCnt;
    uint32_t sampleIdx;
    uint8_t itemSize;
    uint8_t * samples;
} Statistics;

void Statistics_Init(Statistics * stat, uint8_t itemSize, uint32_t samplesCount);
void Statistics_Free(Statistics * stat);
void Statistics_AddSample(Statistics * stat, const void * sample);

#define _STAT_SUPPORT_TYPE(_type, _NameSuffix)    \
_type Statistics_Mean_##_NameSuffix(Statistics * stat);   \
_type Statistics_Max_##_NameSuffix(Statistics * stat);  \
_type Statistics_Min_##_NameSuffix(Statistics * stat);  \

_STAT_SUPPORT_TYPE(uint8_t, U8);
_STAT_SUPPORT_TYPE(int8_t, I8);
_STAT_SUPPORT_TYPE(uint16_t, U16);
_STAT_SUPPORT_TYPE(int16_t, I16);
_STAT_SUPPORT_TYPE(uint32_t, U32);
_STAT_SUPPORT_TYPE(int32_t, I32);
_STAT_SUPPORT_TYPE(float, F);

#ifdef __cplusplus
}
#endif

#endif //STATISTICS_H