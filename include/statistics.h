#ifndef STATISTICS_H
#define STATISTICS_H

/**
 * @file statistics.h
 * @brief Public API for a lightweight statistics helper working with fixed-size samples.
 *
 * This header exposes a small C API to collect samples of arbitrary, fixed size
 * and compute basic statistics for a set of supported scalar types.
 *
 * Design highlights
 * - The storage is a raw contiguous byte buffer; the size of one sample is
 *   given by @ref Statistics::itemSize.
 * - Sample rotation policy is intentionally minimal: the write position is
 *   controlled by @ref Statistics::sampleIdx and wraps to 0 when the configured
 *   capacity is reached. When a full wrap occurs, @ref Statistics::enoughSamples
 *   is set to true.
 * - Memory primitives are provided by the port layer in @ref statistics_config.h.
 *
 * See @ref statistics.c for the implementation details.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <statistics_config.h>

/**
 * @defgroup statistics_core Core API
 * @brief Core structures and functions to collect samples and compute statistics.
 * @{ */

/**
 * @brief Statistics buffer descriptor.
 *
 * The structure describes a ring-like buffer that stores @ref samplesCnt
 * elements each of size @ref itemSize bytes. New samples are written at
 * @ref sampleIdx and the index is advanced with wrap-around.
 */
typedef struct {
    /** Number of samples allocated in the buffer. */
    uint32_t samplesCnt;       /**< Total capacity (number of items). */
    /** Index of the current write position in the buffer. */
    uint32_t sampleIdx;        /**< Current write index [0..samplesCnt-1]. */
    /** Size of a single sample in bytes. */
    uint8_t  itemSize;         /**< Bytes per sample. */
    /** Pointer to contiguous storage of size @c samplesCnt * @c itemSize. */
    uint8_t* samples;          /**< Backing storage pointer. */
    /**
     * Flag indicating that at least @ref samplesCnt samples have been written,
     * i.e., a full cycle completed and statistics are meaningful for the full window.
     */
    bool     enoughSamples;
} Statistics;

/**
 * @brief Initialize a statistics instance.
 *
 * Allocates internal storage for @p samplesCount elements of @p itemSize bytes
 * each and resets internal indices.
 *
 * @param[out] stat          Pointer to the instance to initialize.
 * @param      itemSize      Size of a single sample in bytes.
 * @param      samplesCount  Number of samples in the window (capacity).
 */
void Statistics_Init(Statistics * stat, uint8_t itemSize, uint32_t samplesCount);

/**
 * @brief Release resources held by a statistics instance.
 * @param stat Pointer to the instance to deinitialize.
 */
void Statistics_Free(Statistics * stat);

/**
 * @brief Append a single sample to the buffer at the current write index.
 *
 * The data at @p sample must point to @ref Statistics::itemSize bytes.
 * After storing, the write index advances and wraps when it reaches capacity.
 *
 * @param stat   Pointer to the instance.
 * @param sample Pointer to the sample data to store.
 */
void Statistics_AddSample(Statistics * stat, const void * sample);

/**
 * @brief Indicate whether a full window of samples has been collected.
 *
 * @param stat Pointer to the instance.
 * @return true if at least @ref Statistics::samplesCnt samples have been
 *         written (i.e., a wrap-around occurred), false otherwise.
 */
bool Statistics_HaveEnoughSamples(Statistics * stat);

/**
 * @name Type-specific statistic functions
 *
 * For each enabled scalar type, a set of functions is generated. Replace
 * <T> with one of: U8, I8, U16, I16, U32, I32, F (float).
 *
 * - `<base> Statistics_Mean_<T>(Statistics* stat)`
 * - `<base> Statistics_Max_<T>(Statistics* stat)`
 * - `<base> Statistics_Min_<T>(Statistics* stat)`
 * - `float Statistics_Variance_<T>(Statistics* stat)`
 * - `float Statistics_Stdev_<T>(Statistics* stat)`
 *
 * Where `<base>` is the underlying C type for <T> (e.g., `uint16_t` for U16).
 * @{ */
#define _STAT_SUPPORT_TYPE(_type, _NameSuffix)    \
_type Statistics_Mean_##_NameSuffix(Statistics * stat);   \
_type Statistics_Max_##_NameSuffix(Statistics * stat);    \
_type Statistics_Min_##_NameSuffix(Statistics * stat);    \
float Statistics_Variance_##_NameSuffix(Statistics * stat); \
float Statistics_Stdev_##_NameSuffix(Statistics * stat);    \
/** @} */

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

/** @} */ /* end of statistics_core */

#ifdef __cplusplus
}
#endif

#endif //STATISTICS_H