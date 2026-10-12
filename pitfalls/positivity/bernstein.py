"""Exact conventional Bernstein positivity-certificate baselines.

Polynomials are sequences of power coefficients in ascending order.  The
Bernstein convention is p(t) = sum(c_i * comb(n, i) * t**i * (1-t)**(n-i)).
Certificate pieces store that same convention in the local coordinate
u = (t-a)/(b-a), so an independent checker needs no search implementation.

All arithmetic is rational.  Nonnegative coefficients and positive endpoint
coefficients prove strict positivity on the whole closed interval.  No
floating-point sampling is used as proof.
"""

from __future__ import annotations

from fractions import Fraction
from math import comb
from typing import Iterable


def _polynomial(p: Iterable[Fraction]) -> list[Fraction]:
    result = [Fraction(value) for value in p]
    if not result:
        result = [Fraction(0)]
    while len(result) > 1 and result[-1] == 0:
        result.pop()
    return result


def power_to_bernstein(p: Iterable[Fraction], n: int) -> list[Fraction]:
    """Convert exactly to Bernstein degree n, requiring n >= degree(p).

    The identity t**j = sum_{i=j}^n comb(i,j)/comb(n,j) * B_i^n(t)
    gives the conversion directly; this also handles substantial elevation
    without visiting every intervening degree.
    """
    power = _polynomial(p)
    degree = len(power) - 1
    if n < degree or n < 0:
        raise ValueError("Bernstein degree must be at least the polynomial degree")
    denominators = [comb(n, j) for j in range(degree + 1)]
    return [
        sum(
            (power[j] * Fraction(comb(i, j), denominators[j])
             for j in range(min(i, degree) + 1)),
            Fraction(0),
        )
        for i in range(n + 1)
    ]


def elevate(coeffs: Iterable[Fraction]) -> list[Fraction]:
    """Elevate a Bernstein representation by exactly one degree."""
    values = [Fraction(value) for value in coeffs]
    if not values:
        raise ValueError("A Bernstein coefficient list must be nonempty")
    n = len(values) - 1
    return [values[0]] + [
        Fraction(i, n + 1) * values[i - 1]
        + Fraction(n + 1 - i, n + 1) * values[i]
        for i in range(1, n + 1)
    ] + [values[-1]]


def split(
    coeffs: Iterable[Fraction], at: Fraction = Fraction(1, 2)
) -> tuple[list[Fraction], list[Fraction]]:
    """Exact de Casteljau subdivision in the local coordinate at ``at``."""
    row = [Fraction(value) for value in coeffs]
    at = Fraction(at)
    if not row:
        raise ValueError("A Bernstein coefficient list must be nonempty")
    if not 0 < at < 1:
        raise ValueError("Subdivision point must lie strictly inside (0, 1)")
    left, right = [row[0]], [row[-1]]
    while len(row) > 1:
        row = [(1 - at) * row[i] + at * row[i + 1]
               for i in range(len(row) - 1)]
        left.append(row[0])
        right.append(row[-1])
    return left, right[::-1]


def _positive(coeffs: list[Fraction]) -> bool:
    return coeffs[0] > 0 and coeffs[-1] > 0 and min(coeffs) >= 0


def _piece(a: Fraction, b: Fraction, coeffs: list[Fraction]) -> dict:
    return {"a": a, "b": b, "coefficients": coeffs}


def _result(pieces: list[dict] | None, metadata: dict, reason: str | None = None) -> dict:
    if pieces is None:
        return {"status": "limit", "certificate": None,
                "reason": reason, "metadata": metadata}
    pieces = sorted(pieces, key=lambda piece: piece["a"])
    metadata = dict(metadata)
    metadata["leaves"] = len(pieces)
    metadata["coefficient_count"] = sum(len(piece["coefficients"]) for piece in pieces)
    metadata["max_degree"] = max(len(piece["coefficients"]) - 1 for piece in pieces)
    return {"status": "certified",
            "certificate": {"kind": "bernstein_pieces", "pieces": pieces},
            "metadata": metadata}


def certify_uniform(p: Iterable[Fraction], max_degree: int = 512) -> dict:
    """Find the *smallest* successful uniform Bernstein degree up to the cap.

    Nonnegativity survives degree elevation because the new coefficients are
    convex combinations of neighbours.  Hence exponential bracketing and
    binary search find the same minimum as a sequential degree sweep, using
    only logarithmically many full coefficient conversions.
    """
    power = _polynomial(p)
    degree = len(power) - 1
    metadata = {"method": "uniform", "input_degree": degree,
                "degree_cap": max_degree, "max_depth": 0,
                "degrees_tested": []}
    if max_degree < degree:
        return _result(None, metadata, "degree_cap_below_input_degree")
    if power[0] <= 0 or sum(power, Fraction(0)) <= 0:
        return _result(None, metadata, "nonpositive_endpoint")
    cache: dict[int, list[Fraction]] = {}

    def successful(n: int) -> bool:
        if n not in cache:
            cache[n] = power_to_bernstein(power, n)
            metadata["degrees_tested"].append(n)
        return _positive(cache[n])

    if successful(degree):
        return _result([_piece(Fraction(0), Fraction(1), cache[degree])], metadata)
    lower, increment = degree, 1
    while lower < max_degree:
        upper = min(max_degree, degree + increment)
        if successful(upper):
            break
        lower = upper
        increment *= 2
    else:
        return _result(None, metadata, "degree_cap_exhausted")
    while upper - lower > 1:
        middle = (lower + upper) // 2
        if successful(middle):
            upper = middle
        else:
            lower = middle
    return _result([_piece(Fraction(0), Fraction(1), cache[upper])], metadata)


