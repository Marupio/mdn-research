# Pitfalls: superseded work

These documents were written while MDNs were being judged by ordinary
one-dimensional (1DN) magnitudes, by substituting numbers for the grid shifts
X and Y (X = b·t, X = Y = 5, an interval or a complex domain of X values).
Those scalar interpretations are not the definition of an MDN. The entire
grid represents one number, with equality determined by carries. See the
[current manuscript](../research/manuscript/README.md) for the present claims,
and the [later review](../research/phase_products/REVIEW.md) for qualifications
to the [historical summary](../STATE_OF_THE_ART.md).

Formal variables and faithful algebraic representations are valid tools;
choosing one scalar evaluation loses information. The distinction is what
matters. This folder records approaches set aside and known misunderstandings,
not a verdict that every theorem in these files is false.

The documents are kept because parts of them remain correct and because the
mistakes are instructive. They are unedited apart from link and command-path
fixes. Saved result files still record their original paths.

| Document | Where the 1DN assumption enters | What remains valid |
|---|---|---|
| `VALUE.md` (local code audit, not published) | Grids treated as functions evaluated at chosen X, Y values such as (3,7) and (5,5); the Bernstein substitution X = b·t; convergence judged by \|Y\| < \|X\| | The code audit. Carry conservation is corrected in the current source. Division termination, output aliasing, locking and the stale base cache are not addressed by those changes |
| `chatWithAstra.md` (local conversation, not published) | The same substitution throughout: positivity, zeta-like integrals, diffusion, coefficient norms | The proof that no finite grid equals a non-integer rational. The off-critical-line zeros example is correct, but it concerns the 1DN shadow |
| [positivity/](positivity/README.md) | The whole experiment works with t = X/b on [0, 1] | Its conclusion: nonnegative MDN certificates are equivalent to Bernstein/Handelman certificates of the 1DN shadow, with no size or speed advantage found |
| [infinite/](infinite/README.md) | The "1 = 0 collapse" comes from measuring grids by coefficient size; the proposed cure measures them by their values on an interval of X | `formal_notes.md` §§1–5 and `collapse_notes.md` §§2–3 are magnitude-free and still cited: both opposite expansions cannot converge together; the formal delta; the single-diagonal classification; finite parts of mixed products |
| [decimal_infinite/ANALYTIC_PROOFS.md](decimal_infinite/ANALYTIC_PROOFS.md) | X treated as a complex variable; limits defined by uniform convergence on complex domains | §5 is currently the only proof of additive separation for infinite digit streams (b > 2). §6's one-direction evaluations agree with the X-axis reading |

Claude's first review in the 11 October 2026 session made the same mistake
(a "viewing angle" with X = b·cos²θ). It was not saved.

Moved here on 11 October 2026 from the repository root (`VALUE.md`,
`chatWithAstra.md`), `research/positivity/`, `research/infinite/` and
`research/decimal_infinite/ANALYTIC_PROOFS.md`.
