# Infinite MDN expansions: consistent limits and fuzzy patterns

Investigation completed 10 October 2026. This follows the positivity experiment
and the earlier finite-algebra analysis in `VALUE.md` and `chatWithAstra.md`.
The C++ backend is not used as a mathematical authority or changed here.

**Answer:** the `1 = 0` collapse is caused by incompatible choices of what
“getting small” means. Consistent infinite extensions exist. However, both
opposite fraxis expansions cannot converge as ordinary partial sums in one
nonzero arithmetic with unique limits and continuous multiplication. The fuzzy
patterns can instead be classified as formal coefficient arrays with boundary
freedom and restricted multiplication.

These are precise deductions about MDN from established algebra and analysis.
No claim of a new theorem in the mathematical literature, a physical model,
or a Riemann-hypothesis advance is made.

## A simple explanation of the collapse

MDN's finite carry rule is `X + Y = b`. Think of `X` and `Y` as the value
multipliers for moving a digit one place along either axis.

In base 10, all these formulas represent exactly 1:

\[
1,
\quad \frac{X+Y}{10},
\quad \frac{X^2+2XY+Y^2}{100},
\quad \frac{(X+Y)^n}{10^n}.
\]

If we count only the absolute sizes of the raw coefficients, their totals
are `1, 0.2, 0.04, ...`, approaching zero. But their positions are becoming
more valuable. At `X=Y=5`, the quadratic expression has contributions
`0.25 + 0.50 + 0.25 = 1`.

Concluding that the represented number has gone to zero loses that position
information. Mathematically, a continuous quotient using this raw coefficient
norm must identify 1 with 0. Once that happens every number becomes zero.
The finite carry rule itself has not contradicted arithmetic; the chosen
infinite extension has failed.

This particular example uses rational coefficients, as in the normalized
positivity experiment. There is a stronger example using signed integer
coefficients alone:

\[
E_n=X^{-n}(b-Y)^n.
\]

Every `E_n` is 1 under the carry rule, because `b-Y=X`. Its entire support
lies in grid column `-n`. Every fixed viewing window eventually becomes empty.
Thus unrestricted **cell-by-cell convergence** also fails as a value limit,
at every integer base. The integer coefficients here can be large; this is
not a claim about a proved bounded-digit canonical form.

The useful analogy is an object moving out of a camera's view: an empty
picture does not establish that the object has ceased to exist.

[collapse_notes.md](collapse_notes.md) proves both statements, including the
stronger result that every finite grid collapses under an unrestricted
continuous, Hausdorff cellwise quotient. It also shows how the bad completion
turns `X+Y-b` into an invertible element, making the contradiction explicit.

## A consistent definition of limits

Choose a nondegenerate closed interval `I=[a,c]` of possible `X` values, with
`0<a<c<b`, and substitute `Y=b-X`. A grid is then an exact function of `X`.

**Call an approximation accurate when its largest function error anywhere on
that interval is small.** This is uniform convergence:

\[
\|P-Q\|_I=\max_{x\in I}|P(x,b-x)-Q(x,b-x)|.
\]

It respects all finite carries. Every supposed disappearing copy of 1 remains
at distance exactly 1 from zero. Addition and multiplication respect these
limits, so ordinary arithmetic remains coherent.

There are tradeoffs. The interval is part of the specification. Completing the
finite algebra over real coefficients in this norm gives all continuous
functions on the interval, not just rational or analytic functions. Preserving
analytic structure instead calls for locally uniform convergence on appropriate
complex domains. There is no unique compulsory definition of an infinite MDN.

The two familiar fractional expansions illustrate the domain requirement:

\[
L_N=\frac1X-\frac{Y}{X^2}+\cdots+
          \frac{(-Y)^N}{X^{N+1}},
\qquad
R_N=\frac1Y-\frac{X}{Y^2}+\cdots+
          \frac{(-X)^N}{Y^{N+1}}.
\]

Their exact errors on the carry line are

