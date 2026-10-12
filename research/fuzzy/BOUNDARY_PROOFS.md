# Proofs for a boundary-retaining extension at unbounded depth

This note defines one proposed extension of the finite families in
[PROOFS.md](PROOFS.md). It supplies the mathematical details for
[BOUNDARY.md](BOUNDARY.md). The construction keeps endpoint information
exactly while identifying sequences that differ only at finitely many
initial depths. It is an algebraic construction, not an assertion that the
ordinary partial sums converge, and not a claim of mathematical novelty.
The fractional-input construction covers finite base-\(b\) words and finite
arithmetic expressions. Arbitrary infinite real expansions are not included.

Let \(b\ge2\) be the integer base. Retain the finite carry algebra

\[
\mathcal A_b=\mathbb Z[X^{\pm1},Y^{\pm1}]/(X+Y-b),
\qquad q=-Y/X.
\]

As proved in the finite appendix, this algebra embeds in
\(\mathbb Q(X)\) and is a nonzero integral domain. Its rational coefficient
extension is

\[
\mathcal A_{b,\mathbb Q}
=\mathbb Q\otimes_{\mathbb Z}\mathcal A_b
\cong\mathbb Q[q,q^{-1},(1-q)^{-1}],
\]

with \(X=b/(1-q)\). No ordinary scalar magnitude is assigned to either axis.

Here **positive term counts and zero-based cascade depths are different**:
a cascade with \(\ell\ge1\) terms has GUI depth \(\ell-1\). In particular,

\[
\ell_n=L_{n-1},\qquad r_n=R_{n-1}
\]

use the zero-based \(L_N,R_M\) definitions from the finite study. The index
\(n\) below is a positive term count; depth 20 corresponds to \(n=21\).

## 1. Eventual equality gives exact objects at unbounded depth

Consider sequences \(a=(a_1,a_2,\ldots)\) with \(a_n\in\mathcal A_b\).
Define

\[
a\sim a'
\quad\Longleftrightarrow\quad
\text{there is }n_0\text{ such that }a_n=a'_n
\text{ in }\mathcal A_b\text{ for every }n\ge n_0.
\]

Equality at a depth is full finite carry equality, not equality in a viewing
window. The sequences eventually equal to zero form an ideal: sums vanish
past the larger of their two thresholds, and multiplication by any sequence
preserves eventual vanishing. Therefore

\[
\mathcal G_b
=\mathcal A_b^{\mathbb N_{>0}}/
\{\text{eventually zero sequences}\}
\]

is a commutative associative unital ring. Its elements are eventual
equivalence classes, also called germs of sequences at infinity.

Constant sequences embed injectively: a constant sequence is eventually zero
only when its constant value is zero. Consequently \(1\ne0\) in
\(\mathcal G_b\). This also proves that every exact finite carry identity
remains valid for the represented sequences.

The entire ring \(\mathcal G_b\) is not a domain. The sequence that is 1 at
even indices and 0 at odd indices, and its complementary sequence, are both
nonzero classes whose product is zero. The regular subclass below avoids
these particular zero divisors and has a unique symbolic representation.

Rational bookkeeping takes place in the analogous ring
\(\mathcal G_{b,\mathbb Q}\) formed from \(\mathcal A_{b,\mathbb Q}\).
The injection \(\mathcal A_b\hookrightarrow\mathcal A_{b,\mathbb Q}\)
induces an injection of \(\mathcal G_b\) into this larger ring.

This definition identifies finite initial changes but does not identify two
sequences merely because their difference moves outside each fixed window.
It also does not identify different choices of the depth parameter. A depth
offset or a changed rate is specified as part of the sequence.

## 2. A faithful regular symbolic algebra

Use independent formal symbols \(n,q,E\), and form

\[
\mathcal S=\mathbb Q[n,q^{\pm1},E^{\pm1}].
\]

Its elements have finite, uniquely collected expressions

\[
P(n,q,E)=\sum_{(i,j)\in K}p_{ij}(n)q^iE^j,
\qquad p_{ij}\in\mathbb Q[n],
\]

