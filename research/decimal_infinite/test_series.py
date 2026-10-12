"""Independent coefficient checks for the infinite-decimal construction."""

from collections import defaultdict
from fractions import Fraction
from itertools import product
from math import comb, isqrt
import unittest

from series import SeriesPrefix, digit_prefix, finite_word, binomial_polynomial, poly_evaluate


def generalized_binomial(top, bottom):
    result = Fraction(1)
    for k in range(bottom):
        result *= Fraction(top-k, k+1)
    return result


def raw_word(digits, n, direction):
    grid = defaultdict(int)
    for k, digit in enumerate(digits, 1):
        for j in range(n):
            point = (-k-j, j) if direction == "X" else (j, -k-j)
            grid[point] += digit * (-1)**j * comb(k+j-1, j)
    return {p: d for p, d in grid.items() if d}


def raw_multiply(left, right):
    grid = defaultdict(int)
    for (x, y), a in left.items():
        for (u, v), b in right.items():
            grid[x+u, y+v] += a*b
    return {p: d for p, d in grid.items() if d}


def raw_to_series(grid, order, base):
    # Substitute X=1/z,Y=base-1/z directly into independent raw grid cells.
    coefficients = defaultdict(Fraction)
    for (x, y), digit in grid.items():
        start = -x-y
        for m in range(start, order+1):
            r = m-start
            coefficients[m] += (digit * (-1 if y % 2 else 1)
                                * generalized_binomial(y, r) * (-base)**r)
    assert not any(value for m, value in coefficients.items() if m < 0)
    return tuple(coefficients[m] for m in range(order+1))


class DecimalSeriesTests(unittest.TestCase):
    def test_binomial_polynomial_at_integer_indices(self):
        for offset, degree, n in product(range(5), range(9), range(1, 12)):
            expected = comb(n+offset, degree) if degree <= n+offset else 0
            self.assertEqual(poly_evaluate(binomial_polynomial(offset, degree), n), expected)

    def test_infinite_prefix_coefficients_against_direct_finite_grids(self):
        # Expand grid coordinates independently of the closed coefficient formula.
        sources = ((1,), (3, 3, 3, 3), (1, 4, 2, 8, 5, 7), (4, 1, 4, 2, 1, 3, 5, 6))
        for digits, n, direction, order in product(sources, (1, 2, 4, 9), ("X", "Y"), (2, 5, 10)):
            expression = finite_word(digits, order, direction=direction)
            self.assertEqual(expression.evaluate_count(n),
                             raw_to_series(raw_word(digits, n, direction), order, 10))
        for base, direction, n in product((2, 3, 16), ("X", "Y"), (1, 3, 8)):
            digits = (base-1, 1, 0, base-1)
            self.assertEqual(finite_word(digits, 8, base, direction).evaluate_count(n),
                             raw_to_series(raw_word(digits, n, direction), 8, base))

    def test_unknown_digits_cannot_affect_known_coefficients(self):
        prefix = (1, 4, 2, 8, 5, 7)
        for direction in ("X", "Y"):
            short = digit_prefix(prefix, direction=direction)
            for extension in ((0,)*5, (9,)*5, (1, 2, 3, 4, 5)):
                long = digit_prefix(prefix+extension, direction=direction)
                self.assertTrue(short.agrees_through(long))
            with self.assertRaises(ValueError):
                short.truncate(7)

    def test_nonperiodic_digits_have_exact_prefix_stability(self):
        digits = tuple(map(int, str(isqrt(2*10**40)-10**20).zfill(20)))
        self.assertEqual(digits[:8], (4, 1, 4, 2, 1, 3, 5, 6))
        full = digit_prefix(digits)
        for k in (1, 5, 10, 15):
            self.assertTrue(digit_prefix(digits[:k]).agrees_through(full, k))

    def test_first_place_boundary_and_second_place_are_distinct(self):
        order = 12
        left = finite_word((1,), order)
        right = finite_word((1,), order, direction="Y")
        endpoint = SeriesPrefix(tuple(
            tuple(c * (-10)**m for c in binomial_polynomial(0, m))
            for m in range(order+1)))
        self.assertTrue((1-10*left).agrees_through(endpoint))
        self.assertTrue((endpoint * (1-10*right)).agrees_through(SeriesPrefix.constant(1, order)))
        self.assertTrue((10*left*right).agrees_through(left+right))
        self.assertFalse(finite_word((0, 1), order).agrees_through(left**2))

    def test_axis_exchange_and_mixed_products(self):
        order = 10
        for base in (3, 10):
            left = finite_word((1, 2, 1), order, base)
            right = finite_word((2, 1), order, base, "Y")
            self.assertTrue(left.swap_axes(base).swap_axes(base).agrees_through(left))
            self.assertTrue((left*right).swap_axes(base).agrees_through(
                left.swap_axes(base)*right.swap_axes(base)))
            for n in (1, 3, 7):
                expected = raw_multiply(raw_word((1, 2, 1), n, "X"),
                                        raw_word((2, 1), n, "Y"))
                self.assertEqual((left*right).evaluate_count(n), raw_to_series(expected, order, base))

    def test_no_nine_repeating_identification_with_integer_one(self):
        nines = digit_prefix((9,)*12)
        one = SeriesPrefix.constant(1, 12)
        self.assertFalse(nines.agrees_through(one, 0))
        self.assertEqual(nines.evaluate_count(1), (Fraction(0),)+(Fraction(9),)*12)
        # At one cascade term, X digits have exactly their original power series.
        varied = (1, 4, 2, 8, 5, 7)
        self.assertEqual(digit_prefix(varied).evaluate_count(1), (Fraction(0),)+tuple(map(Fraction, varied)))

    def test_precision_is_not_silently_increased(self):
        short = digit_prefix((3,)*3)
        long = digit_prefix((3,)*8)
        self.assertEqual((short*long).order, 3)
        self.assertEqual((short+long).order, 3)
        self.assertTrue((long-long).agrees_through(SeriesPrefix.constant(0, 8)))
        self.assertTrue((long**0).agrees_through(SeriesPrefix.constant(1, 8)))
        for bad in (-1, True, 0.5):
            with self.assertRaises(ValueError):
                long.truncate(bad)
        with self.assertRaises(ValueError):
            digit_prefix((10,))
        with self.assertRaises(ValueError):
            finite_word((1, 20), 1)
        with self.assertRaises(ValueError):
            long.evaluate_count(0)


if __name__ == "__main__":
    unittest.main()
