#!/usr/bin/env python3
"""Explore corrected-backend display cycles and exact decimal-prefix families.

Run from any directory: python3 research/decimal_infinite/backend_check.py
Only the probe is compiled; the existing corrected library is not rebuilt.
Compiler output is duplicated to the required session log. Full grids stay
in --work; compact provenance and results are written to backend_results.json.

Every prefix receives an exact full carry-polynomial check. The optional
X=4,Y=6 and X=5,Y=5 evaluations are explicitly chosen diagnostic projections,
not MDN magnitudes. Infinite-digit comparisons hold cascade term count fixed;
they do not assert a simultaneous infinite mixed-cascade limit.
"""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from fractions import Fraction
from functools import lru_cache
import hashlib
import json
from math import comb, isqrt
from pathlib import Path
import platform
import shlex
import subprocess
import time

REPOSITORY = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
DEFAULT_WORK = Path("/tmp/mdn-decimal-infinite-01a12881")
DEFAULT_LOG = Path.home() / "output_01a12881-e6c8-7cb2-a1d2-7c899e17aa69.log"
TRACKED = ((-1, -1), (-2, 0), (0, -2), (-1, 0), (0, -1), (0, 0), (-2, 1), (1, -2))


def grid(cells):
    return {(x, y): int(d) for x, y, d in cells if d}


def add(left, right, factor=1):
    result = defaultdict(int, left)
    for position, coefficient in right.items():
        result[position] += factor * coefficient
    return {position: coefficient for position, coefficient in result.items() if coefficient}


@lru_cache(maxsize=2048)
def binomial_row(degree, base):
    return tuple(comb(degree, k) * base**(degree - k) * (-1)**k for k in range(degree + 1))


def carry_equal(left, right, base=10):
    difference = add(left, right, -1)
    if not difference:
        return True
    min_x = min(x for x, y in difference)
    min_y = min(y for x, y in difference)
    coefficients = defaultdict(int)
    for (x, y), value in difference.items():
        for k, coefficient in enumerate(binomial_row(y - min_y, base)):
            coefficients[x - min_x + k] += value * coefficient
    return not any(coefficients.values())


def raw_prefix(digits, term_count, direction):
    """Direct binomial digit-cascade expansion, before any carrying."""
    result = defaultdict(int)
    for place, character in enumerate(digits, 1):
        digit = int(character)
        if not digit:
            continue
        for j in range(term_count):
            coordinate = (-place - j, j) if direction == "X" else (j, -place - j)
            result[coordinate] += digit * (-1)**j * comb(place + j - 1, j)
    return {position: coefficient for position, coefficient in result.items() if coefficient}


def project(value, x, y):
    return sum((digit * Fraction(x)**i * Fraction(y)**j for (i, j), digit in value.items()), Fraction())


def prefix_projection(digits, term_count, primary, other):
    return sum((int(digit) * Fraction(primary)**(-place) *
                sum(((-Fraction(other, primary))**j * comb(place + j - 1, j)
                     for j in range(term_count)), Fraction())
                for place, digit in enumerate(digits, 1)), Fraction())


def digit_tail_bound(prefix_length, term_count, primary, other, max_digit=9):
    """Universal exact bound for any omitted digits in 0..max_digit.

    Requires primary>1, other>=0. Uses the full positive generating series
    sum(k>=1) binom(k+j-1,j)t**k=t/(1-t)**(j+1), t=1/primary.
    """
    if primary <= 1 or other < 0:
        raise ValueError("Require primary>1 and other>=0")
    t = Fraction(1, primary)
    total = Fraction()
    for j in range(term_count):
        tail = t / (1 - t)**(j + 1) - sum(
            (comb(k + j - 1, j) * t**k for k in range(1, prefix_length + 1)), Fraction())
        if tail < 0:
            raise AssertionError("Exact positive tail became negative")
        total += Fraction(other, primary)**j * tail
    return max_digit * total


