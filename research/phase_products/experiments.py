#!/usr/bin/env python3
"""Reproduce the finite canonical windows and their exact carry certificates."""

import json
from pathlib import Path
import sys

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent/"fuzzy"))
from family import CascadeProduct
from canonical import normalize, verify_certificate, window, first_difference

COUNTS = {"F": (1, 1), "FL": (2, 1), "FR": (1, 2), "F2": (2, 2)}
PERIODS = {"F": 10, "FL": 100, "FR": 100, "F2": 500}


def raw(n, name):
    a, c = COUNTS[name]
    return CascadeProduct((n-1,)*a, (n-1,)*c).grid()


def run():
    results = {"base": 10, "digit_range": [-4, 5], "window_radius": 3,
               "scope": "Complete finite grids and local displays, with exact three-cell carry certificates. No scalar projections.",
               "phase_censuses": [], "depth_examples": [], "failures": []}
    certificates = 0
    coordinates = ((-1, -1), (-2, -1), (-1, -2), (-2, -2), (-2, 0))
    for name, period in PERIODS.items():
        patterns = set()
        for n in range(21, 21+period):
            grid = raw(n, name)
            normalized = normalize(grid)
            certificates += 1
            if not verify_certificate(grid, normalized):
                results["failures"].append({"kind": "carry_certificate", "name": name, "n": n})
            patterns.add(tuple(sorted(window(normalized.grid, 3).items())))
        results["phase_censuses"].append({"name": name, "proved_eventual_period": period,
                                           "consecutive_counts": [21, 20+period],
                                           "distinct_radius3_patterns": len(patterns)})
        if len(patterns) != period:
            results["failures"].append({"kind": "phase_count", "name": name})
        grids = {}
        for n in (21, 31, 121, 521):
            grid = raw(n, name)
            normalized = normalize(grid)
            certificates += 1
            if not verify_certificate(grid, normalized):
                results["failures"].append({"kind": "example_certificate", "name": name, "n": n})
            grids[n] = normalized.grid
            results["depth_examples"].append({"name": name, "N": n-1,
                                               "digits": [[*p, normalized.grid.get(p, 0)] for p in coordinates],
                                               "nonzero_cells": len(normalized.grid),
                                               "first_difference_from_depth20": first_difference(grids[21], normalized.grid),
                                               "same_radius3_window_as_depth20": window(grids[21], 3) == window(normalized.grid, 3)})
    results["exact_carry_certificates_checked"] = certificates
    return results


if __name__ == "__main__":
    report = run()
    destination = HERE/"canonical_results.json"
    destination.write_text(json.dumps(report, indent=2)+"\n")
    print(f"{report['exact_carry_certificates_checked']} carry certificates; "
          f"{len(report['failures'])} failures. Saved {destination}")
    raise SystemExit(bool(report["failures"]))
