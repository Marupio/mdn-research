# Proofs for boundary-retaining fuzzy MDN families

This appendix proves the claims in [README.md](README.md) directly from finite
grid arithmetic. It concerns exact finite objects and parameterized families;
it neither chooses an infinite summation rule nor claims mathematical novelty.

Throughout, the base is an integer $b\ge 2$, and $X,Y$ are commuting,
invertible formal grid shifts. **$N,M,P,Q\ge0$ are zero-based cutoff indices:**

\[
L_N=\sum_{i=0}^{N}(-1)^iX^{-i-1}Y^i,
\qquad
R_M=\sum_{j=0}^{M}(-1)^jX^jY^{-j-1},
\qquad F(N,M)=L_NR_M.
\]

Thus $L_N$ has $N+1$ terms. When positive term counts are convenient, we
explicitly write

\[
\ell_n=L_{n-1},\qquad r_m=R_{m-1},\qquad n,m\ge1.
\]

## 1. Finite carry equality and coefficient rings

A raw grid is an element of

\[
\mathcal R=\mathbb Z[X^{\pm1},Y^{\pm1}].
\]

Raw equality means equality of all coefficients. Finite carry equality means
equality in

\[
\mathcal A_b=\mathcal R/(X+Y-b).
\]

In particular, a finite sum of integer multiples of translated carry relations
is zero in $\mathcal A_b$. This definition does not assume a unique bounded
digit representation or a normalization algorithm that decides every carry
equivalence.

Eliminating $Y$, then retaining the inverses of both shifts, gives

\[
\mathcal A_b\cong
\mathbb Z[X,X^{-1},(b-X)^{-1}]\subset\mathbb Q(X).
\]

The inclusion is injective: this is the localization of the integral domain

\[
\mathbb Z[X,Y]/(X+Y-b)\cong\mathbb Z[X]
\]

at the nonzero elements $X$ and $b-X$. Consequently $\mathcal A_b$ is
a nonzero integral domain, and multiplication by a nonzero integer has no
kernel. No real magnitude has been assigned to $X$.

Set

\[
T=X+Y,\qquad q=-Y/X.
\]

The element $q$ is invertible. In the rational coefficient extension,

\[
\mathbb Q\otimes_{\mathbb Z}\mathcal A_b
\cong\mathbb Q[q,q^{-1},(1-q)^{-1}],
\qquad
X=\frac b{1-q},\quad Y=\frac{-bq}{1-q}.
\]

In particular, $q$ is transcendental over $\mathbb Q$. Rational expressions
below use this injective extension for bookkeeping and proofs; they do not
add arbitrary division by $b$ to the integer grid operations. Indeed, if
$p$ is a prime divisor of $b$, then

\[
\mathcal A_b/(p)\cong\mathbb F_p[X^{\pm1}]\ne0.
\]

Thus $b$ is not a unit in $\mathcal A_b$. An expression such as
$(1-q^{N+1})/b$ denotes the particular integer-grid element $L_N$ already
constructed above, not an assertion that every element can be divided by $b$.

## 2. General endpoint identity

Finite geometric telescoping in the **raw** ring gives

\[
TL_N=1-q^{N+1},\qquad TR_M=1-q^{-M-1}. \tag{1}
\]

For example, $T=X(1-q)$, so multiplying
$L_N=X^{-1}\sum_{i=0}^Nq^i$ cancels all intermediate terms. The right
identity follows similarly using $T=Y(1-q^{-1})$.

For any $a,c\ge0$ with $d=a+c\ge1$, define

\[
P=\prod_{i=1}^{a}L_{N_i}\prod_{j=1}^{c}R_{M_j}.
\]

Multiplying (1) proves the general endpoint identity

\[
\boxed{
T^dP=
\prod_{i=1}^{a}(1-q^{N_i+1})
\prod_{j=1}^{c}(1-q^{-M_j-1}).
} \tag{2}
\]

It holds before carrying. In $\mathcal A_b$, replace $T^d$ by $b^d$.
Every endpoint is a power of the same $q$; it must retain its exponent when
products are formed. In particular,

