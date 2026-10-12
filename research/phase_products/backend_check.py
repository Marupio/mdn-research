#!/usr/bin/env python3
"""Exact finite phase-product experiments; no scalar values for MDN positions.

Run: python3 research/phase_products/backend_check.py
Only a probe is compiled, linking the existing corrected library. Compiler
output is duplicated to the session home log. All backend grids, including
every boundary cell, are saved in --work; only observations are windowed.

Two independent checks compare each product with the exact finite input:
formal coefficient elimination using X+Y=base, and a fixed-digit carry sweep
against the raw bounded-sum product coefficients. Neither evaluates X or Y.
Observed display periods are distinguished from mathematical period proofs.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from functools import lru_cache
import hashlib
import json
from pathlib import Path
import platform
import shlex
import subprocess
import time

from canonical import normalize, verify_certificate

REPOSITORY = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
DEFAULT_WORK = Path("/tmp/mdn-phase-products-01a12881")
DEFAULT_LOG = Path.home() / "output_01a12881-e6c8-7cb2-a1d2-7c899e17aa69.log"
OPERATIONS = ("F", "FL", "FR", "F2")
ANCHORS = {"F": (-1, -1), "FL": (-2, -1), "FR": (-1, -2), "F2": (-2, -2)}


def grid(cells):
    return {(x, y): int(digit) for x, y, digit in cells if digit}


def cells(value):
    return sorted([x, y, digit] for (x, y), digit in value.items() if digit)


def add(left, right, factor=1):
    result = defaultdict(int, left)
    for position, coefficient in right.items():
        result[position] += factor * coefficient
    return {position: coefficient for position, coefficient in result.items() if coefficient}


@lru_cache(maxsize=64)
def row(degree, base):
    coefficient = base**degree
    result = [coefficient]
    for k in range(degree):
        coefficient = -coefficient * (degree - k) // ((k + 1) * base)
        result.append(coefficient)
    return tuple(result)


def carry_equal(left, right, base=10):
    """Clear monomial denominators and eliminate Y formally; X stays symbolic."""
    difference = add(left, right, -1)
    if not difference:
        return True
    min_x = min(x for x, y in difference)
    min_y = min(y for x, y in difference)
    coefficients = defaultdict(int)
    for (x, y), coefficient in difference.items():
        for k, term in enumerate(row(y - min_y, base)):
            coefficients[x - min_x + k] += coefficient * term
    return not any(coefficients.values())


def canonical(value, base=10, minimum=-4):
    """Finite upward carry sweep into the fixed set minimum..minimum+base-1.

    This fixed set includes -1,0,1 in every case used here. A conservative
    step budget makes a failure explicit rather than allowing an endless run.
    """
    if not (minimum <= -1 and minimum + base - 1 >= 1):
        raise ValueError("Fixed digit set must include -1,0,1")
    pending = defaultdict(lambda: defaultdict(int))
    for (x, y), coefficient in value.items():
        pending[x + y][x, y] += coefficient
    result, visited = {}, 0
    while pending:
        degree = min(pending)
        current = pending.pop(degree)
        for (x, y), coefficient in current.items():
            visited += 1
            if visited > 2_000_000:
                raise RuntimeError("Canonical carry sweep exceeded its cell budget")
            digit = (coefficient - minimum) % base + minimum
            carry = (coefficient - digit) // base
            if digit:
                result[x, y] = digit
            if carry:
                pending[degree + 1][x + 1, y] += carry
                pending[degree + 1][x, y + 1] += carry
    return result


def raw_product(x_depths, y_depths):
    """Uncarried product from exact convolution of finite boxes of ones."""
    counts = [1]
    for depth in (*x_depths, *y_depths):
        expanded, running = [], 0
        for k in range(len(counts) + depth):
            if k < len(counts):
                running += counts[k]
            if 0 <= k - depth - 1 < len(counts):
                running -= counts[k - depth - 1]
            expanded.append(running)
        counts = expanded
    a, c = len(x_depths), len(y_depths)
    return {(k - a, -k - c): (-1 if k % 2 else 1) * coefficient
            for k, coefficient in enumerate(counts, start=-sum(x_depths)) if coefficient}


def boundary_grid(x_depths, y_depths):
    ledger = {0: 1}
    for exponent in (*[n + 1 for n in x_depths], *[-m - 1 for m in y_depths]):
        following = defaultdict(int, ledger)
        for power, coefficient in ledger.items():
            following[power + exponent] -= coefficient
        ledger = {power: coefficient for power, coefficient in following.items() if coefficient}
    return {(-power, power): (-1 if power % 2 else 1) * coefficient
            for power, coefficient in ledger.items()}


def window(value, radius=3):
    return cells({position: coefficient for position, coefficient in value.items()
                  if max(map(abs, position)) <= radius})


def first_difference(left, right):
    differences = sorted((max(abs(x), abs(y)), x, y, left.get((x, y), 0), right.get((x, y), 0))
                         for x, y in left.keys() | right.keys()
                         if left.get((x, y), 0) != right.get((x, y), 0))
    if not differences:
        return {"radius": None, "nearest": []}
    radius = differences[0][0]
    return {"radius": radius, "nearest": [list(item[1:]) for item in differences if item[0] == radius]}


def compile_probe(args):
    executable = args.work / "backend_probe"
    command = [args.compiler, "-std=c++17", "-O2", "-I", str(REPOSITORY / "library/include"),
               str(HERE / "backend_probe.cpp"), "-L", str(args.library.parent),
               "-Wl,-rpath," + str(args.library.parent), "-lmdn", "-o", str(executable)]
    args.build_log.parent.mkdir(parents=True, exist_ok=True)
    with args.build_log.open("a") as log:
        log.write("\nPhase product probe compile: " + shlex.join(command) + "\n")
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
    n, m, p, q, *other = case
    started = time.monotonic()
    process = subprocess.run([str(executable), "10", "2", *map(str, case)], text=True,
                             capture_output=True, check=True, timeout=args.timeout)
    decoded = json.loads(process.stdout)
    path = args.work / ("case_" + "_".join(map(str, case)) + ".json")
    path.write_text(process.stdout)
    values = {name: grid(raw) for name, raw in decoded.items()}
    factors = {"L_N": ((n,), ()), "R_M": ((), (m,)),
               "L_P": ((p,), ()), "R_Q": ((), (q,)),
               "F": ((n,), (m,)), "FL": ((n, p), (m,)),
               "FR": ((n,), (m, q)), "F2": ((n, n), (m, m))}
    if other:
        u, v = other
        factors.update({"L_U": ((u,), ()), "R_V": ((), (v,)),
                        "G": ((u,), (v,)), "FG": ((n, u), (m, v))})
    checks, observations, fixed = {}, {}, {}
    for name, (xs, ys) in factors.items():
        actual = values[name]
        raw = raw_product(xs, ys)
        if name.startswith(("L_", "R_")):
            checks[name + "_input"] = actual == raw
            continue
        expected_boundary = boundary_grid(xs, ys)
        scale = 10**(len(xs) + len(ys))
        checks[name + "_boundary_identity"] = carry_equal(
            {position: scale * digit for position, digit in actual.items()}, expected_boundary)
        fixed[name] = canonical(actual)
        checks[name + "_canonical_raw_product"] = fixed[name] == canonical(raw)
        certified = normalize(actual)
        checks[name + "_explicit_carry_certificate"] = (
            certified.grid == fixed[name] and verify_certificate(actual, certified))
        observations[name] = {
            "support_size": len(actual), "canonical_support_size": len(fixed[name]),
            "positive_window": window(actual, args.radius),
            "canonical_window": window(fixed[name], args.radius),
            "positive_anchor": actual.get(ANCHORS.get(name, (-2, -2)), 0),
            "canonical_anchor": fixed[name].get(ANCHORS.get(name, (-2, -2)), 0),
            "degree_histogram": dict(sorted(Counter(x + y for x, y in actual).items())),
            "boundary_certificate": cells(expected_boundary),
        }
    checks["all_backend_digits_bounded"] = all(abs(digit) < 10 for value in values.values() for digit in value.values())
    (args.work / (path.stem + "_canonical.json")).write_text(json.dumps({name: cells(value) for name, value in fixed.items()}))
    return {"depths": case, "phase_of_F": (n + 1) % 10 if n == m else None,
            "seconds": round(time.monotonic() - started, 5), "checks": checks,
            "observations": observations, "grid_file": str(path)}


def analyze_matched(results, args):
    matched = {row["depths"][0]: row for row in results
               if len(row["depths"]) == 4 and len(set(row["depths"])) == 1}
    period_observations = {}
    for operation in OPERATIONS:
        period_observations[operation] = {}
        for representation in ("positive", "canonical"):
            periods = []
            for period in (10, 20, 50, 100, 200, 500):
                pairs = [(n, n + period) for n in sorted(matched) if n >= 20 and n + period in matched]
                if not pairs:
                    continue
                mismatches = [(n, m) for n, m in pairs if
                              matched[n]["observations"][operation][representation + "_window"] !=
                              matched[m]["observations"][operation][representation + "_window"]]
                periods.append({"candidate_period": period, "pairs_checked": len(pairs),
                                "all_tested_windows_match": not mismatches,
                                "first_mismatching_depths": mismatches[:3]})
            period_observations[operation][representation] = periods
    comparisons = []
    for n, m in ((20, 30), (20, 120), (20, 520)):
        if n not in matched or m not in matched:
            continue
        first = {name: grid(value) for name, value in json.loads(Path(matched[n]["grid_file"]).read_text()).items()}
        second = {name: grid(value) for name, value in json.loads(Path(matched[m]["grid_file"]).read_text()).items()}
        for operation in OPERATIONS:
            comparisons.append({"N1": n, "N2": m, "operation": operation,
                                "positive": first_difference(first[operation], second[operation]),
                                "canonical": first_difference(canonical(first[operation]), canonical(second[operation])),
                                "finite_carry_equal": carry_equal(first[operation], second[operation]),
                                "anchors": [first[operation].get(ANCHORS[operation], 0), second[operation].get(ANCHORS[operation], 0)]})
    return {"window_radius": args.radius, "candidate_period_checks": period_observations,
            "same_input_phase_comparisons": comparisons,
            "warning": "These are finite experimental window comparisons, not proofs of universal periods or equality of full grids."}


def analyze_independent(results):
    index = {tuple(row["depths"]): row for row in results}
    comparisons = []
    base = (20, 20, 20, 20)
    if base not in index:
        return comparisons
    first = {name: grid(value) for name, value in json.loads(Path(index[base]["grid_file"]).read_text()).items()}
    for case in ((20, 20, 30, 30), (20, 20, 120, 120), (20, 20, 20, 20, 30, 30)):
        if case not in index:
            continue
        second = {name: grid(value) for name, value in json.loads(Path(index[case]["grid_file"]).read_text()).items()}
        pairs = (("FL", "FL"), ("FR", "FR")) if len(case) == 4 else (("F2", "FG"),)
        for name1, name2 in pairs:
            comparisons.append({"first_depths": base, "second_depths": case,
                                "first_operation": name1, "second_operation": name2,
                                "positive": first_difference(first[name1], second[name2]),
                                "canonical": first_difference(canonical(first[name1]), canonical(second[name2])),
                                "finite_carry_equal": carry_equal(first[name1], second[name2])})
    return comparisons


def analyze_pair_cutoffs(results):
    """Same input phase digits, with one or both actual cutoffs changed."""
    index = {tuple(row["depths"]): row for row in results}
    specifications = [
        ("left cutoff +10, right cutoff fixed",
         (20, 20, 20, 20, 25, 25), (30, 30, 30, 30, 25, 25))]
    for offset in (1, 10):
        for second in (30, 120):
            specifications.append((
                f"both cutoffs grow, right minus left = {offset}",
                (20, 20, 20, 20, 20 + offset, 20 + offset),
                (second, second, second, second, second + offset, second + offset)))
    comparisons = []
    for description, first_case, second_case in specifications:
        if first_case not in index or second_case not in index:
            continue
        first = grid(json.loads(Path(index[first_case]["grid_file"]).read_text())["FG"])
        second = grid(json.loads(Path(index[second_case]["grid_file"]).read_text())["FG"])
        comparisons.append({
            "comparison": description,
            "first_depths": first_case, "second_depths": second_case,
            "input_phases": [(first_case[0] + 1) % 10, (first_case[4] + 1) % 10],
            "positive": first_difference(first, second),
            "canonical": first_difference(canonical(first), canonical(second)),
            "finite_carry_equal": carry_equal(first, second),
        })
    return comparisons


def phase_pair_representatives(results):
    representatives = []
    for result in results:
        depths = result["depths"]
        if len(depths) != 6:
            continue
        n, m, p, q, u, v = depths
        if n != m or u != v or not (20 <= n <= 29 and 20 <= u <= 29):
            continue
        observation = result["observations"]["FG"]
        representatives.append({"left_phase": (n + 1) % 10, "right_phase": (u + 1) % 10,
                                "left_depth": n, "right_depth": u,
                                "positive_anchor": observation["positive_anchor"],
                                "canonical_anchor": observation["canonical_anchor"],
                                "positive_window": observation["positive_window"],
                                "canonical_window": observation["canonical_window"],
                                "boundary_certificate": observation["boundary_certificate"]})
    return {"count": len(representatives), "representatives": representatives,
            "warning": "These are products of specified finite representatives, not a multiplication table depending only on the two phase digits."}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, default=REPOSITORY / "build/fraxis/libmdn.so")
    parser.add_argument("--work", type=Path, default=DEFAULT_WORK)
    parser.add_argument("--build-log", type=Path, default=DEFAULT_LOG)
    parser.add_argument("--compiler", default="c++")
    parser.add_argument("--timeout", type=float, default=60)
    parser.add_argument("--radius", type=int, default=3)
    parser.add_argument("--quick", action="store_true")
    parser.add_argument("--output", type=Path, default=HERE / "backend_results.json")
    args = parser.parse_args()
    args.library = args.library.resolve()
    args.work.mkdir(parents=True, exist_ok=True)
    started = time.monotonic()
    executable, command = compile_probe(args)
    if args.quick:
        cases = [(n, n, n, n) for n in (0, 20, 30)]
    else:
        cases = [(n, n, n, n) for n in range(130)]
        cases += [(520, 520, 520, 520)]
        cases += [(20, 20, 30, 30), (20, 20, 120, 120), (20, 20, 20, 20, 30, 30),
                  (20, 20, 20, 20, 120, 120), (20, 21, 20, 21)]
        cases += [(n, n, n, n, m, m) for n in range(20, 30) for m in range(20, 30)]
        cases += [(30, 30, 30, 30, 25, 25)]
        cases += [(n, n, n, n, n + offset, n + offset)
                  for n in (20, 30, 120) for offset in (1, 10)]
        cases = list(dict.fromkeys(cases))
    results, failures = [], []
    for i, case in enumerate(cases, 1):
        try:
            result = check_case(executable, case, args)
            results.append(result)
            bad = [name for name, passed in result["checks"].items() if not passed]
            if bad:
                failures.append({"depths": case, "checks": bad})
            if i % 20 == 0 or max(case) >= 120 or args.quick:
                print(f"Case {i}/{len(cases)} {case}: {result['seconds']} s; failures={bad}", flush=True)
        except Exception as error:
            failures.append({"depths": case, "exception": repr(error)})
            print("FAILED", case, repr(error), flush=True)
    summary = {
        "scope": "Complete finite grids, exact text tenths, no clipping or scalar interpretation of MDN positions.",
        "base": 10, "sign_policy": "Positive", "precision": -1,
        "canonical_digit_set": [-4, 5],
        "library": str(args.library), "library_sha256": hashlib.sha256(args.library.read_bytes()).hexdigest(),
        "compiler": subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0],
        "compile_command": command,
        "dynamic_linkage": subprocess.check_output(["ldd", str(executable)], text=True).splitlines(),
        "python": platform.python_version(), "checks_runtime_seconds": round(time.monotonic() - started, 3),
        "case_count": len(cases), "completed_case_count": len(results),
        "exact_check_count": sum(len(row["checks"]) for row in results),
        "failed_checks": sum(not passed for row in results for passed in row["checks"].values()),
        "failures": failures, "full_grid_directory": str(args.work),
        "matched_analysis": analyze_matched(results, args),
        "independent_cutoff_analysis": analyze_independent(results),
        "pair_cutoff_analysis": analyze_pair_cutoffs(results),
        "phase_pair_representatives": phase_pair_representatives(results),
        "results": results,
    }
    summary["runtime_seconds"] = round(time.monotonic() - started, 3)
    args.output.write_text(json.dumps(summary, indent=2) + "\n")
    print(f"{len(results)}/{len(cases)} cases; {summary['exact_check_count']} exact checks; "
          f"{len(failures)} failures. Results: {args.output}")
    return bool(failures)


if __name__ == "__main__":
    raise SystemExit(main())
