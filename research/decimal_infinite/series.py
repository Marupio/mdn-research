"""Exact finite coefficients of an infinite-decimal MDN expression.

The formal variable z denotes X**-1; Y**-1 = -z/(1-base*z).
Coefficients are rational polynomials in the retained cascade term count n.
An order-M prefix contains coefficients z**0 through z**M, with all later
coefficients UNKNOWN. Agreement of prefixes is not proof of infinite equality.

For a digit stream, these M coefficients depend on exactly its first M digits.
Thus arbitrarily long decimal expansions have a locally finite, exact rule.
No value for z, scalar axis magnitude, or numerical infinity is assigned.
"""

from dataclasses import dataclass
from fractions import Fraction
from functools import lru_cache
from math import comb
from typing import Iterable

Polynomial = tuple[Fraction, ...]


def poly(values: Iterable[int | Fraction]) -> Polynomial:
    values = tuple(values)
    if any(type(v) is not int and not isinstance(v, Fraction) for v in values):
        raise TypeError("Use exact integer or Fraction coefficients")
    result = [Fraction(v) for v in values]
    while result and not result[-1]:
        result.pop()
    return tuple(result)


def poly_add(*values: Polynomial) -> Polynomial:
    result = [Fraction(0)] * max(map(len, values), default=0)
    for value in values:
        for k, coefficient in enumerate(value):
            result[k] += coefficient
    return poly(result)


def poly_scale(value: Polynomial, scale: int | Fraction) -> Polynomial:
    return poly(coefficient * scale for coefficient in value)


def poly_multiply(left: Polynomial, right: Polynomial) -> Polynomial:
    if not left or not right:
        return ()
    result = [Fraction(0)] * (len(left) + len(right) - 1)
    for i, a in enumerate(left):
        for j, b in enumerate(right):
            result[i+j] += a*b
    return poly(result)


def poly_evaluate(value: Polynomial, n: int) -> Fraction:
    result = Fraction(0)
    for coefficient in reversed(value):
        result = n*result + coefficient
    return result


@lru_cache(maxsize=None)
def binomial_polynomial(offset: int, degree: int) -> Polynomial:
    """binom(n+offset, degree), as a polynomial, including its rational factors."""
    result = (Fraction(1),)
    for k in range(degree):
        result = poly_scale(poly_multiply(result, poly((offset-k, 1))), Fraction(1, k+1))
    return result


def _order(value):
    if type(value) is not int or value < 0:
        raise ValueError("Order must be a nonnegative integer")


def _base(value):
    if type(value) is not int or value < 2:
        raise ValueError("Base must be an integer >=2")


@dataclass(frozen=True, eq=False)
class SeriesPrefix:
    """Coefficients through z**order; the remainder is explicitly unspecified.

    Arithmetic keeps the lower input order. Use agrees_through() for a finite
    comparison; there is deliberately no overloaded equality of whole MDNs.
    Each coefficient tuple is in ascending powers of the formal index n.
    """

    coefficients: tuple[Polynomial, ...]

    def __post_init__(self):
        if not self.coefficients:
            raise ValueError("Include at least the constant coefficient")
        object.__setattr__(self, "coefficients", tuple(poly(c) for c in self.coefficients))

    @property
    def order(self):
        return len(self.coefficients)-1

    @classmethod
    def constant(cls, value: int | Fraction, order: int):
        _order(order)
        return cls((poly((value,)),) + ((),)*order)

    def truncate(self, order: int):
        _order(order)
        if order > self.order:
            raise ValueError("Cannot infer unknown higher coefficients")
        return SeriesPrefix(self.coefficients[:order+1])

    def agrees_through(self, other, order: int | None = None):
        if not isinstance(other, SeriesPrefix):
            raise TypeError("Compare two SeriesPrefix objects")
        if order is None:
            order = min(self.order, other.order)
        return self.truncate(order).coefficients == other.truncate(order).coefficients

    def __add__(self, other):
        if type(other) is int or isinstance(other, Fraction):
            other = self.constant(other, self.order)
        if not isinstance(other, SeriesPrefix):
            return NotImplemented
        order = min(self.order, other.order)
        return SeriesPrefix(tuple(poly_add(a, b) for a, b in
                                  zip(self.coefficients[:order+1], other.coefficients[:order+1])))

    __radd__ = __add__

    def __neg__(self):
        return SeriesPrefix(tuple(poly_scale(c, -1) for c in self.coefficients))

    def __sub__(self, other):
        return self + (-other)

    def __rsub__(self, other):
        return (-self) + other

    def __mul__(self, other):
        if type(other) is int or isinstance(other, Fraction):
            other = self.constant(other, self.order)
        if not isinstance(other, SeriesPrefix):
            return NotImplemented
        order = min(self.order, other.order)
        return SeriesPrefix(tuple(poly_add(*(poly_multiply(self.coefficients[j],
                                                           other.coefficients[k-j])
                                             for j in range(k+1))) for k in range(order+1)))

    __rmul__ = __mul__

    def __pow__(self, exponent):
        if type(exponent) is not int or exponent < 0:
            raise ValueError("Power must be a nonnegative integer")
        result = self.constant(1, self.order)
        factor = self
        while exponent:
            if exponent % 2:
                result = result * factor
            exponent //= 2
            if exponent:
                factor = factor * factor
        return result

    def swap_axes(self, base: int = 10):
        """Substitute z -> -z/(1-base*z), retaining the same precision."""
        _base(base)
        result = [self.coefficients[0]]
        for m in range(1, self.order+1):
            result.append(poly_add(*(poly_scale(self.coefficients[k],
                                                (-1)**k * comb(m-1, k-1) * base**(m-k))
                                     for k in range(1, m+1))))
        return SeriesPrefix(tuple(result))

    def evaluate_count(self, n: int):
        """Substitute a finite positive term count, without evaluating z."""
        if type(n) is not int or n < 1:
            raise ValueError("Cascade term count must be a positive integer")
        return tuple(poly_evaluate(c, n) for c in self.coefficients)


def digit_prefix(digits: Iterable[int], base: int = 10, direction: str = "X"):
    """First M digits determine the exact formal series through order M.

    Any continuation of those digits gives these same coefficients. This
    accepts digit values rather than floats or a presumed ordinary magnitude.
    Use finite_word() when an entry is known to terminate in zeros.
    """
    _base(base)
    digits = tuple(digits)
    if any(type(d) is not int or not 0 <= d < base for d in digits):
        raise ValueError("Digits must be integers from 0 through base-1")
    if direction not in ("X", "Y"):
        raise ValueError("Direction must be X or Y")
    coefficients = [()]
    for m in range(1, len(digits)+1):
        coefficients.append(poly_add(*(
            poly_scale(binomial_polynomial(k-1, m),
                       digits[k-1] * (-base)**(m-k) * comb(m-1, k-1))
            for k in range(1, m+1) if digits[k-1]
        )))
    result = SeriesPrefix(tuple(coefficients))
    return result if direction == "X" else result.swap_axes(base)


def finite_word(digits: Iterable[int], order: int, base: int = 10, direction: str = "X"):
    """A known terminating fractional word, with its subsequent digits zero."""
    _order(order)
    digits = tuple(digits)
    # Validate the complete supplied word, even if the requested order is shorter.
    _base(base)
    if any(type(d) is not int or not 0 <= d < base for d in digits):
        raise ValueError("Digits must be integers from 0 through base-1")
    visible = (digits + (0,)*max(0, order-len(digits)))[:order]
    return digit_prefix(visible, base, direction)
