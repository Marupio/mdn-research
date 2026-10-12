# Analytic completion of infinite digit streams

This construction takes an actual infinite-digit limit at each fixed cascade
depth, then retains the complete sequence as the cascade depth grows. It
extends the finite-word construction in
[BOUNDARY_PROOFS.md](../../research/fuzzy/BOUNDARY_PROOFS.md). Its convergence claims are
proved for stated domains and a stated order of operations; it does not
identify all infinite grids or claim a unique compulsory MDN completion.

Let \(b\ge2\) be an integer base. The complex variable \(X\) and the function
\(Y=b-X\) retain the carry relation. Put \(q=-Y/X\). Functions of \(X\) are
used as complete objects; no one scalar weight is selected for either axis.
The positive integer \(n\) is the **number of cascade terms**. The associated
zero-based GUI depth is \(n-1\), so depth 20 means \(n=21\).

## 1. The fixed-depth digit limit

Let \(d=(d_k)_{k\ge1}\) be a bounded sequence of coefficients, with
\(|d_k|\le M\). A usual fractional base-\(b\) word has
\(d_k\in\{0,\ldots,b-1\}\), for which \(M=b-1\). Bounded signed or complex
coefficient sequences are useful when comparing sums and differences.
Define the digit generating function

\[
D(z)=\sum_{k=1}^{\infty}d_kz^k,\qquad |z|<1.
\]

It is holomorphic on the unit disk and has \(D(0)=0\).
At fixed \(n\), the app's fractional-place template is the finite raw grid

\[
H^X_{k,n}
=X^{-k}\sum_{j=0}^{n-1}
\binom{k+j-1}{j}q^j.
\tag{1}
\]

This is the root term plus \(n-1\) additional cascade terms. Its raw
coefficient at \((-k-j,j)\) is
\((-1)^j\binom{k+j-1}{j}\). The corrected
[fractional cascade implementation](../../library/src/Mdn2d.cpp) uses this
binomial recurrence before carrying; additional precision clipping is not
part of (1). The Y template is the transpose under \(X\leftrightarrow Y\).

For \(K\) entered fractional places, set

\[
f^X_{n,K}[D](X)=\sum_{k=1}^{K}d_kH^X_{k,n},
\qquad
f^Y_{n,K}[D](X)=\sum_{k=1}^{K}d_kH^Y_{k,n}.
\]

Use the domains

\[
U_X=\{X\in\mathbb C:|X|>1\},\qquad
U_Y=\{X\in\mathbb C:|b-X|>1\},\qquad
\Omega_b=U_X\cap U_Y.
\]

The common domain is nonempty for every \(b\ge2\): points with sufficiently
large imaginary part belong to it. When \(b>2\), it also contains the real
interval \(1<X<b-1\).

### Convergence theorem

For every fixed positive \(n\), the limits

\[
f^X_n[D]=\lim_{K\to\infty}f^X_{n,K}[D],
\qquad
f^Y_n[D]=\lim_{K\to\infty}f^Y_{n,K}[D]
\tag{2}
\]

exist uniformly on compact subsets of \(U_X\) and \(U_Y\), respectively, and
define holomorphic functions there.

**Proof.** For a compact \(C\subset U_X\), let
\(\rho=\max_C|X|^{-1}<1\) and \(Q=\max_C|q|<\infty\).
The omitted X tail satisfies

\[
\sup_{X\in C}|f^X_n[D](X)-f^X_{n,K}[D](X)|
\le
M\sum_{j=0}^{n-1}Q^j
\sum_{k>K}\binom{k+j-1}{j}\rho^k.
\tag{3}
\]

For each fixed \(j\), the binomial coefficient is a polynomial in \(k\)
of degree \(j\). Multiplication by \(\rho^k\) makes its series summable:
the ratio of successive nonzero terms tends to \(\rho<1\).
There are only \(n\) values of \(j\), so the right side tends to zero.
The same estimates after differentiation add only polynomial factors in
\(k\) and bounded factors on each compact subset. Thus the differentiated
series converge there as well, proving holomorphy and the validity of the
calculus formula below. Interchanging \(X,Y\) proves the Y statement.