\[
b^2F(N,M)=1-q^{N+1}-q^{-M-1}+q^{N-M}. \tag{3}
\]

An extra independently truncated $L_P$ or $R_Q$ contributes its own factor
$1-q^{P+1}$ or $1-q^{-Q-1}$. A second fuzzy member $F(N',M')$ contributes
both of its factors. The square is the specialization $N'=N,M'=M$, not a
replacement of four independently specified indices without justification.

### An exact two-factor reduction

Comparison of finite geometric sums gives the raw identity

\[
\boxed{TL_NR_M=L_N+R_M-E_{N,M},} \tag{4}
\]

where

\[
E_{N,M}=
\begin{cases}
R_{M-N-1},&M>N,\\
0,&M=N,\\
L_{N-M-1},&N>M.
\end{cases}
\]

To check the endpoints, multiply $TL_N=1-q^{N+1}$ by $R_M$. If $M>N$,
then $q^{N+1}R_M=-L_N+R_{M-N-1}$. If $N>M$, it is
$-L_N+L_{N-M-1}$. If $M=N$, it is $-L_N$. These three identities follow
by splitting the shifted finite sum at exponent zero. This proves (4),
including its one-term and empty-tail cases.

### Integer-scaled single-cascade reduction

Expand the right side of (2) as the finite Laurent polynomial

\[
H(q)=\sum_{r\in\mathbb Z}a_rq^r,\qquad a_r\in\mathbb Z.
\]

As a formal polynomial evaluation, $H(1)=0$, because it is a nonempty
product of factors $1-q^u$. This evaluation is only used on the Laurent
polynomial $H$; it is not an evaluation of $X=b/(1-q)$ at $q=1$.
Therefore $\sum_ra_r=0$. Equation (1), after carrying, gives

\[
q^r=
\begin{cases}
1-bL_{r-1},&r>0,\\
1,&r=0,\\
1-bR_{-r-1},&r<0.
\end{cases}
\]

Substituting into $b^dP=H(q)$ cancels all constants. Cancellation of the
remaining common factor $b$, valid because $\mathcal A_b$ has no integer
torsion, proves

\[
\boxed{
b^{d-1}P
=-\sum_{r>0}a_rL_{r-1}-\sum_{r<0}a_rR_{-r-1}.
} \tag{5}
\]

This is an identity using integer coefficients throughout. Every finite
product, after the displayed integer scaling, is a finite linear combination
of single directional cascades with possibly different cutoffs.

## Raw-pattern theorem

For the product $P$ in (2), put

\[
d=a+c,\qquad A=\sum_iN_i,\qquad B=\sum_jM_j,
\]

and let $h_1,\ldots,h_d$ be the concatenation of the left and right cutoff
lists. Every raw nonzero cell has the form

\[
(x,y)=(k-a,-k-c),\qquad -A\le k\le B,
\]

and its coefficient is

\[
\boxed{(-1)^kC(k),\qquad
C(k)=[z^{k+A}]\prod_{\ell=1}^{d}(1+z+\cdots+z^{h_\ell}).} \tag{6}
\]

**Proof.** Choose terms with indices $n_i\in[0,N_i]$ and
$m_j\in[0,M_j]$. Their product has $k=\sum_jm_j-\sum_in_i$, the stated
exponents, and sign
$(-1)^{\sum n_i+\sum m_j}=(-1)^k$. Consequently $C(k)$ counts such tuples.
Replacing each $n_i$ by $N_i-n_i$ turns the condition into a bounded
nonnegative sum with target $k+A$, proving the generating-polynomial
formula. Every integer between zero and $A+B$ can be realized as such a
bounded sum: add the intervals $\{0,\ldots,h_\ell\}$ one at a time.
This also proves the exact support assertion.

Define the truncated-binomial kernel

\[
\mathcal B_r(t)=
\begin{cases}
\binom{t+r}{r},&t\in\mathbb Z_{\ge0},\\
0,&t\in\mathbb Z_{<0}.
\end{cases}
\]

