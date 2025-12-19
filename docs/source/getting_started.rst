Getting started
===============

This page walks you through the basic usage of the library.

Installation and integration into a project
------------------------------------------

The library is written in C and consists of the header ``include/statistics.h``
and the implementation in ``src/statistics.c``. Configuration is controlled via
``include/statistics_config.h``.

Below are the recommended installation methods, similar to the PlatformIO
Installation page:
https://registry.platformio.org/libraries/rici4kubicek/Statistics/installation

- PlatformIO (recommended)
    Add the dependency to your ``platformio.ini``:

     .. code-block:: ini

        [env:your_env]
        platform = ...
        board = ...
        framework = ...
        lib_deps =
            rici4kubicek/Statistics@^1.0.0

     Note: you can pin the version as needed. The latest version is listed on
     the library page in the PlatformIO registry.

- PlatformIO (install directly from the registry via CLI)
  
  Run in the project root:

  .. code-block:: bash

     pio lib install "rici4kubicek/Statistics"

- CMake / manual integration (without PlatformIO)

  1. Add the source files to your build system:

     - Add ``src/statistics.c`` to your compilation.
     - Make sure the ``include`` directory is on the include path.

     Example for CMake:

     .. code-block:: cmake

        add_library(Statistics STATIC
            ${CMAKE_CURRENT_LIST_DIR}/src/statistics.c)
        target_include_directories(Statistics PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/include)

        # optional: your targets that use the library
        target_link_libraries(my_app PRIVATE Statistics)

  2. Adjust the configuration in ``include/statistics_config.h`` as needed
     (enabled data types, window sizes, etc.).

- As a git submodule
  
  .. code-block:: bash

     git submodule add https://github.com/rici4kubicek/statistics.git external/statistics

  Then add the source and include paths (see the CMake example above) to your
  build.

Quick usage example
-------------------

The following example demonstrates working with the ring buffer of samples and
computing statistics for ``uint16_t`` (adjust according to the enabled types in
``statistics_config.h``):

.. code-block:: c

   #include <statistics.h>
   #include <stdio.h>

   int main(void) {
       Statistics stat;
       Statistics_Init(&stat, sizeof(uint16_t), 8); // window of 8 samples

       uint16_t samples[] = { 10, 12, 9, 13, 11, 12, 10, 14, 15, 9 };
       for (size_t i = 0; i < sizeof(samples)/sizeof(samples[0]); ++i) {
           Statistics_AddSample(&stat, &samples[i]);
       }

       if (Statistics_HaveEnoughSamples(&stat)) {
           int64_t mean = Statistics_Mean_U16(&stat);
           uint16_t minv = Statistics_Min_U16(&stat);
           uint16_t maxv = Statistics_Max_U16(&stat);
           int64_t var = Statistics_Variance_U16(&stat);
           int64_t sd = Statistics_Stdev_U16(&stat);
           printf("mean=%lld.%03lld min=%u max=%u var=%lld.%03lld sd=%lld.%03lld\n",
                  (long long)(mean / 1000), (long long)(mean % 1000),
                  minv, maxv,
                  (long long)(var / 1000), (long long)(var % 1000),
                  (long long)(sd / 1000), (long long)(sd % 1000));
       }

       Statistics_Free(&stat);
       return 0;
   }

Fixed-point arithmetic for integer types
-----------------------------------------

To optimize performance on embedded systems without a Floating Point Unit (FPU),
such as STM32F0 or Cortex-M0 processors, the library uses **integer arithmetic
exclusively** for integer data types (U8, I8, U16, I16, U32, I32). The mean,
variance, and standard deviation functions return ``int64_t`` values scaled by
**1000** instead of ``float``.

**Converting fixed-point results:**

- To get the integer part: ``value / 1000``
- To get the fractional part: ``value % 1000``
- Example: A return value of ``890250`` represents ``890.250``

**Benefits:**

- **Much faster** on CPUs without FPU (can be 10-100x faster)
- **No floating-point library dependencies**
- **Good precision** with 3 decimal places
- **Proper rounding** maintains accuracy

**Error handling:**

``Statistics_Mean_*``, ``Statistics_Variance_*``, and ``Statistics_Stdev_*``
return ``-1`` (or ``0`` for mean with no samples) to indicate an error
(e.g., insufficient samples, invalid statistics object).

**Float type:**

For float types (``Statistics_*_F``), the mean function returns ``float``
directly for maximum precision, while variance and standard deviation still
use fixed-point ``int64_t`` scaled by 1000.

Notes
-----

- The path to the Doxygen XML is configured in ``docs/source/conf.py`` via
  ``breathe_projects`` and points to ``../build/doxygen/xml`` (relative to
  ``docs/source``).
- The documentation theme is ``sphinx_rtd_theme``.

Configuring supported types
---------------------------

The public API generates a set of functions (``Mean``, ``Min``, ``Max``,
``Variance``, ``Stdev``) for selected scalar types. Which types are enabled is
controlled in ``include/statistics_config.h`` via the
``STATISTICS_*_ENABLED`` macros.

Further reading
---------------

- :doc:`api` – detailed API reference including Doxygen groups and entities
- ``docs/Doxyfile`` – Doxygen configuration
- ``docs/source/conf.py`` – Sphinx and Breathe configuration
