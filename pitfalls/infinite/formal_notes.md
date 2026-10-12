# Directional expansions and formal fuzzy patterns

These notes distinguish **finite MDN values**, which obey `X + Y = b`, from **raw formal grids**, whose two coordinate shifts are independent. Confusing those two settings is the source of several apparent paradoxes. The results below apply established formal-series and finite-difference mathematics to the MDN notation; they are not claims of new theorems in those subjects.

Throughout, coefficients lie in a characteristic-zero field `K`, such as the rational or real numbers, and `b` is nonzero. This is a deliberate coefficient extension of the original integer-digit system. Bounds imposed on individual digits need separate consideration.

## 1. A consistent directional completion

The finite MDN algebra is

\[
A_b=K[X^{\pm1},Y^{\pm1}]/(X+Y-b).
\]

Set `q = -Y/X`. The carry relation gives

\[
X=\frac{b}{1-q},\qquad Y=\frac{-bq}{1-q}.
\]

Consequently there is an exact algebra isomorphism

\[
A_b\cong K[q,q^{-1},(1-q)^{-1}].
\]

The maps in both directions follow from the displayed formulas, so no additional algebraic identification is being assumed.

This algebra embeds in the formal Laurent-series field `K((q))`: series may have infinitely many positive powers but only finitely many negative powers. A series tends to zero in its valuation topology when its first nonzero exponent tends to positive infinity. A constant nonzero scalar never tends to zero. Multiplication is well-defined because each coefficient receives only finitely many contributions.

In this **formal valuation topology the coefficient field `K` is discrete**. In particular, choosing `K=R` does not import the usual real-number convergence of its coefficients: the sequence of constant series `1/n` does not approach zero here. This construction is a topological ring, and a topological algebra over `K` with its discrete topology; it is not a topological real algebra over the usual topology on the real numbers. The interval-norm construction uses the usual real topology instead. These are distinct, explicitly chosen notions of convergence.

In this completion the left fraxis is

\[
L=X^{-1}\sum_{n\geq0}q^n
  =\frac{1-q}{b}\frac{1}{1-q}=\frac1b.
\]

The partial sums satisfy the exact MDN identity

\[
L_N=\frac{1-q^{N+1}}{b}.
\]

Thus the remainder is `q^(N+1)/b`, which tends to zero in this topology. The quotient remains nonzero and carries remain exact.

There is a second consistent completion, `K((q^{-1}))`, in which negative powers can continue indefinitely but positive powers are bounded above. There

\[
R=-X^{-1}\sum_{n<0}q^n=\frac1b,
\qquad
R_N=\frac{1-q^{-N-1}}b.
\]

Both completions contain the same finite algebra and its rational functions. They have different notions of which tails vanish. An arbitrary formal series in one completion does not have a canonical conversion to the other. Rational functions do: recover the rational function and expand it in the other chart.

It is essential to **impose the finite carry relation before constructing these completions**. For example, completing the independent variables at `X=Y=0` would make

\[
(X+Y-b)^{-1}
=-\frac1b\sum_{n\geq0}\left(\frac{X+Y}{b}\right)^n
\]

a valid formal power series. Subsequently imposing `X+Y-b=0` would set a unit equal to zero and collapse the ring. This is a different, incompatible completion.

## 2. Why the two ordinary directional limits cannot share one continuous arithmetic

Suppose a nonzero Hausdorff topological `K`-algebra contains `A_b`, has jointly continuous multiplication, and makes both `L_N` and `R_N` converge to `1/b`. Their exact remainder identities imply

\[
q^{N+1}\longrightarrow0,
\qquad q^{-N-1}\longrightarrow0.
\]

Joint continuity would then imply

\[
1=q^{N+1}q^{-N-1}\longrightarrow0.
\]

The constant sequence `1` already converges to `1`. Uniqueness of limits in a Hausdorff space gives `1=0`, a contradiction.

Therefore one must change at least one condition: use different directional charts, restrict multiplication, use a summation operation other than ordinary convergence of these partial sums, or abandon the stated topology. This is not a claim that every conceivable generalized-number framework is impossible.

## 3. The exact formal-delta connection

For this section **do not impose `X+Y=b`**. Let raw bilateral grids be arrays of coefficients, with equality meaning equality in every cell. Addition is always defined. Multiplication by a Laurent polynomial is defined because it is a finite local operation. Multiplication of two arbitrary bilateral grids is not generally defined.

The two raw expansions are

\[
L=X^{-1}\sum_{n\geq0}q^n,
\qquad
R=-X^{-1}\sum_{n<0}q^n,
\qquad q=-Y/X.
\]

Direct coefficientwise cancellation gives `(X+Y)L=1` and `(X+Y)R=1`. Their difference is

\[
D=L-R=X^{-1}\sum_{n\in\mathbb Z}q^n.
\]

The bilateral sum `delta(q)=sum q^n` is the usual formal delta distribution. It satisfies

\[
(1-q)\delta(q)=0,
\qquad (X+Y)D=0.
\]

