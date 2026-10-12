# MDN positivity certificate experiment

Completed 10 October 2026. This is a standalone, exact-arithmetic experiment;
it does not use or change the MDN C++ arithmetic backend.

**Result:** MDN inverse carries sometimes find a compact, understandable
positive decomposition. They did not establish a generally smaller or faster
proof system than conventional Bernstein/Handelman methods. For the finite
normalized certificate class tested here, equal-degree expressive power is
exactly the same, in any number of axes.

## What was being tested?

Given a polynomial on the interval from 0 to 1, prove that its value is always
positive. Checking a million sample points is insufficient: a narrow negative
dip could lie between them. A certificate instead rewrites the *whole formula*
as a sum of pieces whose signs are known.

For example,

\[
p(t)=1+t(1-t)^7
\]

is at least 1 throughout this interval. Both `t` and `1-t` are nonnegative,
so their powers and product are nonnegative. The experiment gives the solver
only the expanded polynomial, hiding this convenient expression. Can MDN
recover an expression like it, and do so better than ordinary methods?

It did recover these two pieces. An ordinary sparse Handelman solver recovered
the same two pieces. Bernstein stores the same polynomial as a row of nine
coefficients at degree 8. That is a useful example of compression, but it is
not an MDN-exclusive proof.

## Mathematical scope and the exact equivalence

The original two-axis carry relation is `X+Y=b`. On the nonnegative part of
that line put `X=b*t`, `Y=b*(1-t)`, with `0 <= t <= 1`. Finite grid entries with
nonnegative coordinates become scaled atoms

\[
c_{ij}t^i(1-t)^j,\qquad c_{ij}\geq0.
\]

We allow rational coefficients and an explicit positive rational scale. This
is not a test of bounded integer digits, canonical MDN normalization, negative
grid coordinates, or infinite fuzzy expansions. The scale absorbs the powers
of the original base. The normalized carry identity is

\[
t^i(1-t)^j=t^{i+1}(1-t)^j+t^i(1-t)^{j+1}.
\]

The search merges equal positive contributions in the two children into their
parent. This can remove redundancy while preserving the represented polynomial
exactly.

**Proposition.** For a rational polynomial `p` and integer `D >= degree(p)`,
`p` has a nonnegative MDN atom representation with maximum total degree at most
`D` if and only if its degree-`D` Bernstein coefficients are nonnegative.

**Proof.** Write `u=1-t`. Each atom of degree `i+j <= D` can be homogenized:

\[
t^iu^j=t^iu^j(t+u)^{D-i-j}
 =\sum_{k=0}^{D-i-j}\binom{D-i-j}{k}
     t^{i+k}u^{D-i-k}.
\]

All coefficients remain nonnegative. The resulting homogeneous monomials
are the degree-`D` Bernstein basis
`B[k,D]=binomial(D,k)*t^k*(1-t)^(D-k)`, up to positive factors. The converse
holds because every Bernstein basis element is itself a positive multiple of
an allowed MDN atom. Thus the minimum *global* certificate degree is identical.
For strict positivity on the closed interval we also require positive endpoint
values; the verifiers enforce this.

For `m` axes with nonnegative `x[1],...,x[m]` and sum 1, replace `(t+u)` by
`sum(x)`, and binomial coefficients by multinomial coefficients. The same proof
works. Increasing the number of axes does not remove this equivalence for
this finite, nonnegative simplex certificate class. It says nothing by itself
about every possible MDN extension or infinite expansion.

This places the experiment in established mathematics: positive combinations
of products of domain-defining linear factors are Handelman certificates.
Bernstein degree elevation and subdivision are conventional positivity tools.
References are listed below. The proposition above is proved directly here;
it is not a claimed new theorem.

## Search methods

All methods receive the same expanded rational power coefficients. The hidden
construction, factorization, or original positive atom list is never supplied
to a solver.

| Method | What it does |
|---|---|
| Global Bernstein | Finds the least successful degree, through degree 128 |
| Subdivided Bernstein | Splits intervals at their midpoints and checks each piece at the input degree |
| Hybrid Bernstein | Chooses between limited local degree elevation and splitting, using coefficient count as its heuristic |
| MDN local/global | Starts from global Bernstein and tries several inverse-carry orders |
| MDN targeted/global | Adds greedy extraction of larger positive atoms through degree 32 |
| MDN targeted/subdivision | Applies the same compression separately to Bernstein subdivision leaves |
| MDN targeted/hybrid | Applies the same compression separately to hybrid leaves |
| Sparse Handelman LP | Searches the same positive-atom dictionary by conventional linear programming |

