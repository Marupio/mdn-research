# A formal completion for infinite fractional words

This note constructs one precise extension of the finite carry algebra and
the regular boundary families in
[BOUNDARY_PROOFS.md](../fuzzy/BOUNDARY_PROOFS.md). Infinite fractional words
are completed in their digit position, while the cascade count remains an
explicit variable. This is a choice of completion, not a claim that all
possible infinite grids have a unique value or that cascade depth has an
ordinary limit.

Throughout, the integer base is \(b\ge2\). A positive cascade **term count**
\(n\) corresponds to GUI depth \(n-1\); depth 20 therefore means 21 terms.
The construction uses rational coefficients for bookkeeping, but contains
the original integer carry algebra faithfully.

## 1. A formal coordinate for the carry algebra

The finite carry algebra is

\[
\mathcal A_b=
\mathbb Z[X^{\pm1},Y^{\pm1}]/(X+Y-b).
\]

Here \(X\) and \(Y\) are grid shifts. The relation \(X+Y=b\) records the
carry operation; it does not assign either shift a scalar magnitude.
Eliminating \(Y\) identifies this ring with the localization
\(\mathbb Z[X,X^{-1},(b-X)^{-1}]\), a subring of \(\mathbb Q(X)\).
In particular, it is nonzero and has no zero divisors.

Introduce a formal variable \(z\), and substitute

\[
X=z^{-1},\qquad Y=b-z^{-1},\qquad
q=-Y/X=1-bz,\qquad
Y^{-1}=h(z)=\frac{-z}{1-bz}.
\tag{1}
\]

Expansion of rational functions in \(X\) at \(X=\infty\) gives an
injective map into the field of formal Laurent series \(\mathbb Q((z))\).
Indeed, a nonzero rational function remains a nonzero rational function
after \(X=z^{-1}\), and its Laurent expansion has a nonzero leading term.
Thus (1) preserves every finite carry identity and every distinction between
finite carry classes. No numerical value of \(z\), \(X\), or \(Y\) is
selected.

The completion used below is

\[
\mathcal T=\mathbb Q[n][[z]].
\tag{2}
\]

Its coefficients are polynomials in the formal count variable \(n\).
It is complete in the \(z\)-adic topology: a sequence converges when each
coefficient eventually stabilizes. To include multiplication by arbitrary
finite grids, use

\[
\mathcal T[z^{-1}]=\mathbb Q[n]((z)),
\]

whose elements have a finite lower bound on their powers of \(z\).
This Laurent series ring is a ring, and is not asserted to be a field.

## 2. Exact coefficients of a finite cascade

For fractional place \(k\ge1\) and positive integer term count \(\nu\),
the raw X cascade is

\[
H^X_{k,\nu}
=X^{-k}\sum_{j=0}^{\nu-1}
\binom{k+j-1}{j}q^j.
\tag{3}
\]

These are finite integer grids before normalization by carries. For
\(m\ge k\), their exact coefficients after (1) are

\[
[z^m]H^X_{k,\nu}
=(-b)^{m-k}\binom{m-1}{k-1}
  \binom{\nu+k-1}{m}.
\tag{4}
\]

**Proof.** Put \(r=m-k\). Expanding \((1-bz)^j\) in (3) gives

\[
[z^m]H^X_{k,\nu}
=(-b)^r\sum_{j=r}^{\nu-1}
\binom{k+j-1}{j}\binom jr.
\]

The factorial identity

\[
\binom{k+j-1}{j}\binom jr
=\binom{k+r-1}{r}\binom{k+j-1}{j-r}
\]

and the finite summation identity

\[
\sum_{j=r}^{\nu-1}\binom{k+j-1}{j-r}
=\binom{k+\nu-1}{k+r}
\]

prove (4). The latter identity follows by repeatedly applying Pascal's
identity, starting with a sum containing one term. If \(r\ge\nu\), the
sum is empty and the binomial on the right is zero, so the formula remains
valid. Coefficients below degree \(k\) are zero. \(\square\)