Since

\[
\prod_{\ell=1}^d(1+z+\cdots+z^{h_\ell})
=\frac{\prod_{\ell=1}^d(1-z^{h_\ell+1})}{(1-z)^d},
\]

expanding the numerator and counting nonnegative $d$-tuples of a given sum
in the denominator yields

\[
\boxed{
C(k)=\sum_{J\subseteq\{1,\ldots,d\}}(-1)^{|J|}
\mathcal B_{d-1}\!\left(k+A-\sum_{\ell\in J}(h_\ell+1)\right).
} \tag{7}
\]

The denominator count is $\binom{s+d-1}{d-1}$: arranging $s$ identical
marks and $d-1$ separators records the $d$ nonnegative entries. This
establishes (7) without an assumption about an infinite MDN expansion.

Each active term of (7) is a polynomial in $k$ of degree at most $d-1$.
Its activation point is

\[
k=-A+\sum_{\ell\in J}(h_\ell+1).
\]

These subset sums specify the piecewise-polynomial envelope. Candidate
activation points can coincide, their coefficients can cancel, and short
cutoffs can reduce the degree. The $2^d$ subsets are an upper bound on the
number of candidate points, not a count of distinct visible regions.

### The requested products

| Product | Left cutoff list | Right cutoff list | Raw total degree | Envelope degree |
|---|---|---|---:|---:|
| $F(N,M)$ | $N$ | $M$ | $-2$ | at most 1 |
| $F(N,M)L_P$ | $N,P$ | $M$ | $-3$ | at most 2 |
| $F(N,M)R_Q$ | $N$ | $M,Q$ | $-3$ | at most 2 |
| $F(N,M)F(N',M')$ | $N,N'$ | $M,M'$ | $-4$ | at most 3 |

For $F$, direct interval intersection gives

\[
C(k)=\max\!\bigl(0,\min(N,M-k)-\max(0,-k)+1\bigr).
\]

This is triangular for equal cutoffs and trapezoidal otherwise. With every
cutoff equal to $N$, the unsigned $k=0$ coefficients are

\[
\begin{array}{c|c|c}
\text{product}&\text{cell}&C(0)\\\hline
F(N,N)&(-1,-1)&N+1\\
F(N,N)L_N&(-2,-1)&\binom{N+2}{2}\\
F(N,N)R_N&(-1,-2)&\binom{N+2}{2}\\
F(N,N)^2&(-2,-2)&\dfrac{(N+1)(2N^2+4N+3)}3
\end{array}
\]

For the three-factor products, symmetry reduces coefficient extraction to
$[z^N](1+\cdots+z^N)^3=\binom{N+2}{2}$; no upper bound is active at total
$N$. For the square, convolution of the symmetric triangle gives
$(N+1)^2+2\sum_{j=1}^Nj^2$, which equals the displayed expression.
At $N=20$ the four coefficients are $21,231,231,6181$.

Multiplication concatenates cutoff lists, so all higher products remain in
this parameterized family. This closure permits unbounded order; it does not
give finitely many fixed coefficient patterns as a scalar basis. Also,

\[
R_M=-q^{-M-1}L_M
\]

holds as a raw identity by reversing the finite sum. Since
$-q^{-M-1}=(-1)^M X^{M+1}Y^{-M-1}$, replacing a left factor by the
corresponding right factor translates its raw coordinates by
$(M+1,-M-1)$ and multiplies its coefficients by $(-1)^M$. It preserves
the signs at even cutoff indices and reverses them at odd indices, while
retaining the positional information.

All statements in this section concern **raw integer coefficients before
carrying**. Their growth does not, by itself, settle convergence of a
carry-normalized display whose digits lie on several total-degree lines.

## 4. Families, fixed members, and restricted index sets

For any specified nonempty cutoff index set $I$, define a family to be a function
$I\to\mathcal A_b$. Pointwise operations make
$\mathcal A_b^I$ a commutative associative unital ring. Its constants satisfy
$1\ne0$. Equality means carry equality **at every index in $I$**.