The local MDN portfolio uses deterministic sweeps, heap priorities, random
restarts, and optional constant extraction. The targeted portfolio adds six
greedy extraction policies. These are heuristics, not globally optimal support
or bit-size solvers. MDN compression inherits the success/failure of its
Bernstein starting certificate; this experiment does not demonstrate an
independent MDN feasibility solver.

LP uses degrees at most 32 and the same minimum global degree as the MDN global
comparison. Floating-point linear programming proposes supports. Rational
reconstruction and exact expansion must then succeed before accepting a proof.
A known feasible certificate is retained as fallback. Numerical failures cannot
become false positivity claims. Five LP objectives are used in the main run;
a 25-objective sensitivity run checks whether a stronger conventional search
changes the apparent advantage.

LP is applied globally only, whereas MDN also compresses subdivision and hybrid
leaves. The portfolio comparison therefore does not include conventional sparse
LP on those same leaves; that extra baseline could improve conventional results
further. The global MDN-versus-LP comparisons use matched degrees and domains.

Subdivision has depth limit 16 and leaf limit 4096. Hybrid considers local
degree increases through 16 above the input degree. A cap is a search limit,
not proof that no certificate exists. LP-ineligible cases are reported as
ineligible, not unsuccessful LP searches.

## Benchmarks and the meaning of size

The development set has 82 cases: 22 fixed examples plus 60 generated cases.
After freezing the search methods, the holdout uses a different random seed
and 100 fresh generated examples. This is an internal synthetic holdout, not
an independently curated benchmark or a sample of all mathematical problems.

Each generated set contains equal numbers from five constructions: sparse
positive atoms, random positive Bernstein rows, one polynomial square plus a
positive constant, two squares plus a positive constant, and products of
positive quadratics. The analysis groups the two square constructions together.
The fixed set includes very narrow positive valleys, oscillations, affine
cases, and the earlier motivating quadratic. See `cases.py` for every formula.

Counting only terms favors a sparse expression even when its individual
numbers and grid positions are expensive to store. We therefore report:

1. Nonzero terms and coefficient slots.
2. A declared **numeric-payload bit estimate**, including rational coefficients,
   exponent indices, degrees, and interval endpoints. All methods can use a
   shared rational scale and primitive integer coefficients. Bernstein can use
   its better dense or zero-sparse format. The global unit interval is implicit
   for both. These estimates omit format tags and integer-length delimiters;
   they are not an implemented self-delimiting binary codec or a universal
   proof-complexity measure.
3. Actual compact JSON bytes for the saved certificate format. This includes
   structural overhead and can produce a different ranking.
4. A **degree/partition-only Bernstein recipe**: given the input polynomial,
   tell the checker only which degree to use, and which intervals if needed.
   The checker reconstructs the unique Bernstein coefficients itself.

The input polynomial and common checker are excluded from certificate sizes
for every method. For example, the global degree-8 recipe stores the number 8,
not the nine coefficients. An MDN implementation could use this same recipe
or introduce other reconstruction programs. Consequently, the recipe result
limits a claim about shorter proofs; it does not prove an intrinsic defect in
MDN. This is the distinction between compressing an explicit coefficient list
and minimizing all information a checker needs.

## Results

The main runs saved and exactly verified **1,805 certificates across 182
polynomials**. The sensitivity run adds **137 verified LP certificates**.
All subdivision and hybrid methods certified every main benchmark. Global
Bernstein and its MDN derivatives certified 62/82 development and 82/100 holdout
cases within degree 128. LP was eligible on 58 and 79 cases respectively and
returned checked certificates on all of them.

The following compares the smallest *observed* MDN result among its four
methods with the smallest observed conventional result in the named portfolio.
Choosing the best after running a portfolio is not a single algorithm's runtime
or a proof of optimality.

| Holdout comparator, numeric-payload estimate | MDN smaller | Equal | Comparator smaller |
|---|---:|---:|---:|
| Best explicit Bernstein: global, subdivided, hybrid | 70 | 0 | 30 |
| Above plus sparse Handelman, 5 LP objectives | 43 | 25 | 32 |
| Above plus sparse Handelman, 25 LP objectives | 35 | 23 | 42 |
| Best degree/partition-only Bernstein recipe | 0 | 0 | 100 |