For a symbolic upper argument, use the polynomial convention

\[
\binom{t}{m}=\frac{t(t-1)\cdots(t-m+1)}{m!},
\qquad \binom t0=1.
\]

Equation (4) therefore defines a series in \(\mathcal T\):

\[
\mathsf H^X_k(n,z)=
\sum_{m\ge k}(-b)^{m-k}\binom{m-1}{k-1}
\binom{n+k-1}{m}z^m.
\tag{5}
\]

Coefficientwise evaluation \(n\mapsto\nu\) is a ring homomorphism
\(\mathcal T\to\mathbb Q[[z]]\). For every positive integer \(\nu\),
(5) evaluates to exactly (3), a polynomial in \(z\) of degree at most
\(k+\nu-1\). Symbolic (5) is usually an infinite series even though each
such specialization is a polynomial.

## 3. Infinite words and arithmetic

Let \(d=(d_1,d_2,\ldots)\) be a base-\(b\) digit stream, with
\(0\le d_k\le b-1\). Define

\[
\begin{aligned}
D_X(d;n,z)
&=\sum_{k\ge1}d_k\mathsf H^X_k(n,z)\\
&=\sum_{m\ge1}\left[
\sum_{k=1}^m d_k(-b)^{m-k}\binom{m-1}{k-1}
\binom{n+k-1}{m}\right]z^m.
\end{aligned}
\tag{6}
\]

The inner sum is finite. Thus every coefficient is a polynomial in \(n\)
and depends only on the first \(m\) input digits. The prefix of length
\(K\) differs from (6) by an element of \(z^{K+1}\mathcal T\), proving
convergence of the prefixes in the chosen completion. The same construction
also works formally for arbitrary integer digit coefficients; bounded digits
are needed for the usual real-number interpretation, not for this local
finiteness argument.

Addition, subtraction, and multiplication are the ring operations. At
degree \(m\), a product coefficient is the finite sum
\(\sum_{i=0}^m a_i(n)b_{m-i}(n)\). The Laurent extension has the same
property because each factor has a finite lower degree bound. Consequently
mixed-axis products and repeated products are defined without summing an
uncontrolled infinity of contributions at a coefficient. Associativity and
distributivity follow from the corresponding finite coefficient sums.

The ring is a domain: the product of the leading nonzero coefficients of two
nonzero series is nonzero in \(\mathbb Q[n]\). In particular,
\(1\ne0\). This statement concerns the chosen formal completion; it does
not assert that arbitrary raw infinite-grid convolution is defined.

At each fixed positive integer \(\nu\), evaluating (6) gives the
coefficientwise limit of the exact finite cascades of the digit prefixes at
that same count \(\nu\). Evaluation commutes with this limit because each
coefficient already stabilizes after a finite prefix. It does not exchange
the digit-prefix limit with a limit in the cascade count.

## 4. Both axes survive, and swapping them is an involution

Since \(h(z)=-z+O(z^2)\), substitution into a formal power series is
well-defined: a coefficient at any degree uses only finitely many terms.
Direct computation gives

\[
h(h(z))=z,\qquad
1-bh(z)=(1-bz)^{-1}.
\tag{7}
\]

Hence

\[
\tau(P(n,z))=P(n,h(z))
\]

is an involutive ring automorphism of \(\mathcal T\), fixing \(n\).
It extends to \(\mathbb Q[n]((z))\), since \(h\) has order one and an
invertible leading coefficient. Under (1), it exchanges \(X\) and \(Y\).
Define

\[
\mathsf H^Y_k=\tau(\mathsf H^X_k),\qquad
D_Y(d)=\tau(D_X(d)).
\tag{8}
\]

At every finite count these specialize to exactly the transposed raw grids.
For example, the first-place X template has leading term \(nz\), whereas
its Y transpose has leading term \(-nz\). They are distinct. Choosing the
coordinate \(z=X^{-1}\) therefore has not identified the axes.

