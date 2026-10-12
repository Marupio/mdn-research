"""Independent exact tests for the moving-boundary algebra.

Run: python3 -m unittest discover -s research/fuzzy -p 'test_boundary.py' -v
"""

from collections import defaultdict
from fractions import Fraction
from itertools import product
from math import comb
import unittest

from boundary import BoundaryExpression as B, directional_fraction


def multiply_laurent(left, right):
    result = defaultdict(Fraction)
    for i, a in left.items():
        for j, b in right.items():
            result[i + j] += a * b
    return {exponent: coefficient for exponent, coefficient in result.items() if coefficient}


def direct_fraction(place, length, base, direction):
    """Independent finite-series sum, not the endpoint formula under test."""
    orientation = 1 if direction == "X" else -1
    prefactor = {orientation * j: Fraction((-1)**j * comb(place, j), base**place)
                 for j in range(place + 1)}
    series = {orientation * j: Fraction(comb(place + j - 1, j))
              for j in range(length)}
    return multiply_laurent(prefactor, series)


class BoundaryTests(unittest.TestCase):
    def test_normalization_and_exact_coefficients(self):
        value = B({(0, 1): (Fraction(1, 2), 2, 0), (4, 3): (0, 0)})
        self.assertEqual(value.terms, {(0, 1): (Fraction(1, 2), Fraction(2))})
        self.assertEqual(value.evaluate(3), {1: Fraction(13, 2)})
        self.assertFalse(value - value)
        self.assertEqual(B(), 0)
        self.assertEqual(1, B.constant(1))
        with self.assertRaises(TypeError):
            value.terms[(0, 1)] = (1,)

    def test_arithmetic_and_polynomial_coefficients(self):
        n = B.index()
        self.assertEqual((n + 1) * (n - 1), B({(0, 0): (-1, 0, 1)}))
        self.assertEqual((n + 1)**3, B({(0, 0): (1, 3, 3, 1)}))
        self.assertEqual(3 * n / 2 + n / 2, 2 * n)
        self.assertEqual(4 - n, -(n - 4))
        self.assertEqual(B()**0, 1)

    def test_endpoint_inverse_preserves_the_returning_boundary(self):
        endpoint = B.monomial(1)
        reciprocal = B.monomial(-1)
        self.assertEqual(endpoint * reciprocal, 1)
        self.assertNotEqual(endpoint, 0)
        self.assertNotEqual(reciprocal, 0)
        for n in (1, 2, 21, 100):
            self.assertEqual((endpoint * reciprocal).evaluate(n), {0: Fraction(1)})

    def test_isolated_evaluation_equality_is_not_family_equality(self):
        endpoint = B.monomial(1)
        q = B.monomial(offset=1)
        self.assertEqual(endpoint.evaluate(1), q.evaluate(1))
        self.assertNotEqual(endpoint, q)
        self.assertNotEqual(endpoint.evaluate(2), q.evaluate(2))
        self.assertEqual((endpoint - q).evaluate(1), {})
        self.assertNotEqual(endpoint - q, 0)
        self.assertEqual(((B.index() - 2) * endpoint).evaluate(2), {})
        self.assertNotEqual((B.index() - 2) * endpoint, 0)

    def test_evaluation_respects_multiplication(self):
        choices = [B({(0, 0): (1, 2), (1, -2): 3}),
                   B({(-1, 4): (-1, 0, 1), (2, -1): Fraction(1, 3)}),
                   B({(0, 0): (0, 1), (1, -1): -1})]
        for left, right, n in product(choices, choices, (1, 2, 5, 21)):
            with self.subTest(n=n, left=left, right=right):
                self.assertEqual((left * right).evaluate(n),
                                 multiply_laurent(left.evaluate(n), right.evaluate(n)))

    def test_fractional_places_against_direct_finite_series(self):
        # 810 independent coefficient-dictionary comparisons.
        for place, base, direction, slope, offset, n in product(
            range(1, 6), (2, 3, 10), ("X", "Y"), (1, 2, 3), (-2, 0, 2), (3, 5, 8)
        ):
            with self.subTest(k=place, b=base, d=direction, a=slope, c=offset, n=n):
                length = slope * n + offset
                self.assertGreaterEqual(length, 1)
                expression = directional_fraction(place, base, direction, slope, offset)
                self.assertEqual(expression.evaluate(n), direct_fraction(place, length, base, direction))

    def test_first_place_is_previous_boundary_family(self):
        for base, slope, offset in product((2, 3, 10), (1, 2, 3), (-2, 0, 2)):
            endpoint = B.monomial(slope, offset)
            self.assertEqual(base * directional_fraction(1, base, "X", slope, offset), 1 - endpoint)
            self.assertEqual(base * directional_fraction(1, base, "Y", slope, offset),
                             1 - endpoint.swap_axes())

    def test_constant_positive_term_counts_are_allowed(self):
        for place, length, direction, n in product(range(1, 6), (1, 3, 7), ("X", "Y"), (1, 5)):
            expression = directional_fraction(place, direction=direction, slope=0, offset=length)
            self.assertEqual(expression.evaluate(n), direct_fraction(place, length, 10, direction))

    def test_axis_exchange_is_an_involution_and_multiplicative(self):
        left = directional_fraction(3, slope=2, offset=-1)
        right = directional_fraction(2, direction="Y", offset=2)
        self.assertEqual(left.swap_axes().swap_axes(), left)
        self.assertEqual(left.swap_axes(), directional_fraction(3, direction="Y", slope=2, offset=-1))
        self.assertEqual((left * right).swap_axes(), left.swap_axes() * right.swap_axes())

    def test_matched_products_and_two_component_relations(self):
        endpoint, reciprocal = B.monomial(1), B.monomial(-1)
        for base in (2, 3, 10):
            left = directional_fraction(1, base)
            right = directional_fraction(1, base, "Y")
            fuzzy = left * right
            self.assertEqual(base * fuzzy, left + right)
            self.assertEqual(left**2, base * fuzzy * left - fuzzy)
            self.assertEqual(fuzzy * right, base * fuzzy**2 - fuzzy * left)
            self.assertEqual(base**2 * fuzzy, 2 - endpoint - reciprocal)
            self.assertEqual(base**3 * fuzzy * left, 3 - 3 * endpoint + endpoint**2 - reciprocal)
            self.assertEqual(base**3 * fuzzy * right, 3 - 3 * reciprocal + reciprocal**2 - endpoint)
            self.assertEqual(base**4 * fuzzy**2, 6 - 4 * (endpoint + reciprocal) + endpoint**2 + reciprocal**2)

    def test_independent_affine_cutoffs_keep_every_product_endpoint(self):
        left = directional_fraction(1, slope=2, offset=1)
        right = directional_fraction(1, direction="Y", slope=3, offset=-2)
        extra_left = directional_fraction(1, slope=1, offset=4)
        extra_right = directional_fraction(1, direction="Y", slope=2, offset=3)
        fuzzy = left * right
        expressions = (fuzzy, fuzzy * extra_left, fuzzy * extra_right, fuzzy**2)
        factor_sets = ((left, right), (left, right, extra_left),
                       (left, right, extra_right), (left, right, left, right))
        for expression, factors, n in ((e, f, n) for e, f in zip(expressions, factor_sets) for n in (3, 7, 21)):
            expected = {0: Fraction(1)}
            for factor in factors:
                expected = multiply_laurent(expected, factor.evaluate(n))
            self.assertEqual(expression.evaluate(n), expected)

    def test_decimal_addition_retains_boundary_and_second_place_differs(self):
        left = directional_fraction(1)
        hundredth = directional_fraction(2)
        endpoint = B.monomial(1)
        q = B.monomial(offset=1)
        n = B.index()
        self.assertEqual(9 * left + left, 1 - endpoint)
        self.assertNotEqual(9 * left + left, 1)
        self.assertEqual(9 * hundredth + hundredth, left - n * (1 - q) * endpoint / 10)
        self.assertNotEqual(9 * hundredth + hundredth, left)
        self.assertNotEqual(hundredth, left**2)
        self.assertEqual(100 * hundredth, 1 - endpoint * (1 + n * (1 - q)))
        self.assertEqual((10 * left).evaluate(21), {0: Fraction(1), 21: Fraction(-1)})

    def test_invalid_inputs_rejected_without_float_coercion(self):
        for arguments in ((0,), (1, 1), (1, 10, "Z"), (1, 10, "X", 0),
                          (1, 10, "X", -1, 5), (1, 10, "X", 0, -1), (1, 10, "X", 1, 0.5)):
            with self.assertRaises(ValueError):
                directional_fraction(*arguments)
        for n in (0, -1, 1.5, True):
            with self.assertRaises(ValueError):
                B.index().evaluate(n)
        with self.assertRaises(TypeError):
            B({(0, 0): 0.1})
        with self.assertRaises(TypeError):
            B({(0, 1.5): 1})
        with self.assertRaises(TypeError):
            B.constant(True)
        with self.assertRaises(ZeroDivisionError):
            B.constant(1) / 0
        with self.assertRaises(ValueError):
            B.constant(1)**-1


if __name__ == "__main__":
    unittest.main()
