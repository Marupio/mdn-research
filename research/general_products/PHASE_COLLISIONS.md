# A period can exceed the number of distinct display phases

Consider the matched product

\[
G_n=L_{n-1}^{\,2}R_{n-1}^{\,8},\qquad n\ge1,
\]

where all ten factors have the same positive cascade term count \(n\).
Normalize each finite grid with the fixed canonical digit set
\(D=\{-4,-3,\ldots,5\}\). The zero-based GUI depth is \(n-1\).

For this family:

- The minimal universal eventual display period is
  \(T=250{,}000{,}000\).
- There are exactly **187,500,000 distinct infinite display phase arrays**.
- Residues that differ by \(S=T/2=125{,}000{,}000\) give the same array
  exactly when their term counts are even.

Thus the minimal period and the number of distinct arrays are different
notions. These statements concern cellwise display limits. They neither
identify the complete finite numbers nor discard their endpoints.

## 1. The exact carry criterion

Use the raw-coefficient and carry-locality results in
[PROOFS.md](PROOFS.md). At each fixed integer diagonal index \(k\), and for
all sufficiently large \(n\), the raw coefficient at
\((k-2,-k-8)\) is \((-1)^k\) times

\[
I(n,k)+W(k),\qquad
I(n,k)=\binom{2n+k+7}{9}-10\binom{n+k+7}{9}.
\tag{1}
\]

Here \(W(k)\) is independent of \(n\): it is zero for \(k<2\), and
\(45\binom{k+7}{9}\) for \(k\ge2\). This follows directly by extracting
the coefficient of \(z^{2n+k-2}\) in
\((1+z+\cdots+z^{n-1})^{10}\). The term \(W(k)\) cancels when two
counts are compared.

For \(0\le j\le8\), define the forward-difference comparison polynomial

\[
\begin{aligned}
A_j(n,k;s)={}&
\binom{2(n+s)+k+7}{9-j}
-10\binom{n+s+k+7}{9-j}\\
&-\binom{2n+k+7}{9-j}
+10\binom{n+k+7}{9-j}.
\end{aligned}
\tag{2}
\]

The binomial is interpreted as its falling-product polynomial when the
upper argument is negative. Equation (2) is the \(j\)-th forward
\(k\)-difference of \(I(n+s,k)-I(n,k)\), because
\(\binom{t+1}{r}-\binom tr=\binom t{r-1}\).
Its total degree in \((n,k)\) is at most \(8-j\).

The carry criterion states that the two eventual arrays agree exactly when

\[
10^{j+1}\mid A_j(n,k;s)
\quad\text{for every }j=0,\ldots,8
\text{ and every integer }k.
\tag{3}
\]

Indeed, equal lowest digits require a difference divisible by 10. Each
subsequent carry layer takes one difference along the signed raw diagonal
and divides it by 10. Applying this argument successively gives (3).
The actual carry recurrence uses backward differences; forward differences
give the same all-\(k\) conditions after translating the index by \(j\).
After nine differences the comparison polynomial vanishes, so there is no
additional condition on higher layers. Carry locality makes this an exact
criterion for each fixed window once its finite ancestry is in the interior.

The general theorem already supplies the universal period \(10^9\).
Consequently the residue-indexed arrays used here exist: take positive
counts in a fixed residue modulo \(10^9\), and then use the sharper period
proved below. Evaluating an interior polynomial at a small count such as
zero is a certificate calculation, not a claim that a zero-term input grid
has that interior shape.

## 2. A lower bound valid even for individual phase collisions

At the last relevant layer, equation (2) simplifies to

\[
A_8(n,k;s)=-8s.
\]

Therefore any two equal phase arrays must have count difference satisfying

\[
10^9\mid8s
\quad\Longleftrightarrow\quad
S=125{,}000{,}000\mid s.
\tag{4}
\]

This necessity is independent of \(n\). It applies both to universal
periods and to equality of particular phases.

## 3. Finite exact certificates for the two sufficiency claims

We will prove both

\[
10^{j+1}\mid A_j(n,k;T)
\quad\text{for all integers }n,k,
\tag{5}
\]

and

\[
10^{j+1}\mid A_j(2u,k;S)
\quad\text{for all integers }u,k.
\tag{6}
\]

For either polynomial, put \(t=8-j\). It suffices to evaluate at the
finite triangle

\[
\mathcal N_t=\{(u,k):u,k\ge0,\ u+k\le t\}.
\tag{7}
\]

To justify this reduction, a rational polynomial \(Q(u,k)\) of total
degree at most \(t\) has the finite Newton expansion

\[
Q(u,k)=\sum_{r+v\le t}
(\Delta_u^r\Delta_k^v Q)(0,0)
\binom ur\binom kv.
\]

Each Newton coefficient is an integer linear combination of values at
\(0\le u\le r,\ 0\le k\le v\), all contained in (7). If the sampled
values are divisible by an integer \(M\), so are those coefficients.
The binomials are integers at every integer argument, including negative
ones; hence \(M\mid Q(u,k)\) everywhere. Conversely, universal
divisibility plainly implies divisibility at the sample nodes.

For each layer, let \(g_T\) be the gcd of all values
\(A_j(u,k;T)\) at the nodes (7), and let \(g_E\) be the corresponding gcd
of \(A_j(2u,k;S)\). The exact gcd valuations are:

| Layer \(j\) | Degree bound \(t\) | Nodes | \(v_2(g_T)\) | \(v_5(g_T)\) | \(v_2(g_E)\) | \(v_5(g_E)\) |
|---:|---:|---:|---:|---:|---:|---:|
| 0 | 8 | 45 | 5 | 8 | 5 | 8 |
| 1 | 7 | 36 | 6 | 8 | 5 | 8 |
| 2 | 6 | 28 | 6 | 8 | 5 | 8 |
| 3 | 5 | 21 | 6 | 8 | 5 | 8 |
| 4 | 4 | 15 | 6 | 8 | 6 | 8 |
| 5 | 3 | 10 | 7 | 9 | 6 | 9 |
| 6 | 2 | 6 | 7 | 9 | 8 | 9 |
| 7 | 1 | 3 | 8 | 9 | 8 | 9 |
| 8 | 0 | 1 | 10 | 9 | 9 | 9 |

Every valuation in a layer is at least \(j+1\). Thus each gcd is divisible
by \(10^{j+1}\), proving (5) and (6) through the Newton argument. Each
claim requires 165 exact evaluations, regardless of the numerical size of
the proposed period.

The following standalone Python certificate regenerates the table and
checks its entries using only integer arithmetic. All upper binomial
arguments at these nonnegative nodes are nonnegative, so ordinary
`math.comb` suffices.

```python
from math import comb, gcd

T = 250_000_000
S = T // 2

def q(n, k, j):
    return comb(2*n + k + 7, 9-j) - 10*comb(n + k + 7, 9-j)

def valuation(value, prime):
    assert value != 0
    count = 0
    while value % prime == 0:
        value //= prime
        count += 1
    return count

expected = [
    (5, 8, 5, 8), (6, 8, 5, 8), (6, 8, 5, 8),
    (6, 8, 5, 8), (6, 8, 6, 8), (7, 9, 6, 9),
    (7, 9, 8, 9), (8, 9, 8, 9), (10, 9, 9, 9),
]
total = 0
for j in range(9):
    degree = 8-j
    gt = ge = nodes = 0
    for u in range(degree+1):
        for k in range(degree+1-u):
            gt = gcd(gt, q(u+T, k, j) - q(u, k, j))
            ge = gcd(ge, q(2*u+S, k, j) - q(2*u, k, j))
            nodes += 1
    actual = (valuation(gt, 2), valuation(gt, 5),
              valuation(ge, 2), valuation(ge, 5))
    assert actual == expected[j]
    assert min(actual) >= j+1
    total += nodes
    print(j, degree, nodes, *actual)
assert total == 165
```

This is a finite certificate for universal polynomial congruences, not an
extrapolation from representative depths or a sampled display window.

## 4. The half-period fails at every odd count

At layer 6 and the fixed forward-difference index \(k=-6\), equation (2)
gives the explicit identity

\[
A_6(n,-6;S)
=\frac{S}{3}\bigl(4-3n^2-3Sn-S^2\bigr).
\tag{8}
\]

This follows by expanding the two cubic binomials with upper arguments
\(2n+1\) and \(n+1\), before and after the count shift. Their difference
is an integer; the displayed division by 3 is exact.

Now \(S=2^6 5^9\) is even. If \(n\) is odd, the bracket in (8) is odd.
Division by 3 does not change its power of 2, so

\[
v_2\bigl(A_6(n,-6;S)\bigr)=6<7.
\]

The layer-6 requirement \(10^7\mid A_6\) therefore fails. The two phases
are different at every odd count residue. No scalar values have been
assigned to positional shifts in this argument.

Together with (6), this proves that shifting the count by \(S\) preserves
an individual phase **exactly when the count is even**.

## 5. Minimal period and exact number of arrays

Equation (5) proves that \(T=2S\) is a universal period. Equation (4)
requires every positive period to be a multiple of \(S\), while (8)
excludes \(S\). Therefore \(T\) is the minimal positive universal period.

There are \(T\) residue labels \(r\) modulo \(T\). For any such label,
take positive counts \(n=r+\ell T\) with \(\ell\to\infty\). The carry
criterion and locality prove that every fixed digit eventually stabilizes,
defining its phase array. Equation (4) says that two different residue
labels can define the same array only if they differ by \(S\).

Because \(S\) is even, this possible identification preserves parity:

- The \(T/2=125{,}000{,}000\) odd labels remain distinct by (8).
- The \(T/2=125{,}000{,}000\) even labels are paired by \(r\mapsto r+S\),
  giving \(T/4=62{,}500{,}000\) arrays by (6).

There are no other identifications. Hence the total number of arrays is

\[
\boxed{\frac T2+\frac T4=187{,}500{,}000.}
\]

This finite classification does not make equal-phase finite grids equal.
Their exact boundary certificate remains

\[
10^{10}G_n=(1-E)^2(1-E^{-1})^8,
\qquad E=q^n,\qquad q=-Y/X.
\]

Different positive counts give different finite carry classes. For example,
the largest exponent of \(q\) in this certificate is \(2n\), with nonzero
coefficient, so changing \(n\) changes the Laurent polynomial. The phase
collision concerns only the infinite displays observed at fixed cells as
those different boundaries move away.