Against explicit Bernstein alone, the median paired size ratio is 0.833
(MDN about 16.7% smaller). Against Bernstein plus LP25, it is 1.0. The latter
portfolio totals **24,449 estimated bits**, versus **25,200 for MDN**: MDN is
about 3.1% larger in aggregate. Development results were more favorable to MDN:
36 smaller, 20 tied, 26 larger against LP25, with totals 44,226 versus 49,767.
The difference reinforces the need to retain the fresh holdout result.

Increasing the conventional LP portfolio from 5 to 25 objectives improved
57 of its 137 eligible certificates and worsened none. Even comparing only
the matched global methods, MDN's holdout wins fell from 44/79 against LP5
to 33/79 against LP25 (29 ties, 17 LP25 wins). Search effort materially affects
the apparent advantage.

Every one of the 182 cases had a smaller degree/partition-only recipe than its
best explicit MDN certificate under the declared payload measure. The holdout
recipe total is 1,268 estimated bits. This is a comparison of verifier input
formats; recomputing coefficients is work, which the checker actually performs.

Actual compact JSON also does not support a general storage advantage. On
the holdout, taking the smallest JSON among saved outputs gives 5 MDN wins,
16 ties, and 79 conventional wins against Bernstein plus LP25. Totals are
22,151 bytes for MDN and 18,020 for the conventional portfolio. The searches
selected candidate outputs by the numeric-payload estimate, not by JSON size,
so this is a serialization sensitivity check, not an optimal codec comparison.

### Concrete examples

| Polynomial | Explicit Bernstein | Best observed MDN | Interpretation |
|---|---|---|---|
| `1+t(1-t)^7` | 9 coefficients, 37 estimated bits | 2 atoms, 10 estimated bits | Good compression; ordinary LP finds the same expression |
| `1+7t^2(1-t)^10+3t^9(1-t)` | 13 coefficients, 79 bits | 3 atoms, 22 bits | Same conclusion |
| `10(2t-1)^2+1` | 2 quadratic pieces, 6 nonzero coefficients, 36 bits | 2 pieces, 4 atoms, 38 bits | Fewer terms, slightly more information |
| `(t-1/2)^2+1/10000` | Global cap reached; subdivision gives 6 coefficients, 84 bits | 4 atoms on the same 2 pieces, 66 bits | Subdivision makes the main feasibility improvement |

For the first example the degree-only Bernstein recipe has estimated payload
4 bits. For the motivating quadratic, the original square expression itself
is already an obvious short proof. A comparison restricted to positive-atom
formats is not a competition against every kind of conventional algebraic
proof. Sum-of-squares, factorization, arithmetic circuits, and other methods
could strengthen the conventional comparison further.

Native positive Bernstein rows also expose an MDN disadvantage: one
development case used 17 Bernstein coefficients and 110 estimated bits, while
the best targeted global MDN result used 17 atoms and 322 bits. Explicitly
specifying atom positions and coefficients can cost more than an ordered row.

### Timing

These are single warmed Python runs on small inputs, not a rigorous speed
benchmark or optimized-library comparison. MDN and LP times include their
required parent Bernstein search. Representative holdout medians were:

| Method | Search | Exact verification |
|---|---:|---:|
| Subdivided Bernstein | 0.109 ms | 0.133 ms |
| MDN targeted/subdivision | 10.983 ms | 0.200 ms |
| MDN targeted/global, successful cases | 10.737 ms | 0.156 ms |
| Sparse Handelman LP5, eligible cases | 11.012 ms | 0.165 ms |

The global and LP medians cover different successful subsets from the full
subdivision rows. There is no observed speed advantage over plain Bernstein.
The degree-only subdivision checker took about 0.112 ms median; reconstruction
did not create a material penalty at these small degrees. Larger degrees and
different implementations could change timings.

## More axes

`simplex_probe.py` checks three nonnegative axes with `x+y+z=1`, using an
independent bivariate expansion with `z=1-x-y`. Of 18 deterministic cases,
14 certified through degree 16; 4 reached the cap. Inverse carries reduced
the term count in 12/14, but reduced the analogous payload estimate in only
4/14. A cubic example went from 10 to 3 terms and 46 to 25 estimated bits.
An axis-bump example went from 45 to 38 terms while growing from 111 to 471
estimated bits.

