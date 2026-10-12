# Multiplying the fuzzy phases, with every boundary retained

**The ten original fuzzy phases contain less information than multiplication
needs.** With matching cascade depths, the first products have exact,
finer cycles: 100 phases for multiplication by a directional tenth and
500 phases for the square. The full finite numbers remain distinct even
when their central patterns repeat.

Claude's summary is [STATE_OF_THE_ART.md](../../STATE_OF_THE_ART.md). The
[review of its quadrant and infinity claims](REVIEW.md) identifies what
is useful and where qualifications are needed. The `pitfalls/` arrangement
and that summary were left as found.

## What the quadrants tell us

With the axes in their usual orientation:

```text
       II: X cascades       | I: origin-generated integers
       and their carries   |    and overlap of forward flows
    -----------------------+------------------------------
       III: mixed products | IV: Y cascades
       can put digits here |     and their carries
```

The negative X-axis belongs to the X cascade region; the negative Y-axis
belongs to the Y cascade region. Starting with these inputs, forward carries
only move up or right. Addition and subtraction cannot introduce a digit
into quadrant III. Multiplying different fraxis types can.

That is a useful account of **how the grids are generated**. It is not an
intrinsic classification of their values. One carry changes

```text
10 at (-1,-1)  →  1 at (-1,0), plus 1 at (0,-1).
```

These are equal representations, with different quadrant-III occupancy.
Also, not every symmetric grid in quadrant I is an integer: a lone digit
at `(1,1)` is a counterexample. At depth 9, even the mixed product F has
its raw 10 at `(-1,-1)` carried completely out of quadrant III.

## The experiment and its conventions

Let `L_N` mean `0.1X` at depth N, `R_N` mean `0.1Y` at depth N, and
`F_N=L_N R_N`. Depth N has `n=N+1` cascade terms. First compare these
four families, with **all factors at the same depth**:

| Family | Meaning |
|---|---|
| F | Original mixed product |
| FL | Original mixed product multiplied by another X tenth |
| FR | Original mixed product multiplied by another Y tenth |
| F² | Original mixed product squared |

The native library uses its existing Positive display policy. For exact
phase counting we additionally convert every complete grid to one fixed
digit convention: **digits −4 through 5**, carrying upward and rightward.
This removes ambiguity between different carried displays. It changes
neither the represented number nor the native library's settings.

Every conversion records its integer carry at every changed cell. Replaying
the three-cell carry rule reconstructs the output exactly, including all
boundaries. This comparison does not assign real values to grid positions.

## Ten phases become 100 and 500

Under that fixed digit convention, the exact eventual periods are:

| Family | Depth increment that repeats every fixed window | Number of infinite display phases |
|---|---:|---:|
| F | 10 | 10 |
| FL | 100 | 100 |
| FR | 100 | 100 |
| F² | 500 | 500 |

“Every fixed window” means that for any chosen window, once both endpoints
are sufficiently far away, increasing the depth by the indicated amount
repeats all digits in that window. It does **not** mean that the complete
finite grids have become equal.

Think of the original phase as a clock reading. Seeing the same reading
does not tell us how many times the hands have gone around. Multiplication
can reveal some of that additional information in nearby digits.

A particularly simple example uses the native backend display directly:

| Cascade depth | F at `(-1,-1)` | FL at `(-2,-1)` | FR at `(-1,-2)` |
|---|---:|---:|---:|
| 20 | 1 | 1 | 1 |
| 30 | 1 | 6 | 6 |

The input F patterns have the same phase; their products do not even agree
at these nearby cells. For the square, the distinction persists at another
level:

| Depth | F² at `(-2,-2)` | F² at `(-2,-1)` | F² at `(-2,0)` |
|---|---:|---:|---:|
| 20 | 1 | 2 | −1 |
| 30 | 1 | 3 | −1 |
| 120 | 1 | 2 | −3 |
| 520 | 1 | 2 | −1 |

