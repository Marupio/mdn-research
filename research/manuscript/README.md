# MDN manuscript and reproduction guide

David Gaden's [manuscript](../../output/pdf/mdn_carry_dynamics.pdf),
*Boundary-sensitive products and periodic carry patterns in a two-dimensional
radix system*, is a draft for independent proof and novelty review.
The [LaTeX source](mdn_carry_dynamics.tex) is editable and self-contained.

The construction and first C++ numerical application date to around 2000;
the current application began in 2025. The entire grid is one number.
A carry removes ten units at one cell and adds one at each of its two
forward neighbours. Recent AI assistance helped formalize the author's
observations and conjectures and develop proof drafts and exact checks.

The paper's central result is a finite test for the eventual canonical display
period of a product of X and Y tenths at matched cutoffs. It also shows why
limiting digit displays alone are insufficient to determine products. Boundary
terms are retained throughout. No ordinary scalar magnitude is assigned to
either positional shift, and no general arithmetic of arbitrary infinite real
inputs or novelty claim is assumed.

## Reproduce the finite checks

Use the corrected source, rather than the 1.1.2 or older prebuilt packages.
First follow the [standalone backend instructions](../../tests/README.md).
Then, from the repository root, run these tests (Python standard library only):

```bash
python3 -m unittest discover -s research/fuzzy -p 'test_*.py' -v
python3 -m unittest discover -s research/decimal_infinite -p 'test_*.py' -v
python3 -m unittest discover -s research/phase_products -p 'test_*.py' -v
python3 -m unittest discover -s research/general_products -p 'test_*.py' -v
```

Generate the period certificates and run the two native suites used in the
paper (Linux C++ toolchain):

```bash
python3 research/general_products/experiments.py
python3 research/general_products/backend_check.py --library build/fraxis/libmdn.so --work build/research-checks/general --output build/research-checks/general/results.json
python3 research/vector_circle/backend_check.py --library build/fraxis/libmdn.so --work build/research-checks/vector --output build/research-checks/vector/results.json
```

The general-products suite checks 989 native cases and 14,832 exact conditions,
plus 372 large-shift formula checks. The vector/circle suite checks 45 cases
and 4,045 exact conditions. Saved compact results accompany each study;
full grids and explicit carry certificates are regenerated under `--work`.
Paths and library hashes in older saved JSON files record the original run,
not files that a reader must already possess. A new compiler/build may produce
a different binary hash while preserving every checked identity.

The period calculations certify the mathematical criterion without attempting
to materialize grids at enormous predicted periods. Computation supports the
proofs; it is not a substitute for independent review.

## Compile the manuscript

With a standard LaTeX installation, run from the repository root:

```bash
mkdir -p build/manuscript
pdflatex -interaction=nonstopmode -halt-on-error -output-directory=build/manuscript research/manuscript/mdn_carry_dynamics.tex
pdflatex -interaction=nonstopmode -halt-on-error -output-directory=build/manuscript research/manuscript/mdn_carry_dynamics.tex
```

The repository PDF is the reviewed reading copy. A permanent archival release
and a comparison with the closest prior theorems remain work for submission.
Historical notes under `pitfalls/` and `STATE_OF_THE_ART.md` are explicitly
qualified; the manuscript and its current proofs govern the outreach claims.
