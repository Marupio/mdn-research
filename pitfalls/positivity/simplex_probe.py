"""Small exact three-axis extension probe; not a full benchmark.

Work on x,y,z >= 0, x+y+z=1.  An MDN atom is c*x**i*y**j*z**k
with c >= 0.  A degree-n simplex Bernstein basis atom is the same monomial
multiplied by the positive multinomial coefficient n!/(i!j!k!).

Consequently, for ANY number of axes, the two degree-n nonnegative cones
are identical.  Any nonnegative mixed-degree MDN certificate of maximum
degree n homogenizes to that cone by multiplying each degree-d term by
(x_1+...+x_r)**(n-d)=1.  Conversely, the homogeneous certificate already is
an MDN certificate.  Mixing degrees can compress a certificate, but does
not enlarge the degree-bounded positivity cone.  This argument concerns
nonnegative integer exponents, rational coefficients, and a fixed simplex;
it does not assert completeness at fixed degree for all positive functions.

This probe compares a minimum-degree homogeneous certificate (cap 16) with
greedy inverse carries of equal mass from all three children to their
parent.  Six deterministic tie orders are tried.  No subdivision, linear
programming, factorization, or global support minimization is attempted.
The polynomial identity checker expands z=1-x-y by elementary polynomial
multiplication, independently of the multinomial homogenization routine.
"""

from __future__ import annotations

import argparse
from fractions import Fraction as Q
import json
from math import factorial
from pathlib import Path
from random import Random

try:
    from .common import coefficient_encodings, jsonable, uint_bits
except ImportError:
    from common import coefficient_encodings, jsonable, uint_bits


def coordinates(n):
    return [(i, j, n-i-j) for i in range(n+1) for j in range(n-i+1)]


def multinomial(alpha):
    result = factorial(sum(alpha))
    for exponent in alpha:
        result //= factorial(exponent)
    return result


def poly_add(a, b):
    result = dict(a)
    for exponent, coefficient in b.items():
        result[exponent] = result.get(exponent, Q(0)) + coefficient
    return {exponent: coefficient for exponent, coefficient in result.items() if coefficient}


def poly_mul(a, b):
    result = {}
    for (i, j), c in a.items():
        for (k, ell), d in b.items():
            exponent = (i+k, j+ell)
            result[exponent] = result.get(exponent, Q(0)) + c*d
    return {exponent: coefficient for exponent, coefficient in result.items() if coefficient}


def expand_atoms(atoms):
    """Independent exact checker expansion: repeated multiplication only."""
    z = {(0, 0): Q(1), (1, 0): Q(-1), (0, 1): Q(-1)}
    result = {}
    for (i, j, k), coefficient in atoms.items():
        term = {(i, j): Q(coefficient)}
        for _ in range(k):
            term = poly_mul(term, z)
        result = poly_add(result, term)
    return result


def strictly_positive_atoms(atoms):
    if not atoms or any(c < 0 for c in atoms.values()):
        return False
    # If a vertex value is positive, a positive pure power (or constant)
    # survives there.  At every point at least one coordinate is positive,
    # so these vertex witnesses also cover every edge and the interior.
    return all(sum(c for alpha, c in atoms.items()
                   if all(alpha[j] == 0 for j in range(3) if j != axis)) > 0
               for axis in range(3))


def verify(polynomial, atoms):
    if not strictly_positive_atoms(atoms):
        raise ValueError("Atom signs/corner values do not prove strict positivity")
    if expand_atoms(atoms) != polynomial:
        raise ValueError("Certificate identity differs from input polynomial")
    return True


def homogeneous_weights(polynomial, n):
    """Coefficients of homogenizing x**i*y**j with (x+y+z)**(n-i-j)."""
    degree = max((i+j for i, j in polynomial), default=0)
    if n < degree:
        raise ValueError("Homogeneous degree is below the polynomial degree")
    weights = {alpha: Q(0) for alpha in coordinates(n)}
    for (i, j), coefficient in polynomial.items():
        for a, b, c in coordinates(n-i-j):
            alpha = (i+a, j+b, c)
            weights[alpha] += coefficient*multinomial((a, b, c))
    return weights


