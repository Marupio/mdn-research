#!/usr/bin/env python3
"""Native higher-factor MDN checks with exact boundaries and no scalar evaluation.

Run this file directly. Only the probe is compiled; it dynamically links the
existing corrected library. Full grids and explicit carry certificates are
saved in --work. The compact result file reports observations, not an assertion
that enormous predicted periods were tested by materializing their depths.
"""

from __future__ import annotations

import argparse
from collections import defaultdict
from functools import lru_cache
import hashlib
import json
from math import comb
from pathlib import Path
import platform
import shlex
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent
REPOSITORY = HERE.parents[1]
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "fuzzy"))
sys.path.insert(0, str(HERE.parent / "phase_products"))
from family import CascadeProduct, grid_multiply, q_grid
from canonical import normalize, verify_certificate, first_difference
from periods import local_window, period_certificate, same_phase


def cells(value):
    return sorted([x, y, digit] for (x, y), digit in value.items() if digit)


def grid(value):
    return {(x, y): int(digit) for x, y, digit in value if digit}


def window(value, radius):
    return cells({p: d for p, d in value.items() if max(map(abs, p)) <= radius})


@lru_cache(maxsize=64)
def binomial_row(degree, base):
    coefficient = base**degree
    result = [coefficient]
    for k in range(degree):
        coefficient = -coefficient*(degree-k)//((k+1)*base)
        result.append(coefficient)
    return tuple(result)


def carry_equal(left, right, base=10):
    """Compare every symbolic coefficient after clearing Laurent denominators."""
    difference = defaultdict(int, left)
    for p, coefficient in right.items():
        difference[p] -= coefficient
    difference = {p: d for p, d in difference.items() if d}
    if not difference:
        return True
    xmin = min(x for x, y in difference)
    ymin = min(y for x, y in difference)
    coefficients = defaultdict(int)
    for (x, y), digit in difference.items():
        for k, term in enumerate(binomial_row(y-ymin, base)):
            coefficients[x-xmin+k] += digit*term
    return not any(coefficients.values())


def predict_window(oracle, radius=4):
    """Exact finite canonical window without constructing the whole cascade.

    Every carry moves one coordinate up and leaves the other unchanged.
    A raw cell (k-a,-k-c) outside -radius-c<=k<=radius+a therefore cannot
    reach the requested square. Restricting the raw ancestor cells is safe
    for this display query only, not for full-object equality or products.
    The binomial formula retains the supplied finite cutoffs exactly.
    """
    a, c = len(oracle.x_depths), len(oracle.y_depths)
    ancestors = {}
    for k in range(-radius-c, radius+a+1):
        coefficient = oracle.coefficient_formula(k)
        if coefficient:
            ancestors[k-a, -k-c] = (-1 if k % 2 else 1)*coefficient
    return window(normalize(ancestors).grid, radius)


def compile_probe(args):
    executable = args.work / "backend_probe"
    command = [args.compiler, "-std=c++17", "-O2", "-I", str(REPOSITORY / "library/include"),
               str(HERE / "backend_probe.cpp"), "-L", str(args.library.parent),
               "-Wl,-rpath," + str(args.library.parent), "-lmdn", "-o", str(executable)]
    with args.build_log.open("a") as log:
        log.write("\nGeneral product probe compile: " + shlex.join(command) + "\n")
        log.flush()
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        for line in process.stdout:
            print(line, end="", flush=True)
            log.write(line)
            log.flush()
        if process.wait():
            raise RuntimeError("Probe compile failed; see " + str(args.build_log))
    return executable, command


