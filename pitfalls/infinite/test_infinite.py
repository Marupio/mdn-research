"""Independent exact checks for the finite identities used in infinite analysis."""

from fractions import Fraction as F
from math import comb, factorial
import unittest

from experiments import (
    add, atom, carry_one, directional_partial, evaluate, geometric_inverse_partial,
    integer_escape, multiply, power, raw_l1, scale, weighted_l1,
    alternating_polynomial, recurrence_residual,
)


class InfiniteIdentityTests(unittest.TestCase):
    def test_geometric_tail_identities_in_unconstrained_laurent_ring(self):
        xy = add(atom(1, 0), atom(0, 1))
        for n in range(17):
            l, r = directional_partial(n), directional_partial(n, "right")
            self.assertEqual(multiply(xy, l), add(atom(), atom(-n-1, n+1, (-1)**n)))
            self.assertEqual(multiply(xy, r), add(atom(), atom(n+1, -n-1, (-1)**n)))
            for x, y in ((F(3), F(7)), (F(5), F(5)), (F(-2), F(7))):
                self.assertEqual(evaluate(l, x, y), (1-(-y/x)**(n+1))/(x+y))

    def test_every_mixed_product_coefficient_counts_contributing_pairs(self):
        for n in range(14):
            product = multiply(directional_partial(n), directional_partial(n, "right"))
            # With offset d=ell-k, there are N+1-|d| matching pairs.
            expected = {(d-1, -d-1): F((-1)**abs(d)*(n+1-abs(d))) for d in range(-n, n+1)}
            self.assertEqual(product, expected)
            self.assertEqual(product[(-1, -1)], n+1)

    def test_carry_sequence_matches_independent_binomial_expansion(self):
        for base in (1, 2, 3, 10):
            for n in range(13):
                p = carry_one(n, base)
                independent = scale(power(add(atom(1, 0), atom(0, 1)), n), F(1, base**n))
                self.assertEqual(p, independent)
                self.assertEqual(raw_l1(p), F(2, base)**n)
                x, y = F(base, 3), F(2*base, 3)
                self.assertEqual(evaluate(p, x, y), 1)
                self.assertEqual(weighted_l1(p, x, y), 1)

    def test_integer_escape_has_integer_digits_and_disappears_in_finite_window(self):
        base = 10
        for n in range(13):
            e = integer_escape(n, base)
            independent = multiply(atom(-n, 0), power(add(atom(coefficient=F(base)), atom(0, 1, F(-1))), n))
            self.assertEqual(e, independent)
            self.assertTrue(all(c.denominator == 1 for c in e.values()))
            self.assertEqual(evaluate(e, F(7), F(3)), 1)
            self.assertEqual(raw_l1(e), (base+1)**n)
            if n > 3:
                self.assertTrue(all(i not in range(-3, 4) for i, j in e))

    def test_geometric_inverse_exposes_closed_ideal_collapse(self):
        for base in (3, 10):
            relation = add(atom(coefficient=F(base)), atom(1, 0, F(-1)), atom(0, 1, F(-1)))
            for n in range(11):
                residual = add(atom(), scale(multiply(relation, geometric_inverse_partial(n, base)), F(-1)))
                self.assertEqual(residual, carry_one(n+1, base))
                self.assertEqual(raw_l1(residual), F(2, base)**(n+1))

    def test_chart_error_shrinks_on_only_one_side(self):
        x, y = F(7), F(3)
        previous_l, previous_r = None, None
        for n in range(12):
            le = abs(evaluate(directional_partial(n), x, y)-F(1, 10))
            re = abs(evaluate(directional_partial(n, "right"), x, y)-F(1, 10))
            self.assertEqual(le, F(3, 7)**(n+1)/10)
            self.assertEqual(re, F(7, 3)**(n+1)/10)
            if previous_l is not None:
                self.assertLess(le, previous_l)
                self.assertGreater(re, previous_r)
            previous_l, previous_r = le, re

    def test_abel_boundary_has_finite_value_but_divergent_central_digit(self):
        for r in (F(0), F(1, 2), F(9, 10), F(99, 100)):
            for n in range(9):
                l, right = directional_partial(n, damping=r), directional_partial(n, "right", r)
                product = multiply(l, right)
                self.assertEqual(product[(-1, -1)], sum((r**(2*k) for k in range(n+1)), F(0)))
                self.assertEqual(product[(-1, -1)], (1-r**(2*n+2))/(1-r*r))
                self.assertEqual(evaluate(l, F(5), F(5)), (1-(-r)**(n+1))/(5*(1+r)))
                self.assertEqual(evaluate(product, F(5), F(5)), evaluate(l, F(5), F(5))**2)
        # Infinite-series closed forms: scalar products approach 1/100 from above;
        # central coefficients grow without bound as r increases to one.
        r1, r2 = F(99, 100), F(999, 1000)
        self.assertGreater(1/(1-r2*r2), 1/(1-r1*r1))
        self.assertLess(1/(25*(1+r2)**2)-F(1, 100), 1/(25*(1+r1)**2)-F(1, 100))

    def test_cesaro_averages_do_not_preserve_same_cutoff_products(self):
        values = [evaluate(directional_partial(n), F(5), F(5)) for n in range(20)]
        self.assertEqual(values, [F(1, 5), F(0)]*10)
        mean = sum(values)/20
        mean_product = sum(v*v for v in values)/20
        self.assertEqual(mean, F(1, 10))
        self.assertEqual(mean**2, F(1, 100))
        self.assertEqual(mean_product, F(1, 50))
        self.assertNotEqual(mean_product, mean**2)

    def test_higher_order_fuzzy_recurrence_uses_polynomial_degrees(self):
        for order in range(1, 6):
            for degree in range(order+1):
                coefficients = (F(0),)*degree+(F(1),)
                # Independent check: apply the local neighbour-sum operation
                # repeatedly to a long finite window, retaining only its interior.
                sequence = {k: alternating_polynomial(k, coefficients) for k in range(-12, 13)}
                for _ in range(order):
                    sequence = {k: sequence[k]+sequence[k-1] for k in sequence if k-1 in sequence}
                for k in range(-6, 7):
                    self.assertEqual(sequence[k], recurrence_residual(k, coefficients, order))
                    expected = 0 if degree < order else (-1 if k % 2 else 1)*factorial(order)
                    self.assertEqual(sequence[k], expected)


if __name__ == "__main__":
    unittest.main()
