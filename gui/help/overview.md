## What are Multi-Dimensional Numbers?

Multi-Dimensional Numbers (MDNs) extend the familiar string of digits into a grid of digits. **The entire grid represents one number.** It is not a spreadsheet of independent numbers, and its multiplication is not matrix multiplication.

David Gaden developed the idea and a first C++ numerical test application around 2000. The present application provides a way to enter, display and calculate with these numbers.

### Positions and carrying

In an ordinary decimal numeral, higher places are written to the left. In the MDN display, the forward positional directions are **rightward → and upward ↑**. The origin `(0,0)` is the unity position.

The meaning of those directions comes from the carry rule. In base ten, remove ten units from a cell and add one unit to **each** of its two forward neighbours. Thus `9 + 1` gives:

```text
                     y
                     ↑
                     1  0
                     0  1  → x
                     ↑
                 unity cell
```

Here the lower-left cell is `(0,0)`, the cell to its right is `(1,0)`, and the cell above it is `(0,1)`. This whole two-digit grid is the integer ten.

More generally, a carry of an integer amount `k` changes three digits:

| Cell | Change |
|---|---:|
| `(i,j)` | `−b × k` |
| `(i+1,j)` | `+k` |
| `(i,j+1)` | `+k` |

Here `b` is the base. Negative carry amounts reverse the operation. Grids connected by a finite sequence of these carries represent the same finite MDN.

The symbols `X` and `Y` can mean a one-cell shift right and up. In that notation, **`X + Y = b` expresses the carry rule**. It does not say that each forward cell separately has ordinary value `b`, or that the two cells must each be worth `b/2`. No separate ordinary scalar magnitude is assigned to either axis.

### Arithmetic and different displays

Addition and subtraction combine corresponding digits. Multiplication pairs every digit of one grid with every digit of the other: multiply their coefficients, add their position coordinates, and combine contributions at the same position. Carries can then change the display. These finite operations preserve ordinary associativity, commutativity and distributivity.

A number can have different valid displays. For example, in base ten:

```text
         0  0                    1  0
         1  0        =          -9  1
```

The lower-left cell is unity in each grid. The right grid is obtained by one carry at that cell: `1 = −9 + X + Y`. The application calls this freedom **polymorphism**. Its positive and negative carry conventions select displays of the same number; equality need not mean identical visible digits.

Digit coefficients can be positive or negative, and negating every digit gives the additive negative of the whole MDN. The carry rule does not, by itself, put all MDNs into a single positive-to-negative order. In particular, mixed digit signs do not supply such an ordering.

### Integers and fractional directions

An integer is entered by adding its whole amount to unity and carrying. Starting this way treats the two axes symmetrically. A grid's symmetry alone does not imply that it is an integer: there are other symmetric MDNs.

For fractional input, the application selects a direction called the **fraxis**. An X-fraxis tenth produces a diagonal cascade up and left. A Y-fraxis tenth is its transpose, down and right.

For example, the first four raw terms of an X-fraxis tenth are:

| Position | Digit |
|---|---:|
| `(-1,0)` | `1` |
| `(-2,1)` | `−1` |
| `(-3,2)` | `1` |
| `(-4,3)` | `−1` |

Every listed digit belongs to the same number. Extending the cascade adds further alternating terms. With exactly these four terms retained, multiplying by ten gives a unity digit `+1` at `(0,0)` **and a boundary digit `−1` at `(-4,4)`**. The boundary is part of the finite result.

Deeper decimal places have different coefficients. For example, the first four raw terms of `0.01X` are `1, −2, 3, −4` at `(-2,0), (-3,1), (-4,2), (-5,3)`. They are not just a shifted copy of the alternating tenth. The Y version exchanges the two coordinates.

The GUI sets a finite cascade depth. A fractional label such as `0.1X` therefore specifies a finite directional construction, not an exact ordinary reciprocal of ten. On the appropriate axis, a directional input has its familiar base-ten reading, while the complete grid also retains its off-axis digits and boundary.

Infinite decimal encodings and infinite cascade limits require additional definitions. In particular, retaining distinct X and Y constructions does not make both exact inverses of ten in one ordinary ring: such inverses would have to coincide.

### Mixing the two directions

Adding an X-fraction and a Y-fraction combines their grids. It does not generally combine their input labels as ordinary real numbers: `0.4X + 0.6Y` is not unity.

Multiplication can produce a different kind of pattern. As the cutoffs of `0.1X × 0.1Y` are extended together, central digits can cycle instead of settling. These deterministic changing digits are called **fuzzy digits** in this project. They are not random quantities or fuzzy-set membership values.

A fixed display window can miss distant terms that affect a later product. Keeping the boundary information is therefore essential when studying these mixed constructions and their infinite extensions.

### Division and changing base

The application also implements iterative division, with controls for iteration count and direction, and a transpose operation. Division can be continued to refine a result; its convergence and directional behaviour remain subjects for investigation. The presence of an algorithm is separate from a proof that it converges for every input.

Changing the base changes the carry rule. The present GUI clears the grid on a base change; it does not implement a general conversion between bases. Whether a conversion preserves information depends on the map and the representations allowed, rather than following from the carry rule alone.
