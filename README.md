# Statistics

![CodeRabbit Pull Request Reviews](https://img.shields.io/coderabbit/prs/github/rici4kubicek/statistics?utm_source=oss&utm_medium=github&utm_campaign=rici4kubicek%2Fstatistics&labelColor=171717&color=FF570A&link=https%3A%2F%2Fcoderabbit.ai&label=CodeRabbit+Reviews)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/rici4kubicek/statistics/main.svg)](https://results.pre-commit.ci/latest/github/rici4kubicek/statistics/main)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/rici4kubicek/library/Statistics.svg)](https://registry.platformio.org/libraries/rici4kubicek/Statistics)

A lightweight C library for collecting fixed-size samples and computing basic
statistics (mean, min, max, variance, and standard deviation) for selected
scalar types. The documentation is generated with Doxygen and Sphinx (Breathe).

## Key features
- minimal dependencies, small API surface
- ring buffer with an indicator of “enough” samples collected
- type-specific functions generated based on configuration
