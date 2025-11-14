#ifndef STATISTICS_CONFIG_H
#define STATISTICS_CONFIG_H

/**
 * @file statistics_config.h
 * @brief Port layer and configuration switches for the statistics module.
 *
 * This header provides two things:
 * - A thin porting layer for memory and copying primitives (see @ref statistics_port).
 * - Compile-time feature toggles to enable/disable type-specific APIs (see @ref statistics_types).
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup statistics_port Port layer
 * @brief Replaceable memory/copy primitives used by the module.
 *
 * Define `STATISTICS_PORT_USER` at compile time and provide compatible
 * implementations of the following macros prior to including this header if
 * you need to map them to a platform-specific allocator:
 * - `statPortMalloc(size)`
 * - `statPortFree(ptr)`
 * - `statPortMemcpy(dest, src, count)`
 * - `statPortCalloc(num, size)`
 *
 * @{ */

#ifndef STATISTICS_PORT_USER
    #include <stdlib.h>
    #include <string.h>

    #define statPortMalloc(size)             malloc(size)
    #define statPortFree(ptr)                free(ptr)
    #define statPortMemcpy(dest, src, count) memcpy(dest, src, count)
    #define statPortCalloc(num, size)        calloc(num, size)
#endif

/** @} */

/**
 * @defgroup statistics_types Feature toggles (type support)
 * @brief Enable or disable type-specific statistic functions.
 *
 * Use the following compile-time macros to fine-tune which typed functions are
 * available. If a macro is not pre-defined, the default is `1` (enabled).
 * Set to `0` to disable.
 *
 * How to change values:
 * - Compiler (recommended):
 *   `-DSTATISTICS_U8_ENABLED=0 -DSTATISTICS_FLOAT_ENABLED=0`
 * - Or in code before including this header:
 *   `#define STATISTICS_U8_ENABLED 0`
 * @{ */

#ifndef STATISTICS_U8_ENABLED /* Enable API for uint8_t */
    #define STATISTICS_U8_ENABLED 1
#endif

#ifndef STATISTICS_I8_ENABLED /* Enable API for int8_t */
    #define STATISTICS_I8_ENABLED 1
#endif

#ifndef STATISTICS_U16_ENABLED /* Enable API for uint16_t */
    #define STATISTICS_U16_ENABLED 1
#endif

#ifndef STATISTICS_I16_ENABLED /* Enable API for int16_t */
    #define STATISTICS_I16_ENABLED 1
#endif

#ifndef STATISTICS_U32_ENABLED /* Enable API for uint32_t */
    #define STATISTICS_U32_ENABLED 1
#endif

#ifndef STATISTICS_I32_ENABLED /* Enable API for int32_t */
    #define STATISTICS_I32_ENABLED 1
#endif

#ifndef STATISTICS_FLOAT_ENABLED /* Enable API for float */
    #define STATISTICS_FLOAT_ENABLED 1
#endif

/** @} */ /* end of statistics_types */

#ifdef __cplusplus
}
#endif

#endif /* STATISTICS_CONFIG_H */
