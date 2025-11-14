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

#ifndef STATISTICS_U8_ENABLED
    #define STATISTICS_U8_ENABLED   true
#endif

#ifndef STATISTICS_I8_ENABLED
    #define STATISTICS_I8_ENABLED   true
#endif

#ifndef STATISTICS_U16_ENABLED
    #define STATISTICS_U16_ENABLED   true
#endif

#ifndef STATISTICS_I16_ENABLED
    #define STATISTICS_I16_ENABLED   true
#endif

#ifndef STATISTICS_U32_ENABLED
    #define STATISTICS_U32_ENABLED   true
#endif

#ifndef STATISTICS_I32_ENABLED
    #define STATISTICS_I32_ENABLED   true
#endif

#ifndef STATISTICS_FLOAT_ENABLED
    #define STATISTICS_FLOAT_ENABLED   true
#endif

    #ifdef __cplusplus
}
#endif

#endif /* STATISTICS_CONFIG_H */

