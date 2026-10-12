"""Reproduce the independent exact audit of saved positivity experiments.

Usage:
  python3 pitfalls/positivity/audit.py
  python3 pitfalls/positivity/audit.py --results-dir /path/to/results

Reads development.json, holdout.json, and lp_sensitivity.json. Rechecks every
saved proof and its metrics, recorded source/input hashes, minimum successful
global degrees, and full polynomial expansions. Writes fairness_audit.json.

Source manifests cover the files that existed at the corresponding run time;
new analysis or documentation files do not retroactively enter old manifests.
The audit imports the independent checker, never a search implementation.
"""

import argparse
import hashlib
import json
from fractions import Fraction
from pathlib import Path
from statistics import median

from common import (verify, metrics, canonical_bernstein, expand_bernstein,
                    trim, _get_coefficients)


BERNSTEIN = ["bernstein_global", "bernstein_subdivision", "bernstein_hybrid"]
MDN = ["mdn_local_global", "mdn_targeted_global",
       "mdn_targeted_subdivision", "mdn_targeted_hybrid"]


def source_checks(manifest, directory):
    for name, expected in manifest.items():
        actual = hashlib.sha256((directory/name).read_bytes()).hexdigest()
        if actual != expected:
            raise ValueError(f"Source hash differs: {directory/name}")
    return len(manifest)


def recheck(p, result):
    if result["status"] != "certified":
        return 0
    verify(p, result["certificate"])
    recomputed, repacked = metrics(result["certificate"])
    if recomputed != result["metrics"]:
        raise ValueError("Saved metrics differ from recomputed metrics")
    verify(p, repacked)
    return 1


def smallest(methods, names, metric):
    return min(methods[name]["metrics"][metric] for name in names
               if methods[name]["status"] == "certified")


def compare(rows, extended, metric, include_lp=False, recipes=False):
    paired = []
    for row in rows:
        methods = row["methods"]
        m = smallest(methods, MDN, metric)
        b = smallest(methods, [n+"_recipe" for n in BERNSTEIN]
                     if recipes else BERNSTEIN, metric)
        if include_lp and row["name"] in extended:
            b = min(b, extended[row["name"]]["metrics"][metric])
        paired.append((m, b))
    return dict(cases=len(paired), mdn_smaller=sum(m < b for m, b in paired),
                ties=sum(m == b for m, b in paired),
                conventional_smaller=sum(m > b for m, b in paired),
                total_mdn=sum(m for m, _ in paired),
                total_conventional=sum(b for _, b in paired),
                median_ratio=median(m/b for m, b in paired))


def main():
    here = Path(__file__).resolve().parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results-dir", type=Path, default=here/"results")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    base = args.results_dir
    sensitivity = json.loads((base/"lp_sensitivity.json").read_text())
    source_count = source_checks(sensitivity["source_sha256"], here)
    extended = {}
    sensitivity_count = 0
    for row in sensitivity["cases"]:
        p = list(map(Fraction, row["p"]))
        sensitivity_count += recheck(p, row["lp_extended"])
        extended.setdefault(row["dataset"], {})[row["name"]] = row["lp_extended"]

    output = dict(saved_main_certificates_reverified=0,
                  main_minimum_degrees_le32_reverified=0,
                  datasets={}, additional_large_global_full_expansion_checks=0,
                  sensitivity_saved_certificates_reverified=sensitivity_count,
                  source_manifest_entries_reverified=source_count,
                  sensitivity_input_hashes_reverified=0,
                  source_manifest_scope="Files present at each original run; later-added analysis files are not required in older manifests",
                  json_byte_scope="Best observed outputs; search and packing selected payload bits, not JSON bytes; no optimal-codec claim")
    for label in ("development", "holdout"):
        raw = (base/(label+".json")).read_bytes()
        data = json.loads(raw)
        digest = hashlib.sha256(raw).hexdigest()
        if digest != sensitivity["datasets"][label]["input_sha256"]:
            raise ValueError(f"Sensitivity input hash differs: {label}")
        output["sensitivity_input_hashes_reverified"] += 1
        output["source_manifest_entries_reverified"] += source_checks(data["source_sha256"], here)
        for row in data["cases"]:
            p = list(map(Fraction, row["p"]))
            for result in row["methods"].values():
                output["saved_main_certificates_reverified"] += recheck(p, result)
            global_result = row["methods"]["bernstein_global"]
            if global_result["status"] != "certified":
                continue
            n = global_result["metrics"]["max_degree"]
            for name in ("mdn_local_global", "mdn_targeted_global", "handelman_lp"):
                candidate = row["methods"][name]
                if candidate["status"] == "certified" and candidate["metrics"]["max_degree"] < n:
                    raise ValueError("Same-cone maximum degree contradicts minimal Bernstein degree")
            if n > len(trim(p))-1 and min(canonical_bernstein(p, n-1)) >= 0:
                raise ValueError("Reported minimum Bernstein degree is not minimal")
            if n <= 32:
                coefficients = canonical_bernstein(p, n)
                output["main_minimum_degrees_le32_reverified"] += 1
            else:
                coefficients = _get_coefficients(global_result["certificate"])
                output["additional_large_global_full_expansion_checks"] += 1
            if expand_bernstein(coefficients) != trim(p):
                raise ValueError("Independent full power expansion differs from input")

        rows = data["cases"]
        lp = extended[label]
        bits = compare(rows, lp, "payload_bits", include_lp=True)
        recipes = compare(rows, lp, "payload_bits", recipes=True)
        # Preserve the original audit's concise payload-summary schema.
        output["datasets"][label] = dict(
            cases=bits["cases"], mdn_smaller=bits["mdn_smaller"], ties=bits["ties"],
            conventional_lp25_smaller=bits["conventional_smaller"],
            mdn_vs_recipes_smaller=recipes["mdn_smaller"],
            mdn_vs_recipes_ties=recipes["ties"], recipes_smaller=recipes["conventional_smaller"],
            total_mdn_bits=bits["total_mdn"],
            total_conventional_lp25_bits=bits["total_conventional"],
            median_mdn_over_conventional_lp25_ratio=bits["median_ratio"],
            json_byte_comparisons=dict(
                bernstein_explicit=compare(rows, lp, "json_bytes"),
                bernstein_plus_lp25=compare(rows, lp, "json_bytes", include_lp=True),
                bernstein_recipes=compare(rows, lp, "json_bytes", recipes=True)))
    output_path = args.output or base/"fairness_audit.json"
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(output, indent=2)+"\n")
    print(json.dumps(output, indent=2))
    print(f"Saved {output_path}")


if __name__ == "__main__":
    main()
