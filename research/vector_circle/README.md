# Vector encoding, circle identities, and the first paper

The vector idea produces an exact mathematical application: paired MDN
products test ordinary orthogonality and encode oriented area, while keeping
every boundary. The two proposed circle expressions do not equal ordinary
unity in the existing finite arithmetic. The crossed expression instead
has a useful, fixed mixed carrier under a consistent linear encoding.

The first manuscript is available as a
[manuscript PDF](../../output/pdf/mdn_carry_dynamics.pdf) and
[editable LaTeX source](../manuscript/mdn_carry_dynamics.tex). It puts the
elementary arithmetic and representation issues before the general period
theorem, then develops the geometric application. It is a working draft for
specialist review; novelty and journal suitability are not established.
The PDF compiled successfully with the local `pdflatex` after the app
compiler failed during startup. All seven rendered pages were inspected.

## 1. Explain the ordinary features and the surprises precisely

| Topic | What we can say now | Qualification for publication |
|---|---|---|
| Ordinary arithmetic | Finite grids have addition, subtraction, associative and commutative multiplication, and full distributivity. | Defining infinite multiplication requires more than a visible array. |
| Polymorphism | Many grids can represent the same carry class. | Fixing digits −4 through 5 gives a unique finite display. Polymorphism concerns representation; it does not make finite equality ambiguous. |
| Signs | Digit coefficients have signs, and complete MDNs can be negated. The carry law provides no preferred total order. | Digit signs can change under carrying. There is no total order invariant under swapping X and Y. An additional asymmetric order remains possible. |
| Integer symmetry | Integers built at unity have symmetric canonical displays. | Symmetry alone does not imply an integer: the single cell `XY` is a counterexample. |
| Symmetry breaking | Choosing an X or Y cascade chooses a direction. | Axis exchange reflects across `x=y`; the unity line is `x+y=0`. These are different lines. The mixed product `LR` is symmetric and still fuzzy. |
| Changing base | The base occurs in the carry law, so a conversion needs a specified map. | The current app clears the grid on base change; it does not implement a conversion. Universal information loss is not proved. |
| Fuzzy digits | Canonical digits can march through deterministic phases as the cutoff grows. | A phase describes a limiting display, not the complete boundary record. This is not probabilistic or fuzzy-set arithmetic. |

The sign claim has a short proof. The nonzero number `X−Y` becomes its
negative when the axes are swapped. A total order respecting that swap
would have to give both the same sign, which is impossible. This is more
precise than saying that numbers cannot have signs at all.

For base changes, [Mdn2dBase.cpp](../../library/src/Mdn2dBase.cpp) classifies
changing the base as `AllDigitsCleared` and calls `locked_clear()`. Merely
reading the same grid under a different base fails to preserve equality:
`X+Y=10` in base ten, but `X+Y=12` in base twelve. With rational coefficient
extensions, a reversible formal map is possible by scaling both shifts by
the ratio of the bases; it generally leaves finite integer-digit grids.
Thus base dependence is real, while losslessness depends on the chosen
conversion and representation class.

## 2. What happens at an axis

If the parser encodes both `1X` and `1Y` as the same unity cell, then

```
(1,0) -> 1
(0,1) -> 1
```

Those are two different vectors. Their encoding has lost information.
At the polar origin, by comparison, different angle labels describe the
same zero vector. Thus the analogy is useful for thinking about lost
direction labels, but the two mechanisms differ.

The ambiguity is not restricted to unit-circle endpoints: whenever whole
parts enter the shared integer component, the encoding can forget which
axis supplied them.

A boundary-preserving encoding retains directional units. At depth `N`,
write `L=0.1X`, `R=0.1Y`, `n=N+1`, and `E=q^n`, where `q=−Y/X` is a formal
signed diagonal shift. Then

\[
U=10L=1-E,\qquad V=10R=1-E^{-1}.
\]

The complete `U` and `V` differ, even though sufficiently central display
windows show unity in both. A linear encoding

\[
C(x,y)=xU+yV=(x+y)-xE-yE^{-1}
\]

keeps the two coordinates distinguishable. For rational coordinates this
formula is defined in the rational coefficient extension of the finite
carry algebra. An integer version `aL+bR` uses only native grid operations.
Arbitrary real coefficients are an additional extension; this formula
must not silently be identified with the parser's arbitrary decimal input.

In fact `UV=U+V`. Sending both `U` and `V` to ordinary 1 while preserving
addition, multiplication and unity would give `1=2`. Their directional
boundary content therefore cannot be erased by such an arithmetic map.

## 3. The proposed circle equations, tested exactly

We chose an angle with `sin θ=0.6`, `cos θ=0.8`, so there is no floating-point
or trigonometric approximation. These are the 3–4–5 triangle ratios.

The crossed expression is

\[
(0.6X)(0.6Y)+(0.8X)(0.8Y)
=(6L)(6R)+(8L)(8R)
=100LR=2-E-E^{-1}.
\]

It is not 1. At depth 20 the canonical centre digit is 2, with both
endpoint terms present.

The same-axis-square expression is

\[
(0.6X)^2+(0.8Y)^2=36L^2+64R^2,
\]

with exact identity

\[
100(36L^2+64R^2)=36(1-E)^2+64(1-E^{-1})^2.
\]

It also is not 1. At depth 20 its unity digit is 1, but the nearby digit at
`(-1,0)` is 4. Looking only at the centre misses the difference.

