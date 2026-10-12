# Exact phases of matched fuzzy products

These proofs concern digits, positions and finite carries. The symbols
$X,Y$ are positional shifts, and $X+Y=10$ is the carry relation. No ordinary
value is assigned to either shift.

There are two different statements to distinguish:

1. A finite grid, including its boundary, has an exact value modulo carries.
2. Its digits in each fixed window can repeat when the boundary moves away.

The second statement does not identify the finite values in the first.
The display-period theorems below use the **fixed canonical digit set**
$D=\{-4,-3,\ldots,5\}$. They do not, by themselves, prove a period for the
GUI's Positive sign policy. Backend comparisons with that policy are
separate experimental evidence.

## 1. Notation and finite boundary certificates

Depth $N\ge0$ is zero-based, and $n=N+1$ is the positive term count. Write

\[
L_N=\sum_{i=0}^{N}(-1)^iX^{-i-1}Y^i,
\qquad
R_N=\sum_{i=0}^{N}(-1)^iX^iY^{-i-1},
\qquad F_N=L_NR_N.
\]

Set $q=-Y/X$, a signed shift along a diagonal, and $E=q^n$. Finite
telescoping, followed by the carry relation, gives

\[
10L_N=1-E,\qquad 10R_N=1-E^{-1}.
\]

Consequently the following are exact finite carry identities:

\[
\boxed{100F_N=2-E-E^{-1},}
\]

\[
\boxed{1000F_NL_N=3-3E+E^2-E^{-1},}
\]

\[
\boxed{1000F_NR_N=3-3E^{-1}+E^{-2}-E,}
\]

\[
\boxed{10000F_N^2=6-4(E+E^{-1})+E^2+E^{-2}.}
\]

