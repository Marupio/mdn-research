"""Check finite fractional expansions against an independent exact oracle.

The primary check retains every finite term, including the boundary: clear the
Laurent denominators and compare all coefficients after formal Y = base - X.
Rational specializations below are supplemental diagnostics, not a definition
of MDN magnitude or a proof that an infinite mixed expansion converges.

Run via CTest, or: python3 tests/test_fraxis.py /path/to/fraxis_probe
"""

import json
import math
import subprocess
import sys
import unittest
from fractions import Fraction

PROBE = sys.argv.pop(1)
ALPHABET = "0123456789abcdefghijklmnopqrstuv"


def evaluate(cells, x, y):
    return sum((digit * x**i * y**j for i, j, digit in cells), Fraction())


def raw_expansion(value, base, depth, direction, origin=(0, 0)):
    """Uncarried integer coefficients of the complete finite binomial sum."""
    sign = -1 if value.startswith("-") else 1
    integer, _, fraction = value.lstrip("-").partition(".")
    cells = [(origin[0], origin[1], sign * int(integer or "0", base))]
    for place, char in enumerate(fraction, 1):
        digit = sign * ALPHABET.index(char.lower())
        for n in range(depth + 1):
            i, j = -place - n, n
            if direction == "Y":
                i, j = j, i
            cells.append((origin[0] + i, origin[1] + j,
                          digit * (-1)**n * math.comb(place + n - 1, n)))
    return cells


def multiply_cells(left, right):
    result = {}
    for i, j, digit in left:
        for k, ell, other in right:
            key = (i + k, j + ell)
            result[key] = result.get(key, 0) + digit * other
    return [(i, j, digit) for (i, j), digit in result.items() if digit]


def carry_residual(left, right, base):
    """Zero exactly when the two finite Laurent arrays are carry-equivalent.

    In Z[X^+/-1,Y^+/-1]/(X+Y-base), multiply the difference by a monomial
    to remove all negative exponents, then substitute the polynomial base-X
    for Y. The resulting integer polynomial must have every coefficient zero.
    X and base-X are nonzero in Z[X], so clearing denominators loses nothing.
    """
    difference = {}
    for cells, sign in ((left, 1), (right, -1)):
        for i, j, digit in cells:
            difference[i, j] = difference.get((i, j), 0) + sign * digit
    difference = {key: digit for key, digit in difference.items() if digit}
    if not difference:
        return {}
    min_i = min(0, min(i for i, _ in difference))
    min_j = min(0, min(j for _, j in difference))
    coefficients = {}
    for (i, j), digit in difference.items():
        i, j = i - min_i, j - min_j
        for k in range(j + 1):
            power = i + k
            coefficients[power] = coefficients.get(power, 0) + (
                digit * math.comb(j, k) * base**(j - k) * (-1)**k)
    return {power: value for power, value in coefficients.items() if value}


def expansion(value, base, depth, direction, x, y, origin=(0, 0)):
    """Sum d_k * X^(-k) * sum_n (-Y/X)^n binom(k+n-1,n)."""
    negative = value.startswith("-")
    integer, _, fraction = value.lstrip("-").partition(".")
    result = Fraction(int(integer or "0", base))
    axis, other = (x, y) if direction == "X" else (y, x)
    for place, char in enumerate(fraction, 1):
        digit = ALPHABET.index(char.lower())
        result += digit * sum(
            ((-1)**n * math.comb(place + n - 1, n)
             * axis**(-place - n) * other**n for n in range(depth + 1)),
            Fraction(),
        )
    return (-result if negative else result) * x**origin[0] * y**origin[1]


