# Corrected backend regression checks

The source after release 1.1.2 corrects two problems: decimal places below
tenths used the wrong fractional-cascade coefficients, and sign-cleanup
carries could lose incoming increments. The tests build the same numerical
sources as the application, without requiring Qt. They do not test or repair
the iterative division algorithms.

## Build and run on Linux

Requirements: CMake 3.16 or later, a C++17 compiler, and Python 3. Run from
the repository root:

```bash
set -o pipefail
cmake -S tests -B build/fraxis -DCMAKE_BUILD_TYPE=Release -DCMAKE_UNITY_BUILD=ON
cmake --build build/fraxis --parallel 2 2>&1 | tee ~/output_mdn-backend-build.log
ctest --test-dir build/fraxis --output-on-failure
```

For an assertion-enabled build, repeat with `build/fraxis-debug` and
`-DCMAKE_BUILD_TYPE=Debug`. The suite is standalone: running CTest in the
ordinary GUI build directory does not run these tests.

The 11 test methods cover both fraxes, decimal places down to millionths,
multiple bases, sign conventions, shifted inputs, input methods, large
binomial coefficients, transposition, precision windows and carry conservation.
The primary comparison keeps every finite coefficient and proves carry
equivalence by clearing Laurent denominators and comparing all coefficients
of a formal polynomial. Numerical specializations are supplementary checks;
they do not define a magnitude for an MDN.

## Use the corrected library with the installed Linux GUI

For the matching Linux build, launch from the repository root:

```bash
LD_LIBRARY_PATH="$PWD/build/fraxis" /usr/bin/mdn_gui
```

This selects the locally compiled library for that process. The old download
packages themselves have not changed. A full source build of the GUI is
described in `gui/help/install.md`.

## Scope and performance

The regression suite is finite and does not establish convergence of arbitrary
division or infinite inputs. An additional base-two stress input,
`0.101100100111` at cascade depth 30 with positive sign preference, exceeded a
15-second probe timeout during review. No wrong answer was established by
that timeout; deep cascades can still be expensive.
