#include "statistics.h"
#include <stdio.h>
#include <time.h>

Statistics stat;

int main()
{
    printf("Hello, world!\n");
    fflush(stdout);

    Statistics_Init(&stat, sizeof(uint8_t), 4);

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
    printf("Mean: %d\n", Statistics_Mean_U8(&stat));
    printf("Variance: %f\n", Statistics_Variance_U8(&stat));
    printf("Stdev: %f\n", Statistics_Stdev_U8(&stat));

    return 0;
}