class FraxisTests(unittest.TestCase):
    def test_formal_oracle_detects_what_scalar_samples_can_miss(self):
        base = 10
        # A shifted carry is zero, including at negative coordinates.
        self.assertEqual(carry_residual(
            [(-2, -3, -base), (-1, -3, 1), (-2, -2, 1)], [], base), {})
        # (4X-3b)(5X-4b) vanishes at BOTH supplemental sample points,
        # yet is a nonzero MDN; the all-coefficient check must reject it.
        hidden_difference = [(2, 0, 20), (1, 0, -31 * base),
                             (0, 0, 12 * base**2)]
        for fraction in (Fraction(3, 4), Fraction(4, 5)):
            x, y = base * fraction, base * (1 - fraction)
            self.assertEqual(evaluate(hidden_difference, x, y), 0)
        self.assertNotEqual(carry_residual(hidden_difference, [], base), {})

    def probe(self, value, *, base=10, depth=20, precision=-1, sign=1,
              direction="X", mode="text", digits=6, origin=(0, 0)):
        command = [PROBE, str(base), str(depth), str(precision), str(sign),
                   direction, mode, value, str(digits), *(str(c) for c in origin)]
        result = subprocess.run(command, capture_output=True, text=True, timeout=15)
        self.assertEqual(result.returncode, 0, result.stderr)
        cells = json.loads(result.stdout)
        self.assertTrue(all(abs(d) < base for _, _, d in cells), cells)
        return cells

    def check_exact(self, value, *, base=10, depth=20, sign=1,
                    direction="X", mode="text", origin=(0, 0)):
        cells = self.probe(value, base=base, depth=depth, sign=sign,
                           direction=direction, mode=mode, origin=origin)
        expected_cells = raw_expansion(value, base, depth, direction, origin)
        if mode == "square":
            expected_cells = multiply_cells(expected_cells, expected_cells)
        factor = {"times-base": base, "repeat": 2, "cancel": 0}.get(mode, 1)
        expected_cells = [(i, j, factor * digit) for i, j, digit in expected_cells]
        self.assertEqual(carry_residual(cells, expected_cells, base), {},
                         (value, base, depth, sign, direction, mode, origin))
        # These scalar maps preserve carries but do not define the MDN itself.
        for fraction in (Fraction(3, 4), Fraction(4, 5)):
            x = base * fraction
            y = base - x
            expected = expansion(value, base, depth, direction, x, y, origin)
            if mode == "square": expected *= expected
            if mode == "times-base": expected *= base
            if mode == "repeat": expected *= 2
            if mode == "cancel": expected = 0
            self.assertEqual(evaluate(cells, x, y), expected,
                             (value, base, depth, sign, direction, mode, origin))
        return cells

    def test_fractional_places_and_depths(self):
        for direction in ("X", "Y"):
            for value in ("0.1", "0.01", "0.001", "0.000001", "12.345", "-0.0123"):
                for depth in (0, 1, 2, 6, 20):
                    with self.subTest(value=value, depth=depth, direction=direction):
                        self.check_exact(value, depth=depth, direction=direction)

    def test_bases_and_sign_conventions(self):
        for base, value in ((2, "0.0101"), (3, "0.012"), (16, "a.0af"), (32, "0.01v")):
            for direction in ("X", "Y"):
                for sign in (1, 2, 3):
                    for negative in ("", "-"):
                        with self.subTest(base=base, direction=direction, sign=sign, negative=negative):
                            self.check_exact(negative + value, base=base,
                                             direction=direction, sign=sign)

    def test_large_coefficients(self):
        # These binomial coefficients exceed both Digit and signed 64-bit ranges.
        for direction in ("X", "Y"):
            self.check_exact("0." + "0" * 59 + "1", depth=40, direction=direction)

    def test_add_subtract_overwrite_and_multiply(self):
        for direction in ("X", "Y"):
            for mode in ("repeat", "cancel", "overwrite", "square", "times-base"):
                for sign in (1, 2, 3):
                    with self.subTest(direction=direction, mode=mode, sign=sign):
                        self.check_exact("0.0123", direction=direction, mode=mode, sign=sign)

    def test_translated_origin(self):
        for direction in ("X", "Y"):
            self.check_exact("0.012", direction=direction, origin=(3, -2))

    def test_transposition_preserves_the_full_finite_cascade(self):
        for base, value in ((2, "0.0101"), (10, "-12.0034"), (32, "a.01v")):
            for sign in (1, 2, 3):
                x_cells = self.probe(value, base=base, direction="X", sign=sign,
                                     origin=(3, -2))
                y_cells = self.probe(value, base=base, direction="Y", sign=sign,
                                     origin=(-2, 3))
                transposed = [(j, i, digit) for i, j, digit in x_cells]
                self.assertEqual(carry_residual(transposed, y_cells, base), {},
                                 (base, value, sign))

    def test_floating_point_entry(self):
        for direction in ("X", "Y"):
            for value in ("0.1", "0.01", "0.001", "0.0123", "-0.0123"):
                text = self.probe(value, direction=direction)
                for mode in ("double", "fraction-default", "subtract"):
                    actual = self.probe(value, direction=direction, mode=mode,
                                        digits=len(value.partition('.')[2]))
                    factor = -1 if mode == "subtract" else 1
                    expected_cells = [(i, j, factor * digit) for i, j, digit in text]
                    self.assertEqual(carry_residual(actual, expected_cells, 10), {},
                                     (direction, value, mode))
                    for fraction in (Fraction(3, 4), Fraction(4, 5)):
                        x, y = 10 * fraction, 10 * (1 - fraction)
                        expected = evaluate(text, x, y)
                        if mode == "subtract": expected = -expected
                        self.assertEqual(evaluate(actual, x, y), expected,
                                         (direction, value, mode))

    def test_supplemental_scalar_convergence_of_hundredth_and_tenth_squared(self):
        # This only tests a scalar specialization in its convergence region.
        # The two finite MDNs have different retained boundary terms.
        for direction in ("X", "Y"):
            hundredth = self.probe("0.01", depth=40, direction=direction)
            squared = self.probe("0.1", depth=40, direction=direction, mode="square")
            x, y = (Fraction(8), Fraction(2)) if direction == "X" else (Fraction(2), Fraction(8))
            for cells in (hundredth, squared):
                self.assertLess(abs(evaluate(cells, x, y) - Fraction(1, 100)), Fraction(1, 10**20))

    def test_precision_window_stops_cascade(self):
        for direction in ("X", "Y"):
            cells = self.probe("0.0123", precision=8, depth=100, direction=direction)
            self.assertTrue(cells)
            self.assertLessEqual(max(x for x, _, _ in cells) - min(x for x, _, _ in cells), 8)
            self.assertLessEqual(max(y for _, y, _ in cells) - min(y for _, y, _ in cells), 8)

    def test_sign_cleanup_preserves_incoming_carry(self):
        for base in (2, 10, 32):
            for mode, polarity in (("carry-positive", 1), ("carry-negative", -1)):
                cells = self.probe("0", base=base, mode=mode)
                expected = [(0, 0, polarity), (1, 0, polarity * (base - 1)),
                            (0, 1, -polarity)]
                self.assertEqual(carry_residual(cells, expected, base), {},
                                 (base, mode))
                x, y = Fraction(base * 3, 4), Fraction(base, 4)
                self.assertEqual(evaluate(cells, x, y), polarity * (1 + (base - 1) * x - y))


if __name__ == "__main__":
    unittest.main()