Here `D` is nonzero as a coefficient array. This is perfectly consistent in a module on which polynomials act: a nonzero polynomial may annihilate a nonzero array. It is not consistent to treat that array as a nonzero scalar MDN value while also requiring the MDN carry relation. Applying `X+Y=b` to the last equation gives `bD=0`, hence `D=0` because `b` is invertible in `K`.

Even if one initially avoids scalar `1/b`, allowing an inverse `L` with `bL=1` in a commutative associative algebra makes `b` invertible and yields the same conclusion. There is no surviving scalar ambiguity between two inverses of the same element.

This is the precise sense in which the apparent fuzziness can survive as **representation or boundary information**, while disappearing as an additional scalar value. It does not imply that MDN defines a quantum particle or a vertex algebra.

## 4. Complete classification on a single anti-diagonal

Fix `m >= 1` and consider a raw bilateral grid of total degree `-m`:

\[
F_m=\sum_{k\in\mathbb Z}c_k X^{k-m}Y^{-k}.
\]

All calculations in this section are off the carry quotient. Since multiplication by `(X+Y)^m` is a finite stencil,

\[
(X+Y)^mF_m
=\sum_{k\in\mathbb Z}
 \left[\sum_{j=0}^{m}{m\choose j}c_{k+j}\right]
 X^kY^{-k}.
\]

Consequently `(X+Y)^m F_m=1` is precisely

\[
(I+S)^m c=\delta_0,
\qquad (Sc)_k=c_{k+1}.
\]

The symbol `delta_0` here means the coefficient sequence equal to one at index zero and zero elsewhere, not the bilateral formal delta series.

Given any one solution `c*`, every other solution has the form

\[
\boxed{c_k=c_k^*+(-1)^k P(k),\qquad \deg P\leq m-1.}
\]

To prove this, set `h_k=(-1)^k d_k` for a homogeneous solution. Then

\[
((I+S)^m h)_k=(-1)^{k+m}\Delta^m d_k,
\qquad \Delta d_k=d_{k+1}-d_k.
\]

The bilateral solutions of `Delta^m d=0` are exactly the polynomial sequences of degree at most `m-1`: their `m` initial consecutive values determine the sequence in both directions; the unique interpolating polynomial of degree less than `m` has the same recurrence and same initial data. This proves both existence and completeness of the classification.

Examples of homogeneous freedoms are:

| Order | All homogeneous patterns | Data needed to fix an unrestricted solution |
|---|---|---|
| 1 | `(-1)^k C` | One coefficient |
| 2 | `(-1)^k(A+Bk)` | Two consecutive coefficients |
| 3 | `(-1)^k(A+Bk+Ck^2)` | Three consecutive coefficients |

These anchors solve a formal recurrence. They do not turn its raw bilateral array into an MDN scalar. If the quotient relation is subsequently applied compatibly, `(X+Y)^m F_m=1` requires its scalar value to be the unique `1/b^m`.

A concrete particular solution comes from the one-direction expansion

\[
L^m=X^{-m}\sum_{n\geq0}(-1)^n
 {n+m-1\choose m-1}(Y/X)^n.
\]

In our indexing this has

\[
c_k^*=
\begin{cases}
(-1)^k{m-1-k\choose m-1},& k\leq0,\\
0,&k>0.
\end{cases}
\]

The opposite-direction expansion `R^m` has nonzero coefficients only for `k>=m`. Their difference is the homogeneous sequence

\[
(L^m-R^m)_k=(-1)^k{m-1-k\choose m-1},
\]

where the binomial on the right is interpreted as a polynomial in `k`, valid for every integer `k`. The zeros at `1,...,m-1` fill the gap between the two expansions.

**Digit bounds matter.** In this paragraph coefficients are real or complex, and boundedness uses their usual absolute value, not a non-Archimedean norm or the formal valuation topology. Among the homogeneous sequences above, bounded coefficients force `P` to be constant, regardless of `m`. For `m>=2`, there is no bounded bilateral solution of the inhomogeneous equation `(I+S)^m c=delta_0` on this single anti-diagonal at all. To see this, remove the alternating sign as above. Away from the finite forcing, the sequence is polynomial on each tail. Boundedness makes both tails constant. Summing its `m`th differences over all indices telescopes to zero when `m>=2`, whereas the forcing has nonzero total sum. This is a contradiction.

That boundedness statement does not prohibit a carry-normalized expansion spread across several anti-diagonals. Such an expansion belongs to a different problem than the single-diagonal classification just proved.

## 5. Mixed products and the factor-of-two cutoff effect

Use the partial sums with `N+1` terms on each side. In the symmetrically indexed degree `-2` basis

\[
e_k=X^{k-1}Y^{-k-1}
\]

their ordinary finite product is

\[
L_NR_N=\sum_{k=-N}^{N}
 (-1)^k(N+1-|k|)e_k.
\]

This follows by counting pairs `(n,r)` with `r-n=k`. Every fixed coefficient diverges in magnitude as `N` grows. In particular the central coefficient is `N+1`.

