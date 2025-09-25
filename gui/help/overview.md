# MultiDimensionalNumbers Help

Welcome to **MultiDimensionalNumbers (MDN)**!
This guide will help you get started with creating, editing, and visualizing multidimensional numbers.

---

## Overview

MultiDimensionalNumbers (MDN) is a mathematical framework and software tool that extends ordinary numbers into higher dimensions.
With MDN, you can:

- Represent numbers in **1D, 2D, or higher**.
- Perform arithmetic across dimensions.
- Store, edit, and copy selections like you would in a spreadsheet.
- Visualize structures interactively with the GUI.

---

## Getting Started

### Opening a Project

To create a new MDN project:

1. Click **File → New Project**.
2. Choose a base (e.g. binary, decimal, hexadecimal).
3. Set the desired precision.
4. Press **OK** to begin.

> 💡 *Tip:* Projects can hold multiple **Mdn2d** tabs. Each tab contains its own number grid.

---

## Number Basics

Numbers in MDN are built from two parts:

- **Integer part** (to the left of the radix)
- **Fractional part** (to the right of the radix)

For example:

```

Base 2:   101.11₂  →  5.75
Base 16:  A3.F₁₆   →  163.9375

```

---

## Selections

Selections work like spreadsheet ranges, but with multidimensional awareness:

- **Single cell**: `(3, 4)`
- **Rectangle**: `(0,0) → (7,5)`
- **Infinite edge**: `(−∞, 0) → (10, ∞)`

Selections can span **multiple tabs**, but always share the same grid region.

[See the detailed guide →](selection.md)

---

## Keyboard Shortcuts

- `Ctrl+N` → New Project
- `Ctrl+S` → Save
- `Ctrl+C` → Copy selection
- `Ctrl+V` → Paste

---

## Further Reading

- [Selections in MDN](selection.md)
- [Project Configuration](config.md)
- [GitHub Repository](https://github.com/YourOrg/MultiDimensionalNumbers)

---

*Last updated: September 2025*
```
