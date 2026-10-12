# A general rule for the 10, 100, 500 pattern

**Yes: every product of X and Y tenths at a common cutoff has a finite
repeating canonical display cycle. We can calculate its exact length without
building a cascade millions of cells long.** The answer depends on how
many factors come from each direction, not just the total number.

This study keeps the full finite cascades and their boundary information.
It uses integer digits and the three-cell carry rule; it does not assign
an ordinary scalar magnitude to X or Y.

## 1. Name a product by counting its factors

Write `L` for an X tenth and `R` for a Y tenth, each cut off at the same
zero-based depth `N`. Let `n=N+1` count the terms in each cascade.
Define

\[
G_N^{a,c}=L_N^aR_N^c.
\]

Here `a` counts X factors and `c` counts Y factors. For example:

- `G(1,1) = LR` is the original fuzzy family, with its ten phases.
- Multiplying by another X tenth changes `(a,c)` to `(a+1,c)`.
- Multiplying by another Y tenth changes `(a,c)` to `(a,c+1)`.
- Squaring changes `(a,c)` to `(2a,2c)`.

More generally, multiplication adds the two pairs of factor counts,
provided the factors use the same cutoff. Independently chosen cutoffs
remain separate information; see section 5.

## 2. What repeats, and the first answers

Watch any fixed patch of the numberscape while extending all the
cascades together. Eventually, that patch repeats after a fixed number
of extension steps. The **smallest cycle length** is the number of steps
that works for every fixed patch, once its boundary is sufficiently far
away. The distance needed can differ from patch to patch.

This does not say that two complete finite numbers are equal. Their
boundary positions still differ.

For comparison we use one fixed digit convention, `−4,−3,…,5`, and apply
only legal carries. The GUI's Positive convention can display the same
finite number differently. The table refers to these canonical displays.

| X factors | Y factors | Product | Smallest cycle length |
|---:|---:|---|---:|
| 1 | 1 | `LR` | 10 |
| 2 | 1 | `L²R` | 100 |
| 1 | 2 | `LR²` | 100 |
| 2 | 2 | `L²R² = (LR)²` | 500 |
| 3 | 1 | `L³R` | 1,000 |
| 3 | 2 | `L³R²` | 10,000 |
| 4 | 2 | `L⁴R²` | 25,000 |
| 3 | 3 | `L³R³ = (LR)³` | 50,000 |
| 4 | 3 | `L⁴R³` | 100,000 |
| 4 | 4 | `L⁴R⁴ = (LR)⁴` | 500,000 |
| 2 | 8 | `L²R⁸` | 250,000,000 |

Swapping X and Y preserves the cycle length. A product using only one
direction has cycle length 1: each fixed digit eventually settles.

## 3. The general rule, in plain language

Let `d=a+c` be the total number of factors, with both directions present.
Then:

\[
\boxed{\text{The smallest cycle length divides }10^{d-1}.}
\]

“Divides” means it fits into that number an exact whole number of times.
Thus `10^(d−1)` always works, but a shorter cycle may work too.

The carry explanation is that each successive carry layer divides a
coefficient difference by 10 and combines its two neighbours. For these
products, only finitely many layers can distinguish two candidate phases.
If those layers agree, the remaining difference cancels exactly. That lets us test
the entire eventual array with finitely many integer calculations.

The exact procedure is:

1. Start with `10^(d−1)` as a known cycle length.
2. Try halving it. Keep the shorter length if the carry test passes,
   and repeat until halving fails.
3. Do the same with division by 5.
4. The remaining length is the exact smallest cycle.

Only 2 and 5 are needed because they are the prime factors of 10. The
test checks a finite set of polynomial remainders, with a proof that
passing those checks covers every position and sufficiently deep cutoff.
It does not guess from a small displayed patch. The formulas, proof,
and finite test are in [PROOFS.md](PROOFS.md), sections 3–5; the executable
version is [periods.py](periods.py).

There is also a useful short formula for a necessary part of the period:

\[
T_0=\frac{10^{a+c-1}}
{\gcd\!\left(10^{a+c-1},\binom{a+c-2}{a-1}\right)}.
\]

The true cycle must be a multiple of `T₀` and a divisor of `10^(d−1)`.
Here `gcd` means greatest common divisor, and the bracket is an ordinary
binomial coefficient, counting combinations. These are counts of digit
contributions, not magnitudes assigned to MDN positions.

This explains the first examples: the divisor is 1 for `LR` and `L²R`,
giving 10 and 100; it is 2 for `L²R²`, shortening 1,000 to 500.
The short formula is the exact answer for every mixed product with at
most nine factors. **It is not the general answer:** for `L²R⁸` it gives
125,000,000, but a lower carry layer requires twice that. The complete
test returns 250,000,000.

