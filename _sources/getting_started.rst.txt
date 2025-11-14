Getting started
===============

This page walks you through the basic usage of the library and explains how to
generate the documentation locally (Doxygen → Sphinx).

Installation and project integration
------------------------------------

The library is written in C and consists of the header ``include/statistics.h``
and its implementation in ``src/statistics.c``. To integrate it, include these
files in your build and make sure the configuration header
``include/statistics_config.h`` is available.

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
           uint16_t mean = Statistics_Mean_U16(&stat);
           uint16_t minv = Statistics_Min_U16(&stat);
           uint16_t maxv = Statistics_Max_U16(&stat);
           float var = Statistics_Variance_U16(&stat);
           float sd = Statistics_Stdev_U16(&stat);
           printf("mean=%u min=%u max=%u var=%.3f sd=%.3f\n", mean, minv, maxv, var, sd);
       }

       Statistics_Free(&stat);
       return 0;
   }

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