For example, over $I=\mathbb Z_{\ge0}^2$, define

\[
L(N,M)=L_N,\qquad R(N,M)=R_M,\qquad F(N,M)=L_NR_M.
\]

An independently truncated additional factor enlarges the index set. Setting
$M=N$, or fixing one pair $(N,M)$, is an explicit specialization. Families
on different index sets can be compared after pulling them back to a stated
common index set. A finite member is the evaluation of a family at a specified
index; it is not the whole family.

The code's `equals()` checks the carry equality of finite members at supplied
cutoffs. It does not decide universally quantified equality of arbitrary
families. The latter claims here are proved algebraically. None of these
equalities is defined by a central viewing window or by removal of endpoints.

### Independent directional families have no universal polynomial relation

Suppose $P(s,t)\in\mathbb Q[s,t]$ vanishes on every independently indexed
pair $L_N,R_M$. Set $n=N+1,m=M+1$ and

\[
Q(U,V)=P\!\left(\frac{1-U}{b},\frac{1-V}{b}\right).
\]

Then $Q(q^n,q^{-m})=0$ in $\mathbb Q(q)$ for every $n,m\ge1$. Fix
$n$. Since $q$ is transcendental, the values $q^{-m}$ are infinitely
many distinct elements of this field. A polynomial in $V$ with all these
roots must be zero, so every coefficient of $Q(U,V)$, considered as a
polynomial in $V$, vanishes at $U=q^n$. Varying $n$ gives infinitely
many distinct roots for each of those coefficient polynomials. Therefore
$Q=0$, and the invertible affine substitution gives $P=0$.

Thus the integral algebra generated by these two independent families is a
polynomial ring in two generators. The same argument applies to any finite
list of independently varying directional cutoff indices. Derived products
still have their defining relations, such as $F=LR$. Specializing indices
can introduce additional relations; it does not contradict independence of
the unrestricted families.

## 5. Unique integral two-component form at matched cutoffs

Fix $N\ge0$, and write $L=L_N$, $R=R_N$, $F=LR$. Equation (4), after
carrying, gives

\[
bF=L+R,\qquad R=bF-L,\qquad L^2=bFL-F. \tag{8}
\]

Every integer polynomial in $L,R$ can therefore be written as a polynomial
in $F,L$. Repeated substitution of the monic relation
$L^2-bFL+F=0$ reduces it to

\[
\boxed{A(F)+B(F)L,\qquad A,B\in\mathbb Z[s].} \tag{9}
\]

Here $A(F)$ and $B(F)$ denote evaluation of ordinary integer-coefficient
polynomials in their formal argument $s$. No division is needed for this
reduction.

**Uniqueness.** Put $n=N+1$ and $t=q^n$. In the injective rational
extension,

\[
L=\frac{1-t}{b},\qquad R=\frac{1-t^{-1}}b,
\qquad F=\frac{2-t-t^{-1}}{b^2}.
\]

A nonzero polynomial in $F$ is nonzero: its highest power $F^j$ has a
nonzero coefficient of $t^j$, which lower powers cannot cancel. Axis exchange
$X\leftrightarrow Y$ sends $t\leftrightarrow t^{-1}$, interchanges $L,R$,
and fixes $F$. If $A(F)+B(F)L=0$, applying this exchange and subtracting
gives $B(F)(L-R)=0$. The ambient algebra is a domain and $L-R\ne0$, so
$B(F)=0$; the polynomial observation forces $B=0$. It then forces $A=0$.
This proves uniqueness over $\mathbb Z$.

Equivalently, the generated algebra has the exact presentation

\[
\mathbb Z[L,R]
\cong\mathbb Z[s,u]/(u^2-bsu+s),\qquad s\mapsto F,\quad u\mapsto L,
\]

and is a free rank-two module over $\mathbb Z[F]$, with basis $1,L$.
The same result holds for the diagonal family $N\mapsto(L_N,R_N)$: existence
is pointwise, and evaluating at any one $N\ge0$ proves uniqueness.

