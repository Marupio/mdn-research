"""Exact moving-boundary expressions for affine-cutoff MDN families.

The ambient ring is Q[n, q^{+/-1}, E^{+/-1}], evaluated by E -> q**n.
The positive integer n is an index, not a scalar axis magnitude. Coefficients
may be polynomials in n because higher fractional places require them.

Symbolic equality means equality for every sufficiently large integer n after
that evaluation. No moving endpoint is discarded. This ring uses rational
bookkeeping: only its appropriate integer-grid-generated subalgebra describes
the original backend without extending the permitted scalar operations.
Neither this ring nor evaluate() selects an ordinary topological limit.
"""

from __future__ import annotations

from collections import defaultdict
from fractions import Fraction
from types import MappingProxyType
from typing import Iterable, Mapping

Scalar = int | Fraction
Polynomial = tuple[Fraction, ...]
Exponent = tuple[int, int]  # (power of E, power of q)


def _fraction(value: Scalar) -> Fraction:
    if type(value) is int or isinstance(value, Fraction):
        return Fraction(value)
    raise TypeError("Coefficients must be exact integers or Fractions")


def _polynomial(values: Scalar | Iterable[Scalar]) -> Polynomial:
    if type(values) is int or isinstance(values, Fraction):
        values = (values,)
    result = [_fraction(value) for value in values]
    while result and not result[-1]:
        result.pop()
    return tuple(result)


def _poly_add(left: Polynomial, right: Polynomial) -> Polynomial:
    result = [Fraction(0)] * max(len(left), len(right))
    for i, coefficient in enumerate(left):
        result[i] += coefficient
    for i, coefficient in enumerate(right):
        result[i] += coefficient
    return _polynomial(result)


def _poly_multiply(left: Polynomial, right: Polynomial) -> Polynomial:
    if not left or not right:
        return ()
    result = [Fraction(0)] * (len(left) + len(right) - 1)
    for i, a in enumerate(left):
        for j, b in enumerate(right):
            result[i + j] += a * b
    return _polynomial(result)


def _poly_evaluate(polynomial: Polynomial, n: int) -> Fraction:
    result = Fraction(0)
    for coefficient in reversed(polynomial):
        result = result * n + coefficient
    return result


