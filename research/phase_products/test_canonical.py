"""Checks of deterministic carries, quadrant claims, and product periods."""

from pathlib import Path
import random
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/"fuzzy"))
from family import CascadeProduct, carry_equal, grid_multiply
from canonical import normalize, verify_certificate, window


def product_grid(n, name):
    depth = n-1
    counts = {"F": (1, 1), "FL": (2, 1), "FR": (1, 2), "F2": (2, 2)}
    a, c = counts[name]
    return CascadeProduct((depth,)*a, (depth,)*c).grid()


class CanonicalTests(unittest.TestCase):
    def test_random_carry_certificates_and_representative_invariance(self):
        rng = random.Random(20261012)
        for base in (3, 4, 5, 10, 16):
            lower = -(base//2)
            for _ in range(12):
                raw = {(rng.randrange(-3, 4), rng.randrange(-3, 4)): rng.randrange(-100, 101)
                       for _ in range(12)}
                result = normalize(raw, base, lower)
                self.assertTrue(verify_certificate(raw, result, base))
                self.assertTrue(all(lower <= d < lower+base for d in result.grid.values()))
                self.assertEqual(normalize(result.grid, base, lower).grid, result.grid)
                changed = raw.copy()
                for _ in range(6):
                    x, y, amount = rng.randrange(-3, 4), rng.randrange(-3, 4), rng.randrange(-9, 10)
                    for p, delta in (((x, y), -base*amount), ((x+1, y), amount), ((x, y+1), amount)):
                        changed[p] = changed.get(p, 0)+delta
                self.assertEqual(normalize(changed, base, lower).grid, result.grid)

    def test_quadrant_support_is_a_representation_property(self):
        below = {(-1, -1): 10}
        axes = {(-1, 0): 1, (0, -1): 1}
        self.assertEqual(normalize(below).grid, axes)
        self.assertTrue(carry_equal(below, axes))
        # Separate one-axis cascades and their sum avoid quadrant III.
        for n in (1, 3, 10, 21):
            left = CascadeProduct((n-1,), ()).grid()
            right = CascadeProduct((), (n-1,)).grid()
            combined = left.copy()
            for p, d in right.items():
                combined[p] = combined.get(p, 0)+d
            self.assertFalse(any(x < 0 and y < 0 for x, y in normalize(combined).grid))
            mixed = grid_multiply(left, right)
            self.assertEqual({p: d for p, d in mixed.items() if p[0] < 0 and p[1] < 0},
                             {(-1, -1): n})
        # A genuinely mixed product can have a carried display with empty III.
        self.assertFalse(any(x < 0 and y < 0 for x, y in normalize(product_grid(10, "F")).grid))

    def test_raw_central_coefficients(self):
        for n in range(1, 41):
            self.assertEqual(product_grid(n, "F")[(-1, -1)], n)
            self.assertEqual(product_grid(n, "FL")[(-2, -1)], n*(n+1)//2)
            self.assertEqual(product_grid(n, "FR")[(-1, -2)], n*(n+1)//2)
            self.assertEqual(product_grid(n, "F2")[(-2, -2)], n*(2*n*n+1)//3)

    def test_proved_periods_in_finite_windows(self):
        # Enough interior margin that the finite endpoints are outside this window's carry ancestry.
        for name, period in (("F", 10), ("FL", 100), ("FR", 100), ("F2", 500)):
            for n in range(21, 41):
                before = normalize(product_grid(n, name))
                after = normalize(product_grid(n+period, name))
                self.assertTrue(verify_certificate(product_grid(n, name), before))
                self.assertEqual(window(before.grid, 4), window(after.grid, 4), (name, n))
                self.assertNotEqual(before.grid, after.grid, "Local periodicity must not erase endpoints")

    def test_ten_phase_label_does_not_determine_products(self):
        f21, f31 = (normalize(product_grid(n, "F")).grid for n in (21, 31))
        self.assertEqual(window(f21, 4), window(f31, 4))
        for name in ("FL", "FR", "F2"):
            first, second = (normalize(product_grid(n, name)).grid for n in (21, 31))
            self.assertNotEqual(window(first, 4), window(second, 4), name)

    def test_input_guards(self):
        for kwargs in ({"base": 2}, {"lower": 0}, {"lower": -10}, {"max_cells": 0}):
            with self.assertRaises(ValueError):
                normalize({}, **kwargs)
        with self.assertRaises(ValueError):
            normalize({(0, 0): 1.5})
        with self.assertRaises(RuntimeError):
            normalize({(0, 0): 100}, max_cells=1)


if __name__ == "__main__":
    unittest.main()
