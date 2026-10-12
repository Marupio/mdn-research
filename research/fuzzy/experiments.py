#!/usr/bin/env python3
"""Reproduce finite fuzzy-family examples and raw-pattern figures.

Run from any directory. --no-figures needs only the Python standard library.
The saved numbers are exact coefficients, not scalar values of MDNs.
"""

from __future__ import annotations

import argparse
import json
from math import comb, gcd, prod
from pathlib import Path

from family import (
    CascadeProduct, MatchedElement, affine_boundary_ledger,
    grid_multiply, grid_power, q_grid,
)


def requested_products(n, m, p, q):
    fuzzy = CascadeProduct.F(n, m)
    return {
        "F": fuzzy,
        "FL": fuzzy*CascadeProduct((p,), ()),
        "FR": fuzzy*CascadeProduct((), (q,)),
        "F2": fuzzy*fuzzy,
    }


def describe(pattern):
    lo, hi = pattern.k_bounds
    ledger = pattern.boundary_ledger()
    counts = pattern.envelope()
    # Verify the certificate before quotienting by any carry relation.
    neighbour_sum = {(1, 0): 1, (0, 1): 1}
    assert grid_multiply(grid_power(neighbour_sum, pattern.order),
                         pattern.grid()) == q_grid(ledger)
    assert sum(counts) == prod(h+1 for h in pattern.depths)
    return {
        "x_depths": pattern.x_depths,
        "y_depths": pattern.y_depths,
        "order": pattern.order,
        "raw_line_total_degree": -pattern.order,
        "k_bounds_inclusive": [lo, hi],
        "raw_cell_count": hi-lo+1,
        "raw_k0_coordinate": [-len(pattern.x_depths), -len(pattern.y_depths)],
        "raw_k0_coefficient": pattern.coefficient_formula(0),
        "unsigned_coefficient_sum": sum(counts),
        "unsigned_peak": max(counts),
        "boundary_ledger": [
            {"q_exponent": exponent, "coefficient": coefficient}
            for exponent, coefficient in sorted(ledger.items())
        ],
        "boundary_grid_for_b_power_times_product": [
            {"x": x, "y": y, "coefficient": coefficient}
            for (x, y), coefficient in sorted(q_grid(ledger).items())
        ],
    }


def path_record(name, xs, ys):
    ledger = affine_boundary_ledger(xs, ys)
    return {
        "name": name,
        "x_term_counts_slope_offset": xs,
        "y_term_counts_slope_offset": ys,
        "full_boundary_ledger": [
            {"q_exponent_slope": slope, "q_exponent_offset": offset,
             "coefficient": value}
            for (slope, offset), value in sorted(ledger.items())
        ],
        "stationary_window_observation_only": [
            {"q_exponent": offset, "coefficient": value}
            for (slope, offset), value in sorted(ledger.items()) if slope == 0
        ],
    }


def prime_divisors(value):
    return [p for p in range(2, value+1) if value % p == 0
            and all(p % d for d in range(2, int(p**0.5)+1))]


