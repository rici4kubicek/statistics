#ifndef STATISTICS_CONFIG_H
#define STATISTICS_CONFIG_H

/*
 * statistics_config.h
 *
 * Port layer for the statistics module. Adjust or override memory function
 * macros to match your platform. To provide custom implementations, define
 * STATISTICS_PORT_USER at compile time and implement the macros below before
 * including this header:
 *   - statPortMalloc(size)
 *   - statPortFree(ptr)
 *   - statPortMemcpy(dest, src, count)
 *   - statPortCalloc(num, size)
 */

#ifdef __cplusplus
extern "C" {
    #endif

#ifndef STATISTICS_PORT_USER
    #include <stdlib.h>
    #include <string.h>

    #define statPortMalloc(size) malloc(size)
    #define statPortFree(ptr) free(ptr)
    #define statPortMemcpy(dest, src, count) memcpy(dest, src, count)
    #define statPortCalloc(num, size) calloc(num, size)
#endif

/*
 * Data type support switches
 * --------------------------
 * Use the following compile-time directives to enable/disable the API for
 * individual data types. If not defined by the user, the default value is
 * numeric `1` (enabled). Use `0` to disable.
 *
 * How to change values:
 *  - Compiler (recommended):
 *      -DSTATISTICS_U8_ENABLED=0 -DSTATISTICS_FLOAT_ENABLED=0
 *  - Or define before including this header: #define STATISTICS_U8_ENABLED 0
 */

#ifndef STATISTICS_U8_ENABLED   /* Enable API for uint8_t */
    #define STATISTICS_U8_ENABLED   1
#endif

#ifndef STATISTICS_I8_ENABLED   /* Enable API for int8_t */
    #define STATISTICS_I8_ENABLED   1
#endif

#ifndef STATISTICS_U16_ENABLED  /* Enable API for uint16_t */
    #define STATISTICS_U16_ENABLED   1
#endif

#ifndef STATISTICS_I16_ENABLED  /* Enable API for int16_t */
    #define STATISTICS_I16_ENABLED   1
#endif

#ifndef STATISTICS_U32_ENABLED  /* Enable API for uint32_t */
    #define STATISTICS_U32_ENABLED   1
#endif

#ifndef STATISTICS_I32_ENABLED  /* Enable API for int32_t */
    #define STATISTICS_I32_ENABLED   1
#endif

#ifndef STATISTICS_FLOAT_ENABLED /* Enable API for float */
    #define STATISTICS_FLOAT_ENABLED   1
#endif

    #ifdef __cplusplus
}
#endif

#endif /* STATISTICS_CONFIG_H */

