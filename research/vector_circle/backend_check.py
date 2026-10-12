#!/usr/bin/env python3
"""Test vector/circle proposals with exact finite native MDN grids and boundaries.

All inputs are exact decimal strings. No scalar evaluation of grid shifts and no
floating-point trigonometry is used. (sin(theta), cos(theta))=(.6,.8) is an exact
unit-circle point. Signed variants and axial endpoints are also checked.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shlex
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent
REPOSITORY = HERE.parents[1]
sys.path.insert(0, str(HERE.parent / "fuzzy"))
sys.path.insert(0, str(HERE.parent / "phase_products"))
from family import (CascadeProduct, carry_equal, grid_add, grid_multiply,
                    grid_scale, q_grid)
from canonical import normalize, verify_certificate, first_difference, window


def cells(value):
    return sorted([x, y, digit] for (x, y), digit in value.items() if digit)


def grid(value):
    return {(x, y): int(digit) for x, y, digit in value if digit}


def compile_probe(args):
    executable = args.work / "backend_probe"
    command = [args.compiler, "-std=c++17", "-O2", "-I", str(REPOSITORY / "library/include"),
               str(HERE / "backend_probe.cpp"), "-L", str(args.library.parent),
               "-Wl,-rpath," + str(args.library.parent), "-lmdn", "-o", str(executable)]
    with args.build_log.open("a") as log:
        log.write("\nVector/circle probe compile: " + shlex.join(command) + "\n")
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
    depth, sine, cosine = case
    n = depth+1
    path = args.work / ("case_" + "_".join(map(str, case)) + ".json")
    native = subprocess.run([str(executable), *map(str, case)], capture_output=True,
                            text=True, check=True, timeout=args.timeout)
    path.write_text(native.stdout)
    values = {name: grid(value) for name, value in json.loads(native.stdout).items()}
    L = CascadeProduct((depth,), ()).grid()
    R = CascadeProduct((), (depth,)).grid()
    one = {(0, 0): 1}

    def input_grid(numerator, fraxis):
        if abs(numerator) == 10:
            return grid_scale(one, numerator//10)
        return grid_scale(L if fraxis == "X" else R, numerator)

    A, B = input_grid(sine, "X"), input_grid(cosine, "Y")
    C, D = input_grid(sine, "Y"), input_grid(cosine, "X")
    ac, ad, bc, bd = [grid_multiply(p, q) for p, q in ((A,C), (A,D), (B,C), (B,D))]
    vector, reflected = grid_add(A, B), grid_add(C, D)
    signflip, reflected_signflip = grid_add(A, grid_scale(B, -1)), grid_add(C, grid_scale(D, -1))
    orthogonal = grid_add(input_grid(-cosine, "X"), input_grid(sine, "Y"))
    reflected_orthogonal = grid_add(input_grid(-cosine, "Y"), input_grid(sine, "X"))
    reflected_orthogonal_signflip = grid_add(input_grid(-cosine, "Y"), grid_scale(input_grid(sine, "X"), -1))
    raw = {"L": L, "R": R, "oneX": one, "oneY": one,
           "A": A, "B": B, "C": C, "D": D,
           "vector": grid_add(A, B), "cross": grid_add(ac, bd),
           "squares": grid_add(grid_multiply(A, A), grid_multiply(B, B)),
           "factored": grid_multiply(grid_add(A, B), grid_add(C, D)),
           "expanded": grid_add(ac, ad, bc, bd), "omitted": grid_add(ad, bc),
           "tenL": grid_scale(L, 10), "tenR": grid_scale(R, 10),
           "reflectedVector": reflected, "signflip": signflip,
           "reflectedSignflip": reflected_signflip,
           "normPair": grid_add(grid_multiply(vector, reflected), grid_multiply(signflip, reflected_signflip)),
           "orthogonal": orthogonal, "reflectedOrthogonal": reflected_orthogonal,
           "reflectedOrthogonalSignflip": reflected_orthogonal_signflip,
           "dotPair": grid_add(grid_multiply(vector, reflected_orthogonal), grid_multiply(signflip, reflected_orthogonal_signflip)),
           "determinantPair": grid_add(grid_multiply(vector, reflected_orthogonal), grid_scale(grid_multiply(reflected, orthogonal), -1))}
    checks, fixed, certificates = {}, {}, {}
    for name, value in values.items():
        normalized = normalize(value)
        oracle = normalize(raw[name])
        fixed[name] = normalized.grid
        checks[name + "_exact_canonical_oracle"] = normalized.grid == oracle.grid
        checks[name + "_native_carry_certificate"] = verify_certificate(value, normalized)
        checks[name + "_oracle_carry_certificate"] = verify_certificate(raw[name], oracle)
        certificates[name] = {"canonical": cells(normalized.grid),
                              "native_carries": cells(normalized.carries),
                              "raw": cells(raw[name]), "oracle_carries": cells(oracle.carries)}
    checks["distributivity_complete_grids"] = fixed["factored"] == fixed["expanded"]
    checks["literal_axis_units_identical"] = values["oneX"] == values["oneY"] == one
    checks["oriented_tenths_times_ten_remain_distinct"] = fixed["tenL"] != fixed["tenR"]
    checks["tenL_endpoint_identity"] = carry_equal(values["tenL"], q_grid({0: 1, n: -1}))
    checks["tenR_endpoint_identity"] = carry_equal(values["tenR"], q_grid({0: 1, -n: -1}))
    checks["matched_cutoff_relation_10LR_equals_L_plus_R"] = carry_equal(grid_scale(grid_multiply(values["L"], values["R"]), 10), grid_add(values["L"], values["R"]))
    checks["paired_norm_equals_twice_cross_expression"] = fixed["normPair"] == normalize(grid_scale(values["cross"], 2)).grid
    checks["paired_orthogonal_dot_is_exactly_zero"] = fixed["dotPair"] == {}
    ordinary_endpoint = abs(sine) == 10 or abs(cosine) == 10
    if ordinary_endpoint:
        checks["axial_circle_identities_are_unity"] = fixed["cross"] == fixed["squares"] == one
        checks["axial_omitted_terms_are_zero"] = fixed["omitted"] == {}
        checks["axial_literal_unit_collapse_erases_determinant"] = fixed["determinantPair"] == {}
    else:
        assert sine*sine+cosine*cosine == 100
        checks["cross_boundary_identity"] = carry_equal(values["cross"], q_grid({0: 2, n: -1, -n: -1}))
        square_ledger = {0: 100, n: -2*sine*sine, 2*n: sine*sine,
                         -n: -2*cosine*cosine, -2*n: cosine*cosine}
        checks["squares_boundary_identity"] = carry_equal(grid_scale(values["squares"], 100), q_grid(square_ledger))
        checks["cross_is_not_unity"] = fixed["cross"] != one
        checks["squares_are_not_unity"] = fixed["squares"] != one
        checks["omitted_terms_are_nonzero"] = fixed["omitted"] != {}
        checks["cannot_drop_distributive_terms"] = fixed["factored"] != fixed["cross"]
        checks["paired_norm_boundary_identity"] = carry_equal(values["normPair"], q_grid({0: 4, n: -2, -n: -2}))
        determinant_oracle = grid_scale(grid_add(grid_multiply(L, L), grid_scale(grid_multiply(R, R), -1)), 100)
        checks["determinant_preserves_oriented_area_factor"] = fixed["determinantPair"] == normalize(determinant_oracle).grid
        checks["nonaxial_determinant_is_nonzero"] = fixed["determinantPair"] != {}
        checks["determinant_is_exact_boundary_only_ledger"] = carry_equal(values["determinantPair"], q_grid({n: -2, 2*n: 1, -n: 2, -2*n: -1}))
    (args.work / (path.stem + "_certificates.json")).write_text(json.dumps(certificates, separators=(",", ":")))
    return {"depth": depth, "term_count": n, "sine_tenths": sine,
            "cosine_tenths": cosine, "checks": checks,
            "supports": {name: len(value) for name, value in values.items()},
            "nearest_differences_from_unity": {
                name: first_difference(fixed[name], one) for name in ("cross", "squares", "vector")},
            "nearest_omitted_term": first_difference(fixed["omitted"], {}),
            "canonical_radius4_windows": {name: cells(window(fixed[name], 4))
                for name in ("cross", "squares", "vector", "tenL", "tenR", "normPair", "dotPair", "determinantPair")},
            "grid_file": str(path)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, default=REPOSITORY / "build/fraxis/libmdn.so")
    parser.add_argument("--work", type=Path, default=Path("/tmp/mdn-vector-circle-01a12881"))
    parser.add_argument("--build-log", type=Path, default=Path.home() / "output_01a12881-e6c8-7cb2-a1d2-7c899e17aa69.log")
    parser.add_argument("--compiler", default="c++")
    parser.add_argument("--timeout", type=float, default=60)
    parser.add_argument("--output", type=Path, default=HERE / "backend_results.json")
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    args.library = args.library.resolve()
    started = time.monotonic()
    before = hashlib.sha256(args.library.read_bytes()).hexdigest()
    executable, command = compile_probe(args)
    cases = [(depth, sine, cosine) for depth in (0, 1, 5, 20, 40)
             for sine, cosine in ((6, 8), (8, 6), (-6, 8), (6, -8), (-6, -8),
                                  (0, 10), (10, 0), (0, -10), (-10, 0))]
    results, failures = [], []
    for case in cases:
        try:
            result = check_case(executable, case, args)
            results.append(result)
            bad = [name for name, ok in result["checks"].items() if not ok]
            if bad:
                failures.append({"case": case, "failed_checks": bad})
                print("FAILED", case, bad, flush=True)
        except Exception as error:
            failures.append({"case": case, "exception": repr(error)})
            print("FAILED", case, repr(error), flush=True)
    summary = {
        "scope": "Complete finite native grids and explicit carry certificates; all endpoint terms retained.",
        "base": 10, "native_sign_policy": "Positive", "precision": -1,
        "canonical_digit_set": [-4, 5], "library": str(args.library),
        "library_sha256": before,
        "library_unchanged": hashlib.sha256(args.library.read_bytes()).hexdigest() == before,
        "compile_command": command, "build_log": str(args.build_log),
        "case_count": len(cases), "completed_case_count": len(results),
        "check_count": sum(len(r["checks"]) for r in results),
        "failures": failures, "seconds": round(time.monotonic()-started, 3),
        "identities": {
            "definitions": "L=finite 0.1X, R=finite 0.1Y, n=depth+1, E=(-Y/X)^n",
            "cross_3_4_5": "(6L)(6R)+(8L)(8R)=100LR=2-E-E^-1",
            "square_3_4_5": "100*((6L)^2+(8R)^2)=36*(1-E)^2+64*(1-E^-1)^2",
            "expansion_3_4_5": "(6L+8R)(6R+8L)=100LR+48*(L^2+R^2)",
            "oriented_unit_approaches": "10L=1-E; 10R=1-E^-1",
            "matched_cutoff_relation": "10LR=L+R",
            "paired_norm_3_4_5": "V*sigma(V)+J(V)*sigma(J(V))=200LR=4-2E-2E^-1, where V=6L+8R and J(V)=6L-8R",
            "paired_orthogonal_dot": "V*sigma(W)+J(V)*sigma(J(W))=0 for V=6L+8R and W=-8L+6R",
            "oriented_area": "V*sigma(W)-sigma(V)*W=100*(L^2-R^2) for V=6L+8R and W=-8L+6R",
            "oriented_area_boundary": "100*(L^2-R^2)=-2E+E^2+2E^-1-E^-2; a boundary-only nonzero carrier",
            "operation_scope": "sigma swaps X/Y. J negates the retained Y component; the checker uses the known input decomposition, not an untagged grid-only definition of J.",
            "literal_units": "Entering decimal 1 on either fraxis gives the same unity cell."
        }, "results": results}
    args.output.write_text(json.dumps(summary, indent=2)+"\n")
    print(json.dumps({key: summary[key] for key in ("case_count", "completed_case_count", "check_count", "failures", "library_unchanged", "seconds")}, indent=2))
    return bool(failures)


if __name__ == "__main__":
    raise SystemExit(main())
