# Vector encodings and circle identities with retained boundaries

This appendix proves the geometric identities used in this study. The
ingredients are the original finite integer grids, the prescribed carry,
and their exact endpoint identities. It assigns no scalar magnitude to a
position or to a complete MDN. All statements below hold at every positive
matched term count, and therefore also for the corresponding regular
boundary-retaining recipes.

The vector connection is an explicit encoding with declared operations.
The familiar polarization identities used here are not claimed as new.
Nor does this appendix establish that every real decimal input, or the
ordinary arithmetic of those inputs, has already been embedded faithfully.

## 1. Finite algebra, indexing, and endpoint identities

Work in the decimal carry algebra

\[
\mathcal A=\mathbb Z[X^{\pm1},Y^{\pm1}]/(X+Y-10).
\]

Here multiplication by `X` or `Y` shifts a grid by one lattice cell.
The relation records exactly the allowed carry. The embedding

\[
\mathcal A\cong
\mathbb Z[X,X^{-1},(10-X)^{-1}]\subset\mathbb Q(X)
\]

shows that this algebra is a nonzero integral domain. In particular,
nonzero factors do not multiply to zero. Rational coefficient bookkeeping
takes place in \(\mathcal A_{\mathbb Q}=\mathbb Q\otimes\mathcal A\).
It does not change a backend directional tenth into the ordinary scalar
\(1/10\).

Fix a **term count** \(n\ge1\), corresponding to GUI cascade depth
\(n-1\). Define

\[
q=-Y/X,\qquad E=q^n,
\]

\[
L=X^{-1}\sum_{j=0}^{n-1}q^j,
\qquad
R=Y^{-1}\sum_{j=0}^{n-1}q^{-j}.
\]

These are the matched finite X and Y tenth cascades. The finite geometric
sum and \(10=X+Y\) give

\[
\boxed{10L=1-E,\qquad10R=1-E^{-1}.}
\tag{1}
\]

The endpoint symbols are not zero. In fact \(EE^{-1}=1\).
Under the embedding above, \(q=1-10/X\) is transcendental over
\(\mathbb Q\), and so is \(E=q^n\). Consequently distinct powers of
\(E\) are linearly independent over \(\mathbb Q\).

Let \(\sigma\) exchange the axes. It is an algebra automorphism and
sends \(q\) to \(q^{-1}\), \(E\) to \(E^{-1}\), and \(L\) to
\(R\).

Multiplying (1) gives the useful exact identities

\[
\boxed{100LR=2-E-E^{-1}=10(L+R),\qquad10LR=L+R.}
\tag{2}
\]

The second equality follows from the first by cancellation of the
nonzero integer 10 in the domain. Thus it is also an identity in the
original integer-grid algebra, with no additional native division
operation assumed.

## 2. An injective vector encoding

**Proposition.** The three elements \(1,L,R\) are linearly independent
over \(\mathbb Q\).

**Proof.** If \(A+BL+CR=0\), multiply by 10 and apply (1):

\[
(10A+B+C)-BE-CE^{-1}=0.
\]

Independence of the three Laurent monomials forces \(B=C=0\), followed
by \(A=0\). \(\square\)

In particular,

\[
\mathcal C(a,b)=aL+bR
\tag{3}
\]

is an injective additive encoding of integer vectors into \(\mathcal A\).
It extends to rational vectors in \(\mathcal A_{\mathbb Q}\).
There is no claim that coordinatewise vector multiplication agrees with
native MDN multiplication. Integer coordinate inputs in (3) use only
native integer scalar multiplication, addition, and the two cascades.

To use directional units corresponding to coordinate size one, define

\[
U=10L=1-E,\qquad V=10R=1-E^{-1}.
\]

Then the alternative encoding is

\[
\mathcal D(x,y)=xU+yV=(x+y)-xE-yE^{-1}.
\tag{4}
\]

Its two coordinates can be decoded exactly: \(x\) is minus the
coefficient of \(E\), and \(y\) is minus the coefficient of
\(E^{-1}\). This describes a Laurent-polynomial observable; it does
not define an ordinary scalar magnitude for an MDN.

For native tenths \(x=a/10\), \(y=b/10\), expression (4) is exactly
\(aL+bR\). Arbitrary rational coefficients in (4) are legitimate in the
stated rational extension. They need not equal the backend's result of
freshly parsing a longer decimal, because different fractional places
have different boundary recipes. No complete real-input theorem follows
from this rational extension.

## 3. Directional units and loss of direction

The directional axis vectors are

\[
\mathcal D(1,0)=U=1-E,\qquad
\mathcal D(0,1)=V=1-E^{-1}.
\]

They are distinct, and neither equals the literal root integer 1.
Entering the integer 1 on either fraxis gives the same root integer in
the current input convention. Thus replacing both directional units by
that literal integer loses the axis distinction. It is a change of
encoding, not a consequence of a carry preserving the complete object.

This differs from the origin in polar coordinates: the two cardinal
vectors above are nonzero vectors with definite physical directions.
The proposed scalar-looking encoding identifies them. More generally,
when separately entered vector components send their integer parts to a
shared root, the result records their combined integer contribution and
can forget its allocation between axes.

There is a stronger algebraic obstruction. From (2),

\[
\boxed{UV=U+V.}
\tag{5}
\]

**Proposition.** No unital ring homomorphism from this algebra to a
nonzero ring can send both \(U\) and \(V\) to 1.

**Proof.** Applying such a map to (5) gives \(1=1+1\), hence
\(0=1\), contradicting the nonzero target. \(\square\)

Therefore a rule that makes both completed directional units equal to
the shared integer 1 cannot preserve the full mixed addition and
multiplication. Keeping the distinct endpoint terms avoids this
identification.

## 4. Circle identities: the constant is a retained-boundary carrier

Use the exact ordinary unit-circle point
\((s,t)=(3/5,4/5)=(0.6,0.8)\). With common cutoffs, its native tenth
entries are \(6L\) and \(8R\). The proposed crossed expression is

\[
(6L)(6R)+(8L)(8R)=100LR=2-E-E^{-1}\ne1.
\tag{6}
\]

The proposed same-direction squares give

\[
\begin{aligned}
(6L)^2+(8R)^2
&=\frac{36(1-E)^2+64(1-E^{-1})^2}{100}\\
&=1-\frac{18}{25}E-\frac{32}{25}E^{-1}
 +\frac9{25}E^2+\frac{16}{25}E^{-2}\ne1.
\end{aligned}
\tag{7}
\]

Both inequalities follow from independence of the endpoint monomials.
They are full finite identities, with no limiting approximation.

Equation (6) nevertheless has a useful generalization. For any rational
\(s,t\) satisfying \(s^2+t^2=1\), use the consistent directional
encoding in (4). Then

\[
\boxed{(sU)(sV)+(tU)(tV)=UV.}
\tag{8}
\]

**Proof.** Distributivity gives
\((s^2+t^2)UV=UV\). \(\square\)

Thus the crossed expression is independent of the chosen circle point,
but its constant is the nonzero boundary-retaining element
\(UV=2-E-E^{-1}\), rather than the root integer 1. “Carrier” here means
only this fixed MDN factor.

The algebraic identity also holds for formal scalar symbols satisfying
\(s^2+t^2=1\). Interpreting them as arbitrary real sine and cosine
values requires an explicitly chosen coefficient extension or a proved
infinite-input encoding; it is not justified by finite decimal parsing
alone.

## 5. Exact dot-product and determinant tests

Define a channel reflection on the encoded vector space by

\[
S\mathcal C(a,b)=aL-bR.
\]

Injectivity of (3) makes this a well-defined linear operation. The
geometric axis exchange \(\sigma\) remains the algebra automorphism
defined in Section 1.

**Proposition.** For integer coordinates, and also for rational
coordinates in the rational extension,

\[
\boxed{
\mathcal C(a,b)\sigma\mathcal C(c,d)
+S\mathcal C(a,b)\sigma S\mathcal C(c,d)
=2(ac+bd)LR.
}
\tag{9}
\]

Also,

\[
\boxed{
\mathcal C(a,b)\sigma\mathcal C(c,d)
-\sigma\mathcal C(a,b)\mathcal C(c,d)
=(ad-bc)(L^2-R^2).
}
\tag{10}
\]

**Proof.** The first product is

\[
(aL+bR)(cR+dL)=(ac+bd)LR+adL^2+bcR^2.
\]

The reflected-channel product in (9) is

\[
(aL-bR)(cR-dL)=(ac+bd)LR-adL^2-bcR^2.
\]

Adding them proves (9). The second product subtracted in (10) is

