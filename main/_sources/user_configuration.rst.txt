User configuration
==================

This section explains how to customize the library for your project: enable or
disable type-specific APIs, replace memory/copy primitives, and pass options via
your build system.

Overview
--------

- Type support is controlled by macros in ``include/statistics_config.h``.
- A simple port layer allows you to plug in your own allocator and memcpy.
- The processing window (sample size and count) is chosen at runtime in
  ``Statistics_Init``.

Type support toggles
--------------------

Use these compile-time switches to enable/disable the generated typed functions
for basic statistics. If a macro is not defined, it defaults to ``1`` (enabled).

Defined in ``include/statistics_config.h``:

.. code-block:: c

   #ifndef STATISTICS_U8_ENABLED   /* uint8_t */
   #define STATISTICS_U8_ENABLED   1
   #endif

   #ifndef STATISTICS_I8_ENABLED   /* int8_t */
   #define STATISTICS_I8_ENABLED   1
   #endif

   #ifndef STATISTICS_U16_ENABLED  /* uint16_t */
   #define STATISTICS_U16_ENABLED  1
   #endif

   #ifndef STATISTICS_I16_ENABLED  /* int16_t */
   #define STATISTICS_I16_ENABLED  1
   #endif

   #ifndef STATISTICS_U32_ENABLED  /* uint32_t */
   #define STATISTICS_U32_ENABLED  1
   #endif

   #ifndef STATISTICS_I32_ENABLED  /* int32_t */
   #define STATISTICS_I32_ENABLED  1
   #endif

   #ifndef STATISTICS_FLOAT_ENABLED /* float */
   #define STATISTICS_FLOAT_ENABLED 1
   #endif

How to change values:

- Compiler flags (recommended):

  - GCC/Clang

    .. code-block:: bash

       cc -DSTATISTICS_FLOAT_ENABLED=0 -DSTATISTICS_U8_ENABLED=0 ...

  - CMake

    .. code-block:: cmake

       target_compile_definitions(my_target PRIVATE
         STATISTICS_FLOAT_ENABLED=0
         STATISTICS_U8_ENABLED=0)

  - PlatformIO (``platformio.ini``)

    .. code-block:: ini

       build_flags =
         -DSTATISTICS_FLOAT_ENABLED=0
         -DSTATISTICS_U8_ENABLED=0

- Or define prior to including ``statistics_config.h`` in a translation unit:

  .. code-block:: c

     #define STATISTICS_FLOAT_ENABLED 0
     #include <statistics.h>

Port layer (custom allocators/memcpy)
-------------------------------------

By default the library uses the C runtime: ``malloc``, ``calloc``, ``free`` and
``memcpy``. To override this, define ``STATISTICS_PORT_USER`` in your build and
provide compatible macros before including ``statistics_config.h``:

Required macros when using a custom port:

- ``statPortMalloc(size)``
- ``statPortCalloc(num, size)``
- ``statPortFree(ptr)``
- ``statPortMemcpy(dest, src, count)``
- ``statPortMemset(ptr, val, count)``

Example (wrapping a custom allocator):

.. code-block:: c

   #define STATISTICS_PORT_USER 1
   #include <my_allocator.h>

   #define statPortMalloc(sz)               my_malloc(sz)
   #define statPortCalloc(n, sz)            my_calloc(n, sz)
   #define statPortFree(p)                  my_free(p)
   #define statPortMemcpy(d, s, n)          my_memcpy(d, s, n)
   #define statPortMemset(ptr, val, count)  memset(ptr, val, count)

   #include <statistics.h>

Runtime window configuration
----------------------------

The size (in bytes) of one sample and the window length (number of samples)
are passed to ``Statistics_Init`` at runtime:

.. code-block:: c

   Statistics s;
   Statistics_Init(&s, sizeof(uint16_t), 32); // 32-sample window of uint16_t

Guidelines:

- For typed statistics functions (e.g., ``Statistics_Mean_U16``), make sure the
  ``itemSize`` you pass matches the type.
- Memory footprint is approximately ``itemSize * samplesCount`` bytes per
  instance, plus small overhead for the structure itself.

Build system quick reference
----------------------------

- GCC/Clang CLI

  .. code-block:: bash

     cc -Iinclude -DSTATISTICS_U8_ENABLED=0 -c src/statistics.c

- CMake

  .. code-block:: cmake

     add_library(statistics src/statistics.c)
     target_include_directories(statistics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
     target_compile_definitions(statistics PUBLIC STATISTICS_U8_ENABLED=0)

- PlatformIO

  .. code-block:: ini

     [env:my_board]
     platform = native
     build_flags =
       -I include
       -D STATISTICS_U8_ENABLED=0

Safety and constraints
----------------------

- Ensure that your custom port functions are thread-safe if your environment
  requires it.
- Do not mix different ``itemSize`` values with the same ``Statistics`` instance.
- Check ``Statistics_HaveEnoughSamples`` before reading window-based statistics
  such as variance and standard deviation.

Further reading
---------------

- :doc:`api` – the full API reference
- ``include/statistics_config.h`` – configuration and port layer
