API reference
=============

This section documents the public C API of the library. The documentation is
extracted from header comments and grouped using Doxygen.

Basic statistics
----------------

- Structures and functions live in the ``statistics_core`` group.
- Header file: ``include/statistics.h``.
- A ``Statistics`` instance is a ring buffer of :math:`N` samples of one scalar
  type, allocated in ``Statistics_Init`` (port layer ``statPortMalloc``) and
  released by ``Statistics_Free``. When the buffer is full, the oldest sample
  is overwritten.

Lifecycle and buffer functions:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Function
     - Purpose
   * - ``Statistics_Init(stat, itemSize, samplesCount)``
     - Allocate and zero a window of ``samplesCount`` samples of ``itemSize`` bytes.
   * - ``Statistics_IsValid(stat)``
     - ``false`` after a failed allocation or after ``Statistics_Free``.
   * - ``Statistics_AddSample(stat, &sample)``
     - Store one sample at the write index, then advance and wrap the index.
   * - ``Statistics_HaveEnoughSamples(stat)``
     - ``true`` once the window has been filled at least once.
   * - ``Statistics_Reset(stat)``
     - Zero the samples, reset the write index and the "enough" flag.
   * - ``Statistics_Free(stat)``
     - Release the buffer and mark the instance invalid.

Type-specific functions are generated for each type enabled in
``statistics_config.h`` (see :doc:`user_configuration`), with the suffix
``<T>`` in ``U8``, ``I8``, ``U16``, ``I16``, ``U32``, ``I32`` and ``F``:

.. list-table::
   :header-rows: 1
   :widths: 28 24 24 24

   * - Function
     - Integer types return
     - ``F`` returns
     - On error
   * - ``Statistics_Mean_<T>``
     - ``int64_t``, mean × 1000
     - ``float``
     - ``0``
   * - ``Statistics_Min_<T>`` / ``Statistics_Max_<T>``
     - sample type
     - ``float``
     - ``0``
   * - ``Statistics_Variance_<T>``
     - ``int64_t``, variance × 1000
     - ``float``
     - ``-1`` (``F``: NaN)
   * - ``Statistics_Stdev_<T>``
     - ``int64_t``, stdev × 1000
     - ``float``
     - ``-1`` (``F``: NaN)

An error is an invalid instance, or fewer than two samples for variance and stdev.

.. rubric:: Mathematical definition

The functions always work on all :math:`N` slots of the window
(:math:`N` = ``samplesCnt``, the capacity), :math:`x_1 \dots x_N`. Slots that
have not been written yet hold 0, so check ``Statistics_HaveEnoughSamples``
before using the results.

.. math::

   \mathrm{Max} = \max_i x_i, \qquad
   \mathrm{Min} = \min_i x_i, \qquad
   \bar{x} = \frac{1}{N} \sum_{i=1}^{N} x_i

.. math::

   s^2 = \frac{1}{N-1} \sum_{i=1}^{N} \left(x_i - \bar{x}\right)^2 \quad (N \ge 2), \qquad
   s = \sqrt{s^2}

Integer types use 64-bit integer arithmetic only and return values scaled by
1000. :math:`R(\cdot)` is rounding half away from zero:

.. math::

   \mathrm{Mean} = R\!\left(1000\,\bar{x}\right), \qquad
   \mathrm{Variance} = V \approx R\!\left(1000\,s^2\right), \qquad
   \mathrm{Stdev} = \left\lfloor \sqrt{1000\,V} \right\rfloor \approx 1000\,s

:math:`V` comes from Welford's online algorithm: intermediate divisions
truncate, the final division by :math:`N - 1` rounds. Example: a return value
of ``890250`` means ``890.250``.

The float type computes in ``float``:

.. math::

   \mathrm{Mean} = \bar{x}, \qquad
   \mathrm{Variance} = \frac{\sum x_i^2 - \left(\sum x_i\right)^2 / N}{N - 1}, \qquad
   \mathrm{Stdev} = \frac{\left\lfloor \sqrt{10^{6}\,\mathrm{Variance}} \right\rfloor}{1000}

so the float stdev has a resolution of 0.001.

.. rubric:: Functions

.. doxygengroup:: statistics_core
   :project: Statistics
   :content-only:

.. rubric:: Statistics structure

.. doxygenstruct:: Statistics
   :project: Statistics
   :members:

.. rubric:: Header file

.. doxygenfile:: include/statistics.h
   :project: Statistics

Exponential smoothing filter
-----------------------------

- Structures and functions live in the ``exponential_smoothing_core`` group.
- Header file: ``include/exponential_smoothing.h``.

.. rubric:: Mathematical definition

With the smoothing factor :math:`N` (``factor``), the accumulator
:math:`B` holds the filtered value scaled by :math:`N`:

.. math::

   B_0 = N\,y_0, \qquad
   B_k = B_{k-1} - \frac{B_{k-1}}{N} + x_k, \qquad
   y_k = \frac{B_k}{N}

