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

    #ifdef __cplusplus
}
#endif

#endif /* STATISTICS_CONFIG_H */

