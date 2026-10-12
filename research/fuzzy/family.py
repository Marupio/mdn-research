"""Exact, boundary-retaining finite MDN cascade families.

No infinite sum, scalar axis weight, or floating-point approximation is used.
Depth N means N+1 terms, matching the app's fraxisCascadeDepth setting.
Raw grids have unrestricted integer coefficients; a digit-normalized GUI grid
is another representative modulo the finite carry relation X+Y=base.
"""

from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
from fractions import Fraction
from math import comb
from typing import Iterable

Grid = dict[tuple[int, int], int]
Ledger = dict[int, int]
Polynomial = tuple[int, ...]


def clean(values: dict) -> dict:
    return {key: value for key, value in values.items() if value}


def grid_add(*grids: Grid) -> Grid:
    out = defaultdict(int)
    for grid in grids:
        for coordinate, value in grid.items():
            out[coordinate] += value
    return clean(out)


def grid_scale(grid: Grid, value: int) -> Grid:
    return clean({coordinate: value*digit for coordinate, digit in grid.items()})


def grid_multiply(left: Grid, right: Grid) -> Grid:
    out = defaultdict(int)
    for (x, y), a in left.items():
        for (u, v), b in right.items():
            out[x+u, y+v] += a*b
    return clean(out)


def grid_power(grid: Grid, exponent: int) -> Grid:
    if type(exponent) is not int or exponent < 0:
        raise ValueError("Exponent must be a nonnegative integer")
    out = {(0, 0): 1}
    for _ in range(exponent):
        out = grid_multiply(out, grid)
    return out


def sign(exponent: int) -> int:
    return -1 if exponent % 2 else 1


def q_grid(ledger: Ledger) -> Grid:
    """q=-Y/X as a formal grid shift, with no scalar evaluation."""
    return {(-k, k): sign(k)*value for k, value in ledger.items() if value}


def carry_zero(grid: Grid, base: int = 10) -> bool:
    """Exact independent zero test after clearing Laurent denominators.

    Substitute Y=base-X and compare every integer polynomial coefficient.
    This tests the unrestricted finite carry quotient, not reachability under
    a particular digit-cleanup policy.
    """
    if type(base) is not int or base < 2:
        raise ValueError("Base must be an integer >=2")
    if not grid:
        return True
    xmin = min(x for x, y in grid)
    ymin = min(y for x, y in grid)
    out = defaultdict(int)
    for (x, y), value in grid.items():
        degree = y-ymin
        for k in range(degree+1):
            out[x-xmin+k] += value*comb(degree, k)*base**(degree-k)*sign(k)
    return not any(out.values())


def carry_equal(left: Grid, right: Grid, base: int = 10) -> bool:
    return carry_zero(grid_add(left, grid_scale(right, -1)), base)


def ledger_multiply(left: Ledger, right: Ledger) -> Ledger:
    out = defaultdict(int)
    for i, a in left.items():
        for j, b in right.items():
            out[i+j] += a*b
    return clean(out)


def affine_boundary_ledger(
    x_lengths: Iterable[tuple[int, int]],
    y_lengths: Iterable[tuple[int, int]],
) -> dict[tuple[int, int], int]:
    """Symbolic endpoint ledger for term counts slope*t+offset.

    Slopes are positive integers; offsets may be negative, so the term
    counts are positive for all sufficiently large t. Keys (s,c) denote
    q^(s*t+c). Keep ALL keys for exact multiplication. Filtering s=0 only
    describes what remains in a fixed exponent window as t grows; it is
    not a carry-compatible value limit or an equality operation.
    """
    out = {(0, 0): 1}
    for orientation, lengths in ((1, x_lengths), (-1, y_lengths)):
        for slope, offset in lengths:
            if type(slope) is not int or slope < 1 or type(offset) is not int:
                raise ValueError("Require positive integer slopes and integer offsets")
            ds, dc = orientation*slope, orientation*offset
            updated = defaultdict(int, out)
            for (s, c), value in out.items():
                updated[s+ds, c+dc] -= value
            out = clean(updated)
    return out