Let `H=sum_{k in Z}(-1)^k e_k`; coefficientwise `(X+Y)H=0`. Subtracting `(N+1)H` from the finite product produces a coefficientwise limit

\[
Q_{\rm hard}=\sum_{k\in\mathbb Z}-|k|(-1)^k e_k.
\]

This is a well-defined raw formal distribution, but it satisfies

\[
\boxed{(X+Y)^2Q_{\rm hard}=2,}
\]

not one. Indeed multiplication by `(X+Y)^2` sends coefficients `a_k` to `a_{k-1}+2a_k+a_{k+1}`, and this stencil is two at zero and zero elsewhere for `a_k=-|k|(-1)^k`.

The same discrepancy is visible before any coefficient extraction:

\[
(X+Y)^2L_NR_N
 =2-q^{N+1}-q^{-N-1}.
\]

The two moving tails disappear coefficientwise, leaving two. Their interaction contributes the extra constant; it cannot be discarded independently.

This is why removing a divergent pattern and pinning a coefficient does not, by itself, define the expected inverse-square product. It has specified a particular finite-part operation, with particular consequences.

## 6. Abel damping gives another finite part

Introduce a real parameter `0<r<1` and damp each directional term:

\[
L_r=\sum_{n\geq0}(-r)^nY^nX^{-n-1},
\qquad
R_r=\sum_{n\geq0}(-r)^nX^nY^{-n-1}.
\]

For each fixed grid coefficient their convolution is absolutely summable. In the basis `e_k`, the resulting coefficient is

\[
a_k(r)=\frac{(-1)^k r^{|k|}}{1-r^2}.
\]

Write `epsilon=1-r`. Its expansion is

\[
a_k(r)=(-1)^k\left[
\frac{1}{2\epsilon}+\frac14-\frac{|k|}{2}+O(\epsilon)
\right].
\]

Subtracting `H/(2 epsilon)` leaves the coefficientwise finite part

\[
Q_{\rm Abel}=\sum_k(-1)^k\left(\frac14-\frac{|k|}{2}\right)e_k.
\]

It satisfies `(X+Y)^2 Q_Abel=1`. Subtracting `H/(1-r^2)` instead gives the same result without the `1/4` term. These two subtraction prescriptions differ by the homogeneous term `H/4`.

The term with coefficient `-|k|(-1)^k/2` is exactly `(L^2+R^2)/2`, where each square is formed within its own valid directional formal-series algebra. This identity does not define the undefined raw product `LR`; it identifies one possible extension after choosing a prescription.

As rational functions,

\[
L_r=\frac1{X+rY},\qquad R_r=\frac1{Y+rX}.
\]

At positive scalar evaluations `X+Y=b`, their rational product tends to `1/b^2`. For a fixed `r`, both displayed geometric series converge numerically together only in `r<|Y/X|<1/r`. This annulus shrinks towards `|X|=|Y|` as `r` tends to one. Using the rational expressions outside their series domains is continuation, not ordinary convergence of both original expansions.

At the especially simple decimal evaluation `X=Y=5`, both undamped partial sums alternate `0.2,0,0.2,0,...`. Averaging them first gives `0.1`, whose square is `0.01`. Multiplying matched partial sums first gives `0.04,0,0.04,0,...`, whose average is `0.02`. The order of these operations matters. Ordinary multiplication of the resolved scalar inverses gives `0.01`; averaging matched products is a different operation.

## 7. What has and has not been established

The mathematics provides a consistent account of the one-parameter fuzzy mode, a classification of higher-order raw fuzzy modes, and exact demonstrations of why limits and products need declared rules. These results are useful for specifying a rigorous MDN workbench. They do not establish new scalar numbers, novel physical degrees of freedom, or an advantage over conventional mathematics.

The existing C++ normalization procedure has not been used as an oracle for these conclusions. Relating its particular infinite fuzzy displays to this classification still requires checking its exact recurrence and treatment of boundary carries.

## References and verification

- Victor Kac, *Fundamentals of formal distributions*, scribe notes by Darij Grinberg, version September 20, 2026: [author-hosted notes](https://darijgrinberg.gitlab.io/algebra/va3.pdf). Sections 1.1 and 3 distinguish Laurent series from bilateral distributions, explain the restricted products, and develop the formal delta function. Our MDN formulas above are independently derived specializations of that framework.
- NIST, [DLMF section 3.6, Linear Difference Equations](https://dlmf.nist.gov/3.6), gives the general recurrence and boundary-condition setting; [section 18.1](https://dlmf.nist.gov/18.1) defines the forward-difference notation. The exact polynomial-kernel classification used here is proved above.

Independent exact-integer/fraction checks covered homogeneous patterns for orders 1 through 8 on indices -25 through 25, the hard and Abel finite-part stencils on indices -100 through 100, and all coefficients of the first 20 finite mixed-product patterns. These finite checks corroborate the formulas; the algebraic arguments above establish the infinite statements.