which is the exponential moving average with :math:`\alpha = 1/N`:

.. math::

   y_k = y_{k-1} + \frac{1}{N}\left(x_k - y_{k-1}\right)

.. doxygengroup:: exponential_smoothing_core
   :project: Statistics
   :content-only:

Linear regression
------------------

- Structures and functions live in the ``linear_regression_core`` group.
- Header file: ``include/linear_regression.h``.

.. rubric:: Mathematical definition

Each point :math:`(x_k, y_k)` updates the means and the centered sums with
Welford's online algorithm (:math:`k` = number of points after the update):

.. math::

   \delta_x = x_k - \bar{x}_{k-1}, \quad
   \delta_y = y_k - \bar{y}_{k-1}, \qquad
   \bar{x}_k = \bar{x}_{k-1} + \frac{\delta_x}{k}, \quad
   \bar{y}_k = \bar{y}_{k-1} + \frac{\delta_y}{k}

.. math::

   S_{xx} \mathrel{+}= \delta_x \left(x_k - \bar{x}_k\right), \quad
   S_{yy} \mathrel{+}= \delta_y \left(y_k - \bar{y}_k\right), \quad
   S_{xy} \mathrel{+}= \delta_x \left(y_k - \bar{y}_k\right)

so that over :math:`n` points

.. math::

   S_{xx} = \sum \left(x_i - \bar{x}\right)^2, \quad
   S_{yy} = \sum \left(y_i - \bar{y}\right)^2, \quad
   S_{xy} = \sum \left(x_i - \bar{x}\right)\left(y_i - \bar{y}\right)

The centered sums keep the results stable when the data has a large constant
offset. The result (:math:`n \ge 3`) of the line :math:`y = m x + b`:

.. math::

   m = \frac{S_{xy}}{S_{xx}} \;(0 \text{ if } S_{xx} = 0), \qquad
   b = \bar{y} - m\,\bar{x}, \qquad
   r = \frac{S_{xy}}{\sqrt{S_{xx} S_{yy}}} \;(1 \text{ if } S_{xx} = 0 \text{ or } S_{yy} = 0)

The residual sum of squares of the fitted line, clamped against rounding:

.. math::

   \mathrm{SSE} = \max\!\left(0,\; S_{yy} - \frac{S_{xy}^2}{S_{xx}}\right) \;(S_{xx} > 0), \qquad
   \mathrm{jitter} = \sqrt{\frac{\mathrm{SSE}}{n - 2}}

With :math:`S_{xx} = 0` the jitter uses :math:`S_{yy}` in place of SSE.

Slope uncertainty with the coverage factor :math:`k` and the Y resolution
:math:`\Delta_y`, :math:`u_{\mathrm{res}}^2 = \Delta_y^2 / 3`:

.. math::

   U_m =
   \begin{cases}
   \infty & n \le 1 \text{ or } S_{xx} = 0 \\
   k \sqrt{u_{\mathrm{res}}^2 / S_{xx}} & n = 2 \\
   k \sqrt{\left(\mathrm{SSE} / (n - 2) + u_{\mathrm{res}}^2\right) / S_{xx}} & n \ge 3
   \end{cases}

:math:`U_m` does not change when all X or all Y values are shifted by a
constant.

.. doxygengroup:: linear_regression_core
   :project: Statistics
   :content-only:


Integer math helpers
--------------------

- Structures and functions live in the ``statistics_int_math`` group.
- Header file: ``include/statistics_int_math.h``.

.. rubric:: Mathematical definition

:math:`R(\cdot)` is rounding half away from zero,
:math:`R(q) = \mathrm{sgn}(q) \left\lfloor |q| + \tfrac{1}{2} \right\rfloor`.

.. math::

   \mathrm{FloorShift}(v, s) = \left\lfloor \frac{v}{2^{s}} \right\rfloor \;(s > 63 \text{ acts as } 63), \qquad
   \mathrm{DivRound}(v, d) = R\!\left(\frac{v}{d}\right) \;(d > 0), \qquad
   \mathrm{Isqrt64}(v) = \left\lfloor \sqrt{v} \right\rfloor

.. math::

   \mathrm{Clamp}(v, lo, hi) = \min\!\left(\max(v, lo), hi\right) \quad (lo \le hi)

Linear range mapping from :math:`[x_0, x_1]` (``inMin``, ``inMax``) to
:math:`[y_0, y_1]` (``outMin``, ``outMax``), without output clamping:

.. math::

   \mathrm{Map\_I32}(x) =
   \begin{cases}
   y_0 & x_0 = x_1 \\
   \mathrm{sat}_{32}\!\left(y_0 + R\!\left(\dfrac{(x - x_0)(y_1 - y_0)}{x_1 - x_0}\right)\right) & \text{otherwise}
   \end{cases}

