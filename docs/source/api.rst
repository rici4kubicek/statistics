API reference
=============

This section documents the public C API of the library. The documentation is
extracted from header comments and grouped using Doxygen.

Overview
--------

- Core structures and functions live in the ``statistics_core`` group.
- Main header file: ``include/statistics.h``.

Groups
------

.. doxygengroup:: statistics_core
   :project: Statistics
   :content-only:

Main structure
--------------

.. doxygenstruct:: Statistics
   :project: Statistics
   :members:

Header file
-----------

.. doxygenfile:: include/statistics.h
   :project: Statistics

Exponential smoothing filter
-----------------------------

- Structures and functions live in the ``exponential_smoothing_core`` group.
- Header file: ``include/exponential_smoothing.h``.

.. doxygengroup:: exponential_smoothing_core
   :project: Statistics
   :content-only:

Linear regression
------------------

- Structures and functions live in the ``linear_regression_core`` group.
- Header file: ``include/linear_regression.h``.

.. doxygengroup:: linear_regression_core
   :project: Statistics
   :content-only:


Integer math helpers
--------------------

- Structures and functions live in the ``statistics_int_math`` group.
- Header file: ``include/statistics_int_math.h``.

.. doxygengroup:: statistics_int_math
   :project: Statistics
   :content-only:

Fixed-point exponential smoothing filter
----------------------------------------

- Structures and functions live in the ``exponential_smoothing_i32_core`` group.
- Header file: ``include/exponential_smoothing_i32.h``.

.. doxygengroup:: exponential_smoothing_i32_core
   :project: Statistics
   :content-only:

Integer mean accumulator
------------------------

- Structures and functions live in the ``mean_accumulator_i32_core`` group.
- Header file: ``include/mean_accumulator_i32.h``.

.. doxygengroup:: mean_accumulator_i32_core
   :project: Statistics
   :content-only:

Integer sliding-window regression
---------------------------------

- Structures and functions live in the ``window_regression_i32_core`` group.
- Header file: ``include/window_regression_i32.h``.

.. doxygengroup:: window_regression_i32_core
   :project: Statistics
   :content-only:
