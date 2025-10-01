## What are Multi-Dimensional Numbers?

### One Dimensional Numbers
**1D Numbers** (normal numbers) are expressed by a sequence of digits, written along a line:

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;… a<sub>4</sub> a<sub>3</sub> a<sub>2</sub> a<sub>1</sub> a<sub>0</sub> . a<sub>-1</sub> a<sub>-2</sub> a<sub>-3</sub> a<sub>-4</sub> …

such as:

```     3.1415926536```

and since a _line_ is a **1 dimensional** entity, what happens if we extend this?

### Two Dimensional Numbers

 **2D Numbers** extend this by stating that numbers are expressed by a 2-dimensional grid of digits, kind of like a matrix:


```text
      3  0  0  0  0  0  0  0  0  0 │  0
      8  5 ─2  0  0  0  0  0  0  0 │  0
      4  0  3  0  0  0  0  0  0  0 │  0
      1  4  8  5 ─2  0  0  0  0  0 │  0
      1  8  4  0  3  0  0  0  0  0 │  0
      3  7  0  4  8  5 ─2  0  0  0 │  0
      4  5  1  8  4  0  3  0  0  0 │  0
      6  4  3  7  1  4  8  5 ─2  0 │  0
      6  3  5  6  2  9  5  1  4  1 │  3
     ──────────────────────────────┼────
      0  0  0  0  0  0  0  0  0  0 │  0
```

This shows π, expressed in 2-dimensional numbers.

#### They're Backwards!

Notice how π above is backwards?  That's how we do it for higher dimension numbers.  The digits increase in order of magnitude rightwards → and upwards ↑, because now they seem to be forming a space of some sort.

### Rules

#### Carry-overs

Everything else is the same, except for one thing: carry-overs.  When counting up in 2-dimensions, the carry-over spreads both towards *x*+ and *y*+:

```text
 0 │ 0 0 0        0 │ 0 0 0        0 │ 1 0 0
 0 │ 9 0 0    +   0 │ 1 0 0    =   0 │ 0 1 0
 ──┼───────       ──┼───────       ──┼───────

     9        +       1        =       10
```

#### Extending 1D rules

All the rest of the _rules_ with multi-dimensional numbers (MDN) follow from basic rules in 1-dimensional numbers.  In other words: MDN's must follow the rules of 1-dimensional numbers.

There are some surprising results.

#### Positive / negative

The first surprising result is that we cannot always apply 'negative' or 'positive' to the entire number.  Now it must be a property of the digit.
```text
 0 │-3 0 0
 0 │-2 3 0
 ──┼───────
```

#### Polymorphic carry-overs

Due to sign ambiguities, there are situations where an MDN has more than one valid form.  These are said to be different expressions of the same number, or at least have some form of _polymorphic_ equivalence.

```text
 0 │-3 0 0        0 │-2 0 0
 0 │-2 3 0    =   0 │ 8 2 0
 ──┼───────       ──┼───────
```

#### Conversion between bases

It is not always possible to convert an MDN from one base to another, due to information loss.  Therefore, changing the working base will result in all digits being zeroed.

#### Integers versus fractional amounts

MDN's behave very differently for the integer part and fractional part of a Real number.  The integer part is symmetric across _x_ and _y_, expanding predictably along the positive diagonal.

```text
 0 │  1  0  0  0  0  0  0  0  0
 0 │  2  9  3  0  0  0  0  0  0
 0 │  3  6  3  9  1  0  0  0  0
 0 │  4  0  9  8  4  2  0  0  0
 0 │  5  2  1  1  6  4  1  0  0
 0 │  6  1  5  8  1  8  9  0  0
 0 │  7  9  2  5  1  9  3  3  0
 0 │  8  4  9  1  2  0  6  9  0
 0 │  9  8  7  6  5  4  3  2  1
───┼────────────────────────────
 0 │  0  0  0  0  0  0  0  0  0
```
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**Integer part**

**However** the fractional amount experiences a form of _symmetry breaking_, and can be expressed _either_ as an _x_ form or _y_ form.  The application calls this its **Fraxis**.

```text
 0  0  0  0  0  0  0  0  0 │  0        0 │  3  0  0  0  0  0  0  0  0
 5 ─2  0  0  0  0  0  0  0 │  0       ───┼────────────────────────────
 0  3  0  0  0  0  0  0  0 │  0        0 │  1  0  0  0  0  0  0  0  0
 4  8  5 ─2  0  0  0  0  0 │  0        0 │  4 ─2  0  0  0  0  0  0  0
 8  4  0  3  0  0  0  0  0 │  0        0 │  1  5  0  0  0  0  0  0  0
 7  0  4  8  5 ─2  0  0  0 │  0        0 │  5  8  3 ─2  0  0  0  0  0
 5  1  8  4  0  3  0  0  0 │  0        0 │  9  4  0  5  0  0  0  0  0
 4  3  7  1  4  8  5 ─2  0 │  0        0 │  2  0  4  8  3 ─2  0  0  0
 3  5  6  2  9  5  1  4  1 │  3        0 │  6  7  8  4  0  5  0  0  0
───────────────────────────┼────       0 │  5  3  1  0  4  8  3 ─2  0
 0  0  0  0  0  0  0  0  0 │  0        0 │  4  4  5  7  8  4  0  5  0
```
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**Pi, x fraxis**&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**Pi, y fraxis**