def minimum_homogeneous(polynomial, max_degree=16):
    degree = max((i+j for i, j in polynomial), default=0)
    for n in range(degree, max_degree+1):
        weights = homogeneous_weights(polynomial, n)
        if strictly_positive_atoms(weights):
            verify(polynomial, weights)
            return n, weights
    return None


def inverse_carries(weights, order):
    """Greedy mass-preserving coarsening, with reproducible tie ordering."""
    atoms = {alpha: Q(c) for alpha, c in weights.items() if c}
    degree = max(map(sum, atoms), default=0)
    transcript = []
    for level in range(degree-1, -1, -1):
        candidates = coordinates(level)
        if order == "reverse":
            candidates.reverse()
        elif order.startswith("axis"):
            axis = int(order[-1])
            candidates.sort(key=lambda a: (a[axis], a[(axis+1) % 3], a[(axis+2) % 3]))
        elif order == "balanced":
            candidates.sort(key=lambda a: (max(a)-min(a), a))
        for parent in candidates:
            children = [tuple(parent[j] + (j == axis) for j in range(3)) for axis in range(3)]
            amount = min(atoms.get(child, Q(0)) for child in children)
            if amount <= 0:
                continue
            for child in children:
                atoms[child] -= amount
                if not atoms[child]:
                    del atoms[child]
            atoms[parent] = atoms.get(parent, Q(0)) + amount
            transcript.append((parent, amount))
    return atoms, transcript


def coefficient_bits(values):
    return min(bits for _, _, bits in coefficient_encodings(values))


def bernstein_metrics(n, weights):
    coeffs = [weights[alpha]/multinomial(alpha) for alpha in coordinates(n)]
    nonzero = [(alpha, c) for alpha, c in zip(coordinates(n), coeffs) if c]
    dense_bits = coefficient_bits(coeffs) + uint_bits(n)
    # Homogeneous sparse triples need only two stored indices: k=n-i-j.
    sparse_bits = coefficient_bits([c for _, c in nonzero]) + uint_bits(n)
    sparse_bits += sum(uint_bits(alpha[0])+uint_bits(alpha[1]) for alpha, _ in nonzero)
    return {"support": len(nonzero), "coefficient_slots": len(coeffs),
            "payload_bits": min(dense_bits, sparse_bits),
            "encoding": "dense" if dense_bits <= sparse_bits else "sparse"}


def mdn_metrics(atoms):
    atoms = {a: c for a, c in atoms.items() if c}
    indices = sum(sum(uint_bits(i) for i in alpha) for alpha in atoms)
    return {"support": len(atoms), "coefficient_slots": len(atoms),
            "payload_bits": coefficient_bits(list(atoms.values())) + indices,
            "max_degree": max(map(sum, atoms), default=0)}


