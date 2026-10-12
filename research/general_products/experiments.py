#!/usr/bin/env python3
"""Reproduce exact cycle certificates through order16 and the order10 collision."""

import json
from pathlib import Path

from periods import period_certificate, period_test, layer_difference, same_phase


def run(max_order=16):
    certificates = []
    failures = []
    for d in range(2, max_order+1):
        for a in range(1, d):
            c = d-a
            certificate = period_certificate(a, c)
            period = certificate["period"]
            certificate["accepted_period_check"] = period_test(a, c, period)
            certificate["next_smaller_prime_divisors"] = {
                str(p): period_test(a, c, period//p)
                for p in (2, 5) if period % p == 0}
            if not certificate["accepted_period_check"]["passes"]:
                failures.append({"a": a, "c": c, "kind": "accepted_period"})
            if any(result["passes"] for result in certificate["next_smaller_prime_divisors"].values()):
                failures.append({"a": a, "c": c, "kind": "minimality"})
            certificates.append(certificate)
    by_pair = {(row["a"], row["c"]): row["period"] for row in certificates}
    for (a, c), period in by_pair.items():
        if by_pair[c, a] != period:
            failures.append({"a": a, "c": c, "kind": "reflection"})

    shift = 125000000
    even_nodes = []
    for layer in range(9):
        degree = 8-layer
        modulus = 10**(layer+1)
        for m in range(degree+1):
            for k in range(degree+1-m):
                residue = layer_difference(2, 8, 2*m, k, shift, layer) % modulus
                even_nodes.append({"layer": layer, "m": m, "k": k, "residue": residue})
                if residue:
                    failures.append({"kind": "even_collision", "layer": layer, "m": m, "k": k})
    return {
        "scope": "Finite exact polynomial/carry congruence certificates, not native runs at the enormous resulting periods.",
        "base": 10, "max_order": max_order, "mixed_pairs": len(certificates),
        "warning": "Minimal universal cycle length need not equal the number of distinct phase arrays.",
        "short_lower_bound_exceptions": [
            {k: row[k] for k in ("a", "c", "period", "lower_bound")}
            for row in certificates if row["period"] != row["lower_bound"]],
        "order10_collision": {
            "a": 2, "c": 8, "period": by_pair[2, 8], "half_period": shift,
            "even_n_pair_equal": same_phase(2, 8, 2, 2+shift),
            "odd_n_pair_equal": same_phase(2, 8, 1, 1+shift),
            "even_count_Newton_certificate": even_nodes,
            "distinct_arrays": 187500000,
            "count_proof": "PHASE_COLLISIONS.md: top difference restricts collisions to half-period shifts; even residues pair, odd residues do not."
        },
        "certificates": certificates, "failures": failures
    }


if __name__ == "__main__":
    result = run()
    path = Path(__file__).resolve().with_name("period_results.json")
    path.write_text(json.dumps(result, indent=2)+"\n")
    print(f"{result['mixed_pairs']} exact minimal-period certificates; "
          f"{len(result['short_lower_bound_exceptions'])} shortcut exceptions; "
          f"{len(result['failures'])} failures. Saved {path}")
    raise SystemExit(bool(result["failures"]))