This second table uses the fixed −4…5 convention. The lowest square layer
repeats after 10 depth steps; the next layer needs 100; the next needs 500.
The [proof](PROOFS.md) shows that these are exact minimal periods, not
guesses from the table. It also proves that no further layer needs a longer
period for these particular products.

The Positive-policy windows tested in the backend follow the same candidate
periods. That finite observation is separate from the theorem for the fixed
digit convention. Positive displays can move carries differently farther
from the center, so arbitrary GUI windows are not covered by that theorem.

## What the boundary keeps

Use `E=q^n`, where `q=-Y/X` is a signed diagonal shift. E marks the moving
X endpoint, and `E⁻¹` marks the matching Y endpoint. They obey `EE⁻¹=1`.

The complete products satisfy:

\[
\begin{aligned}
100F&=2-E-E^{-1},\\
1000FL&=3-3E+E^2-E^{-1},\\
1000FR&=3-3E^{-1}+E^{-2}-E,\\
10000F^2&=6-4E-4E^{-1}+E^2+E^{-2}.
\end{aligned}
\]

Each equation is an exact equality of finite grids under carries. The
integer scale on the left avoids assuming an additional division operation.
No endpoint on the right is deleted. Increasing the depth moves E and
changes the complete number even when its local phase repeats.

Thus these products do have a compact pattern description. A table whose
inputs are only the ten original phase digits does not contain enough data
to reproduce their multiplication. We keep the endpoints or the complete
cutoff recipe alongside the displayed phase.

## Multiplying different phases and independent depths

The backend also checked **all 100 ordered pairs** of F representatives at
depths 20 through 29. These cover all ten original phase labels on each side.
The saved table records the actual depths and boundary expression of every
product. It is a table of specified representatives, not a universal
multiplication table for phase labels alone.

For a direct witness, `F20 × F25` and `F30 × F25` have the same two input
phase digits, 1 and 6. Nevertheless the native Positive display at
`(-2,-1)` changes from 1 to 2. The independent carry checks confirm that
the complete products are unequal.

For independent term counts n and m, products contain the shifts
`q^(n−m)` and `q^(m−n)`. If the two depths grow with a fixed difference,
those shifts stay in fixed locations. Consequently, the relative positions
of the two far boundaries can contribute near the origin.

The matched-depth 10/100/500 theorem does not silently include these other
ways of growing the inputs. Their exact boundary formulas, also proved in
[PROOFS.md](PROOFS.md), specify what changes.

## Checks and reproduction

The current corrected `build/fraxis/libmdn.so` was used with exact text
inputs, base 10, Positive sign policy, and unlimited precision. Only the
small probe was compiled; the library and GUI were not changed.

- **241 native cases, 4,953 exact checks, zero failures.** These include
  matched depths 0–129 and 520, the 100 phase-pair representatives, and
  selected independent-cutoff comparisons.
- Full native grids were compared both by exact formal carry identities
  and by independent fixed-digit carry sweeps. Boundary certificates were
  checked for every product.
- **726 explicit carry certificates, zero failures** in the separate
  canonical study. Radius-three windows distinguished all 10, 100, 100,
  and 500 predicted phase patterns over consecutive term counts.
- **Six unit tests passed**, including invariance under alternative finite
  carries, quadrant counterexamples, raw coefficient formulas, and the
  proved local periods.

Finite checks verify the implementation and examples. The infinite-window
classification is established by the proof, which uses only exact integer
coefficients and the local carry rule.

From the repository root:

```bash
python3 -m unittest discover -s research/phase_products -p 'test_*.py'
python3 research/phase_products/experiments.py
python3 research/phase_products/backend_check.py
```

The native suite took about 53 seconds in the recorded run and has an
explicit per-case timeout. Probe compilation is copied to the session's
home-directory build log. Complete native grids are regenerable and stored
in the temporary directory recorded by the results.

Files: [backend results](backend_results.json),
[canonical results](canonical_results.json), [carry implementation](canonical.py),
[backend checker](backend_check.py), [proofs](PROOFS.md), and
[review of Claude's summary](REVIEW.md).
