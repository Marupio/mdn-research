# Review of the 11 October summary and archive

The summary Claude prepared is [STATE_OF_THE_ART.md](../../STATE_OF_THE_ART.md).
The move list and reasons are in [pitfalls/README.md](../../pitfalls/README.md).
The material was moved from `VALUE.md`, `chatWithAstra.md`,
`research/positivity/`, `research/infinite/`, and
`research/decimal_infinite/ANALYTIC_PROOFS.md`. This review leaves that
organization and Claude's summary unchanged.

Several observations in the summary are useful: the explicit carry rule,
axis readings with specified domains, diagonal charges, a fixed digit
convention for resolving redundant displays, and the quadrant support rule.
The new phase-product work builds on the latter two.

## Quadrants: a useful rule for generating grids

Starting from the specified decimal inputs and using forward carries:

- origin-generated integers stay in the northeast region;
- X fractional cascades start northwest, including the negative X-axis;
- Y fractional cascades start southeast, including the negative Y-axis;
- addition and subtraction of these inputs do not create southwest cells;
- multiplying X and Y fractional cascades can create southwest cells.

A negative carry amount still affects the same two forward neighbor
positions, so the support statement also covers subtraction. For the raw
matched product F at term count n, its only strictly southwest cell is
`(-1,-1)`, holding n. The native backend checks reproduce this behavior.

Three qualifications prevent the geometric picture from becoming a false
classification of numbers:

1. **Quadrant I contains more than integers.** The single digit X lies there
   and is not an integer under the carry relation. The one-cell grid XY is
   even symmetric under exchanging axes, but is not an integer either.
   Symmetry is a property of the origin-generated integer displays, not a
   converse test for being an integer.
2. **Reverse carries can introduce quadrant III.** One allowed carry says
   `10 at (-1,-1)` equals `1 at (-1,0) plus 1 at (0,-1)`. These two equal
   representations have different quadrant-III occupancy. In shift notation,
   `10 X^-1 Y^-1 = X^-1 + Y^-1`.
3. **A mixed number can have empty quadrant III after carrying.** For example,
   at depth 9, the raw mixed central coefficient is 10 and carries out of
   that quadrant. The complete mixed product is still present.

Thus “the two fraction types can overlap in quadrant I under forward
carries” is valid. Identifying that entire overlap with a shared integer
component requires additional justification. An invariant classification
needs the full carry class or a specified canonical display.

## A phase is not the entire boundary state

Section 3.5 of the summary correctly distinguishes the ten display phases
from the complete finite numbers. Section 4 then overstates the result by
saying the display depends only on `(N+1) mod 10` and that this last digit
is the only remaining uncertainty. Only the eventual display in each fixed
window has that ten-phase description.

The new products show why this matters: matching F phases can give
different central product digits. The complete boundary expressions are
still different. The [phase-product proof](PROOFS.md) establishes finer
eventual display periods of 100 for FL and FR, and 500 for F², under one
fixed carry convention.

An ultrafilter construction can retain a definite infinite-depth object;
it does not identify every depth with the same residue modulo 10. For
example, the full grids F_N and F_(N+10) are unequal at every finite N, so
passing to a free ultrafilter does not make these two sequences equal.
Their equal ten-phase displays are not enough to determine products.

## Archive labels and mathematical validity

Choosing one ordinary scalar value for each axis is not a definition of
the user's MDN. However, retaining an entire formal expression or function
is a different operation. A faithful representation preserves all the
finite carry distinctions instead of replacing an MDN by one magnitude.

The archived [analytic proof](../../pitfalls/decimal_infinite/ANALYTIC_PROOFS.md)
explicitly keeps complete functions (opening paragraphs) and proves a
faithful embedding of finite carry classes (Section 3). Its notion of
infinite convergence is an optional mathematical construction; rejecting
that choice as the preferred MDN definition does not invalidate the finite
identities or the additive-separation theorem proved there.

Accordingly, the folder is a record of approaches set aside, not a test of
which individual theorems are false. The new experiments here use only
integer grids, recorded carries, and formal boundary expressions. They
assign no scalar magnitude to either axis.

The “exactly three viewpoints” wording in Section 3.1 also needs its stated
specialist scope: the three positional directions arise when ordinary
nonzero coefficient constants are assigned order zero and positions have
additive orders compatible with the carry relation. It is not a proof
that every conceivable definition of MDN size has only three possibilities.
