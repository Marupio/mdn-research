#!/usr/bin/env python3
"""Plot the corrected backend's observed mixed-tenth digit cycle."""

import argparse
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    here = Path(__file__).resolve().parent
    parser.add_argument("--report", type=Path, default=here/"backend_results.json")
    parser.add_argument("--output", type=Path, default=here/"figures/marching_digits.png")
    args = parser.parse_args()
    report = json.loads(args.report.read_text())
    work = Path(report["full_grids_directory"])
    rows = [row for row in report["depth_sweep"]["rows"] if row["N"] <= 60]
    if len(rows) < 41:
        raise ValueError("Generate at least depths 0 through 40 first")

    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import numpy as np

    plt.rcParams.update({"font.size": 11, "axes.spines.top": False,
                         "axes.spines.right": False, "figure.facecolor": "#f7f8fa"})
    fig = plt.figure(figsize=(12, 9))
    layout = fig.add_gridspec(2, 2, height_ratios=(1, 1.65), hspace=0.5, wspace=0.28)
    raw, digit = fig.add_subplot(layout[0, 0]), fig.add_subplot(layout[0, 1])
    depths = [row["N"] for row in rows]
    raw.plot(depths, [row["uncarried_central_coefficient"] for row in rows], color="#226e88", linewidth=2)
    raw.set_title("Before carrying: the coefficient grows", loc="left", fontsize=13)
    raw.set_ylabel("Coefficient at (−1, −1)")
    digit.plot(depths, [row["tracked_digits"][0] for row in rows], ".-", color="#7655a4", markersize=4)
    digit.set_title("After carrying: the displayed digit cycles", loc="left", fontsize=13)
    digit.set_ylabel("Digit at (−1, −1)")
    digit.set_yticks(range(10))
    for ax in (raw, digit):
        ax.set_xlabel("Cascade depth N  (N + 1 terms)")
        ax.grid(alpha=0.2)
        ax.set_xlim(0, depths[-1])

    heat = fig.add_subplot(layout[1, :])
    ks = range(-30, 31)
    array = np.full((len(rows), len(ks)), np.nan)
    for i, row in enumerate(rows):
        n = row["N"]
        cells = json.loads((work/f"depth_{n}.json").read_text())["result"]
        grid = {(x, y): value for x, y, value in cells}
        for j, k in enumerate(ks):
            if abs(k) <= n:
                array[i, j] = grid.get((k-1, -k-1), 0)
    cmap = plt.get_cmap("RdBu_r").copy()
    cmap.set_bad("#e7e9ed")
    im = heat.imshow(array, origin="lower", interpolation="nearest", aspect="auto",
                     extent=(-30.5, 30.5, -0.5, depths[-1]+0.5), cmap=cmap, vmin=-9, vmax=9)
    heat.set_title("The first diagonal: the same ten phases recur as the endpoints move outward",
                   loc="left", fontsize=13, pad=13)
    heat.set_xlabel("Diagonal index k  →  cell (k − 1, −k − 1)")
    heat.set_ylabel("Cascade depth N")
    heat.axvline(0, color="black", linewidth=0.6, alpha=0.4)
    bar = fig.colorbar(im, ax=heat, pad=0.015, fraction=0.04)
    bar.set_label("Displayed digit")
    bar.set_ticks((-8, -4, 0, 4, 8))
    fig.suptitle("Your marching digits, reproduced", fontsize=21, y=0.98)
    fig.text(0.5, 0.935, "Corrected backend · (0.1X) × (0.1Y) · base 10 · all boundaries retained",
             ha="center", fontsize=12, color="#404955")
    fig.text(0.5, 0.025, "Grey lies beyond the finite first-diagonal support. Colours are digit displays, not scalar MDN magnitudes.",
             ha="center", fontsize=10, color="#404955")
    fig.subplots_adjust(left=0.075, right=0.94, top=0.865, bottom=0.095)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(args.output, dpi=160)
    plt.close(fig)
    print(args.output)


if __name__ == "__main__":
    main()
