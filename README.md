# Statistics

![CodeRabbit Pull Request Reviews](https://img.shields.io/coderabbit/prs/github/rici4kubicek/statistics?utm_source=oss&utm_medium=github&utm_campaign=rici4kubicek%2Fstatistics&labelColor=171717&color=FF570A&link=https%3A%2F%2Fcoderabbit.ai&label=CodeRabbit+Reviews)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/rici4kubicek/statistics/main.svg)](https://results.pre-commit.ci/latest/github/rici4kubicek/statistics/main)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/rici4kubicek/library/Statistics.svg)](https://registry.platformio.org/libraries/rici4kubicek/Statistics)

A lightweight C library for collecting fixed-size samples and computing basic
statistics (mean, min, max, variance, and standard deviation) for selected
scalar types. The documentation is generated with Doxygen and Sphinx (Breathe).

## Key features
- minimal dependencies, small API surface
- ring buffer with an indicator of "enough" samples collected
- type-specific functions generated based on configuration
- **no floating-point operations for integer types** - optimized for embedded CPUs without FPU (e.g., STM32F0, Cortex-M0)
- fixed-point arithmetic for integer types (mean, variance, stdev scaled by 1000)
- native float support for float data types
- exponential smoothing (IIR) filter for cheap running averages (`exponential_smoothing.h`)
- online (streaming) linear regression with slope, intercept, correlation and uncertainty (`linear_regression.h`)

### Integer modules (no float, no malloc)

For MCUs without FPU (e.g. Cortex-M0+). All state lives in caller-owned
structs and storage; no module pulls in soft-float code.

- `statistics_int_math.h` - `StatInt_FloorShift`, `StatInt_DivRound` (half away from zero),
  `StatInt_Isqrt64`, `StatInt_Clamp_I32` / `StatInt_Clamp_I64` / `StatInt_Clamp_U32` and `StatInt_Map_I32` (Arduino-style
  range mapping with rounding, reversed ranges, no output clamping, no overflow).
  Float variants `StatFloat_Clamp` / `StatFloat_Map` are in a separate source file and only
  declared with `STATISTICS_FLOAT_ENABLED`.
- `exponential_smoothing_i32.h` - fixed-point EMA,
  `acc += StatInt_FloorShift((x << frac) - acc, shift)`;
  `ExponentialSmoothingI32_SetShift` changes the rate of a running filter
  (e.g. fast start, then slow tracking) without losing the fraction bits.
- `mean_accumulator_i32.h` - int64 sum + count, rounded mean.
- `window_regression_i32.h` - sliding-window least-squares line over several
  y channels sharing one x (time) axis; wrap-safe uint32 time, optional slope
  clamp that keeps the mean level, no 64-bit division in per-sample loops.

```c
#include "window_regression_i32.h"

#define WINDOW 32
#define AXES   3

static uint32_t times[WINDOW];
static int32_t values[WINDOW * AXES];
static WindowRegressionI32Line lines[AXES];
static WindowRegressionI32 drift;

void init(void)
{
    WindowRegressionI32_Init(&drift, times, values, lines, WINDOW, AXES);
}

void onSample(uint32_t now, const int32_t sample[AXES])
{
    WindowRegressionI32_Push(&drift, now, sample);
    if (WindowRegressionI32_Fit(&drift, 8, 0)) {
        int32_t expected;
        WindowRegressionI32_Evaluate(&drift, 0, now + 100, &expected); /* axis 0, 100 ticks ahead */
    }
}
```

Code size on Cortex-M0+ (`arm-none-eabi-gcc` 15.3, `-Os`, text bytes):
`statistics_int_math` 730, `exponential_smoothing_i32` 272,
`mean_accumulator_i32` 94, `window_regression_i32` 1210, plus libgcc 64-bit
helpers (`__aeabi_uldivmod`, `__aeabi_lmul`, shifts) when not already linked.