def check_case(executable, case, args):
    n, a, c, *other = case
    m = other[0] if other else n
    started = time.monotonic()
    path = args.work / ("case_" + "_".join(map(str, case)) + ".json")
    if args.reuse and path.exists():
        decoded = json.loads(path.read_text())
    else:
        process = subprocess.run([str(executable), *map(str, case)], capture_output=True,
                                 text=True, check=True, timeout=args.timeout)
        decoded = json.loads(process.stdout)
        path.write_text(process.stdout)
    values = {name: grid(value) for name, value in decoded.items()}
    oracle = CascadeProduct((n,)*a, (m,)*c)
    raw = oracle.grid()
    boundary = q_grid(oracle.boundary_ledger())
    order = a+c
    raw_fixed = normalize(raw)
    checks = {
        "exact_X_input": values["L"] == CascadeProduct((n,), ()).grid(),
        "exact_Y_input": values["R"] == CascadeProduct((), (m,)).grid(),
        "raw_binomial_oracle": all(abs(digit) == oracle.coefficient_formula(x+a)
                                   for (x, y), digit in raw.items()),
        "raw_endpoint_identity": grid_multiply(raw, {(k, order-k): comb(order, k)
                                                      for k in range(order+1)}) == boundary,
        "raw_carry_certificate": verify_certificate(raw, raw_fixed),
        "local_binomial_prediction": predict_window(oracle, args.radius) == window(raw_fixed.grid, args.radius),
    }
    if n == m:
        checks["root_local_prediction"] = cells(local_window(a, c, n+1, args.radius)) == window(raw_fixed.grid, args.radius)
    saved = {"oracle_raw": cells(raw), "boundary": cells(boundary),
             "canonical": cells(raw_fixed.grid), "raw_carries": cells(raw_fixed.carries)}
    for name in ("product", "reverse"):
        actual = values[name]
        fixed = normalize(actual)
        checks[name + "_canonical_oracle"] = fixed.grid == raw_fixed.grid
        checks[name + "_carry_certificate"] = verify_certificate(actual, fixed)
        checks[name + "_boundary_identity"] = carry_equal(
            {p: digit*10**order for p, digit in actual.items()}, boundary)
        checks[name + "_digits_bounded"] = all(abs(digit) < 10 for digit in actual.values())
        saved[name + "_carries"] = cells(fixed.carries)
    (args.work / (path.stem + "_certificates.json")).write_text(json.dumps(saved, separators=(",", ":")))
    return {"case": case, "term_counts": [n+1, m+1], "order": order,
            "seconds": round(time.monotonic()-started, 5), "checks": checks,
            "raw_max_coefficient": max(map(abs, raw.values()), default=0),
            "supports": {name: len(value) for name, value in values.items()},
            "raw_support": len(raw), "canonical_support": len(raw_fixed.grid),
            "display_order_difference": first_difference(values["product"], values["reverse"]),
            "positive_window": window(values["product"], args.radius),
            "canonical_window": window(raw_fixed.grid, args.radius),
            "boundary_ledger": sorted([power, digit] for power, digit in oracle.boundary_ledger().items()),
            "grid_file": str(path)}


