"""Conventional sparse Handelman search on [0, 1].

The input is a list of rational power coefficients, lowest power first.
Floating-point linear programming only proposes a support.  Every reported
certificate is reconstructed with Fraction arithmetic and checked exactly.
This searches the same cone/dictionary as nonnegative normalized MDN atoms.
The objectives are sparsity proxies, not minimum-support optimization.
"""

from fractions import Fraction
from math import comb
from time import perf_counter


def _trim(p):
    p = [Fraction(v) for v in p]
    while len(p) > 1 and not p[-1]:
        p.pop()
    return p or [Fraction(0)]


def _bernstein(p, n):
    return [
        sum((p[j] * Fraction(comb(k, j), comb(n, j))
             for j in range(min(k, len(p)-1)+1)), Fraction(0))
        for k in range(n+1)
    ]


def _power_column(i, j, n):
    col = [Fraction(0)] * (n+1)
    for k in range(j+1):
        col[i+k] = Fraction((-1)**k * comb(j, k))
    return col


def _recover(p, support, n):
    """Solve an overdetermined exact system with independent support columns."""
    if len(support) > n+1:
        return None
    cols = [_power_column(i, j, n) for i, j in support]
    rows = [[col[r] for col in cols] + [p[r] if r < len(p) else Fraction(0)]
            for r in range(n+1)]
    rank = 0
    for c in range(len(support)):
        pivot = next((r for r in range(rank, n+1) if rows[r][c]), None)
        if pivot is None:
            return None
        rows[rank], rows[pivot] = rows[pivot], rows[rank]
        pivot_value = rows[rank][c]
        rows[rank] = [v / pivot_value for v in rows[rank]]
        for r in range(rank+1, n+1):
            if rows[r][c]:
                factor = rows[r][c]
                rows[r] = [v-factor*u for v, u in zip(rows[r], rows[rank])]
        rank += 1
    if any(row[-1] for row in rows[rank:]):
        return None
    solution = [Fraction(0)]*len(support)
    for r in reversed(range(rank)):
        solution[r] = rows[r][-1] - sum(
            (rows[r][c]*solution[c] for c in range(r+1, rank)), Fraction(0))
    if any(v < 0 for v in solution):
        return None
    result = [(i, j, v) for (i, j), v in zip(support, solution) if v]
    expanded = [Fraction(0)]*(n+1)
    for i, j, v in result:
        for k in range(j+1):
            expanded[i+k] += v*((-1)**k)*comb(j, k)
    if _trim(expanded) != _trim(p):
        return None
    return result


def _score(atoms, objective):
    # Search may consult the shared serialization metric. The independent
    # verifier still imports no search routines and checks the final identity.
    try:
        from .common import metrics
    except ImportError:
        from common import metrics
    values, _ = metrics({"kind": "mdn_atoms", "atoms": atoms})
    size = values["payload_bits"]
    return (size, len(atoms)) if objective == "compact" else (len(atoms), size)


def certify_handelman(p, max_degree=32, seed=1729, restarts=5,
                      degree=None, max_dictionary=2000, time_limit=15.0,
                      objective="support"):
    """Return an exact certificate, or a bounded-search status ``limit``.

    Optional ``degree`` fixes the maximum atom degree. Otherwise use the first
    degree <= max_degree with nonnegative Bernstein coefficients (an exact
    feasibility test for this dictionary). All lower degree atoms are included.
    ``time_limit`` is a per-LP limit, and ``restarts`` bounds LP calls.
    ``objective='compact'`` ranks candidates by the shared encoded payload
    metric before support count; the default reverses those two priorities.
    """
    start = perf_counter()
    if objective not in ("support", "compact"):
        raise ValueError("objective must be 'support' or 'compact'")
    p = _trim(p)
    n0 = len(p)-1
    meta = {"method": "sparse Handelman LP", "seed": seed,
            "requested_restarts": restarts, "lp_calls": 0,
            "exact_reconstructions": 0, "optimal_support": False,
            "objective": objective}

    def finish(status, **kwargs):
        meta["search_seconds"] = perf_counter()-start
        return {"status": status, "metadata": meta.copy(), **kwargs}

    if p[0] <= 0 or sum(p, Fraction(0)) <= 0:
        return finish("limit", reason="nonpositive endpoint prevents strict positivity")
    if degree is not None:
        if degree < n0 or degree > max_degree:
            return finish("limit", reason="requested degree outside search bounds")
        n = degree
        b = _bernstein(p, n)
        if min(b) < 0:
            return finish("limit", reason="dictionary exactly infeasible at requested degree")
    else:
        feasible = next(((n, b) for n in range(n0, max_degree+1)
                         if min(b := _bernstein(p, n)) >= 0), None)
        if feasible is None:
            return finish("limit", reason="no feasible global degree within bound")
        n, b = feasible
    meta["degree"] = n
    atom_count = (n+1)*(n+2)//2
    meta["dictionary_size"] = atom_count
    if atom_count > max_dictionary:
        return finish("limit", reason="dictionary size bound")

    import numpy as np
    from scipy.optimize import linprog

    atoms = [(i, d-i) for d in range(n+1) for i in range(d+1)]
    columns = []
    for i, j in atoms:
        col = [Fraction(comb(n-i-j, k-i), comb(n, k))
               if i <= k <= n-j else Fraction(0) for k in range(n+1)]
        scale = max(col)
        columns.append([float(v/scale) for v in col])
    matrix = np.array(columns, dtype=float).T
    bscale = max(abs(v) for v in b)
    rhs = np.array([float(v/bscale) for v in b], dtype=float)
    rng = np.random.default_rng(seed)
    base_cost = np.array([1.0 + (i+j)/(n+1) for i, j in atoms])
    # The known feasible Bernstein list is itself a certificate in this
    # dictionary. Keep it, so numerical support selection can only improve
    # the incumbent and can never erase an already exact proof.
    best = [(i, n-i, c*comb(n, i)) for i, c in enumerate(b) if c]
    meta["selected_candidate"] = "uniform_fallback"
    failures = []
    for attempt in range(max(1, restarts)):
        if attempt == 0:
            cost = base_cost
        elif attempt == 1:
            cost = np.ones(len(atoms))
        else:
            cost = base_cost*rng.uniform(0.4, 1.6, len(atoms))
        result = linprog(cost, A_eq=matrix, b_eq=rhs, bounds=(0, None),
                         method="highs", options={"time_limit": time_limit,
                                                   "primal_feasibility_tolerance": 1e-9,
                                                   "dual_feasibility_tolerance": 1e-9})
        meta["lp_calls"] += 1
        if not result.success:
            failures.append(str(result.message))
            continue
        recovered = None
        for threshold in (1e-8, 1e-10, 1e-12, 0.0):
            support = [atom for atom, val in zip(atoms, result.x)
                       if val > threshold*max(1.0, float(max(result.x)))]
            recovered = _recover(p, support, n)
            if recovered is not None:
                break
        if recovered is not None:
            meta["exact_reconstructions"] += 1
            if _score(recovered, objective) < _score(best, objective):
                best = recovered
                meta["selected_candidate"] = "lp_support"
        else:
            failures.append("floating support did not recover an exact nonnegative identity")
    if failures:
        meta["numerical_failures"] = failures
    return finish("certified", certificate={"kind": "mdn_atoms", "atoms": best})