\[
L_N-\frac1b=-\frac1b\left(-\frac YX\right)^{N+1},
\qquad
R_N-\frac1b=-\frac1b\left(-\frac XY\right)^{N+1}.
\]

For positive coordinates, `L_N` converges where `X>Y`; `R_N` converges where
`Y>X`. At `X=Y`, both oscillate instead of converging.

For example, in base 10 on the whole interval `7 <= X <= 9`, twenty terms
of `L` have error at most

\[
\frac1{10}\left(\frac37\right)^{20}<4.37\times10^{-9}.
\]

That is a bound for every point in the interval, not a conclusion from samples.
It gives an operational meaning to “enough digits.”

One can also retain absolutely convergent raw grids using suitable position
weights. For the simple norm `sum |d[i,j]| r^i s^j`, with complex coefficients,
the closed carry quotient is nonzero exactly when

\[
|r-s|\le b\le r+s.
\]

This is a sharp survival criterion, not a promise that every surviving norm
preserves the whole finite function algebra faithfully. The proof and a
stronger interval-based weighting construction are in `collapse_notes.md`.
Weighted convolution algebras are established mathematics; see
[Gröchenig's survey](https://arxiv.org/abs/math/0611174).

## Why both opposite expansions cannot converge together

This obstruction survives every choice of norm. Put `q=-Y/X`. If the left
expansion converges, its successive terms force `q^n` to approach zero. If
the right expansion converges, they force `q^(-n)` to approach zero.

But their product is always 1:

\[
q^n q^{-n}=1.
\]

If multiplication respects limits, the product of those two sequences tending
to zero must tend to zero. Unique limits would then force `1=0`.

The assumptions are a nonzero Hausdorff topological ring, jointly continuous
multiplication, and invertible commuting `X,Y`. The proof does not even require
the particular carry relation. Dropping an assumption changes the problem;
it does not evade the theorem under those assumptions.

A useful design is therefore to keep **directional charts**. One chart treats
powers of `q` as small; another treats powers of `q^(-1)` as small. Each is
consistent on its own. In both charts the fractional expression represents
the same exact rational value `1/b`.

The rigorous formal constructions are `K((q))` and `K((q^(-1)))`, formed after
imposing the finite carry relation. Their allowed support and valuation
topologies prevent tails from escaping in both directions. This is not
unrestricted cellwise convergence. Rational functions can be translated
between charts by exact algebra; arbitrary infinite series cannot necessarily
be translated. [formal_notes.md](formal_notes.md) gives the constructions.

## What the fuzzy pattern is

For this part, keep raw infinite coefficient arrays separate from their MDN
values. A recurrence such as

\[
c_k+c_{k+1}=r_k
\]

fixes each neighbouring sum. If one solution is known, adding

\[
\ldots,C,-C,C,-C,\ldots
\]

gives another solution with exactly the same neighbouring sums. One chosen
coefficient fixes `C`, explaining how one anchor can make all the others
“snap into place.” Whether this describes a particular GUI object requires
matching its actual equations and boundary conditions to this recurrence.

There is a systematic higher-order extension. On one anti-diagonal, solving
the raw coefficient equation `(X+Y)^m F=1` leaves precisely the homogeneous
freedom

\[
(-1)^k P(k),\qquad \deg P<m.
\]

| Inverse order | Unrestricted homogeneous freedom | Consecutive anchors |
|---|---|---:|
| 1 | Alternating constant | 1 |
| 2 | Alternating linear trend | 2 |
| 3 | Alternating quadratic trend | 3 |

This is a complete classification for the stated single-diagonal recurrence,
not just a pattern guessed from finite runs. It follows by converting the
recurrence to repeated finite differences. Bounded digits are an additional
restriction: only the constant homogeneous freedom remains bounded, and for
orders at least 2 there is no bounded inverse-power solution confined to that
one anti-diagonal. Carrying into additional diagonals changes the problem.

The difference between the original opposite raw fractional expansions is
exactly a standard formal delta series:

\[
D=L-R=X^{-1}\sum_{k\in\mathbb Z}(-Y/X)^k,
\qquad (X+Y)D=0.
\]

This is meaningful as a coefficient array acted on by finite polynomials.
Arbitrary products of such arrays need not exist. If we also impose
`X+Y=b` with nonzero invertible `b`, the equation becomes `bD=0`, so its
MDN scalar value must be zero. The difference can record a representation
choice; it cannot give two distinct scalar inverses of the same base.

The formal delta's annihilating factor is `X+Y`, not `X-Y`; it must not be
confused with a physical effect at the positive symmetry line `X=Y`.
This is an exact connection to established
[formal-distribution theory](https://darijgrinberg.gitlab.io/algebra/va3.pdf),
not evidence for a quantum interpretation.

## Why subtracting infinity or averaging needs rules

At `X=Y=5`, each fraxis partial-sum sequence is

\[
0.2,\ 0,\ 0.2,\ 0,\ldots
\]

Averaging either sequence gives `0.1`. Multiplying those resolved values gives
`0.01`. But multiplying the matching approximations first gives

\[
0.04,\ 0,\ 0.04,\ 0,\ldots,
\]

whose average is `0.02`. These are different operations, not two valid values
of a previously defined product. A rule that sums each factor must also specify
how multiplication is to work.

The raw mixed product is equally explicit: with `N+1` terms in each factor,
the central coefficient is `N+1`. Every fixed coefficient on its anti-diagonal
diverges in magnitude. Subtracting the divergent alternating pattern under
equal-length truncation leaves a finite part `Q` satisfying
`(X+Y)^2 Q=2`, not 1. Damping the terms first and taking a different finite part
can instead produce `(X+Y)^2 Q=1`. The exact formulas and proofs are in
`formal_notes.md`.

This identifies a real ambiguity in *how a divergent representation is
processed*. It does not establish a new ambiguous scalar multiplication.
Once the two factors are resolved as ordinary inverses of `b`, their ordinary
product is uniquely `1/b^2`.

## What has been learned, and what remains open

The investigation establishes three useful constraints beyond the earlier
warning about a bad norm:

- Unrestricted local disappearance fails even for integer-coefficient grids.
- No compatible ordinary topology makes both opposite geometric tails vanish.
- Fuzzy recurrence freedom has an exact hierarchy: alternating polynomial
  patterns, with the number of free parameters controlled by inverse order.

There is also a constructive result: interval function limits and directional
formal limits both give consistent infinite extensions. They preserve the
finite carry algebra while refusing specific invalid limit operations.

The next well-defined open problem is **bounded-digit normalization within a
chosen convergence framework**: does a specified carry procedure terminate or
converge, does it preserve the value and error bound, and which boundary data
make the result unique? None of the results above assumes or proves that the
current GUI normalization has these properties.

For software, keep four concepts distinct: exact finite value, displayed grid,
directional expansion with a domain or support rule, and finite approximation
with a certified remainder. A raw fuzzy array can be an additional formal
object, with allowed operations declared explicitly. This separation makes
infinite experiments interpretable without losing the interesting patterns.

## Verification and reproduction

`experiments.py` uses only standard-library exact rational arithmetic. It
checks finite Laurent identities before substituting the carry relation.
`test_infinite.py` contains nine tests covering the two collapse sequences,
geometric tails, all coefficients of finite mixed products, Abel damping,
nonmultiplicative averaging, and recurrence orders 1 through 5 with negative
controls. The generated `results.json` records 21 exact Laurent identities
and 460 exact scalar checks, plus a SHA-256 hash of the experiment source.

```bash
python3 -m unittest discover -s pitfalls/infinite -p 'test_*.py' -v
python3 pitfalls/infinite/experiments.py
```

The finite checks support the derivations and catch indexing errors; the
infinite claims rest on the proofs in the two technical notes. No finite
sample is presented as a proof of convergence or literature novelty.