## 5. A faithful realization as sequences with the count retained

Let two sequences in \(\mathbb Q((z))\) be equivalent if they agree at
every sufficiently large positive integer index. Their equivalence classes
form a ring under pointwise operations. There is a homomorphism

\[
\Psi:\mathbb Q[n]((z))\longrightarrow
\mathbb Q((z))^{\mathbb N_{>0}}/
\{\text{eventually zero sequences}\},
\qquad
P\longmapsto[(P(\nu,z))_{\nu\ge1}].
\tag{9}
\]

**This map is injective.** If \(P\ne0\), choose its least nonzero
coefficient \(p(n)\). A nonzero polynomial has only finitely many integer
roots. Thus \(P(\nu,z)\ne0\) for every sufficiently large \(\nu\), and
its sequence is not eventually zero.

The target uses completed Laurent series at each index. An infinite-word
state need not be an element of the finite-grid algebra at each index.
Equation (9) therefore does not identify a completed infinite word with a
finite grid. It verifies that retaining symbolic \(n\) faithfully retains
the eventual sequence represented by that word.

Fixed offsets and rates can be specified by substituting \(n\mapsto an+c\)
in (5), provided the resulting count is eventually positive. Such choices
are data of the state, rather than changes silently imposed by the
completion. Independent counts can instead be kept as separate coefficient
variables in \(\mathbb Q[n_X,n_Y][[z]]\); a common-index path is a further
specialization.

## 6. The earlier endpoint algebra embeds faithfully

The previous regular boundary algebra was

\[
\mathcal S=\mathbb Q[n,q^{\pm1},E^{\pm1}],
\]

where the stage indexed by \(\nu\) sends \(E\) to \(q^\nu\).
There is a homomorphism \(\mathcal S\to\mathcal T\) given by

\[
q\longmapsto1-bz,\qquad
E\longmapsto(1-bz)^n
=\sum_{r\ge0}\binom nr(-bz)^r.
\tag{10}
\]

Both images are units. In particular,

\[
E^{-1}\longmapsto(1-bz)^{-n}
=\sum_{r\ge0}\binom{-n}{r}(-bz)^r.
\tag{11}
\]

To verify that (10) and (11) multiply to 1, evaluate any proposed product
coefficient at each positive integer \(n=\nu\). It is the corresponding
coefficient of \((1-bz)^\nu(1-bz)^{-\nu}=1\). Each coefficient before
evaluation is a polynomial in \(n\); equality at infinitely many integers
proves the polynomial identity.

The homomorphism is **injective**. Write a nonzero element of \(\mathcal S\)
as a finite collected sum

\[
P=\sum_{(i,j)}p_{ij}(n)q^iE^j.
\]

After \(n=\nu\), it becomes
\(\sum p_{ij}(\nu)q^{i+j\nu}\). Distinct pairs have colliding exponents
at at most finitely many indices, and each nonzero coefficient polynomial
has finitely many roots. Past these exceptions the result is a nonzero
Laurent polynomial in \(q\). Its substitution \(q=1-bz\) is nonzero.
Thus the image of \(P\) cannot be zero, proving injectivity. Localizing at
\(1-q\) preserves injectivity and places the full rational finite carry
algebra in the Laurent extension, since \(1-q=bz\).

In particular, the completion does not discard an old endpoint or its
inverse. The correlation \(EE^{-1}=1\) remains exact, and swapping axes
sends \(E\) to \(E^{-1}\). For the first fractional place,

\[
\mathsf H^X_1=\frac{1-E}{b},\qquad
\mathsf H^Y_1=\frac{1-E^{-1}}{b}.
\tag{12}
\]

Neither endpoint is set equal to zero. Declaring both corresponding
fractions to be \(1/b\) would contradict this retained unit relation.

## 7. What it means to encode every ordinary real

