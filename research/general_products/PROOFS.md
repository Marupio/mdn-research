# General matched products: exact eventual display periods

This note gives a finite algorithm, with a proof, for the smallest eventual
canonical display period of

\[
G_N^{a,c}=L_N^aR_N^c,\qquad a,c\ge0.
\]

The base is 10. Depth $N$ is zero-based, and $n=N+1$ is the positive
cascade term count. The digit convention is the fixed consecutive set
$D=\{-4,-3,\ldots,5\}$, using the forward carry sweep proved in
[the phase-product appendix](../phase_products/PROOFS.md#3-a-fixed-canonical-carry-display).
These are theorems about that canonical convention, not unproved claims
about the GUI's Positive sign policy.

All arguments use integer coefficients, grid positions and carries. $X,Y$
are positional shifts, never assigned ordinary scalar values. The finite
endpoint information remains part of every finite product.

## 1. What a period means

A positive integer $s$ is an **eventual display period** if, for every
fixed finite window of grid cells, the canonical displays of $G_{N+s}^{a,c}$
and $G_N^{a,c}$ agree in that window for all sufficiently large $N$.
The threshold may depend on the window. This does not say that the two
complete finite grids are equal modulo carries.

The smallest such $s$ is written $T_{a,c}$. A global repeating cycle of
length $T_{a,c}$ need not consist of $T_{a,c}$ different arrays: particular
positions within a cycle can coincide. Section 8 supplies an exact test
for equality of two limiting arrays, separately from the global period.

If $a=0$ or $c=0$, then $T_{a,c}=1$. In a single-direction product, the
raw coefficient at each fixed position eventually stops changing:
there are only finitely many nonnegative cascade-index tuples that can
contribute to that position. Locality of the carry sweep then gives the
same stabilization for every displayed digit. This also covers the
constant product $a=c=0$.

The rest of the note assumes $a,c\ge1$. Put

\[
d=a+c,\qquad r=d-1.
\]

## 2. Interior coefficient polynomial

Before carrying, $G_N^{a,c}$ is supported on $x+y=-d$. At the cell

\[
(x,y)=(k-a,-k-c)
\]

its coefficient is $(-1)^k C_n(k)$, where

\[
C_n(k)=[z^{k+a(n-1)}](1+z+\cdots+z^{n-1})^d.
\tag{1}
\]

Expand the right side as $(1-z^n)^d/(1-z)^d$. For each fixed $k$ and
all sufficiently large $n$, the terms with inclusion-exclusion index
$u<a$ contribute polynomial coefficients, the term $u=a$ contributes
a fixed truncated coefficient, and terms $u>a$ are absent. Thus

\[
\boxed{C_n(k)=P_{a,c}(n,k)+J_{a,c}(k),}
\tag{2}
\]

where

\[
\boxed{
P_{a,c}(n,k)=
\sum_{u=0}^{a-1}(-1)^u\binom du
\binom{(a-u)n+k+c-1}{r}}
\tag{3}
\]

and

\[
J_{a,c}(k)=
\begin{cases}
(-1)^a\binom da\binom{k+c-1}{r},&k\ge a,\\
0,&k<a.
\end{cases}
\]

The term $J_{a,c}(k)$ is independent of the cutoff. It cancels when two
depths are compared. Formula (3) is a polynomial with rational
coefficients, but it takes integer values at all integer pairs $(n,k)$.
For this polynomial statement, the usual extension
$\binom{x}{h}=x(x-1)\cdots(x-h+1)/h!$ is used for every integer $x$.

The unsigned coefficient $C_n(k)$ counts a finite convolution. Its use
does not attach a scalar magnitude to the grid position.

## 3. The exact divisibility criterion

Let $\Delta_k f(k)=f(k+1)-f(k)$ be the **forward** difference. For
$j=0,\ldots,r-1$, define

\[
\boxed{
Q_{j,s}(n,k)=
\sum_{u=0}^{a-1}(-1)^u\binom du
\left[
\binom{(a-u)(n+s)+k+c-1}{r-j}
-\binom{(a-u)n+k+c-1}{r-j}
\right].}
\tag{4}
\]

The binomial difference identity shows that

\[
Q_{j,s}(n,k)=
\Delta_k^j\bigl(P_{a,c}(n+s,k)-P_{a,c}(n,k)\bigr).
\tag{5}
\]

Its total degree in $(n,k)$ is at most $r-j-1$: the binomial polynomial
has degree $r-j$, and subtraction of its translate in $n$ reduces that
degree by at least one.

**Period criterion.** A positive $s$ is an eventual display period if
and only if

\[
\boxed{
10^{j+1}\mid Q_{j,s}(n,k)
\quad\text{for every integer }n,k
\text{ and }0\le j\le r-1.}
\tag{6}
\]

### Why the criterion describes the carry sweep

At layer $j$, index the cell by $(k-a,-k-c+j)$. If $v_j(k)$ is its
incoming coefficient, the sweep is

\[
h_j(k)=\left\lfloor\frac{v_j(k)+4}{10}\right\rfloor,
\quad d_j(k)=v_j(k)-10h_j(k),
\quad v_{j+1}(k)=h_j(k)+h_j(k-1).
\]

Two incoming coefficients give the same digit precisely when their
difference is divisible by 10. Conditional on agreement of earlier
layers, the incoming difference at layer $j$ is

\[
\frac{(-1)^k}{10^j}\,
\nabla_k^j\bigl(P_{a,c}(n+s,k)-P_{a,c}(n,k)\bigr),
\tag{7}
\]

where $\nabla_k f(k)=f(k)-f(k-1)$ is the backward difference. Forward
and backward $j$th differences differ only by translation of $k$:
$\nabla_k^j f(k)=\Delta_k^j f(k-j)$. Hence the all-$k$ conditions in
(6) are exactly the successive digit-agreement conditions.

The initial difference has $k$-degree at most $r-1$. Its $r$th
difference is zero. Once (6) has ensured agreement of the first $r$
layers, the incoming difference on every subsequent layer vanishes.

Each fixed cell depends on only finitely many raw coefficients: the digit
at layer $j$, index $k$, depends on raw indices $k-j,\ldots,k$. Taking
$n$ sufficiently large puts all those indices inside the range where
(2) applies. This proves sufficiency without discarding distant
boundaries or assuming a uniform bound on the number of carry layers.

For necessity, eventual agreement first gives divisibility of the
lowest-layer polynomial for every fixed $k$ and all sufficiently large
$n$. Inducting through (7) gives the same statement for every $Q_{j,s}$.
The resulting divisibilities extend to all integer pairs. To see this
without any analytic limit, note that an integer-valued polynomial of
degree at most $t$, reduced modulo any positive integer $M$, has an
integer period in each argument. Expand it in the integer binomial
basis; a shift by $M\,\operatorname{lcm}(1,\ldots,t)$ preserves every
basis term modulo $M$ by the finite binomial translation identity.
One can therefore move any fixed integer $n$ forward into the eventual
range without changing the residue. The $k$ values were arbitrary
already. The case $t=0$ is immediate.

## 4. A finite exact test

The universal quantifiers in (6) can be replaced by a small finite set.
For each $j$, put $t=r-j-1$. Then it suffices to test

\[
\boxed{
n,k\ge0,\qquad n+k\le t.}
\tag{8}
\]

Indeed, an integer-valued polynomial $Q(n,k)$ of total degree at most
$t$ has the finite Newton expansion

\[
Q(n,k)=\sum_{i+h\le t}q_{i,h}\binom ni\binom kh,
\qquad
q_{i,h}=(\Delta_n^i\Delta_k^hQ)(0,0)\in\mathbb Z.
\]

If all values in the triangle (8) are divisible by $M$, then every
$q_{i,h}$ is divisible by $M$: its finite-difference formula uses only
values with $n\le i$, $k\le h$, hence $n+k\le i+h\le t$. The expansion
then proves divisibility at every integer pair, including negative
integers. The converse follows by evaluating at the triangular samples.

For (4), all binomial top arguments in these samples are nonnegative.
Thus an implementation can use ordinary exact integer binomial
coefficients, with $\binom{x}{h}=0$ when $0\le x<h$.

For one candidate shift, the total number of sample congruences is

\[
\sum_{m=1}^{r}\frac{m(m+1)}2
=\frac{r(r+1)(r+2)}6.
\]

This is a finite proof test at any multiplication order; it does not
require constructing a cascade whose depth equals the candidate period.

## 5. A universal period and the minimal-period algorithm

**Universal bound.** For every mixed pair $a,c\ge1$,

\[
\boxed{U=10^{a+c-1}=10^r\text{ is an eventual display period}.}
\tag{9}
\]

To prove this, fix a term of (4), let $m=r-j$, and put $v=a-u\ge1$.
The finite binomial translation identity gives

\[
\binom{v(n+U)+k+c-1}{m}
-\binom{vn+k+c-1}{m}
=\sum_{h=1}^{m}\binom{vU}{h}
\binom{vn+k+c-1}{m-h}.
\]

For either prime $p=2$ or $p=5$, the identity

\[
\binom{vU}{h}=\frac{vU}{h}\binom{vU-1}{h-1}
\]

implies

\[
\operatorname{ord}_p\binom{vU}{h}
\ge r-\operatorname{ord}_p(h)
\ge r-(h-1)
\ge r-m+1=j+1.
\]

Here $\operatorname{ord}_p$ counts factors of the prime $p$ in an ordinary
integer coefficient; it is not a magnitude assigned to an MDN. Every
term is divisible by both $2^{j+1}$ and $5^{j+1}$, hence by $10^{j+1}$.
This proves (6) for $s=U$.

The integer periods form a subgroup of $\mathbb Z$. A zero shift is a
period; opposite shifts and sums of periods preserve eventual agreement
in every fixed window. Therefore this subgroup is $T_{a,c}\mathbb Z$,
and (9) implies

\[
T_{a,c}\mid10^r.
\tag{10}
\]

The following algorithm is consequently exact:

1. Start with $T=10^r$.
2. While $T$ is divisible by 2 and $T/2$ passes (4), (6), (8), replace
   $T$ by $T/2$.
3. While $T$ is divisible by 5 and $T/5$ passes the same test, replace
   $T$ by $T/5$.
4. Return $T$.

Every retained value is a period. Since every period is a multiple of
the least period, a failed division by a prime says that the current
exponent of that prime is already minimal. Removing factors of the
other prime cannot change that fact. Thus the algorithm returns exactly
$T_{a,c}$, with at most $2r$ candidate tests.

This is a general exact answer even where no short closed formula is
available.

## 6. A useful necessary bound, and why it is not the whole answer

The highest difference in (5) is independent of $n,k$:

\[
Q_{r-1,s}(n,k)=sA_{a,c},\qquad
A_{a,c}=\sum_{u=0}^{a-1}(-1)^u\binom du(a-u)
=(-1)^{a-1}\binom{d-2}{a-1}.
\]

For the last identity, the finite alternating partial sum
$\sum_{u=0}^{m}(-1)^u\binom du=(-1)^m\binom{d-1}{m}$ follows immediately
by applying Pascal's identity and cancelling adjacent terms. Applying
it twice, or summing those partial sums for $m=0,\ldots,a-1$, gives the
stated weighted sum.

Condition (6) therefore implies

\[
\boxed{
T_0=\frac{10^{d-1}}
{\gcd\left(10^{d-1},\binom{d-2}{a-1}\right)}
\quad\text{divides }T_{a,c}.}
\tag{11}
\]

This lower bound is often the answer, but the lower carry layers can
impose extra conditions. For example, for $(a,c)=(2,8)$ it gives
$T_0=125{,}000{,}000$, whereas the exact test gives

\[
\boxed{T_{2,8}=250{,}000{,}000.}
\]

One explicit obstruction to the smaller shift occurs at $j=6$, $n=1$,
$k=0$ in the **forward-difference** formula (4): its remainder is
$5{,}000{,}000$ modulo $10^7$, rather than zero. A simpler equivalent
symbolic witness uses the sixth **backward** difference, whose underlying
polynomial is

\[
\binom{2n+k+1}{3}-10\binom{n+k+1}{3}.
\]

At $n=1,k=0$, its change under $n\mapsto n+s$ is

\[
\frac{s(1-3s-s^2)}3.
\]

For $s=125{,}000{,}000$, the bracket and denominator are odd, so this
has exactly six factors of 2. Layer 6 requires seven factors of 2 as
part of divisibility by $10^7$. Thus the candidate fails. The finite
criterion verifies that doubling it suffices.

For the special cases $a=1$ or $c=1$, the binomial in (11) is 1.
The lower and upper bounds coincide, proving the closed formula

\[
\boxed{T_{1,c}=10^c,\qquad T_{a,1}=10^a.}
\]

In particular the required global periods grow without bound as
multiplication order grows. In these special cases the same highest
difference also forces any two coinciding phase labels to differ by a
multiple of the full period. They therefore have $10^c$ and $10^a$
distinct arrays, respectively.

## 7. Low-order periods

The exact finite algorithm gives the following values. Each row lists
$a=1,\ldots,d-1$, with $c=d-a$; reflection explains the symmetry.

| Total order $d$ | Minimal periods $T_{a,d-a}$ |
|---|---|
| 2 | 10 |
| 3 | 100, 100 |
| 4 | 1,000; 500; 1,000 |
| 5 | 10,000; 10,000; 10,000; 10,000 |
| 6 | 100,000; 25,000; 50,000; 25,000; 100,000 |
| 7 | 1,000,000; 200,000; 100,000; 100,000; 200,000; 1,000,000 |
| 8 | 10,000,000; 5,000,000; 2,000,000; 500,000; 2,000,000; 5,000,000; 10,000,000 |

These are derived using finite exact congruences, not by rendering
millions of cascade cells. Low-order backend comparisons have the
separate purpose of checking the relationship to actual finite grids
and the GUI's display convention.

## 8. Testing whether two phase arrays coincide

Let $\Phi_n$ denote the eventual canonical array along positive term
counts congruent to the integer label $n$ modulo the universal period
$U=10^r$. This defines an array for every integer label, including 0:
use sufficiently large positive representatives. Section 5 makes the
definition independent of the representative.

For two labels $n$ and $n+s$, the carry argument gives the exact test

\[
\boxed{
\Phi_{n+s}=\Phi_n
\iff
10^{j+1}\mid Q_{j,s}(n,k)
\text{ for }0\le j\le r-1,
\quad k=0,\ldots,r-j-1.}
\tag{12}
\]

Here $n$ is fixed, so only a one-variable Newton test in $k$ is needed.
The total-degree bound in Section 3 bounds its $k$-degree by $r-j-1$.
Equation (12) compares the full arrays, not merely a sampled window.
Changing $n$ by $U$ leaves each residue in (12) unchanged: its change is
the difference of two of the already-proved congruences for the shift
$U$. Thus evaluating at a label such as 0 is equivalent to evaluating
at a sufficiently large positive representative.

Unlike the global period test, it can pass at one label and fail at
another. For $(a,c)=(2,8)$ and $s=125{,}000{,}000$, the finite test (12)
passes at $n=0$ but fails at $n=1$. Thus

\[
\Phi_{125{,}000{,}000}=\Phi_0,
\qquad
\Phi_{125{,}000{,}001}\ne\Phi_1,
\]

even though the minimal global period is $250{,}000{,}000$. This is why
the general result is stated as a smallest repeat period rather than
an automatic count of distinct arrays. For the original $F$, $FL$, $FR$
and $F^2$ families, the separate low-order proofs establish that all
10, 100, 100 and 500 respective arrays are distinct.
The full collision classification for the order-10 example, including
its distinct-array count, is proved in
[PHASE_COLLISIONS.md](PHASE_COLLISIONS.md).

## 9. The finite boundary remains exact

For every finite cutoff, with $E=q^{N+1}$ and $q=-Y/X$, telescoping gives

\[
\boxed{
10^{a+c}G_N^{a,c}=(1-E)^a(1-E^{-1})^c.}
\]

The period algorithm concerns fixed-window canonical digits as the
boundary moves. It does not replace $E$ by zero, identify different
finite cutoffs, or claim that multiplying infinite arrays by ordinary
convolution is defined.

Independent cutoffs require their own endpoint factors. Explicitly, if
the X-directed factors have zero-based depths $N_1,\ldots,N_a$ and the
Y-directed factors have depths $M_1,\ldots,M_c$, then

\[
10^{a+c}\prod_{i=1}^aL_{N_i}\prod_{j=1}^cR_{M_j}
=\prod_{i=1}^a(1-q^{N_i+1})
\prod_{j=1}^c(1-q^{-M_j-1}).
\]

Their offsets and relative growth are not encoded by a single
matched-depth period.
The theorem supplies exact display information for a broad finite
family while keeping that distinction intact.