For $A,B,C,D\in\mathbb Z[F]$, (8) gives the closed multiplication rule

\[
\boxed{
(A+BL)(C+DL)
=(AC-BDF)+(AD+BC+bFBD)L.
} \tag{10}
\]

In particular, $FR=bF^2-FL$. The coefficients in this two-component form
are polynomials in $F$, not numerical constants.

### No finite basis over numerical constants

For any fixed, possibly unequal pair $N,M\ge0$, formula (3) gives

\[
F(N,M)=b^{-2}(1-q^{N+1}-q^{-M-1}+q^{N-M}).
\]

Its highest $q$-exponent is $N+1$, with coefficient $-b^{-2}$. Therefore
$F(N,M)^j$ has highest exponent $j(N+1)$, and no lower power has a term
there. The elements $1,F,F^2,\ldots$ are linearly independent over
$\mathbb Q$, and hence over $\mathbb Z$. A faithful algebra containing
this $F$ and all its powers cannot have finite dimension over those
numerical constants. This is consistent with the rank-two result over the
much larger coefficient ring $\mathbb Z[F]$.

## 6. Fixed unequal cutoffs and integral gcd compression

Here use positive term counts explicitly: $n=N+1,m=M+1$,
$\ell_n=L_{n-1}$, and $r_m=R_{m-1}$. Put

\[
g=\gcd(n,m),\qquad a=n/g,\qquad c=m/g,\qquad z=q^g.
\]

Then $a,c$ are positive and coprime. The fixed-pair generated algebras obey

\[
\boxed{\mathbb Z[\ell_n,r_m]=\mathbb Z[\ell_g,r_g].} \tag{11}
\]

**Proof.** In the left algebra, set $U=1-b\ell_n=z^a$ and
$V=1-br_m=z^{-c}$. Their relation $U^cV^a=1$ puts both inverses in this
algebra explicitly:

\[
U^{-1}=U^{c-1}V^a,\qquad V^{-1}=U^cV^{a-1}.
\]

Coprimality supplies integers $h,k$ with $ah-ck=1$. Adding a sufficiently
large multiple of $(c,a)$ makes both nonnegative without changing that
equation. Hence $z=U^hV^k$ and $z^{-1}$ belong to the left algebra.
For $s\ge0$, let $S_s(w)=1+w+\cdots+w^{s-1}$, with $S_0(w)=0$.
Finite telescoping proves

\[
\ell_g=\ell_nS_h(U)+U^hr_mS_k(V). \tag{12}
\]

Indeed, multiplying the right side by $b$ gives
$(1-U^h)+U^h(1-V^k)=1-z=b\ell_g$, and cancellation of $b$ is valid in
$\mathcal A_b$. Thus (12) uses only known integer-grid elements.
Also $r_g=-z^{-1}\ell_g$, so both right-side generators are in the left
algebra. Conversely,

\[
\ell_n=\ell_gS_a(z),\qquad r_m=r_gS_c(z^{-1}),
\qquad z=1-b\ell_g,\quad z^{-1}=1-br_g,
\]

puts the left-side generators in the right algebra. This proves (11).

Writing $G=\ell_gr_g=F(g-1,g-1)$, Section 5 now gives the unique integral
form $A(G)+B(G)\ell_g$. The anchor has changed from the original unequal
$F(N,M)$ to the matched $G$; this change is essential to the statement.

For later use, set $w=\ell_g$. The same algebra also has the presentation

\[
\mathbb Z[\ell_g,r_g]
=\mathbb Z[w,(1-bw)^{-1}], \tag{13}
\]

because $r_g=-w/(1-bw)$ and $(1-bw)^{-1}=1-br_g$. The element $w$ is
transcendental, so (13) is a localization of the ordinary polynomial ring,
not merely a surjective presentation.

## 7. Retaining the unequal fuzzy member as coefficient variable

Keep the positive counts and coprime $a,c$ from Section 6, but now retain

\[
F=F(N,M)=\ell_nr_m
\]

