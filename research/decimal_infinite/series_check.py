#!/usr/bin/env python3
"""Check the formal decimal coefficients against saved corrected-backend grids.

Run backend_check.py first to generate the native grids. This script neither
recompiles nor reruns the backend. It compares every coefficient through the
requested order, including cancellation of any negative powers of z.
"""

import argparse
from collections import defaultdict
from fractions import Fraction
from functools import lru_cache
import json
from pathlib import Path

from series import finite_word

HERE = Path(__file__).resolve().parent


@lru_cache(maxsize=None)
def binomial_integer(top, bottom):
    value = Fraction(1)
    for k in range(bottom):
        value *= Fraction(top-k, k+1)
    assert value.denominator == 1
    return value.numerator


def grid_coefficients(cells, order, base=10):
    """Direct expansion of each cell X^i Y^j=z^(-i-j)(bz-1)^j."""
    coefficients = defaultdict(int)
    for x, y, digit in cells:
        start = -x-y
        for m in range(start, order+1):
            r = m-start
            coefficients[m] += digit * (-1 if y % 2 else 1) * binomial_integer(y, r) * (-base)**r
    if any(value for m, value in coefficients.items() if m < 0):
        raise AssertionError("A fractional input left an unexpected negative z power")
    return tuple(Fraction(coefficients[m]) for m in range(order+1))


def run(report, work, order):
    rows = []
    left = finite_word((1,), order)
    fuzzy = left * left.swap_axes()
    for entry in report["depth_sweep"]["rows"]:
        depth, n = entry["N"], entry["n"]
        cells = json.loads((work/f"depth_{depth}.json").read_text())["result"]
        equal = grid_coefficients(cells, order) == fuzzy.evaluate_count(n)
        rows.append({"kind": "mixed_tenths", "N": depth, "coefficients_equal": equal})
    for entry in report["prefix_cases"]["rows"]:
        name, n, k, direction = (entry[key] for key in ("stream", "n", "K", "direction"))
        cells = json.loads((work/f"prefix_{name}_{n}_{k}_{direction}.json").read_text())["left"]
        expression = finite_word(tuple(map(int, entry["digits"])), order, direction=direction)
        equal = grid_coefficients(cells, order) == expression.evaluate_count(n)
        rows.append({"kind": "decimal_prefix", "stream": name, "n": n, "K": k,
                     "direction": direction, "coefficients_equal": equal})
    return {
        "scope": "Exact finite formal coefficients; no assertion of equality beyond the checked order.",
        "formal_variable": "z=X^-1 with Y=b-X; coefficients retain the cascade term count n.",
        "order": order,
        "library_sha256": report["library_sha256"],
        "backend_cases": len(rows),
        "coefficient_comparisons": len(rows)*(order+1),
        "negative_power_cancellation_checked": True,
        "failures": [row for row in rows if not row["coefficients_equal"]],
        "rows": rows,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--backend-report", type=Path, default=HERE/"backend_results.json")
    parser.add_argument("--work", type=Path)
    parser.add_argument("--order", type=int, default=12)
    parser.add_argument("--output", type=Path, default=HERE/"series_results.json")
    args = parser.parse_args()
    if args.order < 0:
        parser.error("Order must be nonnegative")
    report = json.loads(args.backend_report.read_text())
    if report["failures"]:
        raise RuntimeError("Resolve the backend report's failures before this comparison")
    result = run(report, args.work or Path(report["full_grids_directory"]), args.order)
    args.output.write_text(json.dumps(result, indent=2)+"\n")
    print(f"{result['backend_cases']} grids; {result['coefficient_comparisons']} coefficient comparisons; "
          f"{len(result['failures'])} failures. Saved {args.output}")
    return bool(result["failures"])


if __name__ == "__main__":
    raise SystemExit(main())
