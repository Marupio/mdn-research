"""Exact cycle periods and local canonical digits of matched MDN products.

L_N**a * R_N**c retains the common term count n=N+1 and all endpoints.
Only integer coefficients and the three-cell carry rule are used. Periods
refer to eventual displays with one fixed complete residue digit set, not
to equality of the entire finite numbers or the GUI's Positive policy.
"""

from functools import lru_cache
from math import comb, gcd


def _counts(a, c):
    if any(type(v) is not int or v < 0 for v in (a, c)):
        raise ValueError("Factor counts must be nonnegative integers")


def layer_difference(a, c, n, k, shift, layer):
    """Forward k-difference of the interior coefficient's n-shift.

    For mixed products n,k>=0 suffice for the finite Newton-grid test.
    The n-independent piece of the raw coefficient cancels exactly.
    """
    degree = a+c-1-layer
    return sum((-1)**(a-t)*comb(a+c, a-t)*(
        comb(t*(n+shift)+k+c-1, degree)-comb(t*n+k+c-1, degree))
        for t in range(1, a+1))


def period_test(a, c, shift, base=10):
    """Decide a universal eventual period using a finite triangular sample.

    A failed test returns an exact divisibility witness. Samples are of the
    interior polynomial, not claims that a depth-zero raw grid is interior.
    """
    _counts(a, c)
    if type(shift) is not int or shift < 1:
        raise ValueError("Period shift must be a positive integer")
    if type(base) is not int or base < 3:
        raise ValueError("Use integer base >=3 with a terminating fixed digit convention")
    if not a or not c:
        return {"passes": True, "samples_checked": 0, "witness": None}
    checked = 0
    r = a+c-1
    for layer in range(r):
        degree = r-layer-1
        modulus = base**(layer+1)
        for n in range(degree+1):
            for k in range(degree+1-n):
                value = layer_difference(a, c, n, k, shift, layer)
                checked += 1
                if value % modulus:
                    return {"passes": False, "samples_checked": checked,
                            "witness": {"layer": layer, "n": n, "k": k,
                                        "modulus": modulus, "residue": value % modulus}}
    return {"passes": True, "samples_checked": checked, "witness": None}


def _prime_factors(base):
    primes = []
    p = 2
    while p*p <= base:
        if base % p == 0:
            primes.append(p)
            while base % p == 0:
                base //= p
        p += 1
    if base > 1:
        primes.append(base)
    return primes


def period_certificate(a, c, base=10):
    """Return the exact minimal universal period and every reduction test."""
    _counts(a, c)
    if type(base) is not int or base < 3:
        raise ValueError("Use integer base >=3")
    if not a or not c:
        return {"a": a, "c": c, "base": base, "period": 1,
                "upper_bound": 1, "lower_bound": 1, "reductions": []}
    r = a+c-1
    upper = base**r
    weight = comb(a+c-2, a-1)
    lower = upper//gcd(upper, weight)
    period = upper
    reductions = []
    for prime in _prime_factors(base):
        while period % prime == 0:
            candidate = period//prime
            result = period_test(a, c, candidate, base)
            reductions.append({"prime": prime, "candidate": candidate, **result})
            if not result["passes"]:
                break
            period = candidate
    return {"a": a, "c": c, "base": base, "period": period,
            "upper_bound": upper, "lower_bound": lower,
            "top_difference_weight": weight, "reductions": reductions}


def same_phase(a, c, first, second, base=10):
    """Equality of eventual phase arrays indexed by term-count residues.

    Residue 0 is allowed: it describes a sequence of positive term counts
    going to infinity in that residue class, not a zero-term input grid.
    """
    _counts(a, c)
    if any(type(n) is not int or n < 0 for n in (first, second)):
        raise ValueError("Use nonnegative term-count residues")
    if type(base) is not int or base < 3:
        raise ValueError("Use integer base >=3")
    if first == second or not a or not c:
        return True
    first, second = sorted((first, second))
    r = a+c-1
    for layer in range(r):
        for k in range(r-layer):
            if layer_difference(a, c, first, k, second-first, layer) % base**(layer+1):
                return False
    return True


def raw_coefficient(a, c, n, k):
    """Exact finite raw digit at (k-a,-k-c), including finite endpoints."""
    _counts(a, c)
    if type(n) is not int or n < 1 or type(k) is not int:
        raise ValueError("Require a positive integer term count and integer diagonal index")
    d = a+c
    if not d:
        return int(k == 0)
    index = k+a*(n-1)
    if index < 0 or index > d*(n-1):
        return 0
    amplitude = sum((-1)**j*comb(d, j)*comb(index-j*n+d-1, d-1)
                    for j in range(min(d, index//n)+1))
    return (-1 if k % 2 else 1)*amplitude


def local_window(a, c, n, radius=3, base=10, lower=-4):
    """Predict a complete canonical window from its finite carry ancestry.

    Unlike enumerating a full grid, cost does not grow with n's support
    width. Every required raw coefficient includes the exact finite cutoff.
    It is the fixed-window output of the same finite carry sweep.
    """
    _counts(a, c)
    if type(n) is not int or n < 1 or type(radius) is not int or radius < 0:
        raise ValueError("Require positive integer count and nonnegative integer radius")
    if type(base) is not int or base < 3 or type(lower) is not int or not lower <= -1 < 1 <= lower+base-1:
        raise ValueError("Use a fixed digit set including -1,0,1")
    d = a+c

    @lru_cache(maxsize=None)
    def incoming(layer, k):
        if layer == 0:
            return raw_coefficient(a, c, n, k)
        return ((incoming(layer-1, k)-lower)//base
                +(incoming(layer-1, k-1)-lower)//base)

    grid = {}
    for x in range(-radius, radius+1):
        for y in range(-radius, radius+1):
            layer, k = x+y+d, x+a
            if layer >= 0:
                value = incoming(layer, k)
                digit = (value-lower) % base+lower
                if digit:
                    grid[x, y] = digit
    return grid