For \(r\in\mathbb R\), write its signed magnitude as
\(r=\sigma(I+f)\), where \(\sigma\in\{-1,0,1\}\),
\(I=\lfloor|r|\rfloor\), and \(0\le f<1\). Choose the canonical
base-\(b\) expansion \(f=\sum_{k\ge1}d_kb^{-k}\): terminating expansions
use trailing zeros, and expansions eventually consisting entirely of
\(b-1\) are excluded. Define

\[
\iota_X(r)=\sigma\bigl(I+D_X(d)\bigr),\qquad
\iota_Y(r)=\sigma\bigl(I+D_Y(d)\bigr).
\tag{13}
\]

Each map is injective. The constant coefficient first records the signed
integer part. If two remaining signed streams first differ at place \(k\),
their difference has leading coefficient

\[
\delta d_k\binom{n+k-1}{k}\ne0.
\]

This proves injectivity for X; the involution proves it for Y. Therefore
every canonical real has a distinct state within each direction, including
irrational inputs. Specifying an arbitrary real may require an infinite
digit stream; this existence statement does not give a finite algorithm for
every real.

These are **set embeddings, not embeddings of the real field**. For
example, in base 10,

\[
\iota_X(0.9)+\iota_X(0.1)=1-E\ne1=\iota_X(1).
\tag{14}
\]

Ordinary addition of the real labels can cross a digit carry, while (14)
retains the associated endpoint. The signed-magnitude convention also
matters: negating a fractional state is not silently replaced by a different
integer-plus-positive-fraction label.

A repeating word makes the same distinction explicit. If every digit is
\(a\), then

\[
D_X(a,a,\ldots)
=\frac{a}{b-1}\left[
1-\left(\frac{1-bz}{1-z}\right)^n\right].
\tag{15}
\]

The power of a unit series \(U(z)=1+O(z)\) means
\(U^n=\sum_{r\ge0}\binom nr(U-1)^r\), a well-defined member of
\(\mathcal T\). To prove (15), fix \(n=\nu\) and use

\[
\sum_{k\ge1}\binom{k+j-1}{j}z^k
=\frac{z}{(1-z)^{j+1}}.
\]

Summing the resulting finite geometric series over \(0\le j<\nu\)
gives (15). Both sides have polynomial coefficients in \(n\), so equality
at all positive integers proves the symbolic identity. In particular, an
all-\((b-1)\) word gives

\[
1-\left(\frac{1-bz}{1-z}\right)^n\ne1.
\]

The endpoint subtracted here is a unit with constant coefficient 1. Choosing
canonical real digits excludes this alternate spelling of the real number
1; it does not impose its equality to the integer state. In base 10, the
infinite word \(0.333\ldots\) has state
\(\frac13[1-((1-10z)/(1-z))^n]\), which is a concrete infinite-input
example with its boundary retained.

## 8. Two indices, and the exact scope of the completion

The decimal-prefix length \(K\) and the cascade count \(n\) play different
roles. Here \(K\to\infty\) is an actual formal limit: each fixed
\(z\)-coefficient stabilizes. The count \(n\) remains polynomial data, or
equivalently the sequence germ in (9). No ordinary \(n\to\infty\) limit
is asserted. In particular, the nonconstant coefficients of
\((1-bz)^n\) have not been replaced by limiting scalar values.

This completion cannot faithfully preserve **all** raw double-index germs
under eventual equality in \((K,n)\). For example, the family

\[
A_{K,n}=H^X_{K+1,n}
\]

is nonzero at every finite pair of positive indices, since its leading
coefficient is \(\binom{n+K}{K+1}\). It therefore represents a nonzero
double-index germ. But its order in \(z\) is \(K+1\), so its
decimal-prefix completion is zero. This explicitly illustrates information
the chosen completion forgets.

What has been proved is narrower and useful: infinite digit words admit
well-defined states and products; every finite carry class and every
previous regular endpoint family embeds faithfully; and the growing
cascade boundary is retained. What has not been imposed is ordinary real
arithmetic on the input labels, a common scalar magnitude for both axes, or
a universal interpretation of every possible infinite array.