as the polynomial coefficient variable. After extending coefficients to
$\mathbb Q$, (11) gives

\[
\mathbb Q[\ell_n,r_m]=\mathbb Q[z,z^{-1}],
\qquad b^2F=(1-z^a)(1-z^{-c}).
\]

Multiplying the second identity by $z^c$ yields

\[
p_F(z)=z^{a+c}-z^a+(b^2F-1)z^c+1=0. \tag{14}
\]

It is monic of degree $a+c$ with constant term 1. Consequently both positive
and negative powers of $z$ reduce to $\mathbb Q[F]$-linear combinations
of $1,z,\ldots,z^{a+c-1}$: monicity reduces high positive powers; dividing
(14) by $z$ expresses $z^{-1}$ as a polynomial in nonnegative powers.

These $a+c$ elements are independent over $\mathbb Q[F]$. To prove this,
use an independent indeterminate $s$ and consider

\[
p_s(z)=(z^a-1)(z^c-1)+b^2sz^c.
\]

As a polynomial in $s$ over $\mathbb Q[z]$, its two coefficients are
relatively prime: the constant coefficient is 1 at $z=0$, while the other
is a nonzero constant times $z^c$. A factorization in $\mathbb Q[z,s]$
would have one factor independent of $s$, because its degree in $s$ is 1.
That factor would divide both coefficients and so be constant. Hence $p_s$
is irreducible in this polynomial ring. It is monic in $z$, and therefore
primitive over $\mathbb Q[s]$; clearing denominators in a factorization
over $\mathbb Q(s)$, and removing common polynomial factors from the
coefficients, would give a factorization in $\mathbb Q[s,z]$. Thus it is
also irreducible over $\mathbb Q(s)$.

The element $F$ is transcendental by Section 5, so replacing $s$ by $F$
identifies $\mathbb Q(s)$ with $\mathbb Q(F)$. Equation (14) is therefore
the minimal polynomial of $z$ over $\mathbb Q(F)$, of degree $a+c$.
It proves the claimed independence and the exact free-module rank

\[
\boxed{\operatorname{rank}_{\mathbb Q[F]}
\mathbb Q[\ell_n,r_m]=a+c=\frac{N+M+2}{\gcd(N+1,M+1)}.} \tag{15}
\]

The ranks for zero-based cutoff pairs $20/20,20/21,20/40$ are respectively
$2,43,62$. These concern fixed pairs and the specified unequal anchor; they
do not describe the independently indexed family of Section 4.

### Why the integer coefficient qualification matters

An analogous finite-module statement over $\mathbb Z[F]$ can fail. Suppose
a prime $p$ divides both $b$ and $ac$. By (13), reduction of the fixed
directional algebra modulo $p$ gives

\[
\mathbb Z[w,(1-bw)^{-1}]/(p)\cong\mathbb F_p[w].
\]

Here $z=1-bw$ reduces to 1. The finite geometric identities in Section 6
give

\[
\ell_n\equiv aw,\qquad r_m\equiv-cw,\qquad F\equiv-acw^2=0\pmod p.
\]

If the original directional algebra were a finitely generated module over
$\mathbb Z[F]$, reducing any finite set of module generators modulo $p$
would make $\mathbb F_p[w]$ a finite-dimensional vector space over the
image of $\mathbb Z[F]$, which here is just $\mathbb F_p$. This is
impossible because the powers of $w$ are independent. For example, at
$b=10,N=20,M=21$, the positive counts are $n=21,m=22$; $p=2$ supplies
this obstruction. It does not affect the matched integral theorem, where
$a=c=1$, or the integral gcd compression to the different anchor $G$.

## 8. Affine cutoff growth and endpoint resonance

The following are statements about finite Laurent expressions and their raw
coefficient windows. They are **not** the family equality or a value-limit
rule in $\mathcal A_b$.

Let the positive term counts grow as

\[
N_i(t)+1=\alpha_i t+u_i,\qquad
M_j(t)+1=\beta_j t+v_j,
\]

