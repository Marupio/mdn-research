// =============================================================
// library/serialize.h — text+binary I/O for core types (no Qt)
// =============================================================
#pragma once
#include <iosfwd>
#include <string>
#include <vector>
#include "Coord.h"
#include "Digit.h"
#include "Rect.h"
#include "Mdn2d.h"     // uses getRowRange / setRowRange / bounds / clear / hasBounds

namespace mdn::serialize {

// ---- Text format (human-readable, stable) ------------------------------
// Format: "MDN2D v1" header, then one line with bounds, then H rows of TSV.
// Coordinate basis: bottom-left (x0,y0) to top-right (x1,y1), inclusive.
// Row order: y = y0 .. y1 (ascending). Column order: x = x0 .. x1 (ascending).
// Cells are decimal integers in [-128, 127]. Zero means empty.
// Example:
//   MDN2D v1\n
//   bounds 0 0 3 2\n
//   1\t2\t3\t4\n
//   5\t0\t0\t9\n
//   0\t0\t0\t0\n
// -----------------------------------------------------------------------

// Save/load whole MDN as text. Returns true on success.
bool saveText(std::ostream& os, const Mdn2d& mdn);
bool loadText(std::istream& is, Mdn2d& mdn);

// Convenience stream operators (delegates to saveText/loadText).
std::ostream& operator<<(std::ostream& os, const Mdn2d& mdn);
std::istream& operator>>(std::istream& is, Mdn2d& mdn);

// Pretty streams for small types (human-friendly; not machine schema).
std::ostream& operator<<(std::ostream& os, const Coord& c);
std::ostream& operator<<(std::ostream& os, const Rect& r);

// ---- Binary format (compact; versioned) -------------------------------
// Magic: 'M' 'D' 'N' 'B' (0x4D 0x44 0x4E 0x42)
// u16 version = 1, u16 reserved = 0
// i32 x0,y0,x1,y1 (inclusive bounds)
// then (H*W) bytes of row-major Digit values, bottom-to-top rows
// Endianness: values are little-endian, but Digit bytes are endian-agnostic.

bool saveBinary(std::ostream& os, const Mdn2d& mdn);
bool loadBinary(std::istream& is, Mdn2d& mdn);

// ---- Utility -----------------------------------------------------------
// TSV helpers reused by copy/paste and text I/O
std::string rowToTsv(const std::vector<Digit>& row);

} // namespace mdn::serialize
