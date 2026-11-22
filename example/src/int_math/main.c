/* Integer helpers: range mapping with rounding, combined with clamping */
#include "statistics_int_math.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    printf("StatInt_Map_I32(512, 0..1023 -> 0..255): %" PRId32 "\n", StatInt_Map_I32(512, 0, 1023, 0, 255));
    printf("StatInt_Clamp_I32(StatInt_Map_I32(1500, ...), 0, 255): %" PRId32 "\n", StatInt_Clamp_I32(StatInt_Map_I32(1500, 0, 1023, 0, 255), 0, 255));

    return 0;
}