def certify_subdivision(
    p: Iterable[Fraction], max_depth: int = 16, max_leaves: int = 4096
) -> dict:
    """Adaptive dyadic de Casteljau subdivision at the original degree.

    A leaf is split only when its current coefficient list does not already
    certify positivity.  For this fixed-degree, dyadic subdivision rule the
    resulting successful tree has the smallest possible set of leaves: each
    unresolved node must be split in any successful tree using that rule.
    """
    if max_depth < 0 or max_leaves < 1:
        raise ValueError("Require max_depth >= 0 and max_leaves >= 1")
    power = _polynomial(p)
    degree = len(power) - 1
    metadata = {"method": "subdivision", "input_degree": degree,
                "depth_cap": max_depth, "leaf_cap": max_leaves,
                "max_depth": 0, "visited_nodes": 0}
    pending = [(Fraction(0), Fraction(1), power_to_bernstein(power, degree), 0)]
    pieces: list[dict] = []
    while pending:
        a, b, coeffs, depth = pending.pop()
        metadata["visited_nodes"] += 1
        metadata["max_depth"] = max(metadata["max_depth"], depth)
        if _positive(coeffs):
            pieces.append(_piece(a, b, coeffs))
            continue
        if coeffs[0] <= 0 or coeffs[-1] <= 0:
            return _result(None, metadata, "nonpositive_subinterval_endpoint")
        if depth >= max_depth:
            return _result(None, metadata, "depth_cap_exhausted")
        # Replacing this current leaf with two leaves increases the eventual
        # number of leaves by one, regardless of the depth-first visit order.
        if len(pieces) + len(pending) + 2 > max_leaves:
            return _result(None, metadata, "leaf_cap_exhausted")
        left, right = split(coeffs)
        middle = (a + b) / 2
        pending.append((middle, b, right, depth + 1))
        pending.append((a, middle, left, depth + 1))
    return _result(pieces, metadata)


def certify_hybrid(
    p: Iterable[Fraction], max_depth: int = 16, max_leaves: int = 4096,
    local_elevation: int = 16,
) -> dict:
    """Heuristically minimize stored coefficients using elevation and splitting.

    At each node, find its first positive representation among degrees d
    through d+local_elevation.  Compare that with recursively subdivided
    children when the children could use fewer coefficients.  Both children
    start again at the original degree d.  A split has at least 2*(d+1)
    coefficients, which allows exact pruning of unhelpful subdivision.

    The maximum number of visited nodes is 2*max_leaves-1.  This work budget,
    the depth bound, and the local-elevation cap make this a heuristic rather
    than a globally optimal mixed-degree partition search.  In particular,
    no non-dyadic split, degree reduction, or coefficient sharing is tried.
    """
    if max_depth < 0 or max_leaves < 1 or local_elevation < 0:
        raise ValueError("Require nonnegative depth/elevation and max_leaves >= 1")
    power = _polynomial(p)
    degree = len(power) - 1
    metadata = {"method": "hybrid", "input_degree": degree,
                "depth_cap": max_depth, "leaf_cap": max_leaves,
                "local_elevation": local_elevation, "max_depth": 0,
                "visited_nodes": 0, "heuristic": True}
    node_cap = 2 * max_leaves - 1
    failure_reason = "search_budget_exhausted"

    def search(a: Fraction, b: Fraction, coeffs: list[Fraction], depth: int):
        nonlocal failure_reason
        if metadata["visited_nodes"] >= node_cap:
            failure_reason = "node_cap_exhausted"
            return None
        metadata["visited_nodes"] += 1
        metadata["max_depth"] = max(metadata["max_depth"], depth)
        if coeffs[0] <= 0 or coeffs[-1] <= 0:
            failure_reason = "nonpositive_subinterval_endpoint"
            return None
        elevated = coeffs
        candidate = None
        for increment in range(local_elevation + 1):
            if _positive(elevated):
                candidate = [_piece(a, b, elevated)]
                break
            if increment < local_elevation:
                elevated = elevate(elevated)
        # Every split stores at least two degree-d lists.  Stop when even
        # this optimistic lower bound cannot beat the single-leaf candidate.
        candidate_cost = len(elevated) if candidate is not None else None
        if candidate_cost is not None and candidate_cost <= 2 * (degree + 1):
            return candidate
        if depth >= max_depth:
            failure_reason = "depth_cap_exhausted"
            return candidate
        left, right = split(coeffs)
        middle = (a + b) / 2
        left_result = search(a, middle, left, depth + 1)
        if left_result is None:
            return candidate
        # The right child must cost at least degree+1.  If the left result
        # already makes subdivision uncompetitive, save the remaining work.
        left_cost = sum(len(piece["coefficients"]) for piece in left_result)
        if candidate_cost is not None and left_cost + degree + 1 >= candidate_cost:
            return candidate
        right_result = search(middle, b, right, depth + 1)
        if right_result is None:
            return candidate
        joined = left_result + right_result
        joined_cost = left_cost + sum(len(piece["coefficients"]) for piece in right_result)
        if len(joined) > max_leaves:
            failure_reason = "leaf_cap_exhausted"
            return candidate
        if candidate_cost is None or joined_cost < candidate_cost:
            return joined
        return candidate

    pieces = search(Fraction(0), Fraction(1), power_to_bernstein(power, degree), 0)
    if pieces is not None:
        # Report the depth of the actual certificate separately from explored
        # depth, since rejected partitions may have involved deeper searches.
        metadata["search_max_depth"] = metadata["max_depth"]
        metadata["max_depth"] = max(
            (piece["b"] - piece["a"]).denominator.bit_length() - 1
            for piece in pieces
        )
    return _result(pieces, metadata, failure_reason if pieces is None else None)
