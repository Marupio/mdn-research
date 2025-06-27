#ifndef MDN2D_H
#define MDN2D_H

#include <functional>
#include <iostream>
#include <map>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "Coord.h"
#include "Digit.h"
#include "Fraxis.h"

namespace mdn {

// Represents a 2D Multi-Dimensional Number (MDN).
class Mdn2d {

public:

    // *** Typedefs

    using WritableLock = std::unique_lock<std::shared_mutex>;
    using ReadOnlyLock = std::shared_lock<std::shared_mutex>;

    // *** Constructors

        // Constructs an empty MDN with a given base.
        Mdn2d(int base = 10);

        // Constructs an MDN initialized with a single integer value added to the
        // digit origin.
        Mdn2d(int base, int initVal);

        // Constructs an MDN with a floating point initial value.
        Mdn2d(int base, double initVal, Fraxis fraxis);


    // *** Member Functions

        // *** Top-level API functionality

            // Adds a real number, expanding the integer part symmetrically and the fractional part
            // along the fraxis
            void addReal(int x, int y, double realNum, Fraxis fraxis);
            void addReal(const Coord& xy, double realNum, Fraxis fraxis);

            // Adds a value to the digit at coordinate (x, y).
            void addInteger(int x, int y, int value);
            void addInteger(const Coord& xy, int value);

            // Adds a fractional value cascading along the fraxis
            void addFraxis(const Coord& xy, double fraction, Fraxis fraxis);

            // Retrieves the value at coordinate (x, y), or 0 if not present.
            Digit getValue(int x, int y) const;
            Digit getValue(const Coord& xy) const;

            // Assembles the row at the given y index value, spanning the x bounds of full MDN
            std::vector<Digit> getRow(int y) const;

            // Assembles the column at the given x index value, spanning the y bounds of full MDN
            std::vector<Digit> getCol(int x) const;

            // Assembles the row at the given y index value, spanning the x bounds of full MDN
            void fillRow(int y, std::vector<Digit>& digits) const;

            // Assembles the column at the given x index value, spanning the y bounds of full MDN
            void fillCol(int x, std::vector<Digit>& digits) const;

            // Changes the value at coordinate (x, y).
            void setValue(int x, int y, int value);
            void setValue(const Coord& xy, int value);

            // Clears all digits in the MDN.
            void clear();

            // Converts the MDN to a human-readable string.
            std::string toString() const;

            // Converts the MDN to an array of strings, representing rows (along x digit axis).
            //  reverse - when true, result[0] is the highest magnitude row (greatest y digit).
            //          - when false, result[0] is the lowest magnitude row (lowest y digit).
            std::vector<std::string> toStringRows(bool reverse=true) const;

            // Converts the MDN to an array of strings, representing columns (along y digit axis).
            //  reverse - when true, result[0] is the highest magnitude col (greatest x digit).
            //          - when false, result[0] is the lowest magnitude col (lowest x digit).
            std::vector<std::string> toStringCols(bool reverse=true) const;


        // *** Low-level functionality

            // Perform a carry-over at coordinate (x, y)
            void carryOver(int x, int y);
            void carryOver(const Coord& xy);

            // Bring metadata up-to-date
            void rebuildMetadata() const;


    // *** Member Operators

        // Accessor to read-only digit at coordinate (x, y).
        Digit operator()(int x, int y) const;
        Digit operator()(const Coord& xy) const;

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

    // *** Private Member Functions

        // Lock m_mutex for writeable reasons (unique_lock)
        WritableLock lockWriteable() const;

        // Lock m_mutex for read-only reasons (shareable_lock)
        ReadOnlyLock lockReadOnly() const;


        // *** Lock functions
        // All these functions require the mutex to be locked first before calling

            void locked_clear();
            void locked_clearMetadata() const;

            // Check xy in terms of bounds, ensuring maxSpan is not exceeded
            //  * if exceeded with smaller magnitude, returns false (i.e. do not set value)
            //  * if exceeded with larger magnitude, drops low magnitude values until maxSpan is
            //      respected.
            // Returns:
            //  * true  - xy is still valid to hold a value
            //  * false - xy is too small for given numerical precision to hold any value
            bool locked_checkBounds(const Coord& xy);
            void locked_updateBounds();

            std::string locked_toString() const;
            std::vector<std::string> locked_toStringRows(bool reverse=true) const;
            std::vector<std::string> locked_toStringCols(bool reverse=true) const;
            Digit locked_getValue(const Coord& xy) const;
            std::vector<Digit> locked_getRow(int y) const;
            std::vector<Digit> locked_getCol(int x) const;
            void locked_fillRow(int y, std::vector<Digit>& digits) const;
            void locked_fillCol(int x, std::vector<Digit>& digits) const;
            void locked_setValue(const Coord& xy, int value);
            void locked_addReal(const Coord& xy, double realNum, Fraxis fraxis);
            void locked_addInteger(const Coord& xy, int value);
            void locked_addFraxis(const Coord& xy, double fraction, Fraxis fraxis);
            void locked_addFraxisX(const Coord& xy, double fraction);
            void locked_addFraxisY(const Coord& xy, double fraction);

        // Returns pointer to existing digit entry, if it exists
        Digit* getPtr(const Coord& xy);

        // Set the given coordinate to zero (i.e. delete), may already be zero
        void setToZero(const Coord& xy);

        // Remove all zero digits from m_raw entries
        void purgeZeroes();

        // Accessor to modifiable digit at coordinate (x, y) - non-const access to raw data is only for
        // internal uses - such modifications require updating metadata
        Digit& operator()(int x, int y);
        Digit& operator()(const Coord& xy);


    // *** Private Member Data

        // Numeric base for digit calculations
        const int m_base;

        // Maximum distance from high magnitude to low magnitude, -1 for no limit
        // A measure of numerical precision.
        //
        //  Digit values:   1 0|0 0 1  <-- span == 4
        //  Digit indices: -2-1 0 1 2
        // maxOrderX - minMagnitudeX
        const int m_maxSpan;

        // Sparse coordinate-to-digit mapping
        std::unordered_map<Coord, Digit> m_raw;

        // Addressing
        mutable std::map<int, std::unordered_set<Coord>> m_xIndex;
        mutable std::map<int, std::unordered_set<Coord>> m_yIndex;

        // Thread safety
        mutable std::shared_mutex m_mutex;


        // *** Metadata

            // Bounding box for non-zero digits
            mutable Coord m_boundsMin;
            mutable Coord m_boundsMax;
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