The hypothesis that \(n\) is fixed matters. This argument does not replace
both limits \(K\to\infty\) and \(n\to\infty\) by an unspecified joint limit.

### Exact positive tail certificate

For \(K\ge1\), \(j\ge0\), and \(0<\rho<1\), the inner tail in (3) has the
finite expression

\[
\boxed{
\sum_{k>K}\binom{k+j-1}{j}\rho^k
=\frac{\rho^{K+1}}{(1-\rho)^{j+1}}
\sum_{a=0}^{j}\binom{K+a-1}{a}(1-\rho)^a.
}
\tag{4}
\]

To verify it, write \(k=m+1\) and use

\[
\sum_{m=0}^{\infty}\binom{m+j}{j}\rho^m
=(1-\rho)^{-j-1}.
\]

This generating identity follows by multiplying \(j+1\) geometric series:
the coefficient counts nonnegative \((j+1)\)-tuples of a given sum.
Subtract the first \(K\) terms, and apply the finite endpoint formula for
fractional order \(j+1\) and term count \(K\), proved in the boundary
appendix. Multiplication by \(\rho\) gives (4).

For decimal digits and the whole interval \(I=[4,6]\), both directions
admit \(\rho=1/4\), \(Q=3/2\), and \(M=9\). Therefore

\[
\boxed{
B(K,n)=
9\sum_{j=0}^{n-1}\left(\frac32\right)^j
\frac{(1/4)^{K+1}}{(3/4)^{j+1}}
\sum_{a=0}^{j}\binom{K+a-1}{a}(3/4)^a
}
\tag{5}
\]

is an exact rational bound for the omitted digit tail, uniformly over every
\(X\in I\). It is not a conclusion from sampling a few points.

## 2. A finite differential formula for every infinite word

Let \(\theta=z\,d/dz\), and define

\[
\mathcal B_0(\theta)=1,\qquad
\mathcal B_j(\theta)=
\frac{\theta(\theta+1)\cdots(\theta+j-1)}{j!}
\quad(j\ge1).
\]

Since
\(\mathcal B_j(\theta)z^k=\binom{k+j-1}{j}z^k\), formula (2) becomes

\[
\boxed{
f^X_n[D](X)=
\sum_{j=0}^{n-1}q(X)^j
\bigl(\mathcal B_j(\theta)D\bigr)(1/X).
}
\tag{6}
\]

In ordinary derivatives, the same operator is

\[
\mathcal B_j(\theta)D(z)
=\frac{z}{j!}\frac{d^j}{dz^j}\bigl(z^{j-1}D(z)\bigr).
\tag{7}
\]

For \(j=0\), the right side is \(D(z)\); \(D(z)/z\) is analytic at zero.
For \(j\ge1\), applying the derivative to each monomial proves (7).
The estimates in Section 1 justify the termwise operations on \(D\).
The Y formula replaces \(1/X\) by \(1/Y\) and \(q\) by \(q^{-1}\).

Thus an infinite word has a finite differential recipe at each specified
cascade count. This need not be a finite polynomial in the older symbols
\(n,q,E\), where \(E=q^n\).

## 3. Finite arithmetic preserves the chosen digit limit

Use locally uniform convergence on \(\Omega_b\).
If \(f_K\to f\) and \(g_K\to g\), addition and subtraction pass to the
limit directly. On any compact \(C\subset\Omega_b\),

\[
\|f_Kg_K-fg\|_C
\le
\|f_K\|_C\|g_K-g\|_C
+\|g\|_C\|f_K-f\|_C
\longrightarrow0.
\tag{8}
\]

The first factor is bounded because \(f_K\) converges uniformly on \(C\).
This proves product compatibility. Independent truncation indices also work:
the same estimate uses \(f_Kg_L\) and lets \(K,L\) grow independently.
By induction, every finite expression formed using addition, subtraction,
and multiplication has the corresponding analytic digit limit.

The resulting functions form a subalgebra of the holomorphic functions on
\(\Omega_b\). It need not consist only of freshly entered digit templates:
multiplying two complete templates is not a claim that their product equals
the template of their ordinary scalar product.

Finite carry equality is respected faithfully. Indeed,

\[
\mathcal A_b
=\mathbb Z[X^{\pm1},Y^{\pm1}]/(X+Y-b)
\hookrightarrow\mathbb Q(X)
\hookrightarrow\mathcal O(\Omega_b)
\]