def cases():
    result = []

    def planted(name, atoms):
        result.append((name, "planted_sparse", expand_atoms(atoms)))

    planted("constant", {(0, 0, 0): Q(1)})
    planted("sparse_cubic", {(0, 0, 0): Q(1, 10), (2, 1, 0): Q(3), (0, 1, 2): Q(2)})
    planted("sparse_sextic", {(0, 0, 0): Q(1, 100), (5, 1, 0): Q(7),
                             (0, 3, 3): Q(5), (1, 0, 5): Q(2)})
    planted("axis_bumps", {(0, 0, 0): Q(1, 10), (8, 0, 0): Q(1),
                          (0, 8, 0): Q(2), (0, 0, 8): Q(3)})
    planted("interior_bump", {(0, 0, 0): Q(1, 100), (2, 2, 2): Q(100)})
    planted("boundary_mixture", {(0, 0, 0): Q(1, 10), (6, 3, 0): Q(1),
                                (0, 6, 3): Q(2), (3, 0, 6): Q(3)})
    rng = Random(630821)
    for n in [6, 8, 10]:
        atoms = {(0, 0, 0): Q(1, 10)}
        for _ in range(5):
            d = rng.randint(2, n)
            i = rng.randrange(d+1)
            j = rng.randrange(d-i+1)
            atoms[(i, j, d-i-j)] = Q(rng.randint(1, 7))
        planted(f"seeded_sparse_{n}", atoms)

    forms = {"x_minus_y": (1, -1, 0), "2x_minus_y_minus_z": (2, -1, -1),
             "x_plus_2y_minus_3z": (1, 2, -3)}
    for name, (a, b, c) in forms.items():
        form = expand_atoms({(1, 0, 0): Q(a), (0, 1, 0): Q(b), (0, 0, 1): Q(c)})
        for epsilon in [Q(1), Q(1, 4), Q(1, 20)]:
            polynomial = poly_add(poly_mul(form, form), {(0, 0): epsilon})
            result.append((f"square_{name}_eps_{epsilon}", "quadratic_square", polynomial))
    return result


def run(max_degree=16):
    records = []
    for name, family, polynomial in cases():
        record = {"name": name, "family": family,
                  "polynomial_xy": [(i, j, c) for (i, j), c in sorted(polynomial.items())]}
        homogeneous = minimum_homogeneous(polynomial, max_degree)
        if homogeneous is None:
            record.update(status="degree_cap", degree_cap=max_degree)
            records.append(record)
            continue
        n, weights = homogeneous
        trials = []
        for order in ["lex", "reverse", "axis0", "axis1", "axis2", "balanced"]:
            atoms, transcript = inverse_carries(weights, order)
            verify(polynomial, atoms)
            trials.append((mdn_metrics(atoms), order, atoms, transcript))
        metrics, order, atoms, transcript = min(trials, key=lambda item: (item[0]["payload_bits"], item[0]["support"]))
        minimum_support = min(item[0]["support"] for item in trials)
        record.update(status="certified", degree=n,
                      bernstein=bernstein_metrics(n, weights), mdn=metrics,
                      best_order=order, minimum_support_across_orders=minimum_support,
                      carries=len(transcript), independently_verified=True,
                      mdn_atoms=[(*alpha, c) for alpha, c in sorted(atoms.items())],
                      uniform_bernstein=[(*alpha, c/multinomial(alpha)) for alpha, c in sorted(weights.items())])
        records.append(record)
    certified = [r for r in records if r["status"] == "certified"]
    return {"scope": "18 deterministic 3-axis simplex cases; degree cap 16; homogeneous Bernstein versus greedy inverse carries only",
            "limits": ["No subdivision, LP, factorization, or globally optimal compression baseline",
                       "Payload is the same explicit rational/common-scale model extended to three axes, not universal coding complexity",
                       "MDN certificate language uses rational coefficients and an explicit scale, not bounded integer digits"],
            "degree_cap": max_degree, "cases": records,
            "summary": {"total": len(records), "certified": len(certified),
                        "degree_cap_failures": len(records)-len(certified),
                        "support_reductions": sum(r["mdn"]["support"] < r["bernstein"]["support"] for r in certified),
                        "payload_reductions": sum(r["mdn"]["payload_bits"] < r["bernstein"]["payload_bits"] for r in certified),
                        "all_certificates_exactly_verified": True}}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path(__file__).parent / "results" / "simplex.json")
    arguments = parser.parse_args()
    output = run()
    arguments.output.parent.mkdir(parents=True, exist_ok=True)
    arguments.output.write_text(json.dumps(jsonable(output), indent=2)+"\n")
    print(json.dumps(output["summary"], indent=2))
    print(arguments.output)