These certificates retain every endpoint. All left sides are constructed
by integer grid operations; the equations do not assume a freely available
operation of division by 10. The formal independence of distinct powers
of $q$ in the rational coefficient extension is proved in
[the finite-family appendix](../fuzzy/PROOFS.md#1-finite-carry-equality-and-coefficient-rings).
It shows, in particular, that increasing $N$ changes each of these finite
values even when their central displays repeat.

Reflection $(x,y)\mapsto(y,x)$ exchanges $L_N$ and $R_N$, exchanges
$F_NL_N$ and $F_NR_N$, and fixes $F_N$ and $F_N^2$.

## 2. Exact raw coefficients in an interior window

All coefficients in this section are **raw integer coefficients**, before
digit normalization. Large coefficients count terms of a finite
convolution; they are not claims about a scalar magnitude of the MDN.

Let

\[
B_n(z)=1+z+\cdots+z^{n-1}.
\]

For a product of $a$ copies of $L_N$ and $c$ copies of $R_N$, put $d=a+c$.
Its raw coefficient at

\[
(x,y)=(k-a,-k-c)
\]

is $(-1)^k$ times the coefficient of $z^{k+a(n-1)}$ in $B_n(z)^d$.
This is a finite counting identity: choose one cascade index from each
factor and collect equal grid positions.

### The original mixed product

For $F_N$ the coefficient at $(k-1,-k-1)$ is

\[
(-1)^k C_n^{F}(k),\qquad
C_n^{F}(k)=n-|k|,\qquad |k|\le n-1.
\]

### One additional directional tenth

For $F_NL_N$ the coefficient at $(k-2,-k-1)$ is
$(-1)^k C_n^{FL}(k)$. For each fixed $k$ and all sufficiently large $n$,

\[
\boxed{
C_n^{FL}(k)=\frac{n(n+1)}2-nk+\phi(k),
\qquad
\phi(k)=
\begin{cases}
-k(k-1),&k\le1,\\
k(k-1)/2,&k\ge2.
\end{cases}}
\]

For example, $n>|k|+2$ is sufficient. To prove the formula, expand

\[
B_n(z)^3=\frac{(1-z^n)^3}{(1-z)^3}.
\]

In the indicated interior window, its coefficient is

\[
\binom{k+2n}{2}-3\binom{k+n}{2}
+\begin{cases}3\binom{k}{2},&k\ge2,\\0,&k\le1.\end{cases}
\]

Expanding these quadratic polynomials gives the displayed result. For
$F_NR_N$, reflect the coordinates; its amplitude at $(k-1,-k-2)$ is
$C_n^{FL}(-k)$.

### The square

For $F_N^2$ the coefficient at $(k-2,-k-2)$ is
$(-1)^k C_n^{F^2}(k)$. In the same eventual interior sense,

\[
\boxed{
C_n^{F^2}(k)=\frac{2n^3+n}{3}-nk^2+
\frac{|k|^3-|k|}{2}.}
\]

Indeed, expand $(1-z^n)^4/(1-z)^4$ and take the coefficient at
$k+2n-2$. In the interior this is

\[
\binom{k+2n+1}{3}-4\binom{k+n+1}{3}
+\begin{cases}6\binom{k+1}{3},&k\ge2,\\0,&k\le1.\end{cases}
\]

For $k\le1$ the last term is absent and the remaining expression is
$(2n^3+n)/3-nk^2-(k^3-k)/2$; for $k\ge2$ it becomes
$(2n^3+n)/3-nk^2+(k^3-k)/2$. These agree with the absolute-value formula.

In particular, the three central raw coefficients are

\[
n,\qquad \frac{n(n+1)}2,\qquad \frac{n(2n^2+1)}3.
\]

## 3. A fixed canonical carry display

For any integer $v$, define its digit and carry by

\[
h(v)=\left\lfloor\frac{v+4}{10}\right\rfloor,
\qquad r(v)=v-10h(v)\in D.
\]

Here $D=\{-4,\ldots,5\}$ contains exactly one representative of every
residue modulo 10. Thus $r(u)=r(v)$ precisely when $u-v$ is divisible by 10.

For a raw input on $(k-a,-k-c)$, set

\[
v_0(k)=(-1)^kC_n(k).
\]

At layer $j\ge0$, the coordinate with index $k$ is

\[
P_{j,k}=(k-a,-k-c+j).
\]

The complete forward normalization recurrence is

\[
\boxed{
h_j(k)=h(v_j(k)),\quad d_j(k)=r(v_j(k)),\quad
v_{j+1}(k)=h_j(k)+h_j(k-1).}
\]

The two terms at layer $j+1$ are exactly the carries from its lower and
left neighbours. No infinite convolution is used.

### Termination and uniqueness for finite grids

For a finite input, every layer has finite support. Once above the highest
original input layer, let $M$ bound the absolute values of the incoming
coefficients. The next layer's coefficients have absolute value at most

\[
2\left\lfloor\frac{M+5}{10}\right\rfloor.
\]

For $M\ge2$ this is strictly smaller than $M$; for $M\le1$ there is no
carry. Thus the sweep stops after finitely many layers, yielding a finite
$D$-grid carry-equivalent to the input.

For uniqueness, a nonzero finite grid that equals zero modulo carries has
all its coefficients on its lowest nonempty diagonal divisible by 10.
One way to see this entirely formally is to write it as
$(X+Y-10)H$ for a finite grid $H$. At the lowest degree $x+y$ present in
$H$, its coefficients are multiplied by $-10$; the $XH$ and $YH$ terms
are on the next diagonal. But the difference of two $D$-grids has every
coefficient between $-9$ and 9. Its lowest nonempty diagonal therefore
cannot belong to a nonzero zero-equivalent grid. This proves uniqueness.

Hence the recurrence defines a canonical display for every finite grid,
not merely a permissible sequence of carries. The GUI's Positive policy
is a different display convention.

### Locality

The digit $d_j(k)$ depends only on the raw coefficients with indices

\[
k-j,k-j+1,\ldots,k.
\]

This follows inductively from the recurrence. Consequently, for any fixed
finite window, all its displayed digits can be calculated from a fixed
finite set of raw coefficients. Taking $n$ sufficiently large puts that
whole set inside the range of the formulas in Section 2. Carries from a
moving far boundary cannot reach these fixed cells without moving through
more layers than the cells' fixed heights allow.

This locality is what justifies the eventual statements below. It is not
an instruction to discard a boundary from a finite value.

## 4. Exact periods: 10, 100 and 500

Fix a positive integer depth increment $s$. Since $n=N+1$, the increment
of $n$ is also $s$. For a raw coefficient array or an incoming layer, write
$\Delta v_j(k)=v_j^{(n+s)}(k)-v_j^{(n)}(k)$.

If the displayed digits on layer $j$ agree, their carry difference is
$\Delta v_j(k)/10$. Thus

\[
\Delta v_{j+1}(k)=
\frac{\Delta v_j(k)+\Delta v_j(k-1)}{10}.
\tag{1}
\]

For a signed polynomial difference $\Delta v_j(k)=(-1)^kp(k)$, its next
layer is $(-1)^k[p(k)-p(k-1)]/10$. Each successful carry comparison takes
one discrete difference of the polynomial. This is the mechanism behind
the periods.

### Original fuzzy product: exactly 10 phases

The raw interior difference is

\[
\Delta v_0(k)=(-1)^ks.
\]

If $10\mid s$, the lowest digits agree and (1) gives $\Delta v_1=0$.
All higher layers then agree. Conversely, agreement of the lowest digit
at $k=0$ requires $10\mid s$. The exact eventual display period is 10.

This conclusion concerns the canonical display. The independently
constructed ten Positive-policy patterns are described in
[DISPLAY_PATTERNS.md](../decimal_infinite/DISPLAY_PATTERNS.md).

### Product with a matched directional tenth: exactly 100 phases

The piecewise term $\phi(k)$ cancels when depths are compared:

\[
\Delta C_n^{FL}(k)=sn+\frac{s(s+1)}2-sk.
\tag{2}
\]

For $s=100$, (2) is divisible by 10 for every integer $n,k$. Thus the
lowest digits agree, and

\[
\Delta v_1(k)=(-1)^k\frac{-s}{10}.
\]

This is again divisible by 10. The second layer's digits agree and its
two neighbouring carry differences cancel, giving $\Delta v_2=0$.
All higher fixed cells agree by the same recurrence and locality.

For necessity, if all fixed cells agree, the lowest digits at two adjacent
indices first imply $10\mid s$ by (2). Agreement of their common next
layer then requires $10\mid(s/10)$, hence $100\mid s$. There is also a
lowest-layer parity requirement: with $s=10u$, the constant term in (2)
is divisible by 10 exactly when $u$ is even. So the lowest layer alone
has exact period 20, while the full canonical display has exact period
100. Reflection proves the identical result for $F_NR_N$.

### Square: exactly 500 phases

Here the raw amplitude difference is

\[
\Delta C_n^{F^2}(k)=
2sn^2+2s^2n+\frac{2s^3+s}{3}-sk^2.
\tag{3}
\]

If $10\mid s$, all terms in (3) together are divisible by 10. For the
term with denominator 3, use that it is an integer and 3 is relatively
prime to 10. Its numerator is divisible by 10.

Provided the digits agree on the preceding layers, (1) gives in turn

\[
\Delta v_1(k)=(-1)^k\frac{-s(2k-1)}{10},
\]

\[
\Delta v_2(k)=(-1)^k\frac{-2s}{100},
\]

\[
\Delta v_3(k)=0.
\]

For $s=500$ each of the first three incoming differences is divisible
by 10. Therefore their displayed digits agree; the difference on the
fourth layer vanishes, and all higher layers agree too.

Conversely, equality on the lowest layer at $k=0,1$ requires $10\mid s$
because the difference of (3) at those indices is $-s$. Equality on the
next layer requires $100\mid s$ by evaluating $\Delta v_1$ at $k=0$.
Equality on the following layer requires $1000\mid 2s$, hence
$500\mid s$. This last condition includes the preceding two. Therefore
the exact full display period is 500, with lower layers resolving it in
stages:

| Layers included, starting at $x+y=-4$ | Exact eventual period |
|---|---:|
| Lowest layer only | 10 |
| Lowest two layers | 100 |
| Lowest three layers, or the entire display | 500 |

### What “exactly this many phases” means

For each residue class of $n$ modulo the stated period, every fixed digit
is eventually constant. These constants define an infinite digit array.
The necessity arguments also distinguish any two different residue
classes: choose sufficiently large representatives, compare the indicated
finite cells, and apply the divisibility requirements above. Thus there
are exactly 10, 100 and 500 different arrays for $F_N$, $F_NL_N$ and
$F_N^2$, respectively. $F_NR_N$ has 100 by reflection.

These are full **cellwise display limits** of finite normalized grids.
Their classification is not a claim that the arrays can be multiplied by
ordinary infinite convolution.

## 5. Small witnesses and why ten phases do not define multiplication

With $D=\{-4,\ldots,5\}$, the recurrence gives these exact local digits:

| Depth $N$ | $F_NL_N$ at $(-2,-1)$ | $F_N^2$ at $(-2,-2)$ | $F_N^2$ at $(-2,-1)$ | $F_N^2$ at $(-2,0)$ |
|---|---:|---:|---:|---:|
| 20 | 1 | 1 | 2 | -1 |
| 30 | -4 | 1 | 3 | -1 |
| 120 | 1 | 1 | 2 | -3 |
| 520 | 1 | 1 | 2 | -1 |

For example, at $N=20$ the raw central coefficients of $F_NL_N$ and
$F_N^2$ are 231 and 6181. The corresponding square coefficient at the
adjacent index $k=-1$ is $-6160$. Its two carries into $(-2,-1)$ are
$618$ and $-616$, giving the digit 2. At $N=30$, those two carries are
$1987$ and $-1984$, giving 3.

Consider the two cutoff recipes $N=20+100r$ and $N=30+100r$, where
$r\to\infty$. Their $F_N$ grids have the same limiting fuzzy phase.
Their $L_N$ grids also have the same cellwise limit: each raw directional
cascade simply extends its unchanged digits $1,-1,1,-1,\ldots$.
Nevertheless the products have different limiting digits at $(-2,-1)$:
1 in the first recipe and $-4$ in the second. Thus equality of input
display phases is not preserved by matched finite multiplication.

Likewise, the recipes $N=20+500r$ and $N=120+500r$ have the same original
fuzzy phase but square to different limiting displays, distinguished by
$(-2,0)$. Keeping only the original central digit loses information that
multiplication can bring back into a fixed window.

One can retain that information by keeping the finite cutoff recipe or
its exact endpoint symbols. Choosing a displayed phase alone has not
defined that multiplication.

## 6. Independent boundaries need still more information

The periods just proved assume **every factor has the same cutoff**.
They do not classify products with independently chosen cutoff lengths.

For $n=N+1$ and $m=M+1$, exact finite arithmetic gives

\[
\begin{aligned}
10000F_NF_M={}&4-2(q^n+q^{-n}+q^m+q^{-m})\\
&+q^{n+m}+q^{n-m}+q^{m-n}+q^{-n-m}.
\end{aligned}
\]

If $m=n+s$ with a fixed offset $s$, the two cross terms $q^{\pm s}$
stay at fixed positions even as both boundaries move out. For matched
cutoffs they instead both equal unity, producing the constant coefficient
6 in Section 1. The endpoint formula retains this distinction exactly.

Similarly, if the extra directional tenth has term count $p=P+1$, then

\[
1000F_NL_P=(2-q^n-q^{-n})(1-q^p).
\]

For $p=n+s$, one of its cross terms is the fixed signed shift $q^s$.
Offsets and growth relations between independent boundaries therefore
remain part of an exact multiplication rule. No endpoint is set to zero
in these identities, and no sum of moving terms is replaced by a claimed
ordinary limit.

The results establish the matched phase periods and a precise obstruction
to using the ten original phases as a complete multiplication system.
They leave the choice of a larger boundary-retaining infinite arithmetic
open.