There is a useful general replacement for the first identity. With the
linear directional units `U,V`, any rational `s,t` satisfying `s²+t²=1`
obeys

\[
(sU)(sV)+(tU)(tV)=UV.
\]

Thus the crossed expression has a constant around this rational circle:
the fixed mixed MDN `UV=2−E−E⁻¹`. The endpoints stay attached. Using real
`sin θ,cos θ` as external scalar coefficients extends this elementary
identity, but does not define intrinsic sine and cosine of an MDN.

With literal parser input at the axes, both proposed expressions become
ordinary 1. That happens because the parser uses the shared integer at
exactly 1, changing the encoding relative to the directional-unit rule.
It is not a carry changing the value of the same complete MDN.

## 4. A useful way to make the cross terms cancel

One product always expands as

\[
(A+B)(C+D)=AC+AD+BC+BD.
\]

For the example it is

\[
(6L+8R)(6R+8L)=100LR+48(L^2+R^2).
\]

The extra terms are nonzero. However, adding or subtracting two deliberately
chosen products can cancel them exactly.

Define `C(a,b)=aL+bR`; let `σ` exchange the two axes; and let `S` negate
the retained Y component, so `SC(a,b)=aL−bR`. Then

\[
\boxed{C(a,b)\sigma C(c,d)+SC(a,b)\sigma SC(c,d)
=2(ac+bd)LR.}
\]

The ordinary dot product `ac+bd` appears multiplied by the same nonzero
mixed carrier. Therefore the complete result is zero **exactly when the
vectors are perpendicular**. Equal paired self-products likewise detect
equal ordinary squared lengths. No MDN division or global sign is needed.

The backend checked the perpendicular pair `(6,8)` and `(-8,6)`. The paired
dot expression was exactly zero in every cell, including all boundaries.

Likewise,

\[
\boxed{C(a,b)\sigma C(c,d)-\sigma C(a,b)C(c,d)
=(ad-bc)(L^2-R^2).}
\]

This represents oriented area times a fixed nonzero carrier. For the same
perpendicular pair, the full result is

\[
100(L^2-R^2)=-2E+E^2+2E^{-1}-E^{-2}.
\]

**In this example all the area information is at the boundary.** The central
window is empty at depth 20, while the complete answer is nonzero. This is
a concrete reason to keep the boundary in a geometric application.

`S` is defined using the unique retained component decomposition; it is not
an automorphism of all MDN multiplication. A quarter-turn can similarly be
defined on the vector space by `J C(a,b)=C(−b,a)`, with `J²=−I`. This is a
linear operation on encoded vectors, not an identification of an MDN axis
with the complex number `i`. The backend probe calls its component sign
flip `J`; in the mathematical exposition we call that flip `S` to keep it
distinct from the quarter-turn.

Proofs are in [PROOFS.md](PROOFS.md). These elementary vector identities
alone are not claimed as a novel vector algebra. Their interaction with
the retained boundaries and phase dynamics is the research question.

## 5. Verification and reproduction

The new native suite ran **45 cases and 4,045 exact checks, with zero
failures**, at depths 0, 1, 5, 20 and 40. It includes the exact circle
point, swapped and signed variants, and the four literal axis endpoints.
The checks compare complete canonical grids, verify explicit carry
certificates, and preserve full endpoint identities. The dot/area claims
are tested on the non-axial linear encodings. Literal endpoint cases are
explicitly classified separately because their channel information has
already collapsed.

The corrected library was unchanged, SHA-256:
`5ef977e17730182221b7ad66dc4030f839fe892123f6f1dae4bf3133ab509b43`.
Results are in [backend_results.json](backend_results.json). Full grids
and carry certificates are in `/tmp/mdn-vector-circle-01a12881` and can be
regenerated from the repository root:

```bash
python3 research/vector_circle/backend_check.py
```

The compilation transcript is appended to
`~/output_01a12881-e6c8-7cb2-a1d2-7c899e17aa69.log`.

## 6. The shortest credible route towards a paper

The draft's main contribution is the exact general period criterion and
its boundary-sensitive interpretation. The vector experiments supply an
accessible application and a reason the retained information matters.
The opening explains ordinary arithmetic, polymorphism, signs, symmetry,
and base dependence with precise qualifications.

The next actions are a specialist novelty/proof review and a frozen,
buildable source-and-data release. The repository includes the backend corrections, regression suite, manuscript
and research scripts. Full generated certificates can be regenerated with the
[reproduction commands](../manuscript/README.md). A submission must identify
and permanently archive the actual source used for the experiments. The
current manuscript remains a draft for independent specialist review.

Recruit a research-active mathematician working on numeration systems,
algebraic dynamics, combinatorial number theory, or symbolic computation.
A PhD student with a relevant supervisor can be a useful collaborator.
Ask them to test the proofs and identify the closest prior theorems; agree
on authorship according to actual contributions. The manuscript and
reproduction scripts provide something concrete to assess.

Relevant primary literature:

- [Scheicher et al., Digit systems over commutative rings](https://arxiv.org/abs/1004.3729).
- [Strasser, Abelian sandpiles and algebraic models](https://arxiv.org/abs/1707.02281).
- [Fiorot et al., Modular binomials with an application to periodic sequences](https://arxiv.org/abs/2307.02366).
