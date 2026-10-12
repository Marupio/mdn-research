# Keeping the boundary as part of an infinite MDN

This continues [the finite fuzzy-family study](README.md). The construction
below makes a precise choice: an infinite object records how its complete
finite grids develop as the cascade grows. Endpoints remain part of that
record. It does not assign ordinary scalar magnitudes to the grid axes.

The next study, [infinite decimals and marching digits](../decimal_infinite/README.md),
extends this model to infinite input digit streams and proves the ten-phase
display pattern of the mixed-tenth family while retaining its boundary recipe.

## What X + Y = b means

The capital letters name positions, not the fractional inputs `0.1X` and
`0.1Y`:

| Symbol | Grid pattern |
|---|---|
| `1` | One at `(0,0)` |
| `X` | One at `(1,0)` |
| `Y` | One at `(0,1)` |
| `b` | `b` ones at `(0,0)`; ten in a decimal grid |

A carry of one removes `b` from the root and adds one to each neighbour.
Therefore these two patterns represent the same number:

\[
\underbrace{b\text{ at the root}}_{b\cdot1}
\quad\longleftrightarrow\quad
\underbrace{1\text{ at each neighbour}}_{X+Y}.
\]

That is the whole meaning of `X+Y=b`. It assigns neither neighbour an
individual real value. Multiplying by `X` shifts an entire pattern one cell
in X; multiplying by `Y` shifts it one cell in Y.

Your general carry of an integer amount `K` at `(i,j)` changes the grid by

\[
-bKX^iY^j+KX^{i+1}Y^j+KX^iY^{j+1}
=KX^iY^j(-b+X+Y)=0.
\]

The last equality says that the allowed carry changes the representation,
while preserving the represented object. The implementation performs exactly
these three updates in `library/src/Mdn2dRules.cpp`, lines 224–226.

## The next definition: keep the whole unending recipe

Imagine running a calculation with 1 cascade term, then 2, then 3, and so
on, always keeping the complete grid. Call that unending recipe an object.

Two recipes represent the same object if, after some finite starting stage,
their complete results are carry-equivalent at **every** subsequent stage.
They need not use the same digit display. Agreement only inside a viewing
window does not qualify.

Addition and multiplication mean doing the operation at corresponding
stages. This makes them well-defined: once two inputs agree exactly, their
sums and products agree exactly too. The constant recipe `1,1,1,...` remains
different from `0,0,0,...`.

This is an algebraic meaning of an infinite object. It records an unending
process, rather than claiming that the visible arrays converge to a single
ordinary real number. Mathematicians call the equivalence class an eventual
germ; the name is less important than the explicit equality rule above.

## A short expression can retain a moving endpoint

Let `n` denote the number of terms. Thus the app's depth 20 has `n=21`.
Use `E` to remember the signed boundary shift after those `n` terms. Its
reverse is written `E⁻¹`, and

\[
EE^{-1}=1.
\]

This is the same rule as shifting a pattern out along a diagonal and then
shifting it back. It does **not** say the two endpoints cancel when added.

More precisely, `q=-Y/X` shifts a digit by `(-1,+1)` and reverses its sign;
at stage `n`, `E=q^n`. `E` records this moving position throughout the
recipe. It is not an ordinary limit of `q^n`.

Write `u` for the X directional tenth and `v` for the Y directional tenth,
using matching term counts. The complete boundary identities are

\[
\boxed{10u=1-E,\qquad 10v=1-E^{-1}.}
\]

These are expressions of the numbers including their boundary content.
They are valid at every finite stage and hence for the unending recipes.
In particular, `u` and `v` remain different.

The earlier fuzzy family, with matched cutoffs, is `F=uv`. Its expression is

\[
\boxed{100F=2-E-E^{-1}.}
\]

The second unity contribution comes from multiplying the two endpoints:
`(-E)(-E⁻¹)=1`. This is why boundary information matters to multiplication,
even when both endpoint cells are far outside the current view.

The contradiction would return if we identified `E` with zero in the full
mixed arithmetic: multiplying that identification by its existing inverse
would give `1=0`. Keeping `E` as part of the number prevents that step.

## Additive mixing, including an important same-axis example

For decimal tenths at the same cutoff, `0.4X=4u` and `0.6Y=6v`. Therefore

\[
S=4u+6v,\qquad
\boxed{10S=10-4E-6E^{-1}.}
\]

It follows that `S` is not 1. Also

\[
5S=5-2E-3E^{-1}.
\]

At depth 20, `E=q^21` is minus one at `(-21,21)`. Thus this last expression
is exactly 5 at unity, plus 2 at `(-21,21)`, plus 3 at `(21,-21)`—the
saved example. There is no approximation in that statement.

Addition combines the X and Y contributions in one object. In this example
their separate endpoint terms remain visible in the expression; multiplication
introduces the cross-products between those terms.

Keeping endpoints also affects addition within a single direction:

\[
\boxed{0.9X+0.1X=10u=1-E.}
\]

At depth 20 the corrected backend gives exactly unity plus a digit 1 at
`(-21,21)`. A freshly entered integer `1` contains unity alone. They are
different carry classes, not merely different displays of the same class.

