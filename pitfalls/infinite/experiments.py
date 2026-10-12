#!/usr/bin/env python3
"""Exact finite probes of proposed infinite MDN operations.

No C++ backend, floating-point arithmetic, or numerical convergence test is used.
The finite identities are verified in Q[X^+-1,Y^+-1] before imposing X+Y=b.
Infinite-limit statements in the report follow the displayed geometric formulas;
a finite experiment by itself is not a proof of convergence.
"""

from __future__ import annotations

import argparse
from fractions import Fraction as F
import hashlib
import json
from math import comb, factorial
from pathlib import Path

Grid = dict[tuple[int, int], F]


def clean(p: Grid) -> Grid:
    return {ij: F(c) for ij, c in p.items() if c}


def atom(i: int = 0, j: int = 0, coefficient: F = F(1)) -> Grid:
    return clean({(i, j): F(coefficient)})


def add(*polynomials: Grid) -> Grid:
    result: Grid = {}
    for p in polynomials:
        for ij, c in p.items():
            result[ij] = result.get(ij, F(0)) + c
    return clean(result)


def scale(p: Grid, c: F) -> Grid:
    return clean({ij: value * c for ij, value in p.items()})


def multiply(p: Grid, q: Grid) -> Grid:
    result: Grid = {}
    for (i, j), a in p.items():
        for (k, ell), b in q.items():
            ij = i + k, j + ell
            result[ij] = result.get(ij, F(0)) + a * b
    return clean(result)


def power(p: Grid, n: int) -> Grid:
    if n < 0:
        raise ValueError("Only nonnegative polynomial powers are implemented")
    result = atom()
    for _ in range(n):
        result = multiply(result, p)
    return result


def evaluate(p: Grid, x: F, y: F) -> F:
    x, y = F(x), F(y)
    if x == 0 or y == 0:
        raise ValueError("Laurent-grid evaluation requires nonzero coordinates")
    return sum((c * x**i * y**j for (i, j), c in p.items()), F(0))


def raw_l1(p: Grid) -> F:
    return sum(map(abs, p.values()), F(0))


def weighted_l1(p: Grid, x: F, y: F) -> F:
    x, y = F(x), F(y)
    if x <= 0 or y <= 0:
        raise ValueError("The demonstrated weights must be strictly positive")
    return sum((abs(c) * x**i * y**j for (i, j), c in p.items()), F(0))


def directional_partial(n: int, direction: str = "left", damping: F = F(1)) -> Grid:
    """N means N+1 terms: X^-1 sum(k=0..N)(-damping Y/X)^k."""
    if n < 0:
        raise ValueError("The final term index must be nonnegative")
    if direction not in {"left", "right"}:
        raise ValueError("Direction must be left or right")
    result = {(-k - 1, k): (-F(damping))**k for k in range(n + 1)}
    return result if direction == "left" else {(j, i): c for (i, j), c in result.items()}


def carry_one(n: int, base: int = 10) -> Grid:
    """Rational coefficients: ((X+Y)/b)^N, equal to one on the carry curve."""
    if n < 0 or base <= 0:
        raise ValueError("Require N>=0 and b>0")
    return {(k, n - k): F(comb(n, k), base**n) for k in range(n + 1)}


def integer_escape(n: int, base: int = 10) -> Grid:
    """Integer coefficients: X^-N(b-Y)^N, equal to one on the carry curve."""
    if n < 0 or base <= 0:
        raise ValueError("Require N>=0 and b>0")
    return {(-n, k): F(comb(n, k) * base**(n - k) * (-1)**k) for k in range(n + 1)}


def geometric_inverse_partial(n: int, base: int = 10) -> Grid:
    """Partial inverse of b-X-Y in the unweighted real/rational l1 algebra."""
    return scale(add(*(carry_one(k, base) for k in range(n + 1))), F(1, base))


def alternating_polynomial(k: int, coefficients: tuple[F, ...]) -> F:
    """c_k=(-1)^k P(k), evaluated exactly even at negative integer indices."""
    sign = -1 if k % 2 else 1
    return sign * sum((c * k**degree for degree, c in enumerate(coefficients)), F(0))


def recurrence_residual(k: int, coefficients: tuple[F, ...], order: int) -> F:
    """(1+S)^order c at k, with backward shift (Sc)_k=c_(k-1)."""
    if order < 0:
        raise ValueError("The recurrence order must be nonnegative")
    return sum((comb(order, j) * alternating_polynomial(k-j, coefficients)
                for j in range(order+1)), F(0))


