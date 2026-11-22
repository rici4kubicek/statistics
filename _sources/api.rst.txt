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