where \(K\subset\mathbb Z^2\) is finite. Define evaluation at a positive
integer index \(\nu\) by

\[
n\longmapsto \nu,\qquad q\longmapsto q,\qquad E\longmapsto q^\nu.
\]

The values lie in \(\mathcal A_{b,\mathbb Q}\), so this defines a ring map

\[
\Phi:\mathcal S\longrightarrow\mathcal G_{b,\mathbb Q},
\qquad
P\longmapsto
\bigl[(P(\nu,q,q^\nu))_{\nu\ge1}\bigr].
\]

### Injectivity theorem

**Every nonzero \(P\in\mathcal S\) evaluates to a nonzero finite carry class
at every sufficiently large index. In particular, \(\Phi\) is injective.**

To prove this, omit the zero coefficient polynomials \(p_{ij}\). Distinct
pairs \((i,j)\) and \((i',j')\) produce the same exponent after evaluation
only if

\[
i+j\nu=i'+j'\nu.
\]

If \(j=j'\), the distinctness of the pairs makes equality impossible.
If \(j\ne j'\), there is at most one possible index \(\nu\). There are
finitely many pairs, so there are finitely many collision indices.
Also, each nonzero polynomial \(p_{ij}\) has only finitely many integer
zeros, by repeated division by its linear factors. Past all these exceptional
indices, every coefficient is nonzero and all the exponents are distinct.
Their sum is a nonzero Laurent polynomial in the transcendental \(q\).
It is therefore nonzero in \(\mathcal A_{b,\mathbb Q}\), as claimed.

Consequently the regular subclass represented by \(\mathcal S\) is a domain,
and equality in it is exactly equality of the collected polynomial
coefficients \(p_{ij}(n)\). It is not necessary to decide equality by numerical
sampling or by a prescribed finite window. A coincidence at a small index
does not establish equality: for example \(E-q\) vanishes at \(\nu=1\) but
is a nonzero regular germ.

The variable \(n\) records the growing index, and \(E\) records the moving
endpoint \(q^n\). Neither symbol has been assigned the numerical value
“infinity.” The injectivity theorem says that treating them as independent
symbols is faithful to eventual equality for this class of expressions.

If every constant element of \(\mathcal A_{b,\mathbb Q}\), including
\(X=b/(1-q)\), is needed, one can further localize \(\mathcal S\) at
\(1-q\). Injectivity persists after multiplying a putative zero by a common
power of \(1-q\). The fractional templates studied here already lie in
\(\mathcal S\), so that enlargement is not needed for their formulas.

### Offsets and rates remain explicit

If a positive term count is

\[
\ell(n)=an+c,\qquad a\in\mathbb Z_{\ge0},\quad c\in\mathbb Z,
\]

require \(c\ge1\) when \(a=0\). When \(a>0\), any fixed integer offset is
allowed because the count is positive at all sufficiently large indices.
In either case,

\[
q^{\ell(n)}\longleftrightarrow q^cE^a.
\]

The corresponding zero-based cascade depth is \(an+c-1\). Thus matching
counts, fixed offsets, and different growth rates remain distinct:
\(q^n\), \(q^{n+1}\), and \(q^{2n}\) become \(E,qE,E^2\).
The case \(a=0\) records a fixed finite count rather than a growing cascade.
The product \(E E^{-1}=1\) records a correlation between opposite endpoints
that cannot be dropped.

This common-index model covers specified affine paths. Genuinely independent
depth parameters require a multivariable index set and separate endpoint
symbols, with equality defined beyond a threshold in every independent
index. A chosen path then specializes those symbols. Neither a common rate
nor independence is silently assumed.

## 3. The app's higher fractional places

For a fractional place \(k\ge1\) and positive term count \(\ell\ge1\), define
the raw integer grid

\[
H^X_{k,\ell}
=X^{-k}\sum_{j=0}^{\ell-1}
\binom{k+j-1}{j}q^j.
\tag{1}
\]

The coefficient at \((-k-j,j)\) is
\((-1)^j\binom{k+j-1}{j}\). Let \(H^Y_{k,\ell}\) be its transpose under
\(X\leftrightarrow Y\); this also sends \(q\leftrightarrow q^{-1}\).
The first-place templates are

\[
H^X_{1,\ell}=L_{\ell-1},\qquad
H^Y_{1,\ell}=R_{\ell-1}.
\]

These are the raw templates implemented by
[internal_fraxisCascade in Mdn2d.cpp](../../library/src/Mdn2d.cpp).
The root term is inserted separately. At each following term \(j\), the
coefficient is multiplied by \((k+j-1)/j\), producing the binomial in (1),
and its sign is reversed. Thus a configured cascade depth \(D\), with no
additional precision clipping, gives \(\ell=D+1\) terms at each entered
fractional place. The corrected implementation distributes large coefficients
through exact carries; the resulting display need not resemble the raw
single-line template. Input conversion, precision clipping, and raw display
equality are separate issues from the finite identity below.

### Higher-place endpoint formula

For all \(k,\ell\ge1\), finite carry equality gives

\[
\boxed{
H^X_{k,\ell}
=b^{-k}\left[
1-q^\ell\sum_{r=0}^{k-1}
\binom{\ell+r-1}{r}(1-q)^r
\right].
}
\tag{2}
\]

The denominator is rational bookkeeping for the integer-grid element (1).
The equation with denominator cleared holds in the original carry algebra;
replacing \(b^k\) by \((X+Y)^k\) makes it a raw Laurent-grid identity.

**Proof.** Put

\[
A_\ell=(1-q)^k
\sum_{j=0}^{\ell-1}\binom{k+j-1}{j}q^j,
\qquad
S_\ell=\sum_{r=0}^{k-1}\binom{\ell+r-1}{r}(1-q)^r.
\]

We show \(A_\ell=1-q^\ell S_\ell\). At \(\ell=1\), the right side is
\(1-q\sum_{r=0}^{k-1}(1-q)^r=(1-q)^k=A_1\), by finite geometric
telescoping. Pascal's binomial identity gives

\[
S_\ell-qS_{\ell+1}
=\binom{\ell+k-1}{k-1}(1-q)^k.
\]

For completeness, write \(z=1-q\). In
\(S_\ell-(1-z)S_{\ell+1}\), the coefficient difference at \(z^r\) is
\(-\binom{\ell+r-1}{r-1}\) for \(r\ge1\). It cancels the contribution from
\(zS_{\ell+1}\) at the same degree. Only the degree-\(k\) term survives,
with the displayed coefficient. It follows that

\[
\begin{aligned}
(1-q^{\ell+1}S_{\ell+1})-(1-q^\ell S_\ell)
&=q^\ell\binom{\ell+k-1}{k-1}(1-q)^k\\
&=A_{\ell+1}-A_\ell.
\end{aligned}
\]

Induction proves the claimed identity. Multiplication of (1) by
\((X+Y)^k=X^k(1-q)^k\) proves (2).

The Y formula is obtained by \(q\mapsto q^{-1}\):

\[
H^Y_{k,\ell}
=b^{-k}\left[
1-q^{-\ell}\sum_{r=0}^{k-1}
\binom{\ell+r-1}{r}(1-q^{-1})^r
\right].
\tag{3}
\]

### A specified integer subalgebra inside the rational model

For \(\ell(n)=an+c\), formula (2) becomes the regular expression

\[
\mathsf H^X_{k;a,c}
=b^{-k}\left[
1-q^cE^a\sum_{r=0}^{k-1}
\binom{an+c+r-1}{r}(1-q)^r
\right]\in\mathcal S.
\tag{4}
\]

For a fixed \(r\), the binomial here means the polynomial

\[
\binom{an+c+r-1}{r}
=\frac{(an+c)(an+c+1)\cdots(an+c+r-1)}{r!},
\]

with empty product 1. Equation (3) similarly defines
\(\mathsf H^Y_{k;a,c}\), replacing \(q^cE^a\) by \(q^{-c}E^{-a}\)
and \(1-q\) by \(1-q^{-1}\).

Although \(\mathcal S\) allows rational coefficients, these particular
expressions evaluate, at every sufficiently large index, to the integer
grids (1) and their transposes. The subring generated over \(\mathbb Z\) by
these templates and by \(n,q^{\pm1},E^{\pm1}\) therefore lies in the image
of \(\mathcal G_b\). Every finite sum, difference, and product in this
specified subring is an actual sequence of integer-grid carry classes.
Finitely many indices at which an affine count is not positive can be filled
arbitrarily without changing the germ.

This is the sense in which the symbolic model retains the integer backend.
It does not assert that every element of the larger rational algebra
\(\mathcal S\) is an allowed integer-grid operation. In particular, the
constant \(1/b\) belongs to the rational model but is not an element of
\(\mathcal A_b\): \(b\) is not a unit there.

## 4. The hundredth and the exact decimal-carry term

At the common positive count \(n\), write

\[
U=[H^X_{1,n}]=\frac{1-E}{b},\qquad
V=[H^Y_{1,n}]=\frac{1-E^{-1}}b,\qquad F=UV.
\]

The brackets identify the represented germ; the right sides are its
faithful rational bookkeeping expressions. The matched finite relation
survives exactly:

\[
bF=U+V,\qquad U^2=bFU-F.
\tag{5}
\]

However, the app's freshly entered second-place template is

\[
\boxed{
W_X=[H^X_{2,n}]
=\frac{1-E\{1+n(1-q)\}}{b^2}.
}
\tag{6}
\]

For base 10, \(W_X\) is the family of directly entered \(0.01X\) values.
It is not the square of the family of directly entered tenths:

\[
W_X-U^2
=\frac{E\{1-E-n(1-q)\}}{b^2}\ne0.
\tag{7}
\]

Nonvanishing follows from the injectivity theorem, or directly from the
nonzero coefficient of \(E^2\). The coefficient \(n\) in (6) also explains
why endpoint symbols alone, with coefficients independent of depth, are
insufficient for higher fractional places.

Multiplying (6) by \(b\) gives the exact carry comparison

\[
\boxed{bW_X=U-nX^{-1}E,}
\qquad X^{-1}=(1-q)/b.
\tag{8}
\]

More generally, for \(k\ge2\) and a finite positive count \(\ell\),

\[
\boxed{
bH^X_{k,\ell}
=H^X_{k-1,\ell}
-\binom{k+\ell-2}{k-1}X^{-(k-1)}q^\ell.
}
\tag{9}
\]

To prove this, multiply the truncated sum in (1) by \(1-q\).
Pascal's identity turns every coefficient below exponent \(\ell\) into
\(\binom{k+j-2}{j}\), while its last term is
\(-\binom{k+\ell-2}{\ell-1}q^\ell\). The common factor
\(X^{-(k-1)}\) yields (9). For \(k=1\), the separate formula is
\(bH^X_{1,\ell}=1-q^\ell\). Transposition gives both Y formulas.

Thus, in base 10, the retained germ for \(0.09X+0.01X\) is
\(10W_X=U-nX^{-1}E\). It differs from a freshly entered \(0.1X\) by the
specified endpoint. This does not violate the finite grid carry relation:
it compares different truncated decimal templates and retains their full
endpoints. Finite decimal digit carries cannot be treated as equality of
freshly constructed templates without accounting for this term.

## 5. Addition, mixed multiplication, and directional interpretation

The extension preserves ordinary addition and multiplication of its exact
objects. For example, the base-10 mixed sum from the GUI experiment is

\[
S=4U+6V
=1-\frac25E-\frac35E^{-1},
\qquad
5S=5-2E-3E^{-1}\ne5.
\]

The directional terms add normally, but their endpoints are part of the
result. Likewise,

\[
b^2F=2-E-E^{-1}.
\]

The matched two-component representation from the finite proof remains
available for the subalgebra generated by \(U,V\):
\(A(F)+B(F)U\), with \(U^2=bFU-F\). This is similar to using two components
for complex arithmetic, but the coefficients are polynomials in the retained
quantity \(F\); they are not restricted to ordinary scalar constants.
Higher-place templates such as (6) can require \(n\) and \(q\) as well.

### Each direction admits a separate ordinary interpretation

The subring \(\mathbb Q[n,q^{\pm1},E]\), which does not adjoin \(E^{-1}\),
has the evaluation homomorphism

\[
\pi_X:E\mapsto0.
\]

It sends every X template (4), with positive slope \(a\), to \(b^{-k}\).
Consequently a finite decimal expression built from these growing X templates has
its usual rational interpretation under \(\pi_X\). In the opposite subring
\(\mathbb Q[n,q^{\pm1},E^{-1}]\), the analogous map
\(\pi_Y:E^{-1}\mapsto0\) interprets the growing Y templates the same way.
These are explicitly defined algebraic interpretations on their respective
subrings, not proofs of ordinary convergence.

Neither interpretation extends to a homomorphism that deletes both
endpoints in the mixed algebra. Such a map would give

\[
1=\pi(EE^{-1})=\pi(E)\pi(E^{-1})=0.
\]

Thus “each direction has an ordinary interpretation” is compatible with
the construction, while “both interpretations form one operation-preserving
deletion of mixed boundaries” is not.

### Two unital copies of the reals cannot have distinct rational tenths

Suppose a nonzero unital associative algebra had two unital embeddings
\(\iota_X,\iota_Y\) of the ordinary real numbers. Both would send \(b\) to
the same integer multiple \(b\cdot1\). Both
\(\iota_X(1/b)\) and \(\iota_Y(1/b)\) would then be inverses of that same
element. Inverses are unique: if \(u,v\) both invert \(b\cdot1\), then
\(u=u(bv)=(ub)v=v\). The same reasoning makes the embeddings agree on
every rational number.

In particular, distinct \(0.1X\) and \(0.1Y\) cannot simultaneously be
the ordinary real \(1/10\) in two shared-unit real subalgebras with those
laws. Here the resolution is explicit:

\[
bU=1-E,\qquad bV=1-E^{-1},
\]

so these directional templates retain extra information and are not
literal copies of the exact ordinary reciprocal \(1/b\).
The construction keeps the usual algebraic laws and the directional
distinction; it does not also assert those incompatible reciprocal
identities. Extending a treatment of finite decimal inputs to arbitrary
infinite real decimal expansions would require an additional definition.

## 6. Why this is not an ordinary limit in disguise

An eventual germ is an exact equivalence class of an unending process. It
can be used as one algebraic object, with well-defined operations and a
faithful finite symbolic representation in the regular subclass. That does
not make it the ordinary limit of the individual terms.

For example, suppose a Hausdorff topological algebra with continuous
multiplication by the fixed \(q\) made \(q^n\) converge to an invertible
retained element \(E\). The shifted sequence \(q^{n+1}\) would converge to
the same \(E\), since dropping the first term does not change a limit.
Continuity would also make it converge to \(qE\). Uniqueness of limits would
give \(qE=E\), hence \(q=1\), contrary to the formal finite algebra.
Therefore this retained endpoint cannot be obtained as that ordinary limit.

The proposed extension changes the object being used: from one finite member
to its eventual class with endpoint growth intact. It does not discard both
opposite tails, choose a scalar magnitude for them, or make every divergent
family converge. Other equivalence or convergence choices define different
mathematical problems and need their own consistency checks.

## Verification supporting the formulas

The endpoint and carry formulas were checked independently with exact
integer polynomials for every \(1\le k\le15\) and \(1\le\ell\le80\):
1,200 endpoint checks and 1,120 deeper-place carry checks, all passing.
Those 2,320 checks support the implementation of the formulas; the proofs
above establish them for all stated indices. They are separate from any
numerical backend comparison or proof about bounded-digit normalization.