def fraction_json(value):
    if isinstance(value, F):
        return str(value.numerator) if value.denominator == 1 else f"{value.numerator}/{value.denominator}"
    if isinstance(value, dict):
        return {key: fraction_json(item) for key, item in value.items()}
    if isinstance(value, (list, tuple)):
        return [fraction_json(item) for item in value]
    return value


def checked_report() -> dict:
    b = 10
    xy = add(atom(1, 0), atom(0, 1))
    one = atom()
    relation = add(atom(coefficient=F(b)), scale(xy, F(-1)))
    counts = {"exact_laurent_identities": 0, "exact_scalar_checks": 0}

    # Check the two geometric tails as independent Laurent-polynomial identities.
    tail_rows = []
    for n in (0, 1, 2, 4, 8, 16, 32):
        left = directional_partial(n)
        right = directional_partial(n, "right")
        left_tail = atom(-n - 1, n + 1, F((-1)**n))
        right_tail = atom(n + 1, -n - 1, F((-1)**n))
        assert multiply(xy, left) == add(one, left_tail)
        assert multiply(xy, right) == add(one, right_tail)
        product = multiply(left, right)
        assert product[(-1, -1)] == n + 1
        counts["exact_laurent_identities"] += 2
        counts["exact_scalar_checks"] += 1
        values = []
        for x, y in ((7, 3), (3, 7), (5, 5)):
            lv, rv = evaluate(left, F(x), F(y)), evaluate(right, F(x), F(y))
            assert lv - F(1, b) == -(-F(y, x))**(n + 1) / b
            assert rv - F(1, b) == -(-F(x, y))**(n + 1) / b
            counts["exact_scalar_checks"] += 2
            values.append({"X": x, "Y": y, "left": lv, "right": rv,
                           "left_error": abs(lv - F(1, b)),
                           "right_error": abs(rv - F(1, b)),
                           "product": lv * rv})
        tail_rows.append({"N": n, "terms_per_factor": n + 1,
                          "raw_central_product_coefficient": product[(-1, -1)],
                          "evaluations": values})

    # A stronger form of the collapse: the forbidden relation becomes invertible.
    collapse_rows = []
    for n in (0, 1, 2, 4, 8, 16, 32):
        p = carry_one(n, b)
        e = integer_escape(n, b)
        assert raw_l1(p) == F(2, b)**n
        assert raw_l1(e) == (b + 1)**n
        assert evaluate(e, F(7), F(3)) == 1
        for x, y in ((7, 3), (3, 7), (5, 5)):
            assert evaluate(p, F(x), F(y)) == 1
            assert weighted_l1(p, F(x), F(y)) == 1
            counts["exact_scalar_checks"] += 2
        q = geometric_inverse_partial(n, b)
        assert multiply(relation, q) == add(one, scale(carry_one(n + 1, b), F(-1)))
        counts["exact_laurent_identities"] += 1
        counts["exact_scalar_checks"] += 3
        collapse_rows.append({"N": n, "rational_carry_raw_l1": raw_l1(p),
                              "rational_carry_positive_weighted_l1": F(1),
                              "represented_value": F(1),
                              "integer_escape_raw_l1": raw_l1(e),
                              "integer_escape_x_coordinate": -n,
                              "relation_inverse_residual_raw_l1": F(2, b)**(n + 1)})

    # On the symmetry line, both Abel-damped geometric series converge absolutely.
    # Their values have a finite limit while their individual product digits do not.
    abel_rows = []
    x = y = F(5)
    for damping in (F(0), F(1, 2), F(9, 10), F(99, 100), F(999, 1000)):
        left_value = 1 / (x + damping * y)
        right_value = 1 / (y + damping * x)
        raw_central_limit = 1 / (1 - damping**2)
        for n in (0, 1, 3, 10):
            l = directional_partial(n, damping=damping)
            r = directional_partial(n, "right", damping=damping)
            prod = multiply(l, r)
            expected = (1 - damping**(2 * n + 2)) / (1 - damping**2)
            assert prod.get((-1, -1), F(0)) == expected
            assert evaluate(l, x, y) == left_value * (1 - (-damping)**(n + 1))
            counts["exact_scalar_checks"] += 2
        abel_rows.append({"r": damping, "left_infinite_sum": left_value,
                          "right_infinite_sum": right_value,
                          "product_of_sums": left_value * right_value,
                          "raw_central_product_coefficient": raw_central_limit})

    # Averaging is not multiplicative: show the finite even-length means exactly.
    cesaro_rows = []
    for cutoffs in (2, 4, 10, 40):
        values = [evaluate(directional_partial(n), x, y) for n in range(cutoffs)]
        mean = sum(values, F(0)) / cutoffs
        mean_of_products = sum((v * v for v in values), F(0)) / cutoffs
        assert mean == F(1, 10)
        assert mean_of_products == F(1, 50)
        counts["exact_scalar_checks"] += 2
        cesaro_rows.append({"number_of_cutoffs": cutoffs, "mean_of_left": mean,
                            "product_of_means": mean**2,
                            "mean_of_same_cutoff_products": mean_of_products})

    recurrence_rows = []
    for order in range(1, 6):
        for degree in range(order+1):
            coefficients = (F(0),)*degree + (F(1),)
            residuals = [recurrence_residual(k, coefficients, order) for k in range(-7, 8)]
            if degree < order:
                assert residuals == [F(0)]*15
            else:
                assert residuals == [F((-1 if k % 2 else 1)*factorial(order)) for k in range(-7, 8)]
            counts["exact_scalar_checks"] += 15
            recurrence_rows.append({"operator_order": order, "polynomial_degree": degree,
                                    "indices": [-7, 7], "all_zero": degree < order,
                                    "residual_formula": "0" if degree < order else f"(-1)^k * {factorial(order)}"})

    return fraction_json({
        "arithmetic": "Python standard-library Fraction; exact signed Laurent grids",
        "source_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "base": b,
        "definitions": {
            "left": "L_N = sum(k=0..N) (-1)^k X^(-k-1) Y^k",
            "right": "R_N = sum(k=0..N) (-1)^k Y^(-k-1) X^k",
            "tail_identity": "(X+Y)L_N = 1 - (-Y/X)^(N+1)",
            "rational_carry": "P_N = (X+Y)^N / b^N; rational coefficients are allowed",
            "integer_escape": "E_N = X^(-N)(b-Y)^N; signed integer coefficients are unbounded",
            "inverse_relation": "Q_N = (1/b) sum(k=0..N) ((X+Y)/b)^k",
            "inverse_identity": "(b-X-Y)Q_N = 1-P_(N+1)",
            "weighted_norm": "sum |c_ij| X^i Y^j, with X,Y>0 and X+Y=b",
            "abel_scope": "Only X=Y=b/2 and 0<=r<1: both series converge absolutely",
        },
        "checks_passed": counts,
        "directional_tails_and_product": tail_rows,
        "collapse_and_noncollapsing_weights": collapse_rows,
        "boundary_abel_regularization": abel_rows,
        "boundary_averaging_not_multiplicative": cesaro_rows,
        "alternating_polynomial_recurrences": recurrence_rows,
        "conclusions_supported_by_formulas": [
            "For b>2, unweighted l1 makes P_N tend to zero although its carry value is one.",
            "In that l1 completion b-X-Y has a geometric-series inverse; quotienting by the carry relation forces 1=0.",
            "Integer E_N also represents one and disappears at each fixed cell, so coordinatewise limits cannot descend continuously to carry values.",
            "Positive-coordinate weighted norms keep P_N at norm one and do not have this collapse.",
            "The left and right ordinary convergence regions are disjoint; at the symmetry line ordinary partial sums oscillate.",
            "Abel damping gives the expected boundary scalar inverse and product, but no coefficientwise limit of the product grid.",
            "Averaging same-cutoff products is different from multiplying separately averaged factors; no unrestricted multiplicative summation was established.",
            "For orders 1..5, exact checks confirm that (1+S)^m annihilates (-1)^k P(k) for degree(P)<m, but not the tested degree-m monomial; S shifts k backwards.",
        ],
        "limits": [
            "Finite checks corroborate exact formulas; the accompanying mathematical argument establishes limits.",
            "This file defines no unrestricted multiplication on bilateral infinite grids.",
            "No normalized bounded-integer-digit convergence theorem is claimed.",
        ],
    })


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path(__file__).with_name("results.json"))
    args = parser.parse_args()
    report = checked_report()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({"output": str(args.output.resolve()), "checks_passed": report["checks_passed"]}))


if __name__ == "__main__":
    main()