def check_large_shifts(radius=4):
    """Finite exact formula computations at large cutoffs, not native runs."""
    specifications = [(a, d-a) for d in range(2, 9) for a in range(1, d)]
    specifications += [(2, 8), (4, 6), (5, 5)]
    results = []
    for a, c in specifications:
        certificate = period_certificate(a, c)
        period = certificate["period"]
        queries = []
        for n in (21, 22, 25, 30):
            left = predict_window(CascadeProduct((n-1,)*a, (n-1,)*c), radius)
            right = predict_window(CascadeProduct((n+period-1,)*a, (n+period-1,)*c), radius)
            queries.append({"term_count": n, "shifted_term_count": n+period, "checks": {
                "predicted_windows_repeat": left == right,
                "root_base_window_agrees": left == cells(local_window(a, c, n, radius)),
                "root_shifted_window_agrees": right == cells(local_window(a, c, n+period, radius)),
            }})
        witnesses = []
        for prime in (2, 5):
            if period % prime:
                continue
            candidate = period//prime
            witness = None
            for n in range(21, 31):
                left = grid(predict_window(CascadeProduct((n-1,)*a, (n-1,)*c), radius))
                right = grid(predict_window(CascadeProduct((n+candidate-1,)*a, (n+candidate-1,)*c), radius))
                difference = first_difference(left, right)
                if difference is not None:
                    witness = {"term_count": n, "difference": difference}
                    break
            witnesses.append({"divisor": prime, "smaller_shift": candidate, "window_witness": witness})
        results.append({"a": a, "c": c, "period": period, "queries": queries,
                        "smaller_shift_witnesses": witnesses})
    collisions = []
    for n in (41, 42):
        a, c, shift = 2, 8, 125_000_000
        left = grid(predict_window(CascadeProduct((n-1,)*a, (n-1,)*c), radius))
        right = grid(predict_window(CascadeProduct((n+shift-1,)*a, (n+shift-1,)*c), radius))
        collisions.append({"a": a, "c": c, "term_count": n, "shift": shift,
                           "windows_equal": left == right,
                           "eventual_phase_equal_by_finite_criterion": same_phase(a, c, n, n+shift),
                           "nearest_difference": first_difference(left, right)})
    return {"scope": "Exact Python finite-window computations at large term counts; no native backend grids at these huge depths were constructed.",
            "window_radius": radius, "products": results, "nonuniform_collision_examples": collisions,
            "check_count": sum(len(q["checks"]) for r in results for q in r["queries"]),
            "failed_checks": sum(not v for r in results for q in r["queries"] for v in q["checks"].values())}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, default=REPOSITORY / "build/fraxis/libmdn.so")
    parser.add_argument("--work", type=Path, default=Path("/tmp/mdn-general-products-01a12881"))
    parser.add_argument("--build-log", type=Path, default=Path.home() / "output_01a12881-e6c8-7cb2-a1d2-7c899e17aa69.log")
    parser.add_argument("--compiler", default="c++")
    parser.add_argument("--timeout", type=float, default=60)
    parser.add_argument("--radius", type=int, default=4)
    parser.add_argument("--quick", action="store_true")
    parser.add_argument("--reuse", action="store_true", help="Recheck existing complete grids when present")
    parser.add_argument("--output", type=Path, default=HERE / "backend_results.json")
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    args.library = args.library.resolve()
    started = time.monotonic()
    before_hash = hashlib.sha256(args.library.read_bytes()).hexdigest()
    executable, command = compile_probe(args)
    if args.quick:
        cases = [(0, 0, 0), (20, 1, 1), (30, 3, 3), (60, 4, 4)]
    else:
        cases = [(0, 0, 0)]
        cases += [(n, a, degree-a) for degree in range(1, 7) for a in range(degree+1)
                  for n in range(31)]
        cases += [(n, a, degree-a) for degree in (7, 8) for a in range(degree+1)
                  for n in (0, 1, 2, 5, 10, 20, 30)]
        cases += [(n, degree//2, degree-degree//2) for degree in range(4, 9) for n in (40, 60)]
        cases += [(120, 2, 2), (20, 2, 3, 21), (20, 3, 3, 30), (20, 4, 4, 30)]
        cases += [(n, a, c) for a, c in ((2, 8), (4, 6), (5, 5))
                  for n in (0, 1, 2, 5, 10, 20)]
        cases = list(dict.fromkeys(cases))
    results, failures = [], []
    for index, case in enumerate(cases, 1):
        try:
            result = check_case(executable, case, args)
            results.append(result)
            bad = [name for name, passed in result["checks"].items() if not passed]
            if bad:
                failures.append({"case": case, "checks": bad})
            if index % 50 == 0 or max(case) >= 40 or args.quick or bad:
                print(f"Case {index}/{len(cases)} {case}: {result['seconds']} s; failures={bad}", flush=True)
        except Exception as error:
            failures.append({"case": case, "exception": repr(error)})
            print("FAILED", case, repr(error), flush=True)
    summary = {
        "scope": "Complete native finite grids, no discarded boundaries, no scalar assignment to positional shifts.",
        "base": 10, "sign_policy": "Positive", "precision": -1,
        "canonical_digit_set": [-4, 5], "window_radius": args.radius,
        "library": str(args.library), "library_sha256": before_hash,
        "library_unchanged": hashlib.sha256(args.library.read_bytes()).hexdigest() == before_hash,
        "compiler": subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0],
        "compile_command": command, "build_log": str(args.build_log),
        "dynamic_linkage": subprocess.check_output(["ldd", str(executable)], text=True).splitlines(),
        "python": platform.python_version(), "runtime_seconds": round(time.monotonic()-started, 3),
        "case_count": len(cases), "completed_case_count": len(results),
        "exact_check_count": sum(len(result["checks"]) for result in results),
        "failed_checks": sum(not passed for result in results for passed in result["checks"].values()),
        "failures": failures,
        "different_display_by_multiplication_order": sum(result["display_order_difference"] is not None for result in results),
        "largest_raw_coefficient": max(result["raw_max_coefficient"] for result in results),
        "full_grid_directory": str(args.work),
        "period_testing_limit": "Finite depths only; large predicted periods are not tested by materializing a full-period shift.",
        "results": results,
    }
    if not args.quick:
        summary["large_shift_formula_checks"] = check_large_shifts(args.radius)
    summary["runtime_seconds"] = round(time.monotonic()-started, 3)
    args.output.write_text(json.dumps(summary, indent=2) + "\n")
    print(f"{len(results)}/{len(cases)} cases; {summary['exact_check_count']} exact checks; "
          f"{len(failures)} failures. Results: {args.output}")
    return (bool(failures) or not summary["library_unchanged"]
            or bool(summary.get("large_shift_formula_checks", {}).get("failed_checks", 0)))


if __name__ == "__main__":
    raise SystemExit(main())
