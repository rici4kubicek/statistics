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

#include <statistics_config.h>
#include <stdbool.h>
#include <stdint.h>

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
    uint32_t samplesCnt; /**< Total capacity (number of items). */
    /** Index of the current write position in the buffer. */
    uint32_t sampleIdx; /**< Current write index [0..samplesCnt-1]. */
    /** Size of a single sample in bytes. */
    uint8_t itemSize; /**< Bytes per sample. */
    /** Pointer to contiguous storage of size @c samplesCnt * @c itemSize. */
    uint8_t * samples; /**< Backing storage pointer. */
    /**
     * Flag indicating that at least @ref samplesCnt samples have been written,
     * i.e., a full cycle completed and statistics are meaningful for the full window.
     */
    bool enoughSamples;
    /**
     * Internal validity flag. Set to true only if initialization allocated
     * the samples buffer successfully. Public callers can query this via
     * @ref Statistics_IsValid and must not call statistic functions if false.
     */
    bool valid;
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
 * Resets the statistical data contained in the provided Statistics structure.
 *
 * This function clears all sample data stored in the Statistics instance, resets
 * the sample index, and indicates that not enough samples are available to calculate
 * meaningful statistics. It safely handles a null pointer by exiting early without
 * performing any operations.
 *
 * @param stat A pointer to the Statistics structure to reset. If the pointer is null,
 *             the function will safely exit without performing any operations.
 */
void Statistics_Reset(Statistics * stat);

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
 * @brief Check whether the statistics instance has been initialized successfully.
 *
 * This becomes false if memory allocation during @ref Statistics_Init failed or
 * after @ref Statistics_Free.
 *
 * @param stat Pointer to the instance.
 * @return true if the instance is valid and ready to use, false otherwise.
 */
bool Statistics_IsValid(const Statistics * stat);

/**
 * @name Type-specific statistic functions
 *
 * For each enabled scalar type, a set of functions is generated. Replace
 * <T> with one of: U8, I8, U16, I16, U32, I32, F (float).
 *
 * **Integer types (U8, I8, U16, I16, U32, I32):**
 * - `int64_t Statistics_Mean_<T>(Statistics* stat)` - Returns mean * 1000
 * - `<base> Statistics_Max_<T>(Statistics* stat)`
 * - `<base> Statistics_Min_<T>(Statistics* stat)`
 * - `int64_t Statistics_Variance_<T>(Statistics* stat)` - Returns variance * 1000
 * - `int64_t Statistics_Stdev_<T>(Statistics* stat)` - Returns stdev * 1000
 *
 * **Float type (F):**
 * - `float Statistics_Mean_F(Statistics* stat)` - Returns mean as float
 * - `float Statistics_Max_F(Statistics* stat)`
 * - `float Statistics_Min_F(Statistics* stat)`
 * - `int64_t Statistics_Variance_F(Statistics* stat)` - Returns variance * 1000
 * - `int64_t Statistics_Stdev_F(Statistics* stat)` - Returns stdev * 1000
 *
 * Where `<base>` is the underlying C type for <T> (e.g., `uint16_t` for U16).
 *
 * **Note:** Integer types use fixed-point arithmetic scaled by 1000 to avoid
 * floating-point operations. Divide the result by 1000 to get the actual value.
 * This is much faster on CPUs without FPU (e.g., STM32F0, Cortex-M0).
 * @{ */
#define _STAT_SUPPORT_TYPE_INT(_type, _NameSuffix) \
    int64_t Statistics_Mean_##_NameSuffix(Statistics * stat); \
    _type Statistics_Max_##_NameSuffix(Statistics * stat); \
    _type Statistics_Min_##_NameSuffix(Statistics * stat); \
    int64_t Statistics_Variance_##_NameSuffix(Statistics * stat); \
    int64_t Statistics_Stdev_##_NameSuffix(Statistics * stat); \
    /** @} */

#define _STAT_SUPPORT_TYPE_FLOAT(_type, _NameSuffix) \
    _type Statistics_Mean_##_NameSuffix(Statistics * stat); \
    _type Statistics_Max_##_NameSuffix(Statistics * stat); \
    _type Statistics_Min_##_NameSuffix(Statistics * stat); \
    int64_t Statistics_Variance_##_NameSuffix(Statistics * stat); \
    int64_t Statistics_Stdev_##_NameSuffix(Statistics * stat); \
    /** @} */

#if STATISTICS_U8_ENABLED
_STAT_SUPPORT_TYPE_INT(uint8_t, U8);
#endif

#if STATISTICS_I8_ENABLED
_STAT_SUPPORT_TYPE_INT(int8_t, I8);
#endif

#if STATISTICS_U16_ENABLED
_STAT_SUPPORT_TYPE_INT(uint16_t, U16);
#endif

#if STATISTICS_I16_ENABLED
_STAT_SUPPORT_TYPE_INT(int16_t, I16);
#endif

#if STATISTICS_U32_ENABLED
_STAT_SUPPORT_TYPE_INT(uint32_t, U32);
#endif

#if STATISTICS_I32_ENABLED
_STAT_SUPPORT_TYPE_INT(int32_t, I32);
#endif

#if STATISTICS_FLOAT_ENABLED
_STAT_SUPPORT_TYPE_FLOAT(float, F);
#endif

/** @} */ /* end of statistics_core */

#ifdef __cplusplus
}
#endif

#endif // STATISTICS_H