# Statistics

![CodeRabbit Pull Request Reviews](https://img.shields.io/coderabbit/prs/github/rici4kubicek/statistics?utm_source=oss&utm_medium=github&utm_campaign=rici4kubicek%2Fstatistics&labelColor=171717&color=FF570A&link=https%3A%2F%2Fcoderabbit.ai&label=CodeRabbit+Reviews)

Implementation of simple statistics functionality for embedded systems.

## Configuration directives (STATISTICS_*_ENABLED)

The library allows you to enable/disable support for individual data types at compile time. This helps minimize the binary size and shorten build times on embedded targets.

Configuration is done via preprocessor directives in `include/statistics_config.h` or via compiler switches (recommended). If the directives are not defined by the user, their default value is `true` (enabled).

Supported directives:

- `STATISTICS_U8_ENABLED` — enables the API for `uint8_t`. Default: `true`.
- `STATISTICS_I8_ENABLED` — enables the API for `int8_t`. Default: `true`.
- `STATISTICS_U16_ENABLED` — enables the API for `uint16_t`. Default: `true`.
- `STATISTICS_I16_ENABLED` — enables the API for `int16_t`. Default: `true`.
- `STATISTICS_U32_ENABLED` — enables the API for `uint32_t`. Default: `true`.
- `STATISTICS_I32_ENABLED` — enables the API for `int32_t`. Default: `true`.
- `STATISTICS_FLOAT_ENABLED` — enables the API for `float`. Default: `true`.

How to disable specific types:

- Makefile / GCC/Clang:
  - Add `-DSTATISTICS_U16_ENABLED=false -DSTATISTICS_FLOAT_ENABLED=false` to `CFLAGS`/`CPPFLAGS`.
  - Example: `CFLAGS += -DSTATISTICS_U16_ENABLED=false -DSTATISTICS_FLOAT_ENABLED=false`

- CMake:
  - `target_compile_definitions(your_target PRIVATE STATISTICS_U16_ENABLED=false STATISTICS_FLOAT_ENABLED=false)`

- PlatformIO (`platformio.ini`):
  - `build_flags = -DSTATISTICS_U16_ENABLED=false -DSTATISTICS_FLOAT_ENABLED=false`

Notes:

- Use plain preprocessor literals `true`/`false` (without quotes).
- If your application doesn’t use a given type, disabling it will reduce the resulting code size and may lower memory footprint.
- To fully control the port layer (memory allocation, etc.), define `STATISTICS_PORT_USER` at compile time and provide the macros described in the `statistics_config.h` header.