def _depths(values: Iterable[int]) -> tuple[int, ...]:
    values = tuple(values)
    if any(type(value) is not int or value < 0 for value in values):
        raise ValueError("Every cascade depth must be a nonnegative integer")
    return tuple(sorted(values))


@dataclass(frozen=True)
class CascadeProduct:
    """A product of directional tenths, each retaining its own finite depth.

    In base b, these are finite directional entries of 0.1, not exact inverses.
    Concatenating the cutoff tuples defines multiplication. Addition belongs
    to CascadeExpression, a finite integer linear combination of products.
    """

    x_depths: tuple[int, ...] = ()
    y_depths: tuple[int, ...] = ()

    def __post_init__(self):
        object.__setattr__(self, "x_depths", _depths(self.x_depths))
        object.__setattr__(self, "y_depths", _depths(self.y_depths))

    @classmethod
    def F(cls, n: int, m: int) -> CascadeProduct:
        return cls((n,), (m,))

    @property
    def order(self) -> int:
        return len(self.x_depths)+len(self.y_depths)

    @property
    def depths(self) -> tuple[int, ...]:
        return self.x_depths+self.y_depths

    @property
    def k_bounds(self) -> tuple[int, int]:
        return -sum(self.x_depths), sum(self.y_depths)

    def __mul__(self, other: CascadeProduct) -> CascadeProduct:
        if not isinstance(other, CascadeProduct):
            return NotImplemented
        return CascadeProduct(self.x_depths+other.x_depths,
                              self.y_depths+other.y_depths)

    def envelope(self) -> list[int]:
        """Unsigned coefficients: convolution of finite boxes of ones.

        Sliding-window sums cost O(order * total support), rather than
        enumerating all digit tuples or expanding every subset boundary.
        """
        counts = [1]
        for depth in self.depths:
            expanded = []
            running = 0
            for k in range(len(counts)+depth):
                if k < len(counts):
                    running += counts[k]
                if 0 <= k-depth-1 < len(counts):
                    running -= counts[k-depth-1]
                expanded.append(running)
            counts = expanded
        return counts

    def box_boundaries(self) -> Ledger:
        """Coefficients of product (1-z^(depth+1)), all positive exponents."""
        out = {0: 1}
        for depth in self.depths:
            out = ledger_multiply(out, {0: 1, depth+1: -1})
        return out

    def coefficient_formula(self, k: int) -> int:
        """Unsigned coefficient using shifted truncated binomial kernels.

        This is an independent formula for envelope(), not its algorithm.
        B_r(t)=binom(t+r,r) for t>=0 and zero otherwise.
        """
        if self.order == 0:
            return int(k == 0)
        t = k+sum(self.x_depths)
        degree = self.order-1
        return sum(value*comb(t-boundary+degree, degree)
                   for boundary, value in self.box_boundaries().items()
                   if t >= boundary)

    def grid(self) -> Grid:
        a, c = len(self.x_depths), len(self.y_depths)
        kmin, _ = self.k_bounds
        return {(k-a, -k-c): sign(k)*count
                for k, count in enumerate(self.envelope(), start=kmin)
                if count}

    def boundary_ledger(self) -> Ledger:
        """Exact numerator of b^order * product in the carry quotient.

        Its stronger raw identity uses (X+Y)^order instead of b^order.
        Endpoint exponents are powers of the SAME q, so correlations and
        collisions are retained. No endpoint is replaced by zero.
        """
        out = {0: 1}
        for n in self.x_depths:
            out = ledger_multiply(out, {0: 1, n+1: -1})
        for m in self.y_depths:
            out = ledger_multiply(out, {0: 1, -m-1: -1})
        return out

    def normal_form(self, base: int = 10) -> dict[int, Fraction]:
        if type(base) is not int or base < 2:
            raise ValueError("Base must be an integer >=2")
        return {k: Fraction(value, base**self.order)
                for k, value in self.boundary_ledger().items()}