:math:`\mathrm{sat}_{32}` limits to :math:`[-2^{31}, 2^{31} - 1]`.
``StatFloat_Clamp`` uses the same clamp definition. ``StatFloat_Map`` is the
same mapping without :math:`R` and without saturation.

.. doxygengroup:: statistics_int_math
   :project: Statistics
   :content-only:

Fixed-point exponential smoothing filter
----------------------------------------

- Structures and functions live in the ``exponential_smoothing_i32_core`` group.
- Header file: ``include/exponential_smoothing_i32.h``.

.. rubric:: Mathematical definition

The accumulator :math:`A` holds the filtered value in Q\ :math:`F` fixed point
(:math:`F` = ``frac``, :math:`s` = ``shift``). Inputs are first clamped to the
representable range, :math:`c(x) = \mathrm{Clamp}\left(x, \lfloor -2^{31} / 2^F \rfloor, \lfloor (2^{31} - 1) / 2^F \rfloor\right)`:

.. math::

   A_0 = 2^{F} c(y_0), \qquad
   A_k = A_{k-1} + \left\lfloor \frac{2^{F} c(x_k) - A_{k-1}}{2^{s}} \right\rfloor

.. math::

   \mathrm{GetValue} = \left\lfloor \frac{A_k + 2^{F-1}}{2^{F}} \right\rfloor \;(F > 0), \qquad
   \mathrm{GetValue} = A_k \;(F = 0)

This is the exponential moving average
:math:`y_k = y_{k-1} + \alpha (x_k - y_{k-1})` with :math:`\alpha = 2^{-s}`.

.. doxygengroup:: exponential_smoothing_i32_core
   :project: Statistics
   :content-only:

Integer mean accumulator
------------------------

- Structures and functions live in the ``mean_accumulator_i32_core`` group.
- Header file: ``include/mean_accumulator_i32.h``.

.. rubric:: Mathematical definition

For the :math:`n` added samples :math:`x_1 \dots x_n`:

.. math::

   S = \sum_{i=1}^{n} x_i, \qquad
   \mathrm{Mean} =
   \begin{cases}
   0 & n = 0 \\
   R\!\left(S / n\right) & n \ge 1
   \end{cases}

with :math:`R` rounding half away from zero (``StatInt_DivRound``).

.. doxygengroup:: mean_accumulator_i32_core
   :project: Statistics
   :content-only:

Integer sliding-window regression
---------------------------------

- Structures and functions live in the ``window_regression_i32_core`` group.
- Header file: ``include/window_regression_i32.h``.

.. rubric:: Mathematical definition

The window holds :math:`n` samples :math:`(t_i, y_i)` per channel, with
:math:`t_N`, :math:`y_N` the newest one. Ages are wrap-safe uint32
differences, limited to the oldest age :math:`a_{\max}`, and scaled by the
smallest :math:`s` with :math:`\lfloor a_{\max} / 2^s \rfloor < 2^{13}`:

.. math::

   a_i = (t_N - t_i) \bmod 2^{32}, \qquad
   x_i = -\left\lfloor \frac{a_i}{2^{s}} \right\rfloor, \qquad
   d_i = \mathrm{Clamp}\!\left(y_i - y_N, -2^{20}, 2^{20}\right)

.. math::

   S_x = \sum x_i, \quad S_{xx} = \sum x_i^2, \quad
   S_d = \sum d_i, \quad S_{xd} = \sum x_i d_i

The fit needs :math:`n \ge \max(\mathrm{minCount}, 2)` and
:math:`n S_{xx} - S_x^2 > 0`. Slope :math:`m` in Q16 per x unit, clamped to
:math:`\pm m_{\max}` when :math:`m_{\max}` (``slopeMaxQ16``) is not 0:

.. math::

   m = \mathrm{Clamp}\!\left(R\!\left(\frac{2^{16}\,(n S_{xd} - S_x S_d)}{2^{s}\,(n S_{xx} - S_x^2)}\right), -m_{\max}, m_{\max}\right)

Level :math:`L` in Q16 at :math:`t_N`: the line goes through the centroid
:math:`(\bar{t}, \bar{y})`, also when :math:`m` is clamped:

.. math::

   L = 2^{16} y_N + R\!\left(\frac{2^{16} S_d + m \sum a_i}{n}\right)
   \;\approx\; 2^{16}\left(\bar{y} + \frac{m}{2^{16}} \left(t_N - \bar{t}\right)\right)

Evaluation at :math:`t`, with the signed difference
:math:`\Delta = (t - t_N) \bmod 2^{32}` taken in :math:`[-2^{31}, 2^{31})`:

.. math::

   \mathrm{Evaluate}(t) = \mathrm{sat}_{32}\!\left(\left\lfloor \frac{L + m\,\Delta + 2^{15}}{2^{16}} \right\rfloor\right)

.. doxygengroup:: window_regression_i32_core
   :project: Statistics
   :content-only:
