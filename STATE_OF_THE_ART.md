# MDN research: historical snapshot from 11 October 2026

**Historical working notes.** Read the [current manuscript](research/manuscript/README.md)
and [corrections to this summary](research/phase_products/REVIEW.md) first.
The quadrant, phase and positional-order claims below require the qualifications
in that review. The old statements that backend corrections are uncommitted
describe the date of this snapshot; current source includes those corrections.
The original calculation counts below are historical reports, with the stated
limits on script availability. Conversation records remain local.

*Snapshot of 11 October 2026, prepared by Claude (Anthropic) from this
repository, the saved GUI projects in `~/GADEN/MDN/`, and the conversation
records. Results marked **(new)** were derived and checked while preparing
this summary. Everything else summarises work already in the repository,
with links.*

**Status labels.** **Proved**: a proof exists, either here or at the link.
**Checked**: exact computer checks only. **Proposed**: a definition or
interpretation offered for adoption. **Open**: not yet settled.

---

## 1. Ground rules

### What an MDN is

- A number is a grid of integer digits on cells (x, y), where x and y are any
  integers.
- **Carry.** Remove b·k from a cell, and add k to its right neighbour
  (x+1, y) and to its upper neighbour (x, y+1). Carries may also run in
  reverse. Two grids are **equal** when carries connect them
  ("carry-equivalent").
- **Integers** are built by adding unity at (0,0) and carrying. They come out
  symmetric under swapping x and y.
- **Fractions** must choose a direction, the *fraxis*.
  - 0.1X cascades up-left along the anti-diagonal x + y = −1, with raw digits
    1, −1, 1, −1, …
  - 0.1Y is the mirror image, cascading down-right.
  - Deeper decimal places use deeper anti-diagonals with binomial
    coefficients; 0.01X has raw digits 1, −2, 3, −4, …
- Cascades are cut off at a finite **depth** N, meaning N + 1 terms. The saved
  experiments use depth 20.

### The rule that matters most

**Never give grid positions ordinary (1DN) values.** Substituting numbers for
X and Y imports a notion of size that MDN does not have. Examples are
X = b·t, a viewing angle, X = Y = 5, or an interval or complex domain of X
values.

Every early analysis made this mistake: `VALUE.md`, the first conversation
with Astra, and Claude's first review. That work now lives in
[pitfalls/](pitfalls/README.md). The correction is recorded in
the local corrected conversation.

Judge an MDN only by its digits, positions and carries. Ordinary numbers
enter only through explicitly defined *readings* (Section 2.2).

### Notation

| Symbol | Meaning |
|---|---|
| X, Y | "One cell right" and "one cell up". Names of positions, not values. |
| X + Y = b | The carry rule: b units in a cell equal one unit in each forward neighbour. |
| L_N, R_N | The raw cascades of 0.1X and 0.1Y at depth N. |
| q | −Y/X: one step up-left along an anti-diagonal, with a sign flip. |
| E | q^(N+1). It marks where the X-cascade stops: one cell directly above its last cell, on the anti-diagonal x + y = 0. E⁻¹ is the matching cell directly right of the Y-cascade's last cell. E·E⁻¹ = 1. |
| F | L·R, the "fuzzy" product 0.1X × 0.1Y. |

Formally, finite grids modulo carries form the ring
A_b = ℤ[X^±1, Y^±1]/(X + Y − b). This is shorthand for "integer grids with
carries allowed"; it assigns no values to X or Y.

---

## 2. Finite MDNs

### 2.1 Finite grids behave well — Proved

Finite grids under carry-equality form an ordinary commutative arithmetic.
It has no zero divisors, and 1 ≠ 0
([research/fuzzy/PROOFS.md](research/fuzzy/PROOFS.md) §1). Equality of two
finite grids can always be decided exactly.

### 2.2 Exact readings that need no magnitudes — Proved and checked (new)

**X-axis reading.** For a grid with nothing below the X-axis, read the
X-axis row as an ordinary base-b numeral.
- **Carries leave it unchanged.** A carry from the axis row moves one unit
  right, which is worth b times as much, and one unit up, which the reading
  ignores.
- **Arithmetic passes through.** The reading of a sum or product is the sum
  or product of the readings.
- **Examples.** 0.4X reads 0.4. 0.9X + 0.1X reads exactly 1, even though the
  full grid is 1 − E, because E lies above the axis.
- **Meaning.** This is the precise sense in which "X alone behaves like the
  reals": the off-axis digits are neither given values nor discarded; the
  reading simply does not include them.
- **Limits.** The Y-axis column gives the mirror reading. A grid with cells on
  both sides of the origin, as any X/Y mixture has, supports neither reading.

**Diagonal charges.** On each anti-diagonal x + y = n, add the digits with
sign + where x is even and − where x is odd.
- A carry never changes the charge of the diagonal it pushes into. It changes
  the charge of the diagonal it comes from by a multiple of b.
- **Every diagonal's charge is unchanged by carries modulo b.** Modulo the
  base, the whole grid collapses onto these charges, so they record exactly
  "the number modulo b".
- **Main charge:** the charge of the diagonal x + y = 0. Two grids with
  nothing above that diagonal can be equal only if their main charges are
  exactly equal.
- Fraction cascades lie below the main diagonal, so their main charge is 0. A
  whole number at the origin has its own value as its main charge.

Applied to the saved projects:

| Tab | X-axis reading | Main charge |
|---|---|---|
| `0.4X`, `0.2X`, `X20` | 0.4, 0.2, 0.1 | 0 |
| `5` | 5 | 5 |
| `S×5` (5, with blips 2 and 3) | none | 5 − 2 − 3 = 0 |
| `100F20-20` (2, with blips 1 and 1) | none | 2 − 1 − 1 = 0 |
| `100F20-21`, `100F20-40` | none | 0 |
| `(S×0.2X)×5` | none | 1 − 1 = 0 |

**The "edge-of-cascade errors" are forced.** S × 5 cannot equal 5 exactly,
because its main charge must be 5 × 0 = 0. The blips carry exactly −5. In
general, no expression built from finite fraction cascades can equal a
nonzero integer.

### 2.3 What the carried digits are worth — Proved (new)

- **No finite grid times b equals 1**, so every fraction needs an infinite
  cascade. *Proof:* the main charge of b·G is a multiple of b modulo b, while
  that of 1 is 1.
- **The lone digit X satisfies no polynomial equation with integer
  coefficients, and the integers are the only finite grids that do.** So the
  digits of "10 = X + Y" have no individual values: the whole is ten by
  construction, and any split (5 + 5? 3 + 7?) is an outside choice.
  *Proof:*
  - Finite grids embed in the rational functions of X
    ([research/fuzzy/PROOFS.md](research/fuzzy/PROOFS.md) §1). The only
    elements there that satisfy rational polynomial equations are constants.
  - If a grid equals a constant a/c in lowest terms, clearing denominators
    gives c·P(X) = a·X^m·(b − X)^n. The leading coefficient there is ±a,
    which c must divide, so c = 1.

### 2.4 Canonical forms end polymorphism — Proved and checked (new)

**Lowest-diagonal lemma.** If a finite grid is carry-equivalent to zero, every
digit on its lowest non-empty anti-diagonal is a multiple of b. To see this,
write the grid as (X + Y − b)·Q and compare lowest anti-diagonals: only −b·Q
reaches there.

**Theorem.** Let b ≥ 3, and let D be any b consecutive integers that include
−1, 0 and 1 (in base 10, −4…5 or −5…4). Then every finite MDN has exactly one
finite grid with all digits in D.

One sweep finds it. Work up the anti-diagonals from the lowest. At each cell,
write the digit as r + b·k with r in D, keep r, and carry k into both forward
neighbours.

- **Uniqueness.** Two such grids differ by digits smaller than b, so the lemma
  forces them to be equal.
- **The sweep always finishes.** Above the original cells, each cell receives
  two carries. The largest carry drops by at least 1 per anti-diagonal until
  it is 1. After that, the set of carrying cells shrinks at every step.
- **The sweep is forced.** The lemma fixes the carries one diagonal at a time,
  so a finite D-grid exists exactly when the sweep finishes.
- **The digit condition is necessary.** With digits 0…9, the integer −1 never
  finishes. It becomes an endless triangle, 9s on the edges and 8s inside,
  marching up-right: the 2D version of the 10-adic …999. Base 2 cannot meet
  the condition and has no canonical forms.

*Checked:* 3,000 random grids in bases 3, 4, 5, 10 and 16, with both balanced
digit conventions.
- All finished within 24 diagonal sweeps.
- All kept their exact value.
- Randomly re-carried copies gave identical results.
- The user guide's two "polymorphic" forms, (−2, 6, −3) and (8, 5, −4), give
  the same canonical grid.

So polymorphism is the 2D version of signed-digit redundancy. The GUI's sign
conventions can remain display preferences, with canonical forms underneath
for exact equality tests and a definite display.

### 2.5 Quadrants explain how X and Y fractions mix — Proved (new)

Forward carrying moves value only up or right.

```
    II: X-fractions    |   I: whole numbers (shared)
   --------------------+---------------------------
    III: mixed only    |   IV: Y-fractions
```

The negative x-axis belongs to II, and the negative y-axis to IV.

- X-fraction cascades lie in II and Y-fraction cascades in IV. Carries from
  both flow toward I, so **X and Y fractions meet only through the
  whole-number part.**
- Addition with forward carrying never puts anything in III. Multiplying an
  X-fraction by a Y-fraction can.
- The raw F = 0.1X × 0.1Y has exactly one cell in III: (−1, −1), holding
  N + 1 (21 at depth 20). This is the "marching digit".
- **Additive separation for finite inputs.** Take a constant, plus finitely
  many X decimal places, plus finitely many Y decimal places, with proper
  digits (|d| < b). Their sum is zero only if every part is zero. This holds
  in any base and at any cascade depths.
  - *Proof:* apply the lemma to the deepest place. Its first X cell, (−K, 0),
    and its first Y cell, (0, −K), each hold a single digit. Each must be a
    multiple of b, hence zero. Repeat for the next place up.
  - *Checked* on all 50,616 small cases in bases 2, 3 and 10.
  - For infinite digit streams, the only proof so far is analytic
    ([pitfalls/decimal_infinite/ANALYTIC_PROOFS.md](pitfalls/decimal_infinite/ANALYTIC_PROOFS.md)
    §5, which requires b > 2).

---

## 3. Fractions, boundaries and infinity

### 3.1 Size is positional: three viewpoints — Proposed (components proved; new)

MDN has no single magnitude, but it does have intrinsic notions of "small"
defined by position. The precedent is the p-adic numbers, where "small" means
far along the digit string, with no reference to real size. The carry rule
allows exactly three such notions:

| Viewpoint | What counts as small | Its reading | The boundaries E, E⁻¹ |
|---|---|---|---|
| X | Rows far above the X-axis | The X-axis row | E negligible; E⁻¹ infinitely large |
| Y | Columns far right of the Y-axis | The Y-axis column | E⁻¹ negligible; E infinitely large |
| Symmetric | Cells far down-left | Main charge (whole numbers only) | Both read as exactly 1 |

- **The X viewpoint.** Here 10 = X + Y reads as "ten on the axis plus a
  negligible upward leak". The cascade 0.1X is exactly the sequence of
  corrections that cancels those leaks.
- **Why there are exactly two fraxis.** A tenth needs a viewpoint whose reading
  can equal 1/b. The X and Y readings can; the symmetric reading is always
  whole. Section 3.2 gives the matching count from the cascades themselves.
- **Why mixed products are fuzzy.** Each cascade's far end is negligible from
  its own viewpoint and infinitely large from the other's. No viewpoint can
  discard both ends, because E·E⁻¹ = 1.
- **Where the current formal work sits.**
  [research/decimal_infinite/FORMAL_PROOFS.md](research/decimal_infinite/FORMAL_PROOFS.md)
  works in the symmetric viewpoint, using series in z = 1/X. That is why it
  can keep both boundaries; the price is rational coefficients used for
  bookkeeping.
- **For specialists.** These are the three rays of the tropicalisation of
  X + Y = b: completions as Laurent series in Y, in X, and in 1/X. The carry
  direction itself, up-right, gives a fourth, arithmetic notion of smallness:
  divisibility by b. That is where −1 becomes the triangle of 9s and 8s
  (Section 2.4).

### 3.2 Directional expansions and the fuzzy mode — Proved

- **Exactly two one-sided solutions.** On an anti-diagonal, multiplying by
  X + Y adds each pair of neighbouring cells. Solving (X + Y)·G = 1 there has
  exactly two one-sided solutions: the X-cascade and the Y-cascade.
- **Their difference is the fuzzy mode.** It is the alternating pattern
  +C, −C, +C, …, which neighbour-adding wipes out. CFD readers will know it
  as the checkerboard mode of linear face interpolation.
- **Higher orders.** For (X + Y)^m·G = 1 on one anti-diagonal, the free
  patterns are (−1)^k times a polynomial in k of degree below m: alternating
  constant, linear, quadratic, and so on.
  This is proved in
  [pitfalls/infinite/formal_notes.md](pitfalls/infinite/formal_notes.md) §4;
  that section is magnitude-free despite its folder.
- **Both cascades can never converge together.** No rule for limits makes both
  opposite cascades converge in one arithmetic with continuous
  multiplication, because their tails q^N and q^−N multiply to 1
  (formal_notes.md §2).
- **Two exact tenths would coincide.** Two tenths that are both exact inverses
  of 10 must be equal: u = u·(10v) = (10u)·v = v. Distinct directional tenths
  must therefore carry boundary information
  (the local corrected conversation).

### 3.3 Boundary retention — Proved

Source: [research/fuzzy/BOUNDARY.md](research/fuzzy/BOUNDARY.md) and
[BOUNDARY_PROOFS.md](research/fuzzy/BOUNDARY_PROOFS.md). At matched depth,
exactly:

- 10 · 0.1X = 1 − E, and 10 · 0.1Y = 1 − E⁻¹.
- 100 · F = 2 − E − E⁻¹. The second unit is E·E⁻¹, where the two boundaries
  meet. With unequal depths it moves off the origin, as the `100F20-21` and
  `100F20-40` tabs show.
- 0.9X + 0.1X = 1 − E, not 1. Likewise 5·(0.4X + 0.6Y) = 5 − 2E − 3E⁻¹, which
  is the `S×5` tab.
- (S × 0.2X) × 5 = 1 + 0.6Y + a distant tail. The boundary digit 3 at
  (21, −21), multiplied by the far end of 0.2X, lands at (0, −1) and rebuilds
  0.6Y (the local corrected conversation).
- A freshly entered 0.01X is not (0.1X)². Deeper places need coefficients
  that depend on depth.
- **Infinite-depth objects ("germs").** A germ is a sequence of grids indexed
  by depth. Two sequences are equal when they eventually agree as full carry
  classes.
  - Expressions in n, q, E and E⁻¹ are a faithful notation for a large class
    of germs (the injectivity theorem).
  - Each direction alone has an ordinary interpretation: send E to 0, or E⁻¹
    to 0. No single interpretation can delete both.

### 3.4 The finite fuzzy family — Proved

Source: [research/fuzzy/README.md](research/fuzzy/README.md) and
[PROOFS.md](research/fuzzy/PROOFS.md).

- **Raw shape.** Every product of a X-tenths and c Y-tenths lies, before
  carrying, on one anti-diagonal. Its coefficients alternate in sign and
  follow a piecewise-polynomial envelope of degree at most a + c − 1. One
  formula covers every cutoff. At depth 20 the raw central coefficients are
  21 for F, 231 for F·0.1X and F·0.1Y, and 6,181 for F².
- **A two-component arithmetic at matched depth.** Every expression in the two
  tenths has a unique form A(F) + B(F)·L, with A and B integer polynomials:
  - Relations: b·F = L + R and L² = b·F·L − F.
  - Product rule:
    (A + BL)(C + DL) = (AC − BDF) + (AD + BC + bFBD)·L.

  This is the concrete form of the imaginary-number hunch: two components,
  with coefficients that are polynomials in F rather than plain numbers.
- **Unequal depths.** The structure depends on gcd(N+1, M+1). Over the
  rationals, the number of components is (N + M + 2)/gcd(N+1, M+1): 2, 43
  and 62 for depths 20/20, 20/21 and 20/40.
- **Growing depths.** Depths growing at different rates leave different
  stationary terms (a subset-sum rule). When a X-factors and c Y-factors grow
  together, the stationary term is C(a + c, a): 2, 3 and 6 for F, F·L and F².
- **Checked:** 2,318 exact backend checks. In 15 cases, different
  multiplication orders gave different displays but equal numbers.

### 3.5 Marching digits — Proved and backend-checked

Source:
[research/decimal_infinite/DISPLAY_PATTERNS.md](research/decimal_infinite/DISPLAY_PATTERNS.md).

- The displayed digit of F at (−1, −1) is (N + 1) mod 10. An explicit
  two-layer construction gives every displayed digit, and it matches the
  corrected backend for depths 0 to 120.
- Along depths N₀, N₀ + 10, N₀ + 20, … every fixed cell settles. This gives
  ten infinite display phases, and the central digit selects the phase:
  "one digit snaps everything into place".
- A phase does not fix the boundary. Depths 20 and 30 agree over a large
  window but are different numbers.
- Multiplying two phase patterns cell by cell is undefined, because it would
  need an infinite sum at (−2, −2). The boundary information is required.

### 3.6 Infinite decimal inputs — Proved

Source:
[research/decimal_infinite/README.md](research/decimal_infinite/README.md)
and [FORMAL_PROOFS.md](research/decimal_infinite/FORMAL_PROOFS.md).

- **Infinite digit streams converge.** Write grids as series in z = 1/X, with
  coefficients that are polynomials in the term count n. Then infinite digit
  streams, including 0.333… and irrationals, converge: coefficient m depends
  only on the first m digits.
- **Arithmetic works.** Addition and multiplication, including mixed X/Y
  products, are defined. The ring has no zero divisors, and swapping X and Y
  is preserved.
- **Nothing earlier is lost.** The boundary algebra of Section 3.3 embeds
  without loss, and E stays invertible.
- **Reals are encoded, not reproduced.** Every real number has a distinct
  encoding in each direction. These encode the *set* of reals; they are not
  copies of real arithmetic. For example, 0.999…X ≠ 1, and
  3 × 0.333…X = 1 − (boundary).

---

## 4. Interpretation — Proposed

**The fuzzy number is definite, not divergent.**
- At every depth it is an exact finite grid with bounded digits. Its display
  depends on the depth only through (N + 1) mod 10. Earlier verdicts of
  "divergence" came from giving far-away cells numeric weight.
- A natural way to take the depth to infinity is nonstandard analysis. Choose
  an infinite depth H, which is a definite (hyperfinite) number. Then F_H is
  definite, and its phase is (H + 1) mod 10.
- The only "fuzz" left is that an infinite depth's last digit is not known in
  advance. Seeing one digit reveals it, and the rest snaps into place.
- The germ construction (Section 3.3) is the first half of this. Choosing an
  ultrafilter completes it, and also removes the even/odd zero divisors the
  germ ring currently has.

**Rebuilding notation from MDN is coherent if it keeps the integers.** Digits,
the base and carrying are all integer arithmetic. The programme is to keep the
integers, which are the symmetric numbers, and rebuild fractions, size and
limits from positions. The precedent is p-adic analysis, which rebuilt
calculus on positional size and reshaped number theory.

**Where new mathematics is most likely:** quadrant III (products across the
two fraxis), the viewpoint structure, and the digit patterns of canonical
forms. The algebra of finite grids on its own is classical; the digit and
carry structure built on it is where the novelty lies.

---

## 5. Open problems, in suggested order

1. **Canonical forms in the library and GUI.** They give an exact equality
   test and a definite display (Section 2.4).
2. **Official definitions of size and limits.** Build them from the three
   viewpoints and the charges, replacing the magnitude-based limit rules now in
   `pitfalls/`.
3. **Quadrant III.** Classify what can live there and how it multiplies. Any
   "new type", as in the imaginary-number analogy, would live there.
4. **Infinite depth.** Build the ultrafilter (nonstandard) version, and
   restate the fuzzy family with phase = depth mod b.
5. **Additive separation for infinite streams,** with a magnitude-free proof.
   The finite case is in Section 2.5.