Every certified probe identity was independently checked by exact expansion.
It is a narrow sanity check, with no simplex subdivision,
LP baseline, or optimal-compression search. Its payload implementation is
separate from the main interval benchmark. The general equivalence proof,
rather than these 18 cases, is what establishes equal degree-bounded power
in arbitrary simplex dimension.

## What the result supports

MDN is a useful way to *see and manipulate* these representations. Local carries
can expose a compact positive structure, and targeted extraction improves
simple local carrying. This can be useful for an exploratory interface and for
generating human-readable candidate identities.

The present evidence does not show a new positivity theorem, smaller minimum
degree, better general proof size, faster certification, a Riemann-hypothesis
advance, or a physical interpretation of fuzzy digits. Finite normalized
positive MDN certificates have landed inside a known mathematical framework.
That is a concrete connection, and it also gives a precise limit on what this
particular route can establish.

A reasonable continuation is an MDN/Bernstein workbench: show the grid,
polynomial, and positivity proof together; permit local carries; and check
every operation against exact polynomial semantics. If pursuing a research
claim about compression, the next test should use substantially larger
independently sourced problems and compare against sparse LP, subdivision,
factorization and sum-of-squares. It should declare whether the objective is
human readability, stored bytes, discovery time, or checking time. A benefit
in one does not imply a benefit in the others.

## Reproduction and files

Run from the repository root with Python 3.12 and NumPy/SciPy available. The
saved runs record Python 3.12.3, NumPy 2.5.1 and SciPy 1.18.0; consult their
`environment` fields for the exact platform. Matplotlib is only needed for
the optional figure. No C++ build is needed.

```bash
python3 -m unittest discover -s pitfalls/positivity -p 'test_*.py' -v
python3 pitfalls/positivity/run.py --output pitfalls/positivity/results/development.json
python3 pitfalls/positivity/run.py --seed 20261117 --per-family 20 --generated-only --output pitfalls/positivity/results/holdout.json
python3 pitfalls/positivity/analyze.py pitfalls/positivity/results/development.json pitfalls/positivity/results/holdout.json --output pitfalls/positivity/results/summary.json
python3 pitfalls/positivity/lp_sensitivity.py pitfalls/positivity/results/development.json pitfalls/positivity/results/holdout.json --output pitfalls/positivity/results/lp_sensitivity.json
python3 pitfalls/positivity/simplex_probe.py --output pitfalls/positivity/results/simplex.json
python3 pitfalls/positivity/audit.py
python3 pitfalls/positivity/plot_results.py
```

The eight unit tests check independent polynomial expansion, degree elevation,
subdivision, minimum degree, carry identities, strict-positivity controls,
tiny rational perturbations, malformed partitions, and saved-format roundtrips.
`common.py` is the search-independent verifier. It checks rational identities
and interval coverage, not sampled values. `audit.py` additionally checks saved
metrics, source/input hashes, minimum global degrees, and full expansions of
successful global certificates. `results/fairness_audit.json` records the audit.

The main JSON files contain the input polynomials, successful certificates,
caps and exclusions, metrics, timings, parameters, and source SHA-256 hashes.
Hashes identify the source files present when those runs were made; later
reporting/plotting/auditing files do not require a solver rerun. Search seed is
1729, development generation seed 20261010, holdout seed 20261117. LP choices
can depend on numerical-library versions; exact acceptance never does.

The scripts are intended as reproducible research code, not as a hardened
untrusted-input certificate service. In particular, no resource limits are
enforced on a maliciously large certificate passed directly to the checker.

## Primary references

- David Handelman, *Representing polynomials by positive linear functions on
  compact convex polyhedra*, Pacific Journal of Mathematics 132(1), 35–62
  (1988), [doi:10.2140/pjm.1988.132.35](https://doi.org/10.2140/pjm.1988.132.35).
- Fatima Boudaoud, Fabrizio Caruso, Marie-Françoise Roy, *Certificates of
  Positivity in the Bernstein Basis*, Discrete & Computational Geometry 39,
  639–655 (2008), [doi:10.1007/s00454-007-9042-x](https://link.springer.com/article/10.1007/s00454-007-9042-x).

These references establish the conventional context. The numerical results
and implementation choices above are from the local experiment, not claims
made by those papers.