class BoundaryExpression:
    """A finite sum of p(n) * E**s * q**c with exact rational p.

    Construct from ``{(s, c): (p0, p1, ...)}``, where coefficients are in
    ascending powers of n; a scalar coefficient is also accepted. Terms and
    coefficient tuples are immutable. Arithmetic accepts integer/Fraction
    constants; division is supported only by nonzero numerical constants.

    Distinct (s,c) can coincide at particular n. Such isolated coincidences
    do not imply symbolic equality: distinct affine exponents separate for
    all sufficiently large n, while a nonzero polynomial has finitely many
    roots. This proves the eventual-equality interpretation of the model.
    """

    __slots__ = ("_terms",)

    def __init__(
        self,
        terms: Mapping[Exponent, Scalar | Iterable[Scalar]] | None = None,
    ):
        normalized: dict[Exponent, Polynomial] = {}
        for exponent, coefficient in (terms or {}).items():
            if len(exponent) != 2 or any(type(value) is not int for value in exponent):
                raise TypeError("Exponents must be pairs of integers (E power, q power)")
            polynomial = _polynomial(coefficient)
            if polynomial:
                normalized[tuple(exponent)] = polynomial
        self._terms = MappingProxyType(normalized)

    @property
    def terms(self) -> Mapping[Exponent, Polynomial]:
        return self._terms

    @classmethod
    def constant(cls, value: Scalar) -> BoundaryExpression:
        return cls({(0, 0): _fraction(value)})

    @classmethod
    def index(cls) -> BoundaryExpression:
        """The polynomial coefficient n."""
        return cls({(0, 0): (0, 1)})

    @classmethod
    def monomial(
        cls, slope: int = 0, offset: int = 0,
        coefficient: Scalar | Iterable[Scalar] = 1,
    ) -> BoundaryExpression:
        """coefficient(n) * E**slope * q**offset, allowing signed powers."""
        return cls({(slope, offset): coefficient})

    @staticmethod
    def _coerce(other) -> BoundaryExpression | None:
        if isinstance(other, BoundaryExpression):
            return other
        if type(other) is int or isinstance(other, Fraction):
            return BoundaryExpression.constant(other)
        return None

    def __eq__(self, other) -> bool:
        other = self._coerce(other)
        if other is None:
            return NotImplemented
        return self.terms == other.terms

    def __bool__(self) -> bool:
        return bool(self.terms)

    def __repr__(self) -> str:
        return f"BoundaryExpression({dict(sorted(self.terms.items()))!r})"

    def __neg__(self) -> BoundaryExpression:
        return BoundaryExpression({exponent: tuple(-value for value in coefficient)
                                   for exponent, coefficient in self.terms.items()})

    def __add__(self, other) -> BoundaryExpression:
        other = self._coerce(other)
        if other is None:
            return NotImplemented
        result = dict(self.terms)
        for exponent, coefficient in other.terms.items():
            result[exponent] = _poly_add(result.get(exponent, ()), coefficient)
        return BoundaryExpression(result)

    __radd__ = __add__

    def __sub__(self, other) -> BoundaryExpression:
        other = self._coerce(other)
        if other is None:
            return NotImplemented
        return self + (-other)

    def __rsub__(self, other) -> BoundaryExpression:
        other = self._coerce(other)
        if other is None:
            return NotImplemented
        return other + (-self)

    def __mul__(self, other) -> BoundaryExpression:
        other = self._coerce(other)
        if other is None:
            return NotImplemented
        result: dict[Exponent, Polynomial] = {}
        for (s, c), a in self.terms.items():
            for (t, d), b in other.terms.items():
                exponent = (s + t, c + d)
                coefficient = _poly_multiply(a, b)
                result[exponent] = _poly_add(result.get(exponent, ()), coefficient)
        return BoundaryExpression(result)

    __rmul__ = __mul__

    def __truediv__(self, divisor: Scalar) -> BoundaryExpression:
        divisor = _fraction(divisor)
        if not divisor:
            raise ZeroDivisionError("Cannot divide a boundary expression by zero")
        return self * (1 / divisor)

    def __pow__(self, exponent: int) -> BoundaryExpression:
        if type(exponent) is not int or exponent < 0:
            raise ValueError("Power must be a nonnegative integer; use monomial for signed shifts")
        result = BoundaryExpression.constant(1)
        factor = self
        while exponent:
            if exponent & 1:
                result = result * factor
            exponent //= 2
            if exponent:
                factor = factor * factor
        return result

    def swap_axes(self) -> BoundaryExpression:
        """q -> q**-1 and E -> E**-1, with coefficient index n unchanged."""
        return BoundaryExpression({(-s, -c): coefficient
                                   for (s, c), coefficient in self.terms.items()})

    def evaluate(self, n: int) -> dict[int, Fraction]:
        """Substitute positive integer n and E=q**n, retaining every q term.

        This is formal substitution, not scalar evaluation of q or a limit.
        A directional fraction with count a*n+c represents an actual finite
        cascade only when that count is positive at the chosen index.
        """
        if type(n) is not int or n < 1:
            raise ValueError("Index n must be a positive integer")
        result = defaultdict(Fraction)
        for (slope, offset), coefficient in self.terms.items():
            result[slope * n + offset] += _poly_evaluate(coefficient, n)
        return {exponent: value for exponent, value in result.items() if value}


def directional_fraction(
    place: int,
    base: int = 10,
    direction: str = "X",
    slope: int = 1,
    offset: int = 0,
) -> BoundaryExpression:
    """Exact family for one digit at fractional place k with ell=a*n+c terms.

    The X result is

      b**(-k) [1 - q**ell sum(r=0..k-1)
                         binom(ell+r-1,r) (1-q)**r].

    The Y result substitutes q**-1. Each binomial is retained as an exact
    polynomial in n. Nonnegative slope with eventually positive ell is required;
    evaluating it as a finite input requires ell >= 1. The fraction label is
    input syntax, not a declaration that this family equals scalar b**(-k).
    """
    if type(place) is not int or place < 1:
        raise ValueError("Fractional place must be a positive integer")
    if type(base) is not int or base < 2:
        raise ValueError("Base must be an integer >=2")
    if (type(slope) is not int or slope < 0 or type(offset) is not int
            or (slope == 0 and offset < 1)):
        raise ValueError("Require an integer affine term count that is eventually positive")
    if direction not in ("X", "Y"):
        raise ValueError("Direction must be 'X' or 'Y'")

    q = BoundaryExpression.monomial(offset=1)
    endpoint = BoundaryExpression.monomial(slope, offset)
    ell = slope * BoundaryExpression.index() + offset
    kernel = BoundaryExpression.constant(0)
    coefficient = BoundaryExpression.constant(1)
    difference_power = BoundaryExpression.constant(1)
    for r in range(place):
        if r:
            coefficient = coefficient * (ell + r - 1) / r
            difference_power = difference_power * (1 - q)
        kernel = kernel + coefficient * difference_power
    result = (1 - endpoint * kernel) / base**place
    return result if direction == "X" else result.swap_axes()