Consequently, the label `0.1X` needs careful interpretation: it denotes a
directional entry with boundary content. It is not already an exact inverse
of the shared integer 10. If both directional entries were exact inverses
of the same 10, ordinary associative multiplication would force them equal:

\[
u=u(10v)=(10u)v=v.
\]

Thus retaining distinct directional tenths requires revising the claim that
each direction is already an exact copy of real arithmetic with the same
integers. One-sided interpretations that discard one endpoint can still be
studied separately; they do not extend to the full mixed arithmetic containing
the reverse endpoint.

## More decimal places need more boundary information

The corrected parser uses different cascade coefficients at different decimal
places. For example, writing `w` for a freshly entered `0.01X` gives

\[
\boxed{100w=1-E\bigl[1+n(1-q)\bigr].}
\]

By comparison,

\[
100u^2=1-2E+E^2.
\]

These expressions differ. The distinction comes from their different
truncation recipes. At finite depth, squaring the first-place entry is not
the same operation as entering the second-place decimal afresh.

This reveals what the boundary record needs to contain:

1. `E`: how far the boundary moves as the cascade grows;
2. `q`: a fixed diagonal offset, including its sign;
3. `n`: coefficients that depend on the number of terms.

A compact expression is a finite sum of terms such as
`(2n+3) q⁻² E`, with exact rational coefficients allowed as bookkeeping.
In a product, multiply the coefficient polynomials and add the shift
exponents. Equal terms combine. **No terms are discarded because they move
away.**

For example, one extra term on the X side replaces `E` by `qE`. Twice as
many terms replaces it by `E²`. Different cutoff paths therefore remain
distinguishable in the same notation.

The [proofs](BOUNDARY_PROOFS.md) establish a unique expression of this kind
for this class of recipes. They cover every finite decimal input and every
finite sequence of additions, subtractions, and multiplications with affine
cutoff growth. Arbitrary infinite decimal streams and intrinsic MDN sine,
cosine, or division need additional definitions.

## Vectors, the complex plane, and the proposed blips

The two directions can serve as channels in a vector encoding. The endpoint
case is important: the vector component `(1,0)` must remain distinguishable
from `(0,1)`. Replacing either completed directional component by the same
undirected integer 1 throws away that distinction.

This suggests a useful experiment: compare two ways of making a circle.
One repeatedly enters ordinary sine/cosine decimal values, letting the parser
split their integer and fractional parts. The other retains a consistent
directional encoding even when a component reaches 1. They need not give the
same MDN at the endpoint.

A carry itself cannot change the complete represented MDN; it changes the
display. But changing the input recipe can change the boundary content. A
visible blip near 1 could therefore be a carry, rounding, or a change of
encoding. The exact boundary expression distinguishes these explanations.
There is no established physical effect from the blip alone.

Similarly, capital `X` here is a positional shift, not the imaginary unit.
Setting that shift to the complex number `i` would require `Y=10-i` by the
carry relation. It would also identify some previously different MDNs—for
example, `X²+1` would become zero. That is a possible further interpretation,
not a property already proved for the original MDN arithmetic. A complex
number encoding needs its own explicit multiplication rule.

## Implementation and scope

- [boundary.py](boundary.py) implements these exact expressions, including
  arbitrary finite fractional places and affine cascade lengths.
- [test_boundary.py](test_boundary.py) checks the expressions against direct
  finite binomial cascades and checks mixed arithmetic. All 13 test groups
  pass, including 870 direct coefficient comparisons across fractional places
  1–5, bases 2/3/10, both directions, and growing or fixed cascade lengths.
- [BOUNDARY_PROOFS.md](BOUNDARY_PROOFS.md) gives the precise construction,
  equality proof, and its limitations.

The coefficient ring used by the implementation allows rational bookkeeping;
the original backend-generated objects form the specified integer-generated
subalgebra inside it. This does not redefine a backend directional `0.1X` as
the rational scalar `1/10`.

From the repository root:

```bash
python3 -m unittest discover -s research/fuzzy -p 'test_boundary.py' -v
```

For a small example, from `research/fuzzy`:

```python
from boundary import directional_fraction

u = directional_fraction(1)                 # 0.1X, n terms
v = directional_fraction(1, direction="Y")  # 0.1Y, n terms
w = directional_fraction(2)                 # freshly entered 0.01X
f = u * v

assert 10 * f == u + v
assert 10 * u != 1
assert w != u**2

# All q coefficients of 5*(0.4X+0.6Y) at GUI depth 20:
print((5 * (4*u + 6*v)).evaluate(21))
# {0: 5, 21: -2, -21: -3}, as exact Fractions.
# Since q includes a sign reversal, these are grid digits 5, +2, +3.
```

This construction answers the boundary-retention question affirmatively for
the stated class. It preserves `1 != 0`, distinguishes the directions, and
gives exact addition and multiplication. It chooses a meaning for an infinite
object based on its complete recipe; it does not prove a unique compulsory
notion of convergence or a physical application.