is injective. A nonzero rational function in the middle cannot vanish
identically on a nonempty open set: after a denominator is cleared, that
would give a nonzero polynomial infinitely many roots. This is a function
model of the whole finite carry class, not an evaluation at one chosen
scalar pair.

The ambient analytic algebra allows complex constants. The integer backend
objects and their specified digit limits occupy a particular subalgebra
inside it; using the ambient algebra does not assert that every arbitrary
complex coefficient is a backend digit operation.

## 4. Repeated and periodic words

### One repeated digit

For the word with every digit equal to \(a\),

\[
D(z)=\frac{az}{1-z},\qquad
\mathcal B_j(\theta)D(z)=\frac{az}{(1-z)^{j+1}}.
\]

The latter identity follows by the same geometric generating series used
in Section 1. Summing the finite geometric row in (6) gives

\[
\boxed{
f^X_n[a\,a\,a\ldots](X)
=\frac{a}{b-1}
\left[1-\left(\frac{-Y}{X-1}\right)^n\right].
}
\tag{9}
\]

For example, with \(z=1/X\),
\(1-z-q=(b-1)z\); this cancels the prefactor in the finite sum. There is no
singularity at \(X=1\) inside \(U_X\), since that point is excluded.
Transposition gives

\[
f^Y_n[a\,a\,a\ldots](X)
=\frac{a}{b-1}
\left[1-\left(\frac{-X}{Y-1}\right)^n\right].
\tag{10}
\]

These are exact fixed-\(n\) limits of the finite digit inputs. Their new
moving endpoints depend on \(X-1\) or \(Y-1\), not just on \(X\) and \(Y\).
For instance,

\[
\left(\frac{-Y}{X-1}\right)^n
=\left(\frac{bq}{b-1+q}\right)^n.
\]

Consequently infinite words genuinely enlarge the regular finite-word model;
the earlier finite normal form in \(n,q,E\) is not asserted for them.

### Any finite repeating block

For a period \(a_1,\ldots,a_p\),

\[
\boxed{
D(z)=\frac{a_1z+a_2z^2+\cdots+a_pz^p}{1-z^p}.
}
\tag{11}
\]

Expand \(1/(1-z^p)=\sum_{r\ge0}z^{rp}\) to verify every digit position.
A finite prefix \(c_1,\ldots,c_s\) followed by this period gives

\[
D(z)=\sum_{k=1}^{s}c_kz^k
+z^s\frac{a_1z+\cdots+a_pz^p}{1-z^p}.
\tag{12}
\]

Substitution into (6) is an exact finite derivative formula at every \(n\).
In particular, periodic words give rational functions of \(X\), with no
numerical summation ambiguity on the stated domains.

## 5. Single-channel and additive two-channel injectivity

### A single word is recoverable at every fixed count

Write (6) in the variable \(z=1/X\):

\[
G_n[D](z)=
\sum_{j=0}^{n-1}(1-bz)^j\mathcal B_j(\theta)D(z).
\]

If \(D\ne0\), let \(k\) be the earliest nonzero coefficient. The coefficient
of \(z^k\) in \(G_n[D]\) is

\[
d_k\sum_{j=0}^{n-1}\binom{k+j-1}{j}
=d_k\binom{k+n-1}{n-1}\ne0.
\tag{13}
\]

The sum identity follows by induction from Pascal's rule. Thus the transform
is triangular with nonzero diagonal and is injective on bounded coefficient
streams. A nonzero \(G_n[D]\) cannot vanish identically on \(\Omega_b\):
that domain includes all sufficiently large \(X\), whose reciprocals include
a punctured neighborhood of \(z=0\).

Also \(G_n[D](0)=0\). Therefore every X-channel function tends to zero as
\(X\to\infty\), and every Y-channel function does too. Their leading terms
are \(nd_1/X\) and \(nd_1/Y\), respectively, when \(d_1\ne0\).

### Unique constant, X, and Y additive parts for \(b>2\)

Fix specified positive counts \(n_X,n_Y\), not necessarily equal. Suppose