where $t$ is an integer tending to infinity, the slopes
$\alpha_i,\beta_j$ are positive integers, and the offsets are fixed integers.
All counts are positive once $t$ is large enough. Expanding (2), a pair of
selected subsets $I\subseteq\{1,\ldots,a\}$ and
$J\subseteq\{1,\ldots,c\}$ contributes

\[
(-1)^{|I|+|J|}
q^{\,t(\sum_{i\in I}\alpha_i-\sum_{j\in J}\beta_j)
+\sum_{i\in I}u_i-\sum_{j\in J}v_j}. \tag{16}
\]

There are finitely many subset pairs. If the coefficient of $t$ in an
exponent is nonzero, that monomial leaves any fixed finite exponent window.
Terms with zero coefficient of $t$ stay fixed. Thus the coefficientwise
residual of the endpoint expression is exactly

\[
\boxed{
\sum_{\sum_{i\in I}\alpha_i=\sum_{j\in J}\beta_j}
(-1)^{|I|+|J|}
q^{\sum_{i\in I}u_i-\sum_{j\in J}v_j}.
} \tag{17}
\]

This proves the subset-sum resonance rule, including possible cancellations.
The moving monomials still belong to every full finite endpoint expression.

### A common growth rate with fixed offsets

If all slopes equal 1, the surviving subsets have $|I|=|J|$. Their signs
are positive, so (17) becomes

\[
\sum_{r=0}^{\min(a,c)}\quad
\sum_{|I|=|J|=r}q^{\sum_{i\in I}u_i-\sum_{j\in J}v_j}. \tag{18}
\]

Counting with multiplicity, this has

\[
\sum_r\binom ar\binom cr=\binom{a+c}{a}
\]

terms. For a direct counting proof, choose $a$ elements from disjoint sets
of sizes $a,c$, grouping choices by the number $r$ taken from the second
set. Coincident exponents can combine the terms; unequal offsets need not
put them all at exponent zero.

For matched cutoffs $N_i=M_j=N$, set $n=N+1$ and $u=q^n$. The endpoint
expression is $(1-u)^a(1-u^{-1})^c$. Its constant coefficient is
$\binom{a+c}{a}$, since

\[
(1-u)^a(1-u^{-1})^c=(-1)^cu^{-c}(1-u)^{a+c}.
\]

In particular, the requested matched ledgers are

\[
\begin{aligned}
b^2F(N,N)&=2-u-u^{-1},\\
b^3F(N,N)L_N&=3-3u+u^2-u^{-1},\\
b^3F(N,N)R_N&=3-3u^{-1}+u^{-2}-u,\\
b^4F(N,N)^2&=6-4(u+u^{-1})+u^2+u^{-2}.
\end{aligned}
\]

The stationary coefficients $2,3,6$ are therefore forced finite endpoint
interactions. Discarding every remaining term is a separate operation and
has not been defined as preserving the represented value.

Unequal growth rates can instead produce negative resonant terms. For two
left counts $t,t$ and one right count $2t$, the ledger is
$(1-q^t)^2(1-q^{-2t})$. Its stationary coefficient is $1-1=0$, in contrast
to the coefficient 3 for three matched counts. The full finite product is
nonzero: its other endpoint terms remain. This example demonstrates why
matching depths, fixed depth differences, and different growth rates cannot
be silently treated as one infinite object.

## 9. What the construction proves

The finite family algebra retains integer grid operations, finite carry
equality, and all endpoint correlations. The raw-pattern theorem describes
every requested product; the endpoint identity certifies it independently;
and the matched and gcd results provide exact integral multiplication forms.
None of these constructions identifies $bL_N$ or $bR_M$ with 1 at finite
depth. Therefore the usual finite laws coexist with $1\ne0$ without deleting
the terms that obstruct those identifications.

A raw infinite array, a finite carry class, a family of such classes, and a
limit under an additional convergence rule are different objects. The proofs
do not select the last of these, establish bounded-digit normalization, or
turn a fixed-window residual into a scalar value. Those would require further
definitions and further theorems.