class CascadeExpression:
    """Finite integer sums; exact equality is carry equality at a chosen base.

    The factored expression records cutoff choices. Different expressions
    may be equal in the finite carry quotient; normal_form decides that
    equality. It does not identify different approximation families merely
    because their central windows agree.
    """

    def __init__(self, terms: Iterable[tuple[CascadeProduct, int]] = ()):
        out = defaultdict(int)
        for product, coefficient in terms:
            if not isinstance(product, CascadeProduct) or type(coefficient) is not int:
                raise TypeError("Terms must be (CascadeProduct, integer) pairs")
            out[product] += coefficient
        self.terms = clean(out)

    def __add__(self, other: CascadeExpression) -> CascadeExpression:
        return CascadeExpression((*self.terms.items(), *other.terms.items()))

    def __mul__(self, other: CascadeExpression) -> CascadeExpression:
        return CascadeExpression((left*right, a*b)
                                 for left, a in self.terms.items()
                                 for right, b in other.terms.items())

    def grid(self) -> Grid:
        return grid_add(*(grid_scale(product.grid(), coefficient)
                          for product, coefficient in self.terms.items()))

    def normal_form(self, base: int = 10) -> dict[int, Fraction]:
        if type(base) is not int or base < 2:
            raise ValueError("Base must be an integer >=2")
        out = defaultdict(Fraction)
        for product, coefficient in self.terms.items():
            for exponent, value in product.normal_form(base).items():
                out[exponent] += coefficient*value
        return clean(out)

    def equals(self, other: CascadeExpression, base: int = 10) -> bool:
        return self.normal_form(base) == other.normal_form(base)


def poly_trim(values: Iterable[int]) -> Polynomial:
    values = list(values)
    while values and values[-1] == 0:
        values.pop()
    return tuple(values)


def poly_add(*polynomials: Polynomial) -> Polynomial:
    out = [0]*max(map(len, polynomials), default=0)
    for polynomial in polynomials:
        for i, value in enumerate(polynomial):
            out[i] += value
    return poly_trim(out)


def poly_scale(polynomial: Polynomial, value: int) -> Polynomial:
    return poly_trim(value*coefficient for coefficient in polynomial)


def poly_multiply(left: Polynomial, right: Polynomial) -> Polynomial:
    if not left or not right:
        return ()
    out = [0]*(len(left)+len(right)-1)
    for i, a in enumerate(left):
        for j, b in enumerate(right):
            out[i+j] += a*b
    return poly_trim(out)


@dataclass(frozen=True)
class MatchedElement:
    """a(F)+c(F)L at ONE shared cutoff, with integer polynomial coefficients.

    L=L_N, R=R_N, F=LR. The exact relation L^2=base*F*L-F makes
    this a two-component module over Z[F], not a two-dimensional algebra
    over scalar integers or rationals. Increasing powers of F remain.
    """

    a: Polynomial = ()
    c: Polynomial = ()
    base: int = 10

    def __post_init__(self):
        if type(self.base) is not int or self.base < 2:
            raise ValueError("Base must be an integer >=2")
        if any(type(x) is not int for x in (*self.a, *self.c)):
            raise TypeError("Polynomial coefficients must be integers")
        object.__setattr__(self, "a", poly_trim(self.a))
        object.__setattr__(self, "c", poly_trim(self.c))

    def __add__(self, other: MatchedElement) -> MatchedElement:
        if self.base != other.base:
            raise ValueError("Bases differ")
        return MatchedElement(poly_add(self.a, other.a),
                              poly_add(self.c, other.c), self.base)

    def __mul__(self, other: MatchedElement) -> MatchedElement:
        if self.base != other.base:
            raise ValueError("Bases differ")
        ce = poly_multiply(self.c, other.c)
        fce = poly_multiply((0, 1), ce)
        return MatchedElement(
            poly_add(poly_multiply(self.a, other.a), poly_scale(fce, -1)),
            poly_add(poly_multiply(self.a, other.c),
                     poly_multiply(self.c, other.a), poly_scale(fce, self.base)),
            self.base)

    def grid(self, depth: int) -> Grid:
        left = CascadeProduct((depth,), ()).grid()
        fuzzy = CascadeProduct.F(depth, depth).grid()

        def evaluate(polynomial: Polynomial) -> Grid:
            out = {}
            for value in reversed(polynomial):
                out = grid_add(grid_multiply(out, fuzzy), {(0, 0): value})
            return clean(out)

        return grid_add(evaluate(self.a), grid_multiply(evaluate(self.c), left))
