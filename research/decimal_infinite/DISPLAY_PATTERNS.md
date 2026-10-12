# Why the normalized mixed product cycles

The matched product `F_N=(0.1X)_N(0.1Y)_N` has a precise family of ten
infinite display patterns in base 10. Choosing its central digit selects
one of these patterns. It does **not** determine the finite cascade length
or remove the boundary information needed for multiplication.

The construction below gives a bounded-digit, carry-equivalent display for
every finite `N>=0`. The corrected backend, with Positive sign policy and
unlimited precision, selected exactly this display for **all tested depths
0 through 120**. This is not a claim that the cleanup algorithm has been
proved to select it at every larger depth or under other sign policies.

## 1. An exact two-layer construction

Write `n=N+1`, the number of terms in each directional cascade. Before
carrying, the mixed product is supported at

\[
P_k=(k-1,-k-1),\qquad -N\le k\le N,
\]

with coefficient

\[
c_k=(-1)^k a_k,\qquad a_k=n-|k|.
\]

Thus its central raw coefficient is `n`, and all these cells satisfy
`x+y=-2`. Define a remainder digit at each of them by

\[
d_k=
\begin{cases}
a_k\bmod10,&k\text{ even},\\
1-((a_k+1)\bmod10),&k\text{ odd}.
\end{cases}
\]

The even remainders lie in `0,...,9`; the odd ones lie in `-8,...,1`.
Both are congruent to `c_k` modulo 10. Put

\[
t_k=(c_k-d_k)/10
\]

inside the support, and `t_k=0` outside it. Carrying this integer amount
from `P_k` adds `t_k` to each of its two upper neighbours. If

\[
Q_h=(h-1,-h),
\]

those neighbours are `Q_k` and `Q_(k+1)`. The complete second layer is
therefore

\[
s_h=t_h+t_{h-1}\quad\text{at }Q_h.
\]

There are no other layers in this construction. The resulting finite grid
is carry-equivalent to `F_N` by the explicit finite collection of carries
just described. No scalar interpretation of the coordinates is involved.

In fact `s_h` is always 0 or -1. To see this, inside the support

\[
t_k=
\begin{cases}
\lfloor(n-|k|)/10\rfloor,&k\text{ even},\\
-\lfloor(n-|k|+1)/10\rfloor,&k\text{ odd}.
\end{cases}
\]

For positive odd `h`, the two terms in `s_h` cancel. For positive even `h`,
their floor arguments differ by 2, giving 0 or -1. Reflection covers the
negative side. The central pair cancels, and the endpoint carries are zero.
Thus both layers already have digits of magnitude less than 10.

## 2. Why a single central digit selects a phase

Increase the depth by 10 while looking at a fixed interior position. Its
raw coefficient changes by `10*(-1)^k`. Its remainder `d_k` does not change,
while its carry changes by `(-1)^k`. On the next layer the two changes
cancel:

\[
\Delta s_h=(-1)^h+(-1)^{h-1}=0.
\]

Hence every fixed cell eventually has the same digit along any subsequence
`N=N0+10r`. These are genuine **cellwise limits of the constructed raw digit
arrays**. Every finite window eventually stops changing on such a subsequence.

The ten possible phases are distinguished at the central position
`P_0=(-1,-1)`:

\[
d_0=n\bmod10\in\{0,1,\ldots,9\}.
\]

Once this phase is chosen, every digit of the corresponding infinite array
is determined by the two-layer formulas. This validates the intuition about
one digit determining the rest **within this particular phase family**.
It is not a statement about arbitrary MDNs that happen to share one digit.

For example, if the central digit is `r`, the adjacent first-layer digits
at `(-2,0)` and `(0,-2)` are `1-r`. The digits at `(-2,1)` and `(1,-2)` are
-1 for phases 0 and 1, and zero otherwise. Some cells, including unity,
stay zero in every phase; not every digit varies during the cycle.

## 3. The same phase does not fix the finite boundaries

The backend gives these exact comparisons. Radius means `max(|x|,|y|)`.

| Depths compared | Central digit in both | First differing radius | Example coordinate | Digits at that coordinate | Nonzero cells |
|---|---:|---:|---|---|---|
| 20 and 30 | 1 | 22 | `(-22,21)` | 0 and -1 | 41 and 61 |
| 20 and 120 | 1 | 22 | `(-22,21)` | 0 and -1 | 41 and 241 |
| 30 and 120 | 1 | 32 | `(-32,31)` | 0 and -1 | 61 and 241 |

The reflected coordinates differ too. Each pair is **unequal under the
full finite carry relation**, despite the identical central display inside
the indicated radius. The endpoint identity still records the distinction:

\[
100F_N=2-q^{N+1}-q^{-N-1},\qquad q=-Y/X.
\]

Thus the central digit determines the eventual display phase, whereas `N`
also determines where the finite pattern ends. Both pieces of information
can matter when the numbers are multiplied.

## 4. A display-array limit does not yet define its multiplication

The phase limits just constructed are not automatically limits in an
algebra where multiplication is continuous. There is a direct obstruction
to simply multiplying their infinite grids by ordinary convolution.

In any one phase, the lowest-layer digits obey `d_k=d_(-k)` and are nonzero
periodically. Its naive square would need the coefficient

\[
\sum_{k\in\mathbb Z}d_kd_{-k}
=\sum_{k\in\mathbb Z}d_k^2
\]

at `(-2,-2)`. This sum diverges. Products involving the upper layer cannot
cancel it at that total degree: their total degrees are -3 or -2, not -4.

This rules out that **naive infinite convolution**, not every possible new
definition of multiplication. The finite products and retained boundary
expressions remain well-defined and provide information a new rule can use.
Selecting a phase alone is therefore not yet a complete arithmetic system.

## 5. Reproducible checks

Run `python3 research/decimal_infinite/backend_check.py` from the repository
root. [backend_results.json](backend_results.json) records:

- 121 exact finite boundary-identity checks for depths 0 through 120;
- complete grid agreement with the two-layer construction in all 121 cases,
  including 14,641 first-layer positions;
- the same-phase/different-boundary comparisons above;
- 120 additional exact decimal-prefix checks for repeating and nonperiodic
  digit streams, with both directional inputs.

The proof establishes the construction and its phase limits for every depth.
The numerical sweep establishes its agreement with the tested backend displays
over the stated finite range. The backend library was not changed.