When one direction occurs just once, the short formula is always exact:
`LR^c` has cycle `10^c`, and `L^aR` has cycle `10^a`. Therefore the
family's cycle lengths grow without bound as more factors are allowed.

## 4. Cycle length and number of different phases can diverge

For the original 10, 100 and 500 cases, all positions in the cycle give
different arrays. That need not remain true at higher powers.

For `L²R⁸`, the full cycle has **250,000,000 steps**, but contains
**187,500,000 different arrays**. Shifting by half the cycle gives the
same array at every even term count `n`, and a different array at every
odd term count. The whole sequence therefore needs the full cycle to
repeat, even though some of its entries coincide.

This is an exact array statement, proved in
[PHASE_COLLISIONS.md](PHASE_COLLISIONS.md). The phase label also does not
replace the boundary record: equal limiting displays can retain different
boundary histories.

## 5. Keep the boundary attached

Let `q=−Y/X` denote a signed diagonal positional shift. Let
`E=q^n` record the X cascade's endpoint; the Y endpoint is `E⁻¹` when
the depths match. Here the inverse means the opposite grid shift, not
an assumption about an ordinary reciprocal magnitude.

The complete finite boundary identity is

\[
\boxed{10^{a+c}G_N^{a,c}=(1-E)^a(1-E^{-1})^c.}
\]

Read this as: each X factor brings its endpoint factor `1−E`; each Y
factor brings `1−E⁻¹`. Multiplication keeps all of them. It is an exact
consequence of carries, for every finite depth. Neither endpoint is
declared zero.

For example, the starting fuzzy product satisfies
`100 LR = 2−E−E⁻¹`. Its square retains
`10,000 L²R² = E²−4E+6−4E⁻¹+E⁻²`.
Those endpoint terms remain part of the number even while a fixed patch
of its display repeats.

With independent term counts `nᵢ` and `mⱼ`, the record instead is

\[
10^{a+c}\prod_iL_{n_i-1}\prod_jR_{m_j-1}
=\prod_i(1-q^{n_i})\prod_j(1-q^{-m_j}).
\]

The period rule above covers extending all factors at the same depth.
Different offsets or rates require their own boundary records and
analysis. Infinite objects can retain their finite refinement recipe
and these identities; multiplying bare infinite display arrays is not
being assumed to work.

## 6. What was checked

The native probe links the corrected `build/fraxis/libmdn.so`, with
unlimited precision (`−1`). It constructs the finite X and Y cascades
and multiplies them in both factor orders.

- **989 native backend cases; 14,832 exact checks; zero failures.** These
  include every orientation through six factors at depths 0–30, selected
  seven-, eight- and ten-factor products, deeper cases up to depth 120,
  and three independent-cutoff cases. Checks compare complete canonical
  grids, verify explicit carry certificates, and verify full boundary
  identities.
- **372 separate exact formula checks at large cutoffs; zero failures.**
  These compare local windows across predicted periods, including
  250,000,000. No native grid hundreds of millions of cells long was built.
  Two independently implemented ways to calculate local digits agree.
- **120 minimal-period certificates**, covering every mixed pair through
  sixteen factors, with zero failures. Each records a passing period and
  exact witnesses rejecting its next smaller prime-divisor candidates.
  The general proof is not limited to sixteen factors.
- **Six unit-test groups pass**, including raw coefficients against
  independent convolution, local predictions against complete carrying,
  the shortcut's exception, and the phase collision.

In 340 native cases, changing multiplication order changed the Positive
display. All those pairs gave the same complete canonical number, with
valid carry certificates. This is why display comparison needs a fixed
convention.

The tested library was unchanged, with SHA-256
`5ef977e17730182221b7ad66dc4030f839fe892123f6f1dae4bf3133ab509b43`.
The compact records are [backend_results.json](backend_results.json) and
[period_results.json](period_results.json). Full native grids and carry
certificates from this run are in `/tmp/mdn-general-products-01a12881`;
that temporary directory can be regenerated.

## 7. Reproduce the work

From the repository root:

```bash
python3 -m unittest discover -s research/general_products -p 'test_*.py' -v
python3 research/general_products/experiments.py
python3 research/general_products/backend_check.py
```

The last command compiles only the research probe, links the existing
corrected backend, and saves full finite grids and carry certificates.
Its compilation transcript is appended to
`~/output_01a12881-e6c8-7cb2-a1d2-7c899e17aa69.log`.
Use `--work` and `--build-log` to choose other locations. The core library
and GUI have not been edited by this study.
