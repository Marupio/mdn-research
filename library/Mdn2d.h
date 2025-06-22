#ifndef MDN2D_H
#define MDN2D_H

#include <unordered_map>
#include <utility>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "Digit.h"
#include "Coord.h"

namespace mdn {

// Represents a 2D Multi-Dimensional Number (MDN).
class Mdn2d {

public:

    // *** Constructors

        // Constructs an empty MDN with a given base.
        Mdn2d(int base = 10);

        // Constructs an MDN initialized with a single integer value added to the
        // digit origin.
        Mdn2d(int base, int initVal);

        // Constructs an MDN with a floating point initial value.
        Mdn2d(int base, float initVal, int fraxis);


    // *** Member Functions

        // *** Top-level API functionality

            // Adds a value to the digit at coordinate (x, y).
            void addValueAt(int x, int y, int value);
            void addValueAt(Coord xy, int value);

            // Retrieves the value at coordinate (x, y), or 0 if not present.
            Digit getValueAt(int x, int y) const;
            Digit getValueAt(Coord xy) const;

            // Clears all digits in the MDN.
            void clear();

            // Converts the MDN to a human-readable string.
            std::string toString() const;


            // Remove all zero digits from m_raw entries
            // All stored values should be non-zero, but zeroes can be created via direct accessors
            void purgeZeroes();


        // *** Low-level functionality

            // Perform a carry-over at coordinate (x, y)



    // *** Member Operators

        // Accessor to modifiable digit at coordinate (x, y).
        Digit& operator()(int x, int y);
        Digit& operator()(Coord xy);

        // Accessor to read-only digit at coordinate (x, y).
        Digit operator()(int x, int y) const;
        Digit operator()(Coord xy) const;

        // Element-wise addition.
        Mdn2d& operator+=(const Mdn2d& rhs);

        // Element-wise subtraction.
        Mdn2d& operator-=(const Mdn2d& rhs);

        // Placeholder for MDN multiplication.
        Mdn2d& operator*=(const Mdn2d& rhs);

        // Placeholder for MDN division.
        Mdn2d& operator/=(const Mdn2d& rhs);

        // Scalar multiplication.
        Mdn2d& operator*=(int scalar);

        // Scalar division.
        Mdn2d& operator/=(int scalar);

        // Equality comparison.
        bool operator==(const Mdn2d& rhs) const;

        // Inequality comparison.
        bool operator!=(const Mdn2d& rhs) const;


private:

    // Numeric base for digit calculations
    int m_base;

    // Sparse coordinate-to-digit mapping
    std::unordered_map<Coord, Digit> m_raw;
};

// Arithmetic binary operators
inline Mdn2d operator+(Mdn2d lhs, const Mdn2d& rhs) { return lhs += rhs; }
inline Mdn2d operator-(Mdn2d lhs, const Mdn2d& rhs) { return lhs -= rhs; }
inline Mdn2d operator*(Mdn2d lhs, const Mdn2d& rhs) { return lhs *= rhs; }
inline Mdn2d operator/(Mdn2d lhs, const Mdn2d& rhs) { return lhs /= rhs; }

inline Mdn2d operator*(Mdn2d lhs, int scalar) { return lhs *= scalar; }
inline Mdn2d operator/(Mdn2d lhs, int scalar) { return lhs /= scalar; }

} // end namespace mdn

#endif // MDN2D_H