def periodic_digit_limit(period, term_count, primary, other):
    """Exact rational generating-function jet, at FIXED term count.

    D(z)=sum(d_r*z**r)/(1-z**p). Coefficients in u of D(t/(1-u))
    are the digit-weighted binomial sums required by the cascade.
    """
    p, t = len(period), Fraction(1, primary)
    numerator = [Fraction(0)] * max(p, term_count)
    for r, character in enumerate(period, 1):
        for j in range(p - r + 1):
            numerator[j] += int(character) * t**r * (-1)**j * comb(p - r, j)
    denominator = [Fraction((-1)**j * comb(p, j)) for j in range(p + 1)]
    denominator[0] -= t**p
    coefficients = []
    for j in range(term_count):
        convolution = sum((denominator[i] * coefficients[j - i]
                           for i in range(1, min(j, p) + 1)), Fraction())
        coefficients.append((numerator[j] - convolution) / denominator[0])
    return sum((coefficient * (-Fraction(other, primary))**j
                for j, coefficient in enumerate(coefficients)), Fraction())


def sqrt2_minus_one_digits(length):
    """Floor(sqrt(2)*10**length)-10**length, with integer square root only."""
    scale = 10**length
    return str(isqrt(2 * scale * scale) - scale).zfill(length)


def scientific(value):
    return f"{float(value):.8g}"


def compile_probe(args):
    executable = args.work / "backend_probe"
    command = [args.compiler, "-std=c++17", "-O2", "-I", str(REPOSITORY / "library/include"),
               str(HERE / "backend_probe.cpp"), "-L", str(args.library.parent),
               "-Wl,-rpath," + str(args.library.parent), "-lmdn", "-o", str(executable)]
    with args.build_log.open("a") as log:
        log.write("\nDecimal family probe compile: " + shlex.join(command) + "\n")
        log.flush()
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        for line in process.stdout:
            print(line, end="", flush=True)
            log.write(line)
            log.flush()
        if process.wait():
            raise RuntimeError("Probe compile failed; see " + str(args.build_log))
    return executable, command


def run_probe(executable, args, depth, direction, text, extra=()):
    command = [str(executable), "10", str(depth), "2", direction, text, *extra]
    started = time.monotonic()
    completed = subprocess.run(command, text=True, capture_output=True, check=True, timeout=args.timeout)
    return json.loads(completed.stdout), time.monotonic() - started


def matched_display_candidate(depth):
    """A two-layer finite carry representation observed for Positive base10.

    The remainder choice on odd k is in -8..1, on even k in 0..9.
    Carries from the first layer give the second layer exactly. This function
    constructs a carry-equivalent grid; its agreement with this backend's
    particular display policy is tested separately rather than assumed.
    """
    result, carries = {}, {}
    for k in range(-depth, depth + 1):
        amplitude = depth + 1 - abs(k)
        raw = (-1 if k % 2 else 1) * amplitude
        digit = 1 - (amplitude + 1) % 10 if k % 2 else amplitude % 10
        if digit:
            result[k - 1, -k - 1] = digit
        if (raw - digit) % 10:
            raise AssertionError("Noninteger carry in display formula")
        carries[k] = (raw - digit) // 10
    for h in range(-depth, depth + 2):
        digit = carries.get(h, 0) + carries.get(h - 1, 0)
        if digit:
            result[h - 1, -h] = digit
    return result


def same_phase_comparisons(args):
    comparisons = []
    for n1, n2 in ((20, 30), (20, 120), (30, 120)):
        if n2 > args.max_depth:
            continue
        first = grid(json.loads((args.work / f"depth_{n1}.json").read_text())["result"])
        second = grid(json.loads((args.work / f"depth_{n2}.json").read_text())["result"])
        differences = sorted(
            (max(abs(x), abs(y)), x, y, first.get((x, y), 0), second.get((x, y), 0))
            for x, y in first.keys() | second.keys()
            if first.get((x, y), 0) != second.get((x, y), 0))
        radius = differences[0][0] if differences else None
        comparisons.append({
            "N1": n1, "N2": n2,
            "anchor_digits": [first.get((-1, -1), 0), second.get((-1, -1), 0)],
            "first_different_maxnorm_radius": radius,
            "nearest_differences_x_y_digit1_digit2": [list(row[1:]) for row in differences if row[0] == radius],
            "support_sizes": [len(first), len(second)],
            "finite_carry_equal": carry_equal(first, second),
        })
    return comparisons


