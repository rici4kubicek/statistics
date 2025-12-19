#include "statistics.h"
#include <inttypes.h>
#include <stdio.h>
#include <time.h>

Statistics stat;

int main()
{
    printf("Hello, world!\n");
    fflush(stdout);

    Statistics_Init(&stat, sizeof(uint8_t), 4);
    if (!Statistics_IsValid(&stat)) {
        fprintf(stderr, "Statistics init failed: out of memory or invalid parameters.\n");
        return 1;
    }

    uint8_t v = 1;
    Statistics_AddSample(&stat, &v);
    v = 21;
    Statistics_AddSample(&stat, &v);
    v = 79;
    Statistics_AddSample(&stat, &v);
    v = 100;
    Statistics_AddSample(&stat, &v);
    v = 31;
    Statistics_AddSample(&stat, &v);
    v = 85;
    Statistics_AddSample(&stat, &v);

    printf("Max: %d\n", Statistics_Max_U8(&stat));
    printf("Min: %d\n", Statistics_Min_U8(&stat));

    int64_t mean = Statistics_Mean_U8(&stat);
    int64_t variance = Statistics_Variance_U8(&stat);
    int64_t stdev = Statistics_Stdev_U8(&stat);

    printf("Mean: %" PRId64 " (actual: %" PRId64 ".%03" PRId64 ")\n", mean, mean / 1000, mean % 1000);
    printf("Variance: %" PRId64 " (actual: %" PRId64 ".%03" PRId64 ")\n", variance, variance / 1000, variance % 1000);
    printf("Stdev: %" PRId64 " (actual: %" PRId64 ".%03" PRId64 ")\n", stdev, stdev / 1000, stdev % 1000);

    return 0;
}