def make_results():
    base = 10
    cases = []
    for n, m, p, q in ((0, 0, 0, 0), (2, 3, 1, 4),
                       (20, 20, 20, 20), (20, 21, 20, 21),
                       (20, 40, 20, 40), (40, 40, 40, 40)):
        cases.append({
            "cutoffs_N_M_P_Q": [n, m, p, q],
            "products": {name: describe(pattern) for name, pattern
                         in requested_products(n, m, p, q).items()},
        })

    fuzzy = MatchedElement((0, 1), (), base)
    left = MatchedElement((), (1,), base)
    right = MatchedElement((0, base), (-1,), base)
    matched = {name: {"A_coefficients_ascending": value.a,
                      "B_coefficients_ascending": value.c}
               for name, value in {"F": fuzzy, "FL": fuzzy*left,
                                   "FR": fuzzy*right, "F2": fuzzy*fuzzy}.items()}

    gcd_cases = []
    for n_depth, m_depth in ((20, 20), (20, 21), (20, 40), (19, 29)):
        n, m = n_depth+1, m_depth+1
        g = gcd(n, m)
        a, c = n//g, m//g
        gcd_cases.append({
            "N": n_depth, "M": m_depth, "term_counts": [n, m],
            "gcd_term_counts": g, "matched_anchor_depth": g-1,
            "rank_over_rational_original_F_polynomial_ring": a+c,
            "rank_over_integer_gcd_anchor_polynomial_ring": 2,
            "primes_proving_failure_of_integer_finiteness_over_original_F":
                [p for p in prime_divisors(base) if (a*c) % p == 0],
        })

    return {
        "schema": "mdn.fuzzy.finite-experiments.v1",
        "base": base,
        "scope": "Exact finite integer grids and their carry classes X+Y=b.",
        "depth_convention": "Depth N includes N+1 cascade terms.",
        "q_convention": "q=-Y/X is a signed formal diagonal shift.",
        "raw_coefficient_warning": "Unsigned counts describe grids before carrying, not scalar magnitudes.",
        "window_warning": "Stationary terms are observations of boundary ledgers, not equality or a value limit.",
        "gcd_warning": "An empty obstruction-prime list makes no claim about integer finiteness.",
        "cases": cases,
        "matched_normal_forms_A_F_plus_B_F_L": matched,
        "matched_raw_k0_growth": [
            {"depth": n, "F": n+1, "FL": comb(n+2, 2),
             "FR": comb(n+2, 2), "F2": (n+1)*(2*n*n+4*n+3)//3}
            for n in (0, 1, 2, 5, 10, 20, 40, 80, 160, 1000)
        ],
        "cutoff_paths": [
            path_record("F matched", ((1, 0),), ((1, 0),)),
            path_record("F Y one term longer", ((1, 0),), ((1, 1),)),
            path_record("F Y twice as many terms", ((1, 0),), ((2, 0),)),
            path_record("FL all matched", ((1, 0), (1, 0)), ((1, 0),)),
            path_record("FR all matched", ((1, 0),), ((1, 0), (1, 0))),
            path_record("F2 all matched", ((1, 0), (1, 0)), ((1, 0), (1, 0))),
            path_record("FL stationary cancellation", ((1, 0), (1, 0)), ((2, 0),)),
            path_record("four unequal offsets", ((1, 0), (1, 2)), ((1, 1), (1, 4))),
        ],
        "fixed_cutoff_gcd_examples": gcd_cases,
        "backend_verification": "backend_results.json contains a separate check against the corrected native library.",
    }


def make_figure(directory):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    directory.mkdir(parents=True, exist_ok=True)
    plt.rcParams.update({"font.size": 11, "axes.spines.top": False,
                         "axes.spines.right": False, "figure.facecolor": "#f7f8fa"})
    fig, axes = plt.subplots(2, 2, figsize=(12, 8.5))
    titles = {"F": r"$F=L_{20}R_{20}$  ·  piecewise linear",
              "FL": r"$FL_{20}$  ·  piecewise quadratic",
              "FR": r"$FR_{20}$  ·  piecewise quadratic",
              "F2": r"$F^2$  ·  piecewise cubic"}
    colors = ("#176f88", "#7358a5", "#bb6b27", "#258166")
    for ax, (name, pattern), color in zip(
            axes.flat, requested_products(20, 20, 20, 20).items(), colors):
        lo, hi = pattern.k_bounds
        counts = pattern.envelope()
        ks = list(range(lo, hi+1))
        ax.fill_between(ks, counts, color=color, alpha=0.09)
        ax.plot(ks, counts, ".-", markersize=3.5, linewidth=1.4, color=color)
        ax.axvline(0, color="#89919b", linewidth=0.9, linestyle="--")
        center = pattern.coefficient_formula(0)
        ax.scatter([0], [center], color=color, s=38, zorder=4)
        ax.set_title(titles[name], loc="left", fontsize=13, pad=12)
        ax.text(0.98, 0.93, f"k = 0: {center:,}", transform=ax.transAxes,
                ha="right", va="top", fontsize=10,
                bbox={"facecolor": "white", "edgecolor": "none", "alpha": 0.9})
        ax.set_xlabel("Diagonal index k")
        ax.set_ylabel("Unsigned raw coefficient C(k)")
        ax.set_ylim(bottom=0)
        ax.grid(axis="y", alpha=0.2)
    fig.suptitle("One convolution rule generates all four patterns", fontsize=19, y=0.98)
    fig.text(0.5, 0.927, "Every factor has depth 20 (21 terms). Every endpoint is retained.",
             ha="center", fontsize=12, color="#404955")
    fig.text(0.5, 0.022,
             "Actual raw digits alternate sign: (−1)ᵏ C(k). These counts are before carrying; they are not MDN magnitudes.",
             ha="center", fontsize=10, color="#404955")
    fig.tight_layout(rect=(0.02, 0.055, 0.99, 0.90), h_pad=2.4, w_pad=2.3)
    target = directory/"product_patterns.png"
    fig.savefig(target, dpi=160)
    plt.close(fig)
    return target


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    directory = Path(__file__).resolve().parent
    parser.add_argument("--output", type=Path, default=directory/"results.json")
    parser.add_argument("--figure-dir", type=Path, default=directory/"figures")
    parser.add_argument("--no-figures", action="store_true")
    args = parser.parse_args()
    results = make_results()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(results, indent=2)+"\n")
    print(f"Saved {args.output}; verified 24 raw boundary identities.")
    if not args.no_figures:
        print(f"Saved {make_figure(args.figure_dir)}")


if __name__ == "__main__":
    main()