def sweep_depths(executable, args):
    rows, failures = [], []
    for depth in range(args.max_depth + 1):
        cells, seconds = run_probe(executable, args, depth, "X", "0.1", ("multiply", "Y", "0.1"))
        (args.work / f"depth_{depth}.json").write_text(json.dumps(cells))
        value = grid(cells["result"])
        n = depth + 1
        expected = {(0, 0): 2, (-n, n): -(-1)**n, (n, -n): -(-1)**n}
        exact = carry_equal({position: 100 * digit for position, digit in value.items()}, expected)
        bounded = all(abs(digit) < 10 for digit in value.values())
        candidate = matched_display_candidate(depth)
        display_match = value == candidate
        if not (exact and bounded and display_match):
            failures.append({"kind": "depth", "N": depth, "identity": exact,
                             "bounded": bounded, "display_formula": display_match})
        rows.append({
            "N": depth, "n": n, "backend_seconds": round(seconds, 5),
            "uncarried_central_coefficient": n,
            "tracked_digits": [value.get(position, 0) for position in TRACKED],
            "window_radius2": sorted([x, y, d] for (x, y), d in value.items() if abs(x) <= 2 and abs(y) <= 2),
            "support_size": len(value),
            "degree_histogram": dict(sorted(Counter(x + y for x, y in value).items())),
            "exact_boundary_identity": exact, "bounded_digits": bounded,
            "two_layer_display_formula_matches": display_match,
        })
    phase_check = all(row["tracked_digits"][0] == (row["N"] + 1) % 10 for row in rows)
    period_start = next((start for start in range(max(0, len(rows) - 20))
                         if all(rows[j]["window_radius2"] == rows[j - 10]["window_radius2"]
                                for j in range(start + 10, len(rows)))), None)
    return {"cases": len(rows), "tracked_coordinates": TRACKED,
            "observed_central_digit_formula": "(N+1) modulo 10",
            "central_digit_formula_holds_in_sweep": phase_check,
            "radius2_observed_period": 10 if period_start is not None else None,
            "radius2_period_starts_at_N": period_start,
            "two_layer_display_formula": {
                "first_layer_position": "P_k=(k-1,-k-1), -N<=k<=N",
                "amplitude": "a=N+1-|k|",
                "first_layer_digit": "a mod10 for even k; 1-((a+1) mod10) for odd k",
                "carry": "t_k=((-1)^k*a-d_k)/10, t_k=0 outside -N..N",
                "second_layer": "digit t_h+t_(h-1) at Q_h=(h-1,-h)",
                "first_layer_positions_checked": sum(2 * row["N"] + 1 for row in rows),
                "complete_display_matches_in_all_cases": all(row["two_layer_display_formula_matches"] for row in rows),
                "scope": "Exact finite carry construction; observed match to this backend/Positive display policy over the stated sweep.",
            },
            "same_anchor_different_boundaries": same_phase_comparisons(args),
            "warning": "Observed display periodicity is not equality of finite MDNs or a proof of an infinite normalized digit array.",
            "rows": rows}, failures


