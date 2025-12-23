# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Statistics is a lightweight C library for collecting fixed-size samples and computing basic statistics (mean, min, max, variance, standard deviation) for embedded systems. The library is designed to work without floating-point operations for integer types, making it suitable for embedded CPUs without FPU (e.g., STM32F0, Cortex-M0).

**Key Design Principles:**
- Ring buffer implementation with fixed capacity
- Type-specific functions generated via macros
- Fixed-point arithmetic (scaled by 1000) for integer types to avoid FPU dependency
- Welford's online algorithm for variance/stdev to prevent overflow
- Configurable type support via compile-time flags
- Custom memory allocator support via port layer

## Build and Test Commands

### PlatformIO (Primary Build System)

Navigate to `example/` directory for all PlatformIO commands:

```bash
cd example

# Build and run the example (native environment)
pio run -e native

# Build for a specific environment
pio run -e <env_name>

# Run unit tests
pio test -e native_unittest

# Clean build artifacts
pio run --target clean
```

**Note:** The `native` environment automatically runs the compiled program after building via the `run_native.py` post-action script.

### Documentation

Documentation is generated using Doxygen and Sphinx (with Breathe extension):

```bash
cd docs

# Generate Doxygen XML files (required first)
doxygen Doxyfile

# Generate HTML documentation for current version
make html

# Generate documentation with version selector (all git tags + branches)
make multiversion

# Generate other formats
make latexpdf   # PDF via LaTeX
make singlehtml # Single HTML file
make clean      # Clean build artifacts
```

**Version Selector:**
- The documentation uses `sphinx-multiversion` for multi-version support
- Builds documentation for all git tags matching `v*.*.*` (semantic versioning) and `main`/`master`/`develop` branches
- Each version is built in a separate directory: `docs/build/html/<version>/`
- A root `index.html` redirects to the main branch automatically
- The version selector appears at the bottom left of each documentation page

**Configuration:**
- Version patterns configured in `docs/source/conf.py` (lines 63-81)
- Custom templates in `docs/source/_templates/`
- Styling in `docs/source/_static/version-selector.css`

Documentation source files are in `docs/source/`. The build outputs to `docs/build/`.

### Code Formatting

Pre-commit hooks are configured for automatic formatting:

```bash
# Install pre-commit hooks
pre-commit install

# Run manually on all files
pre-commit run --all-files

# Run only clang-format
clang-format -i src/*.c include/*.h
```

Formatting configuration is in `.clang-format`.

## Code Architecture

### Core Components

**1. Statistics Structure (`Statistics`)**
- Ring buffer descriptor in `include/statistics.h:43`
- Fields:
  - `samples`: Raw byte buffer for sample storage
  - `samplesCnt`: Total capacity (number of samples)
  - `sampleIdx`: Current write position (wraps at capacity)
  - `itemSize`: Size of each sample in bytes
  - `enoughSamples`: True after first full rotation
  - `valid`: Internal flag indicating successful initialization

**2. Configuration Layer (`statistics_config.h`)**
- Port layer abstraction for memory operations (`statPortMalloc`, `statPortFree`, `statPortMemcpy`, `statPortCalloc`, `statPortMemset`)
- Type enablement flags: `STATISTICS_<TYPE>_ENABLED` (U8, I8, U16, I16, U32, I32, FLOAT)
- Override by defining `STATISTICS_PORT_USER` before including the header

**3. Type-Specific API Generation**
- Macros `_STAT_SUPPORT_TYPE_INT` and `_STAT_SUPPORT_TYPE_FLOAT` in `include/statistics.h:153`
- Implementation macros `STAT_SUPPORT_TYPE_INT` and `STAT_SUPPORT_TYPE_FLOAT` in `src/statistics.c:205`
- Generated functions: `Statistics_Mean_<T>`, `Statistics_Max_<T>`, `Statistics_Min_<T>`, `Statistics_Variance_<T>`, `Statistics_Stdev_<T>`
- Type suffixes: U8, I8, U16, I16, U32, I32, F (float)

### Critical Implementation Details

