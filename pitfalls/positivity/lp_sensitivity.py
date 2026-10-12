"""Check whether a larger ordinary Handelman LP portfolio changes comparisons.

The main experiment uses five LP objectives. This independent sensitivity run
uses 25, with unchanged algorithms, expanded power coefficients as the only
polynomial input, and exact verification of each resulting saved certificate.
Only cases eligible for the main global degree<=32 LP comparison are included.

Example:
  python3 pitfalls/positivity/lp_sensitivity.py \
    pitfalls/positivity/results/development.json \
    pitfalls/positivity/results/holdout.json \
    --output pitfalls/positivity/results/lp_sensitivity.json
"""

import argparse
import hashlib
import json
from fractions import Fraction
from pathlib import Path
from time import perf_counter

from common import jsonable, metrics, verify
from handelman import certify_handelman


def summarize(rows):
    good = [row for row in rows if row["lp_extended"]["status"] == "certified"]
    totals = dict(eligible=len(rows), certified=len(good),
                  mdn_smaller=0, ties=0, lp_extended_smaller=0,
                  lp_extended_improved=0, lp_unchanged=0,
                  lp_extended_regressed=0)
    for row in good:
        bits = row["lp_extended"]["metrics"]["payload_bits"]
        mdn = row["mdn_targeted_global_metrics"]["payload_bits"]
        baseline = row["main_lp_metrics"]["payload_bits"]
        totals["mdn_smaller" if mdn < bits else
               "lp_extended_smaller" if mdn > bits else "ties"] += 1
        totals["lp_extended_improved" if bits < baseline else
               "lp_extended_regressed" if bits > baseline else "lp_unchanged"] += 1
    return totals


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--restarts", type=int, default=25)
    parser.add_argument("--time-limit", type=float, default=1.0)
    args = parser.parse_args()

    certify_handelman([1], max_degree=0, restarts=1, objective="compact")
    here = Path(__file__).resolve().parent
    output = dict(
        method="Frozen conventional sparse Handelman LP with a larger objective portfolio",
        scope="Only main-comparison global degrees at most 32; no optimal-support claim",
        settings=dict(restarts=args.restarts, per_lp_time_limit=args.time_limit,
                      degree_cap=32, objective="compact"),
        source_sha256={name: hashlib.sha256((here/name).read_bytes()).hexdigest()
                       for name in ("handelman.py", "common.py", "lp_sensitivity.py")},
        datasets={}, cases=[])
    started = perf_counter()
    for path in args.inputs:
        raw = path.read_bytes()
        data = json.loads(raw)
        seed = data["settings"]["search_seed"]
        rows = []
        for case in data["cases"]:
            baseline = case["methods"]["handelman_lp"]
            if baseline["status"] != "certified":
                continue
            degree = baseline["metadata"]["degree"]
            if degree > 32:
                continue
            p = list(map(Fraction, case["p"]))
            result = certify_handelman(
                p, max_degree=32, degree=degree, seed=seed,
                restarts=args.restarts, time_limit=args.time_limit,
                objective="compact")
            candidate = dict(status=result["status"], metadata=result["metadata"])
            if result["status"] == "certified":
                verify(p, result["certificate"])
                stats, packed = metrics(result["certificate"])
                verify(p, packed)
                candidate.update(metrics=stats, certificate=packed,
                                 exact_identity_verified=True)
            else:
                candidate["reason"] = result.get("reason")
            row = dict(dataset=path.stem, name=case["name"], family=case["family"],
                       p=case["p"], degree=degree,
                       main_lp_metrics=baseline["metrics"],
                       mdn_targeted_global_metrics=case["methods"]["mdn_targeted_global"]["metrics"],
                       lp_extended=candidate)
            rows.append(row)
            print(f"{path.stem}/{case['name']}: "
                  f"MDN {row['mdn_targeted_global_metrics']['payload_bits']}, "
                  f"main LP {baseline['metrics']['payload_bits']}, "
                  f"extended LP {candidate.get('metrics', {}).get('payload_bits', candidate['status'])}",
                  flush=True)
        output["datasets"][path.stem] = dict(
            input=str(path), input_sha256=hashlib.sha256(raw).hexdigest(),
            search_seed=seed, main_restarts=data["settings"]["lp_restarts"],
            summary=summarize(rows))
        output["cases"].extend(rows)
    output["summary"] = summarize(output["cases"])
    output["total_wall_seconds"] = perf_counter()-started
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(jsonable(output), indent=2)+"\n")
    print(json.dumps(output["summary"], indent=2), flush=True)
    print(f"Saved {args.output}", flush=True)


if __name__ == "__main__":
    main()