\[
A+f^X_{n_X}[D](X)+f^Y_{n_Y}[J](X)=0
\quad\text{on }\Omega_b,
\tag{14}
\]

where \(A\) is constant and both coefficient streams are bounded.
If \(b>2\), then

\[
\boxed{A=0,\qquad D=0,\qquad J=0.}
\tag{15}
\]

**Proof.** Let \(X\to\infty\) in (14) to obtain \(A=0\).
The remaining functions \(f_X\) and \(-f_Y\) agree on their overlap.
The two closed unit disks centered at 0 and \(b\) are disjoint when \(b>2\).
Consequently \(U_X\cup U_Y=\mathbb C\), and these functions patch to one
entire function \(h\). It tends to zero at infinity. Continuity on a closed
bounded disk then makes it globally bounded, say by \(M_0\).

For any \(z_0\) and any circle of radius \(R\) about \(z_0\), the
circle-integral derivative formula gives

\[
|h'(z_0)|\le \frac{M_0}{R}.
\]

Letting \(R\to\infty\) forces \(h'(z_0)=0\) everywhere. Thus \(h\) is
constant, and its limit at infinity makes it zero. This is the usual
Cauchy-estimate proof for bounded entire functions; the estimate and its
short proof are also given in
[MIT's complex-analysis notes, Sections 4.6.2–4.6.3](https://ocw.mit.edu/courses/18-04-complex-variables-with-applications-spring-2018/b1a90df19f643b974555bbbb93138a48_MIT18_04S18_topic4.pdf).
Finally, (13) gives \(D=J=0\).

Applying this argument to the difference of two proposed decompositions
proves uniqueness of a constant plus an X word plus a Y word, for the
specified counts. The same conclusion holds for eventual cascade germs
with the same specified count paths: equality of germs gives equality
at any sufficiently large stage, where (15) applies.

This theorem supports distinct additive fractional channels. It does not
claim that a product of channels is necessarily another pair of bounded
digit templates, or that the cutoff labels themselves can be reconstructed
when different count paths are allowed on the two sides of an equality.

The condition \(b>2\) is essential for this result. At \(b=2,n_X=n_Y=1\),
the all-1 words give

\[
f_X=\frac1{X-1},\qquad
f_Y=\frac1{Y-1}=-\frac1{X-1}.
\]

They are nonzero channels whose sum is zero. The analytic digit-limit
construction still works for base 2 on \(\Omega_2\), but the separated-domain
injectivity theorem does not.

## 6. Ordinary real content is a separate interpretation

### Exact one-channel evaluations

An X-channel function is defined at \(X=b\), which lies in \(U_X\).
There \(q=0\), so (6) gives, for every fixed \(n\),

\[
\boxed{f^X_n[D](b)=D(1/b)=\sum_{k\ge1}d_kb^{-k}.}
\tag{16}
\]

The Y-channel evaluation at \(X=0\), where \(Y=b\), gives the same ordinary
value. These are legitimate evaluations on their respective one-channel
analytic algebras. They are not a common evaluation of the mixed algebra:
the opposite channel need not be defined at the chosen point.
In particular, setting \(Y=0\) is not a permitted evaluation of the original
full Laurent-grid algebra containing \(Y^{-1}\).

### An optional one-sided cascade limit, with an explicit bound

There is also a sufficient analytic condition for the ordinary
\(n\to\infty\) limit of one channel. If

\[
|X|-|Y|>1,
\]

put \(r=1/|X|\) and \(t=|Y|/|X|\). Then \(r+t<1\), and

\[
\sum_{k\ge1}\sum_{j\ge0}
|d_k|\binom{k+j-1}{j}r^kt^j
\le \frac{Mr}{1-r-t}<\infty.
\]

To see the bound, sum over \(k\) first:
\(\sum_{k\ge1}\binom{k+j-1}{j}r^k=r/(1-r)^{j+1}\), then sum the
geometric row in \(j\). Absolute convergence permits the reverse order.
Summing the binomial series in \(j\) gives
\(X^{-k}(1-q)^{-k}=b^{-k}\), so the total is \(D(1/b)\).
The same positive majorant for \(j\ge n\) proves

\[
\boxed{
|f^X_n[D](X)-D(1/b)|
\le
\frac{M}{|X|-|Y|-1}
\left(\frac{|Y|}{|X|-1}\right)^n.
}
\tag{17}
\]

On a compact subset of this open chart, the denominator is bounded below
by a positive number and the ratio is bounded above by a number less than
1. Hence this convergence is locally uniform there.

For Y the sufficient condition is \(|Y|-|X|>1\), with the transposed
bound. The two guaranteed charts cannot overlap. Particular words may
converge on larger sets; (17) is a uniform sufficient result for all bounded
digit streams, not a classification of every possible convergence domain.
These limits are additional interpretations of the retained recipes,
not the equality relation imposed on their full mixed germs.

### The repeating-9 issue

Taking \(a=b-1\) in (9) gives

\[
\boxed{
f^X_n[(b-1)(b-1)(b-1)\ldots]
=1-\left(\frac{-Y}{X-1}\right)^n\ne1.
}
\tag{18}
\]

This remains different from 1 at every stage and as an eventual germ.
For decimal input, the ordinary identity \(0.999\ldots=1\) therefore does
not identify the two retained construction recipes.

There is a stronger obstruction to imposing that identification in this
mixed extension. The retained difference in (18) is a unit; its inverse is

\[
\left(\frac{-(X-1)}Y\right)^n.
\]

At every finite \(n\), this inverse is an integer Laurent grid, already in
\(\mathcal A_b\). Thus identifying the difference with zero while faithfully
retaining those finite-grid germs would identify a unit with zero and force
\(1=0\). Separate one-channel interpretations such as (16) do not contain
that inverse at the evaluation point and do not incur this contradiction.

Choosing the canonical base-\(b\) expansion, excluding eventually-\((b-1)\)
fractional tails, does give every ordinary real number an injective encoding
as an integer plus one fractional word. The integer is recovered as the
limit at \(X=\infty\), and (13) recovers the fractional word. This is an
encoding of the set of reals, not a field embedding that preserves ordinary
decimal carries. Two genuine shared-unit real-field embeddings would agree
on all rational numbers by uniqueness of inverses, so they could not provide
distinct rational tenths.

## 7. Retain the cascade recipe after completing the digits

After constructing each analytic \(f_n\), form its eventual class in

\[
\mathcal G_{\mathrm{an}}
=\mathcal O(\Omega_b)^{\mathbb N_{>0}}/
\{\text{eventually zero sequences of functions}\}.
\]

Equality means equality as complete holomorphic functions at every
sufficiently large index. Pointwise addition and multiplication descend to
this quotient. Constant sequences embed, and \(1\ne0\).
The quotient need not be a domain: the even-index and odd-index indicator
sequences again give nonzero classes with zero product.

The faithful finite carry map in Section 3 induces an injection

\[
\mathcal A_b^{\mathbb N_{>0}}/
\{\text{eventually zero finite classes}\}
\hookrightarrow \mathcal G_{\mathrm{an}}.
\tag{19}
\]

Indeed, if a finite-grid sequence maps to zero eventually, injectivity
at each stage makes the original finite classes zero eventually.
In particular, the existing finite-word boundary germs are preserved.

Equation (19) does **not** say that the entire two-index
\((K,n)\) net-germ algebra embeds faithfully after taking the chosen
\(K\)-completion. For example, the finite grids \(X^{-K}\) are nonzero
for every \(K,n\), but converge to zero uniformly on compact subsets of
\(\Omega_b\) as \(K\to\infty\). The completion deliberately regards that
convergent digit-direction limit as zero. This is consistent: it does not
also make the inverse sequence \(X^K\) converge there.

Likewise, a diagonal finite truncation \(K=K(n)\) is generally not exactly
the same eventual germ as the analytically completed infinite word, even
if (5) makes its error extremely small. Such a truncation is a certified
approximation under a chosen observation norm. The exact object uses the
stated order: complete the digits at fixed \(n\), then retain the unending
cascade recipe without deleting its endpoints.

The construction therefore admits every bounded infinite digit stream,
preserves the previously defined finite boundary objects, and supports
exact mixed addition and multiplication in an analytic function algebra.
It does not establish a raw cell-by-cell infinite-grid limit, a universal
finite symbolic normal form for all streams, or an operation-preserving
identification of both directional channels with ordinary real scalars.
