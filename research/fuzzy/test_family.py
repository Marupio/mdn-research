"""Independent finite checks of the theorems in README.md and PROOFS.md."""

from collections import Counter
from fractions import Fraction
from itertools import product
from math import comb, gcd, prod
import unittest

from family import (
    CascadeExpression, CascadeProduct, MatchedElement, affine_boundary_ledger, carry_equal, carry_zero,
    grid_add, grid_multiply, grid_power, grid_scale, q_grid,
)


def direct_cascade(depth, direction):
    cells = {(-k-1, k): -1 if k % 2 else 1 for k in range(depth+1)}
    return cells if direction == "X" else {(y, x): d for (x, y), d in cells.items()}


def direct_product(xs, ys):
    out = {(0, 0): 1}
    for direction, depths in (("X", xs), ("Y", ys)):
        for depth in depths:
            out = grid_multiply(out, direct_cascade(depth, direction))
    return out


class FamilyTests(unittest.TestCase):
    def test_definition_and_depth_validation(self):
        self.assertEqual(CascadeProduct().grid(), {(0, 0): 1})
        self.assertEqual(CascadeProduct().boundary_ledger(), {0: 1})
        for bad in (-1, 1.5, True):
            with self.assertRaises(ValueError):
                CascadeProduct((bad,), ())
        for base in (0, 1, True, 2.5):
            with self.assertRaises(ValueError):
                CascadeProduct.F(2, 3).normal_form(base)
            with self.assertRaises(ValueError):
                CascadeExpression().normal_form(base)

    def test_box_formula_against_independent_tuple_enumeration(self):
        # The oracle enumerates digit choices, not convolutions of box arrays.
        for order in range(1, 5):
            for xs_count in range(order+1):
                for depths in product(range(3), repeat=order):
                    xs, ys = depths[:xs_count], depths[xs_count:]
                    pattern = CascadeProduct(xs, ys)
                    counts = Counter(sum(choice[xs_count:])-sum(choice[:xs_count])
                                     for choice in product(*(range(h+1) for h in depths)))
                    lo, hi = pattern.k_bounds
                    self.assertEqual(pattern.envelope(), [counts[k] for k in range(lo, hi+1)])
                    self.assertEqual(sum(pattern.envelope()), prod(h+1 for h in depths))
                    self.assertEqual(pattern.envelope(), pattern.envelope()[::-1])
                    for k in range(lo-1, hi+2):
                        self.assertEqual(pattern.coefficient_formula(k), counts[k])

    def test_requested_grids_against_direct_geometric_multiplication(self):
        for n, m, p, q in product(range(4), repeat=4):
            f = CascadeProduct.F(n, m)
            for pattern in (f, f*CascadeProduct((p,), ()),
                            f*CascadeProduct((), (q,)), f*f,
                            f*CascadeProduct.F(p, q)):
                self.assertEqual(pattern.grid(), direct_product(pattern.x_depths, pattern.y_depths))

    def test_raw_boundary_identity_before_any_carry_quotient(self):
        neighbour_sum = {(1, 0): 1, (0, 1): 1}
        examples = [CascadeProduct(), CascadeProduct.F(0, 0)]
        for n, m, p, q in ((1, 2, 3, 4), (2, 2, 2, 2), (0, 3, 0, 1)):
            f = CascadeProduct.F(n, m)
            examples += [f, f*CascadeProduct((p,), ()), f*CascadeProduct((), (q,)), f*f]
        for pattern in examples:
            actual = grid_multiply(grid_power(neighbour_sum, pattern.order), pattern.grid())
            self.assertEqual(actual, q_grid(pattern.boundary_ledger()))

    def test_base_carry_identity_and_formal_normal_form(self):
        for base in (2, 3, 10, 16):
            for n, m in product(range(5), repeat=2):
                f = CascadeProduct.F(n, m)
                self.assertTrue(carry_equal(grid_scale(f.grid(), base**2),
                                            q_grid(f.boundary_ledger()), base))
                self.assertEqual(f.normal_form(base),
                                 {k: Fraction(v, base**2) for k, v in f.boundary_ledger().items()})
                self.assertFalse(carry_zero(f.grid(), base))

    def test_triangle_and_trapezoid_formula(self):
        for n, m in product(range(12), repeat=2):
            f = CascadeProduct.F(n, m)
            for k in range(-n-1, m+2):
                expected = max(0, min(n, m-k)-max(0, -k)+1)
                self.assertEqual(f.coefficient_formula(k), expected)

    def test_matched_central_coefficients(self):
        for n in range(41):
            f = CascadeProduct.F(n, n)
            self.assertEqual(f.grid()[(-1, -1)], n+1)
            self.assertEqual((f*CascadeProduct((n,), ())).grid()[(-2, -1)], comb(n+2, 2))
            self.assertEqual((f*CascadeProduct((), (n,))).grid()[(-1, -2)], comb(n+2, 2))
            self.assertEqual((f*f).grid()[(-2, -2)], (n+1)*(2*n*n+4*n+3)//3)

    def test_matched_and_unequal_short_cascade_reduction(self):
        neighbour_sum = {(1, 0): 1, (0, 1): 1}
        for n, m in product(range(9), repeat=2):
            f = CascadeProduct.F(n, m)
            correction = (direct_cascade(m-n-1, "Y") if m > n else
                          direct_cascade(n-m-1, "X") if n > m else {})
            rhs = grid_add(direct_cascade(n, "X"), direct_cascade(m, "Y"),
                           grid_scale(correction, -1))
            self.assertEqual(grid_multiply(neighbour_sum, f.grid()), rhs)

    def test_scaled_product_reduces_to_single_directional_cascades(self):
        for base in (2, 10):
            for pattern in (CascadeProduct.F(3, 5), CascadeProduct((1, 4), (3,)),
                            CascadeProduct((2, 2), (2, 2)), CascadeProduct((0,), ())):
                ledger = pattern.boundary_ledger()
                self.assertEqual(sum(ledger.values()), 0)
                rhs = grid_add(*(grid_scale(direct_cascade(abs(k)-1, "X" if k > 0 else "Y"), -v)
                                 for k, v in ledger.items() if k))
                self.assertTrue(carry_equal(grid_scale(pattern.grid(), base**(pattern.order-1)), rhs, base))

    def test_axis_exchange_and_orientation_shift(self):
        for xs, ys in (((2, 4), (3,)), ((1,), (2, 3)), ((0, 4), (0, 1))):
            pattern = CascadeProduct(xs, ys)
            swapped = CascadeProduct(ys, xs).grid()
            self.assertEqual(swapped, {(y, x): d for (x, y), d in pattern.grid().items()})
            # R_M=-q^(-M-1)L_M, so one profile handles every orientation.
            shift = -sum(m+1 for m in ys)
            oriented = grid_multiply(q_grid({shift: (-1)**len(ys)}),
                                     CascadeProduct(xs+ys, ()).grid())
            self.assertEqual(oriented, pattern.grid())

    def test_finite_difference_reveals_exact_boundary_knots(self):
        for pattern in (CascadeProduct.F(20, 30), CascadeProduct((20, 30), (10,)),
                        CascadeProduct((20, 20), (20, 20))):
            sequence = dict(enumerate(pattern.envelope()))
            for _ in range(pattern.order):
                keys = set(sequence) | {k+1 for k in sequence}
                sequence = {k: sequence.get(k, 0)-sequence.get(k-1, 0) for k in keys}
                sequence = {k: c for k, c in sequence.items() if c}
            self.assertEqual(sequence, pattern.box_boundaries())

    def test_expression_algebra_and_carry_equality(self):
        for base in (2, 3, 10):
            for n in (0, 1, 4):
                left, right = CascadeProduct((n,), ()), CascadeProduct((), (n,))
                f = left*right
                scaled_f = CascadeExpression(((f, base),))
                sum_lr = CascadeExpression(((left, 1), (right, 1)))
                self.assertTrue(scaled_f.equals(sum_lr, base))
                self.assertTrue(carry_equal(scaled_f.grid(), sum_lr.grid(), base))
                self.assertFalse(CascadeExpression(((left, base),)).equals(
                    CascadeExpression(((CascadeProduct(), 1),)), base))
                a, b, c = scaled_f, sum_lr, CascadeExpression(((f, -2), (left, 3)))
                self.assertTrue(((a+b)*c).equals(a*c+b*c, base))

    def test_matched_two_component_multiplication_against_raw_grids(self):
        for base in (2, 3, 10):
            one = MatchedElement((1,), (), base)
            f = MatchedElement((0, 1), (), base)
            left = MatchedElement((), (1,), base)
            right = MatchedElement((0, base), (-1,), base)
            self.assertEqual(left*right, f)
            self.assertEqual(left*left, MatchedElement((0, -1), (0, base), base))
            elements = (one, f, left, right, f*f, f*left,
                        MatchedElement((2, -3), (1, 2), base))
            for n in (0, 1, 3):
                for a, b in product(elements, repeat=2):
                    self.assertTrue(carry_equal((a*b).grid(n),
                                                grid_multiply(a.grid(n), b.grid(n)), base))

    def test_boundary_collisions_and_unequal_growth_are_not_discarded(self):
        for n in (10, 20, 40):
            self.assertEqual(CascadeProduct.F(n, n).boundary_ledger()[0], 2)
            unequal = CascadeProduct.F(n, n+1).boundary_ledger()
            self.assertEqual(unequal[0], 1)
            self.assertEqual(unequal[-1], 1)
            # Term counts n,n,2n: a resonance cancels the central ledger term.
            self.assertNotIn(0, CascadeProduct((n-1, n-1), (2*n-1,)).boundary_ledger())
            self.assertTrue(CascadeProduct((n-1, n-1), (2*n-1,)).grid())

    def test_fixed_unequal_cutoffs_compress_to_gcd_anchor_without_division(self):
        for n, m in product(range(1, 10), repeat=2):
            g = gcd(n, m)
            a, c = n//g, m//g
            h = 1 if c == 1 else pow(a, -1, c)
            k = (a*h-1)//c
            self.assertGreaterEqual(k, 0)
            self.assertEqual(a*h-c*k, 1)
            sum_u = grid_add(*(q_grid({n*i: 1}) for i in range(h)))
            sum_v = grid_add(*(q_grid({-m*j: 1}) for j in range(k)))
            first = grid_multiply(direct_cascade(n-1, "X"), sum_u)
            second = grid_multiply(q_grid({n*h: 1}),
                                   grid_multiply(direct_cascade(m-1, "Y"), sum_v))
            anchor = grid_add(first, second)
            self.assertEqual(anchor, direct_cascade(g-1, "X"))
            self.assertEqual(grid_multiply(q_grid({-g: -1}), anchor), direct_cascade(g-1, "Y"))

    def test_affine_ledger_retains_moving_and_stationary_terms(self):
        examples = [(((1, 0),), ((1, 0),)),
                    (((1, -1), (1, 2)), ((1, 0),)),
                    (((1, 0), (1, 0)), ((2, 0),)),
                    (((2, 3), (1, -2)), ((3, 1), (1, 0)))]
        for xs, ys in examples:
            symbolic = affine_boundary_ledger(xs, ys)
            for t in (5, 10, 20):
                pattern = CascadeProduct(tuple(s*t+c-1 for s, c in xs),
                                         tuple(s*t+c-1 for s, c in ys))
                evaluated = Counter()
                for (s, c), value in symbolic.items():
                    evaluated[s*t+c] += value
                self.assertEqual(pattern.boundary_ledger(), {k: v for k, v in evaluated.items() if v})
        square = affine_boundary_ledger(((1, 0),)*2, ((1, 0),)*2)
        self.assertEqual(square[(0, 0)], 6)
        canceled = affine_boundary_ledger(((1, 0),)*2, ((2, 0),))
        self.assertNotIn((0, 0), canceled)
        self.assertTrue(canceled)


if __name__ == "__main__":
    unittest.main()