def prefix_cases(executable, args):
    rows, failures = [], []
    reference_digits = sqrt2_minus_one_digits(96)
    sources = {"third": ("3", "3" * 32), "seventh": ("142857", ("142857" * 6)[:32]),
               "sqrt2_minus_one": (None, reference_digits[:32])}
    for name, (period, full_digits) in sources.items():
        for n in (1, 3, 6, 11, 21):
            for length in (4, 8, 16, 32):
                digits = full_digits[:length]
                for direction in ("X", "Y"):
                    cells, seconds = run_probe(executable, args, n - 1, direction, "0." + digits)
                    (args.work / f"prefix_{name}_{n}_{length}_{direction}.json").write_text(json.dumps(cells))
                    value = grid(cells["left"])
                    expected = raw_prefix(digits, n, direction)
                    exact = carry_equal(value, expected)
                    bounded = all(abs(digit) < 10 for digit in value.values())
                    if not (exact and bounded):
                        failures.append({"kind": "prefix", "stream": name, "n": n, "K": length,
                                         "direction": direction, "exact": exact, "bounded": bounded})
                    diagnostics = []
                    for x, y in ((4, 6), (5, 5)):
                        primary, other = (x, y) if direction == "X" else (y, x)
                        actual = project(value, x, y)
                        direct = prefix_projection(digits, n, primary, other)
                        bound = digit_tail_bound(length, n, primary, other)
                        if actual != direct:
                            failures.append({"kind": "projection", "stream": name, "n": n, "K": length})
                        diagnostic = {"X": x, "Y": y, "prefix_projection": str(actual),
                                      "prefix_projection_approx": scientific(actual),
                                      "universal_omitted_digit_bound": str(bound),
                                      "universal_bound_approx": scientific(bound)}
                        if period:
                            limit = periodic_digit_limit(period, n, primary, other)
                            error = abs(actual - limit)
                            diagnostic.update({"fixed_n_digit_limit": str(limit),
                                               "fixed_n_digit_limit_approx": scientific(limit),
                                               "actual_error_approx": scientific(error),
                                               "error_within_universal_bound": error <= bound})
                            if error > bound:
                                failures.append({"kind": "periodic_bound", "stream": name, "n": n, "K": length})
                            if period == "3":
                                closed = Fraction(1, 3) * (1 - (-Fraction(other, primary - 1))**n)
                                if closed != limit:
                                    failures.append({"kind": "third_closed_formula", "n": n})
                        else:
                            reference = prefix_projection(reference_digits, n, primary, other)
                            reference_bound = digit_tail_bound(96, n, primary, other)
                            compatible = abs(actual - reference) <= bound + reference_bound
                            diagnostic.update({"reference_prefix_K": 96,
                                               "certified_digit_limit_interval": [str(reference - reference_bound), str(reference + reference_bound)],
                                               "interval_midpoint_approx": scientific(reference),
                                               "interval_radius_approx": scientific(reference_bound),
                                               "prefix_compatible_with_interval": compatible})
                            if not compatible:
                                failures.append({"kind": "nonperiodic_bound", "n": n, "K": length})
                        diagnostics.append(diagnostic)
                    rows.append({"stream": name, "digits": digits, "n": n, "N": n - 1, "K": length,
                                 "direction": direction, "backend_seconds": round(seconds, 5),
                                 "support_size": len(value), "exact_carry_check": exact,
                                 "bounded_digits": bounded, "diagnostic_projections": diagnostics})
            print(f"Checked {name}, cascade term count n={n}", flush=True)
    return {"cases": len(rows), "sqrt2_digits_method": "integer square root of 2*10^(2K), subtract 10^K",
            "warning": "Projection digit limits hold n fixed; chosen X,Y are diagnostic maps, not canonical magnitudes.",
            "rows": rows}, failures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, default=REPOSITORY / "build/fraxis/libmdn.so")
    parser.add_argument("--work", type=Path, default=DEFAULT_WORK)
    parser.add_argument("--build-log", type=Path, default=DEFAULT_LOG)
    parser.add_argument("--compiler", default="c++")
    parser.add_argument("--timeout", type=float, default=30)
    parser.add_argument("--max-depth", type=int, default=120)
    parser.add_argument("--output", type=Path, default=HERE / "backend_results.json")
    args = parser.parse_args()
    if args.max_depth < 0:
        parser.error("--max-depth must be nonnegative")
    args.library = args.library.resolve()
    args.work.mkdir(parents=True, exist_ok=True)
    args.build_log.parent.mkdir(parents=True, exist_ok=True)
    started = time.monotonic()
    executable, command = compile_probe(args)
    sweep, failures = sweep_depths(executable, args)
    print(f"Checked {sweep['cases']} mixed depths", flush=True)
    prefixes, prefix_failures = prefix_cases(executable, args)
    failures.extend(prefix_failures)
    summary = {
        "library": str(args.library), "library_sha256": hashlib.sha256(args.library.read_bytes()).hexdigest(),
        "compiler": subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0],
        "compile_command": command,
        "dynamic_linkage": subprocess.check_output(["ldd", str(executable)], text=True).splitlines(),
        "python": platform.python_version(), "runtime_seconds": round(time.monotonic() - started, 3),
        "precision": -1, "sign_policy": "Positive", "base": 10,
        "full_grids_directory": str(args.work), "failures": failures,
        "depth_sweep": sweep, "prefix_cases": prefixes,
    }
    args.output.write_text(json.dumps(summary, indent=2) + "\n")
    print(f"{sweep['cases']} mixed-depth cases and {prefixes['cases']} prefix cases; "
          f"{len(failures)} failures; {summary['runtime_seconds']} s. Results: {args.output}")
    return bool(failures)


if __name__ == "__main__":
    raise SystemExit(main())
