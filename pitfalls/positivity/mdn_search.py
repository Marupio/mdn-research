"""Exact local inverse-carry searches for nonnegative MDN certificates.

The normalized axes are t and u=1-t. A certificate is a positive sum of
t**i*u**j. Coefficients are Fractions: an explicit common rational scale is
allowed. These are also conventional mixed-degree Handelman certificates.

Input coefficients use the normalized degree-n Bernstein basis, so the raw
coefficient of t**i*u**(n-i) is comb(n, i)*coeffs[i]. An inverse carry replaces
equal amounts at (i+1,j) and (i,j+1) by that amount at (i,j). Every operation is
exact. No floating-point optimization or polynomial factorization is used.
"""

from fractions import Fraction
from heapq import heappop, heappush
from math import comb, gcd, lcm
from random import Random
from time import perf_counter


def _size(atoms):
    """Same primitive shared-scale/individual-rational model as common.metrics."""
    if not atoms:
        return 0
    coefficients = [c for _, _, c in atoms]
    individual = sum(1 + abs(c.numerator).bit_length() + c.denominator.bit_length() for c in coefficients)
    denominator = lcm(*(c.denominator for c in coefficients))
    integers = [c.numerator * (denominator // c.denominator) for c in coefficients]
    divisor = gcd(*integers)
    scale = Fraction(divisor, denominator)
    shared = 0 if scale == 1 else 1 + abs(scale.numerator).bit_length() + scale.denominator.bit_length()
    shared += sum(1 + abs(v // divisor).bit_length() for v in integers)
    indices = sum(max(1, i.bit_length()) + max(1, j.bit_length()) for i, j, _ in atoms)
    return min(individual, shared) + indices


def _score(atoms, compact=False):
    first = (_size(atoms), len(atoms)) if compact else (len(atoms), _size(atoms))
    return first + (max((i + j for i, j, _ in atoms), default=0),)


def _peel(raw, priority, constant=Fraction(0)):
    """Extract maximal elevated atoms, then collapse each binomial triangle.

    Each extraction zeros at least one remaining coefficient. All atoms up to
    the input degree are candidates; degree-n atoms ensure termination in at
    most n+1 extractions. This is also a conventional greedy Handelman search,
    expressed as targeted batches of inverse carries. It is not an LP solver.
    """
    degree = len(raw) - 1
    remaining = list(raw)
    atoms = []
    moves = 0
    if constant:
        remaining = [c - constant * comb(degree, k) for k, c in enumerate(raw)]
        atoms.append((0, 0, constant))
        moves += degree * (degree + 1) // 2
    candidates = []
    for d in range(degree + 1):
        for i in range(d + 1):
            j = d - i
            shape = [(k, comb(degree - d, k - i)) for k in range(i, degree - j + 1)]
            candidates.append((i, j, shape))
    while any(remaining):
        viable = []
        best = None
        for i, j, shape in candidates:
            if any(not remaining[k] for k, _ in shape):
                continue
            viable.append((i, j, shape))
            amount = min(remaining[k] / v for k, v in shape)
            zeros = sum(remaining[k] == amount * v for k, v in shape)
            d = i + j
            mass = amount / ((d + 1) * comb(d, i))
            bits = _size([(i, j, amount)])
            if priority == "mass":
                key = (mass, zeros, -d, -bits)
            elif priority == "lowdegree":
                key = (-d, zeros, mass, -bits)
            elif priority == "zeros":
                key = (zeros, -d, mass, -bits)
            else:
                raise ValueError("Unknown atom peeling priority: " + priority)
            if best is None or key > best[0]:
                best = (key, i, j, amount, shape)
        candidates = viable
        if best is None:
            raise AssertionError("The degree-n atom dictionary must span the residual")
        _, i, j, amount, shape = best
        atoms.append((i, j, amount))
        for k, v in shape:
            remaining[k] -= amount * v
        refinement = degree - i - j
        moves += refinement * (refinement + 1) // 2
    return atoms, moves


def _coarsen(raw, order, *, seed=0):
    """Process each degree once; all degree-d operations precede degree d-1.

    At a degree, each maximal inverse carry permanently removes at least one
    occupied child. Thus there are at most O(n**2) carries. Priority strategies
    maintain a heap with lazy invalidation rather than rescanning the triangle.
    """
    current = list(raw)
    atoms = []
    moves = 0
    rng = Random(seed)
    for degree in range(len(raw) - 1, 0, -1):
        parents = [Fraction(0)] * degree

        def merge(edge):
            nonlocal moves
            amount = min(current[edge], current[edge + 1])
            if amount:
                current[edge] -= amount
                current[edge + 1] -= amount
                parents[edge] += amount
                moves += 1
            return amount

        if order in ("left", "right", "alternating", "random"):
            edges = list(range(degree))
            if order == "right" or (order == "alternating" and degree % 2):
                edges.reverse()
            elif order == "random":
                rng.shuffle(edges)
            for edge in edges:
                merge(edge)
        else:
            heap = []
            versions = [0] * (degree + 1)

            def push(edge):
                if not 0 <= edge < degree:
                    return
                a, b = current[edge], current[edge + 1]
                if not a or not b:
                    return
                if order == "equal":
                    key = (a != b, abs(a - b), -min(a, b))
                elif order == "mass":
                    key = (-min(a, b), abs(a - b))
                elif order == "small":
                    key = (min(a, b), abs(a - b))
                else:
                    raise ValueError("Unknown carry order: " + order)
                heappush(heap, (key, edge, versions[edge], versions[edge + 1]))

            for edge in range(degree):
                push(edge)
            while heap:
                _, edge, left_version, right_version = heappop(heap)
                if (versions[edge], versions[edge + 1]) != (left_version, right_version):
                    continue
                if merge(edge):
                    versions[edge] += 1
                    versions[edge + 1] += 1
                    push(edge - 1)
                    push(edge + 1)
        atoms.extend((i, degree - i, c) for i, c in enumerate(current) if c)
        current = parents
        if not any(current):
            break
    else:
        if current and current[0]:
            atoms.append((0, 0, current[0]))
    return atoms, moves


def coarsen_bernstein(coeffs, strategy="portfolio", *, seed=1729, restarts=4):
    """Find a compact positive mixed-degree atom certificate.

    ``coeffs`` must be nonnegative degree-n Bernstein coefficients. The return
    value contains ``certificate`` with kind ``mdn_atoms`` and exact atoms,
    plus metadata. The portfolio retains the original representation, so its
    atom count never increases. ``strategy='compact'`` uses the same portfolio
    but minimizes stored integer bits before atom count. It compares left/right/alternating sweeps,
    three heap priorities (degrees <= 128), and deterministic random orders
    (degrees <= 64). Each strategy is also tried after extracting the largest
    globally available constant: min(coeffs). This is itself a legal batch of
    inverse carries, since its elevated raw coefficients are binomial(n,i)*c.
    ``strategy='peel'`` (alias ``'targeted'``) additionally tries greedy atom
    peeling through degree 32, minimizing payload bits. Peeling searches the
    same atom dictionary as Handelman LP. Compact does not enable peeling.

    Strategies are heuristics, not globally optimal searches. Degree never
    increases; certificates requiring initial refinement may be missed.
    ``strategy`` may also name one carry order (left, right, alternating,
    equal, mass, small, random); then only that order and the input are tried.
    The caller should independently check the final polynomial identity.
    """
    started = perf_counter()
    coeffs = [Fraction(c) for c in coeffs]
    if not coeffs:
        raise ValueError("At least one Bernstein coefficient is required")
    if any(c < 0 for c in coeffs):
        raise ValueError("Inverse-carry coarsening requires nonnegative coefficients")
    degree = len(coeffs) - 1
    binomials = [comb(degree, i) for i in range(degree + 1)]
    raw = [c * b for c, b in zip(coeffs, binomials)]
    initial_atoms = [(i, degree - i, c) for i, c in enumerate(raw) if c]
    best_atoms = initial_atoms
    compact = strategy in ("compact", "peel", "targeted")
    best_score = _score(best_atoms, compact)
    best_name = "unchanged"
    best_moves = 0
    candidates = [{"strategy": best_name, "atoms": len(best_atoms), "payload_bits": _size(best_atoms), "moves": 0}]
    if strategy in ("portfolio", "compact", "peel", "targeted"):
        orders = [(name, seed) for name in ("left", "right", "alternating")]
        if degree <= 128:
            orders += [(name, seed) for name in ("equal", "mass", "small")]
        if degree <= 64:
            orders += [("random", seed + k) for k in range(restarts)]
    else:
        orders = [(strategy, seed)]
    constant = min(coeffs)
    for peel in (False, True):
        if peel and (not constant or degree == 0):
            continue
        trial_raw = [a - constant * b for a, b in zip(raw, binomials)] if peel else raw
        for order, trial_seed in orders:
            name = order + (":" + str(trial_seed) if order == "random" else "")
            if peel:
                name = "constant+" + name
            atoms, moves = _coarsen(trial_raw, order, seed=trial_seed)
            if peel:
                atoms.append((0, 0, constant))
                # Uniform binomial expansion of c merges to c with this many
                # maximal pair operations, independently of residual atoms.
                moves += degree * (degree + 1) // 2
            # A separately peeled constant and coarsened residual can coincide.
            merged = {}
            for i, j, c in atoms:
                merged[i, j] = merged.get((i, j), Fraction(0)) + c
            atoms = [(i, j, c) for (i, j), c in merged.items() if c]
            score = _score(atoms, compact)
            candidates.append({"strategy": name, "atoms": len(atoms), "payload_bits": _size(atoms), "moves": moves})
            if score < best_score:
                best_atoms, best_score, best_name, best_moves = atoms, score, name, moves
    run_peeling = strategy in ("peel", "targeted") and degree <= 32
    if run_peeling:
        for extracted_constant in [Fraction(0)] + ([constant] if constant else []):
            for priority in ("mass", "lowdegree", "zeros"):
                name = "peel:" + ("constant+" if extracted_constant else "") + priority
                atoms, moves = _peel(raw, priority, extracted_constant)
                score = _score(atoms, compact)
                candidates.append({"strategy": name, "atoms": len(atoms), "payload_bits": _size(atoms), "moves": moves})
                if score < best_score:
                    best_atoms, best_score, best_name, best_moves = atoms, score, name, moves
    best_atoms.sort(key=lambda atom: (atom[0] + atom[1], atom[0]))
    return {
        "certificate": {"kind": "mdn_atoms", "atoms": best_atoms},
        "metadata": {
            "strategy": best_name,
            "moves": best_moves,
            "input_degree": degree,
            "input_atoms": len(initial_atoms),
            "output_atoms": len(best_atoms),
            "payload_bits": _size(best_atoms),
            "objective": "payload_bits_then_atoms" if compact else "atoms_then_payload_bits",
            "candidates": candidates,
            "seconds": perf_counter() - started,
            "search_scope": "descending inverse carries, constant extraction" + (", targeted atom peeling" if run_peeling else ""),
            "optimality_proved": False,
        },
    }