\[
(aR+bL)(cL+dR)=(ac+bd)LR+bcL^2+adR^2.
\]

Subtracting proves (10). \(\square\)

Both carrier factors are nonzero. The identities
\(L=(1-E)/10\) and \(R=(1-E^{-1})/10\) show \(L,R\ne0\), hence
\(LR\ne0\) in the domain. Moreover,

\[
100(L^2-R^2)=E^2-E^{-2}-2E+2E^{-1}\ne0.
\]

Consequently the left side of (9) vanishes **if and only if**
\(ac+bd=0\), exactly the ordinary orthogonality condition. The left
side of (10) vanishes **if and only if** \(ad-bc=0\), exactly the
condition that the two two-dimensional vectors are linearly dependent.
These are equality tests and require neither division by an MDN nor a
total order or global sign on MDNs.

Taking the same vector in both arguments of (9) gives

\[
\mathcal C(a,b)\sigma\mathcal C(a,b)
+S\mathcal C(a,b)\sigma S\mathcal C(a,b)
=2(a^2+b^2)LR.
\tag{11}
\]

Two encoded vectors therefore have equal paired self-products exactly
when they have equal ordinary squared lengths. In the normalized
\(U,V\) encoding, every rational unit-circle point has paired
self-product \(2UV\).

The determinant identity carries the signed rational determinant as a
coefficient of a fixed nonzero element. Reading its sign requires the
declared coordinate observable; this does not supply an intrinsic total
order for the whole MDN algebra.

For example, integer channel coordinates \((6,8)\) and \((-8,6)\)
make (9) zero at every matched cutoff. Their paired self-products agree,
and each equals \(200LR\).

## 6. Distributivity and rotations

Nothing in these constructions changes the distributive law:

\[
(aL+bR)(cL+dR)=acL^2+(ad+bc)LR+bdR^2.
\]

Cross terms do not disappear merely because the factors use different
fraxes. The paired products in (9) cancel terms by ordinary algebra.

The channel reflection \(S\) is not an algebra automorphism of the
whole MDN algebra. If a unital homomorphism extended
\(L\mapsto L,\ R\mapsto-R\), applying it to \(10LR=L+R\) would
give \(-10LR=L-R\). Together with the original identity this implies
\(2L=0\), which is impossible in the characteristic-zero domain.

A quarter-turn is well-defined on the vector subspace:

\[
J\mathcal C(a,b)=\mathcal C(-b,a),\qquad J^2=-I.
\tag{12}
\]

This is an exact linear complex structure on that two-dimensional
space. It is an operator on encoded vectors, rather than a declaration
that a position or a fraxis element is the imaginary unit.

It cannot extend to an algebra homomorphism with
\(L\mapsto R,\ R\mapsto-L\). Applying such a map to (2) gives
\(-10LR=R-L\), and the original identity then implies \(2R=0\),
again impossible. Nor can this quarter-turn be multiplication by a
fixed element of the present domain: if \(hL=R\) and \(hR=-L\),
commutativity gives \(R^2=-L^2\), contradicting the nonzero Laurent
expression for \(L^2+R^2\).

The dot-product test is nevertheless invariant under a common
quarter-turn of its two input vectors, because their scalar dot product
is unchanged. More generally, any declared rational orthogonal
transformation preserves (9) and (11). This is vector geometry realized
through the encoding and its observables, not a claim that native MDN
multiplication already is complex multiplication.

## 7. Boundaries, infinite recipes, and scope

Every proof above used the exact finite identities at a common term
count. Therefore the identities also hold stage by stage in the regular
unbounded recipes described in
[the boundary appendix](../fuzzy/BOUNDARY_PROOFS.md). At that level,
the independent endpoint symbol continues to retain its reciprocal and
its motion with depth. No step identifies a moving endpoint with zero.

The fixed carriers in (8)--(11) depend on the chosen matched-cutoff
recipe. Choosing independently growing X and Y cutoffs changes their
boundary expressions. Such a choice can be studied explicitly, but
cannot silently be substituted into these particular matched formulas.

This establishes concrete exact connections with vector algebra and
quadratic identities. It does not establish computational superiority,
a physical interpretation of a display change, or a novel replacement
for Euclidean or complex arithmetic. The experimental scripts in this
directory check the finite native backend separately; their generated
results state the tested depths and operation counts.