**Integer Arithmetic (Fixed-Point):**
- All integer statistics return values scaled by 1000
- Example: Mean of [10, 20, 30] returns 20000 (divide by 1000 to get 20.0)
- Variance and stdev use Welford's algorithm (`src/statistics.c:260`) to avoid overflow from squaring large values
- Integer square root implementation: `isqrt64()` in `src/statistics.c:42`

**Float Arithmetic:**
- Float type returns native float values (no scaling)
- Uses standard floating-point operations
- Variance uses textbook formula (sum of squares method) in `src/statistics.c:362`

**Ring Buffer Behavior:**
- Write index advances after each `Statistics_AddSample()`
- When `sampleIdx` reaches `samplesCnt`, it wraps to 0 and sets `enoughSamples = true`
- Statistics functions iterate over all allocated samples regardless of fill level
- Check `Statistics_HaveEnoughSamples()` to verify full window is available

### Memory Management

- `Statistics_Init()`: Allocates zero-filled buffer, sets `valid` flag only on success
- `Statistics_Free()`: Releases buffer, resets all fields, sets `valid = false`
- `Statistics_Reset()`: Zeros buffer and resets indices without reallocation
- Always check `Statistics_IsValid()` after initialization before using statistics functions

## Testing

**Test Framework:** Unity (ThrowTheSwitch/Unity)

**Test Location:** `example/test/test_statistics.c`

**Test Coverage:**
- Initialization and cleanup
- Mean calculation (U8, U16, Float)
- Max/Min determination (all types)
- Ring buffer rotation behavior
- Edge cases (all zeros, rotation overwrites)

**Running Tests:**
```bash
cd example
pio test -e native_unittest
```

## Project Structure

```
.
├── include/
│   ├── statistics.h         # Public API declarations
│   └── statistics_config.h  # Port layer and type configuration
├── src/
│   └── statistics.c         # Core implementation
├── example/                 # PlatformIO example project
│   ├── platformio.ini       # PlatformIO configuration
│   ├── src/main.c          # Example usage
│   ├── test/               # Unit tests
│   └── scripts/            # Build scripts
├── docs/                    # Sphinx/Doxygen documentation
├── library.json            # PlatformIO library manifest
└── .clang-format           # Code style configuration
```

## Configuration and Customization

**Disabling Type Support:**
```c
// In code before including statistics.h
#define STATISTICS_FLOAT_ENABLED 0
#include "statistics.h"

// Or via compiler flags (recommended)
// -DSTATISTICS_FLOAT_ENABLED=0
```

**Custom Memory Allocator:**
```c
#define STATISTICS_PORT_USER
#define statPortMalloc(size) my_malloc(size)
#define statPortFree(ptr) my_free(ptr)
#define statPortMemcpy(dest, src, count) my_memcpy(dest, src, count)
#define statPortCalloc(num, size) my_calloc(num, size)
#define statPortMemset(ptr, val, count) my_memset(ptr, val, count)
#include "statistics_config.h"
```

## Typical Usage Pattern

```c
Statistics stat;
Statistics_Init(&stat, sizeof(uint16_t), 100);  // 100 samples of uint16_t

if (!Statistics_IsValid(&stat)) {
    // Handle initialization failure
    return ERROR;
}

// Collect samples
for (int i = 0; i < 150; i++) {
    uint16_t value = read_sensor();
    Statistics_AddSample(&stat, &value);
}

// Compute statistics (after rotation)
if (Statistics_HaveEnoughSamples(&stat)) {
    int64_t mean = Statistics_Mean_U16(&stat);     // Returns mean * 1000
    uint16_t max = Statistics_Max_U16(&stat);
    uint16_t min = Statistics_Min_U16(&stat);
    int64_t variance = Statistics_Variance_U16(&stat);  // Returns variance * 1000
    int64_t stdev = Statistics_Stdev_U16(&stat);   // Returns stdev * 1000
}

Statistics_Free(&stat);
```

## Important Notes

- The library uses `library.json` for PlatformIO Registry packaging
- Version is defined in `library.json:3` (currently 1.2.0)
- Pre-commit hooks enforce clang-format style (`.pre-commit-config.yaml`)
- When adding new types, update both declaration macros in `.h` and implementation macros in `.c`
- Integer overflow is handled by Welford's algorithm in variance/stdev calculations
- Always test with rotation scenarios to verify ring buffer behavior