6. **3DN and beyond.** With X + Y + Z = b, expect three fraxis (one per axis)
   and an octant-style region structure. Test this before building GUI
   support.
7. **Digit patterns of canonical forms.** Are rationals exactly the
   eventually periodic cascades? What do integers look like modulo small
   primes?
8. **Finer grids (speculative).** Allowing half-step positions turns the carry
   rule into u² + v² = b, with carries jumping two half-steps. Steps of 1/n
   give uⁿ + vⁿ = b, the Fermat curves. This is a formal extension with deep
   connections in number theory.

---

## 6. Pitfalls

| # | Pitfall | Example | Do instead |
|---|---|---|---|
| 1 | Giving X and Y numeric values | X = b·t; X = Y = 5; convergence for X in an interval or complex domain | Use positions, carries and the defined readings |
| 2 | Judging equality by the display or a window | Polymorphic forms; matching phases with different boundaries; a grid that leaves every window yet still equals 1 | Compare carry classes or canonical forms |
| 3 | Deleting boundary terms as "errors" | Treating S × 5 as exactly 5 | Keep them: the main charge forces them, and deleting both E and E⁻¹ gives 1 = 0 |
| 4 | Treating 0.1X and 0.1Y as exact inverses of 10 | Then u = v | Keep the boundary, or stay within one fraxis |
| 5 | Generalising from base 2 | No canonical forms; analytic separation fails | Use b ≥ 3 for general claims |
| 6 | Confusing X with 0.1X | X is one cell; 0.1X is a whole cascade | Write cascades explicitly (L_N, R_N) |
| 7 | Using rational coefficients as if they were digits | 1/b is not a finite grid | Declare such bookkeeping explicitly |
| 8 | Reproducing with the wrong software | The installed GUI loads the old backend; the fixes are uncommitted; `getTotalValue` is not carry-invariant | Run `LD_LIBRARY_PATH=/home/d.gaden/GADEN/Code/mdn-research/build/fraxis /usr/bin/mdn_gui`; commit the fixes |

[pitfalls/README.md](pitfalls/README.md) explains, for each superseded
document, what is wrong and what still holds.

---

## 7. Map of the repository

| Path | Contents |
|---|---|
| `STATE_OF_THE_ART.md` | This file |
| Local conversation archive (not published) | The discussion where the 1DN framing was abandoned |
| `research/fuzzy/` | Finite fuzzy families and boundary retention: README, PROOFS, BOUNDARY, BOUNDARY_PROOFS, Python models, tests and backend checks |
| `research/decimal_infinite/` | Infinite decimal inputs and marching digits: README, FORMAL_PROOFS, DISPLAY_PATTERNS, series model, tests and backend checks |
| `pitfalls/` | Superseded studies and reviews |
| `library/`, `gui/` | The C++ library and Qt application |
| `build/fraxis/libmdn.so` | The corrected backend build |
| `tests/` | Backend fraxis tests |
| `~/GADEN/MDN/` (outside the repository) | GUI projects: `forAstra.mdnproj` (S, S×5, (S×0.2X)×5); `fuzzy-cutoff-experiment.mdnproj` (the 100F tabs); `fuzzy-family-examples.mdnproj` (22 generated examples) |

The fixes to carry conservation and to the cascades of deeper decimal places
are in the working tree but not yet committed. The other library defects
listed in `pitfalls/VALUE.md` (division termination, output aliasing,
locking, a stale base cache) are not addressed by those changes.

---

## 8. How the new results were checked

The results marked **(new)** were checked with exact integer computations in
Python. The scripts are not stored in this repository.

| Result | Check |
|---|---|
| Canonical forms | 3,000 random grids in bases 3, 4, 5, 10 and 16. Values compared by exact polynomial identity testing at random points modulo 2⁶¹ − 1 |
| X-axis reading | 2,000 random grids with random carries and products |
| Diagonal charges | 2,000 random carry sequences in bases 2, 3, 10 and 16, then applied to every saved tab in Section 2.2 |
| Additive separation | All 50,616 small combinations in bases 2, 3 and 10 |
| Quadrant-III content of F | Depths 5 and 20 |

The proofs above, not these samples, establish the general statements.
