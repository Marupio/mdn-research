"""Deterministic finite digit displays and explicit carry certificates.

Uses only integer digits, grid positions, and the user's three-cell carry.
It is a research display convention, not a change to the native GUI policy.
Default digits are -4 through 5, one representative of each residue mod 10.
"""

from collections import defaultdict
from dataclasses import dataclass
from heapq import heappop, heappush


@dataclass
class Normalized:
    grid: dict[tuple[int, int], int]
    carries: dict[tuple[int, int], int]


def normalize(grid, base=10, lower=-4, max_cells=2_000_000):
    """Sweep forward, recording the exact integer carry at every changed cell.

    Consecutive allowed digits must include -1, 0, and 1. The processing
    limit is a safety guard, not a silent truncation of the number.
    """
    if type(base) is not int or base < 3:
        raise ValueError("This finite canonical convention requires integer base >=3")
    if type(lower) is not int or not lower <= -1 < 1 <= lower+base-1:
        raise ValueError("Allowed digits must include -1, 0, and 1")
    if type(max_cells) is not int or max_cells <= 0:
        raise ValueError("max_cells must be a positive integer")
    pending = {}
    for position, digit in grid.items():
        if (not isinstance(position, tuple) or len(position) != 2
                or any(type(c) is not int for c in position) or type(digit) is not int):
            raise ValueError("Require integer digit values and pairs of integer coordinates")
        if digit:
            pending[position] = digit
    heap = []
    scheduled = set(pending)
    for x, y in pending:
        heappush(heap, (x+y, x, y))
    result, carries = {}, {}
    processed = 0
    while heap:
        _, x, y = heappop(heap)
        processed += 1
        if processed > max_cells:
            raise RuntimeError("Carry sweep exceeded its processing limit; no truncated answer returned")
        position = (x, y)
        scheduled.remove(position)
        value = pending.pop(position, 0)
        quotient = (value-lower)//base
        remainder = value-base*quotient
        if remainder:
            result[position] = remainder
        if quotient:
            carries[position] = quotient
            for neighbor in ((x+1, y), (x, y+1)):
                pending[neighbor] = pending.get(neighbor, 0)+quotient
                if neighbor not in scheduled:
                    scheduled.add(neighbor)
                    heappush(heap, (sum(neighbor), *neighbor))
    return Normalized(result, carries)


def verify_certificate(original, result, base=10):
    """Reconstruct every three-cell carry independently, using integer sums."""
    reconstructed = defaultdict(int, original)
    for (x, y), amount in result.carries.items():
        reconstructed[x, y] -= base*amount
        reconstructed[x+1, y] += amount
        reconstructed[x, y+1] += amount
    return {p: d for p, d in reconstructed.items() if d} == result.grid


def window(grid, radius=4):
    return {p: d for p, d in grid.items() if max(map(abs, p)) <= radius and d}


def first_difference(left, right):
    """Nearest differing cell, with a fixed tie order, or None for equal grids."""
    cells = {p for p in left.keys() | right.keys() if left.get(p, 0) != right.get(p, 0)}
    if not cells:
        return None
    point = min(cells, key=lambda p: (max(map(abs, p)), p))
    return {"position": list(point), "left": left.get(point, 0),
            "right": right.get(point, 0), "radius": max(map(abs, point))}
