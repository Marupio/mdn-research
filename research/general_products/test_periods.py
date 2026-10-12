"""Exact checks against independent full-grid convolution and carrying."""

from pathlib import Path
import sys
import unittest

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent/"fuzzy"))
sys.path.insert(0, str(HERE.parent/"phase_products"))
from family import CascadeProduct
from canonical import normalize, window
from periods import period_certificate, period_test, same_phase, raw_coefficient, local_window


class PeriodTests(unittest.TestCase):
    def test_finite_coefficients_against_box_convolution(self):
        for d in range(1, 8):
            for a in range(d+1):
                c = d-a
                for n in (1, 2, 5, 8):
                    grid = CascadeProduct((n-1,)*a, (n-1,)*c).grid()
                    for k in range(-a*(n-1)-1, c*(n-1)+2):
                        self.assertEqual(raw_coefficient(a, c, n, k), grid.get((k-a, -k-c), 0))

    def test_local_prediction_against_complete_normalization(self):
        for d in range(1, 8):
            for a in range(d+1):
                for n in (1, 3, 11, 25):
                    c = d-a
                    grid = CascadeProduct((n-1,)*a, (n-1,)*c).grid()
                    self.assertEqual(local_window(a, c, n), window(normalize(grid).grid, 3), (a, c, n))
        self.assertEqual(local_window(0, 0, 1), {(0, 0): 1})

    def test_known_periods_and_pure_families(self):
        expected = {(1, 1): 10, (2, 1): 100, (1, 2): 100, (2, 2): 500,
                    (3, 1): 1000, (3, 2): 10000, (4, 2): 25000, (3, 3): 50000,
                    (4, 3): 100000, (4, 4): 500000, (2, 8): 250000000}
        for (a, c), period in expected.items():
            self.assertEqual(period_certificate(a, c)["period"], period, (a, c))
            self.assertEqual(period_certificate(c, a)["period"], period, (c, a))
            self.assertTrue(period_test(a, c, period)["passes"])
        for count in range(8):
            self.assertEqual(period_certificate(count, 0)["period"], 1)
            self.assertEqual(period_certificate(0, count)["period"], 1)

    def test_shortcut_exception_and_phase_collisions(self):
        certificate = period_certificate(2, 8)
        lower = certificate["lower_bound"]
        self.assertEqual(lower, 125000000)
        self.assertFalse(period_test(2, 8, lower)["passes"])
        for n in (2, 4, 6, 8):
            self.assertTrue(same_phase(2, 8, n, n+lower))
        for n in (1, 3, 5, 7):
            self.assertFalse(same_phase(2, 8, n, n+lower))
        # Actual finite windows at sufficiently interior representatives agree.
        self.assertEqual(local_window(2, 8, 42, radius=6), local_window(2, 8, 42+lower, radius=6))
        self.assertNotEqual(local_window(2, 8, 41, radius=6), local_window(2, 8, 41+lower, radius=6))

    def test_large_periods_without_constructing_large_grids(self):
        for a, c in ((3, 2), (4, 2), (3, 3), (4, 4), (2, 8)):
            period = period_certificate(a, c)["period"]
            for n in (31, 32, 47):
                self.assertEqual(local_window(a, c, n, radius=6),
                                 local_window(a, c, n+period, radius=6), (a, c, n))

    def test_other_bases_and_invalid_arguments(self):
        for base in (3, 4, 5, 16):
            for a, c in ((1, 1), (2, 1), (2, 2), (3, 2)):
                period = period_certificate(a, c, base)["period"]
                self.assertTrue(period_test(a, c, period, base)["passes"])
                lower = -(base//2)
                self.assertEqual(local_window(a, c, 31, base=base, lower=lower),
                                 local_window(a, c, 31+period, base=base, lower=lower))
        for counts in ((-1, 2), (True, 1), (1.5, 2)):
            with self.assertRaises(ValueError):
                period_certificate(*counts)
        with self.assertRaises(ValueError):
            period_test(1, 1, 0)
        with self.assertRaises(ValueError):
            raw_coefficient(1, 1, 0, 0)


if __name__ == "__main__":
    unittest.main()
