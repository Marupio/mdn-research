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

    static const int m_intMax;
    static const int m_intMin;

    // *** Constructors

        // Constructs an empty MDN with a given base.
        Mdn2d(int base = 10, int maxSpan = 16);

        // Constructs an MDN initialized with a single integer value added to the
        // digit origin.
        Mdn2d(int base, int maxSpan, int initVal);

        // Constructs an MDN with a floating point initial value.
        Mdn2d(int base, int maxSpan, double initVal, Fraxis fraxis);

        // *** Rule of five

            // Copy constructor
            Mdn2d(const Mdn2d& other);

            // Assignment operator
            Mdn2d& operator=(const Mdn2d& other);

            // Move operator
            Mdn2d(Mdn2d&& other) noexcept;

            // Move assignment operator
            Mdn2d& operator=(Mdn2d&& other) noexcept;


    // *** Member Functions

        // *** Top-level API functionality

            // *** Getters

                // Retrieves the value at coordinate (x, y), or 0 if not present.
                Digit getValue(const Coord& xy) const;

                // Assembles the row at the given y index value, spanning the x bounds of full MDN
                std::vector<Digit> getRow(int y) const;

                // Assembles the row at the given y index value, spanning the x bounds of full MDN
                void getRow(int y, std::vector<Digit>& digits) const;

                // Assembles the column at the given x index value, spanning the y bounds of full MDN
                std::vector<Digit> getCol(int x) const;

                // Assembles the column at the given x index value, spanning the y bounds of full MDN
                void getCol(int x, std::vector<Digit>& digits) const;


            // *** Setters

                // Clears all digits in the MDN.
                void clear();

                // Changes the value at coordinate (x, y).
                void setValue(const Coord& xy, Digit value);
                void setValue(const Coord& xy, int value);
                void setValue(const Coord& xy, long value);
                void setValue(const Coord& xy, long long value);


            // *** Full Mdn2d mathematical operations

                // Add / subtract a full Mdn2d: *this + rhs = ans
                // where ans only needs to have the same base and maxSpan - it will be overwritten
                void plus(const Mdn2d& rhs, Mdn2d& ans) const;
                void minus(const Mdn2d& rhs, Mdn2d& ans) const;

                // Multiply / divide full Mdn2d: *this x rhs = ans
                // where ans only needs to have the same base and maxSpan - it will be overwritten
                void multiply(const Mdn2d& rhs, Mdn2d& ans) const;
                void divide(const Mdn2d& rhs, Mdn2d& ans) const;


            // *** Addition / subtraction

                // Add a real number, in two parts: integer and fraxis
                void add(const Coord& xy, float realNum, Fraxis fraxis);
                void add(const Coord& xy, double realNum, Fraxis fraxis);

                // Subtract a real number, in two parts: integer and fraxis
                void subtract(const Coord& xy, float realNum, Fraxis fraxis);
                void subtract(const Coord& xy, double realNum, Fraxis fraxis);

                // Add a value to the digit at coordinate (x, y).
                void add(const Coord& xy, Digit value, Fraxis unused=Fraxis::Unknown);
                void add(const Coord& xy, int value, Fraxis unused=Fraxis::Unknown);
                void add(const Coord& xy, long value, Fraxis unused=Fraxis::Unknown);
                void add(const Coord& xy, long long value, Fraxis unused=Fraxis::Unknown);

                // Subtract a value to the digit at coordinate (x, y).
                void subtract(const Coord& xy, Digit value, Fraxis unused=Fraxis::Unknown);
                void subtract(const Coord& xy, int value, Fraxis unused=Fraxis::Unknown);
                void subtract(const Coord& xy, long value, Fraxis unused=Fraxis::Unknown);
                void subtract(const Coord& xy, long long value, Fraxis unused=Fraxis::Unknown);

                // Add a fractional value cascading along the fraxis
                void addFraxis(const Coord& xy, float fraction, Fraxis fraxis);
                void addFraxis(const Coord& xy, double fraction, Fraxis fraxis);

                // Subtract a fractional value cascading along the fraxis
                void subtractFraxis(const Coord& xy, float fraction, Fraxis fraxis);
                void subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis);


            // *** Multiplication / divide
            // Remember, *this is a Mdn2d, and scalars (e.g. int, float) are 1D numbers.  In most
            // cases, there is no valid multiplication / subtraction operation between 1D and 2D
            // numbers.  Just element-wise integer multiplication, which is a step in the multiply
            // algorithm for Mdn2d x Mdn2d.

                // Multiply the full Mdn2d by an integer
                void multiply(Digit value);
                void multiply(int value);
                void multiply(long value);
                void multiply(long long value);


            // *** Conversion / display

                // Converts the MDN to a human-readable string.
                std::string toString() const;

                // Converts the MDN to an array of strings, representing rows (along x digit axis).
                std::vector<std::string> toStringRows() const;

                // Converts the MDN to an array of strings, representing columns (along y digit axis).
                std::vector<std::string> toStringCols() const;


        // *** Transformations

            // Perform a carry-over at coordinate (x, y)
            void carryOver(const Coord& xy);

            // Shift all digits in a direction (R=+X, L=-X, U=+Y, D=-Y)
            void shiftRight(int nDigits);
            void shiftLeft(int nDigits);
            void shiftUp(int nDigits);
            void shiftDown(int nDigits);

            // Swap X and Y
            void transpose();

            // Clear and recalculate metadata
            void rebuildMetadata() const;

            // Returns true if m_boundsMin and m_boundsMax are both valid, finite numbers
            bool hasBounds() const;
            bool locked_hasBounds() const;

            // // Remove all zero digits from m_raw entries
            // void purgeZeroes();

        // *** Other functionality

            int getPrecision() const;
            int locked_getPrecision() const;

            // Change the setting for m_maxSpan
            void setPrecision(int newMaxSpan);
            void locked_setPrecision(int newMaxSpan);



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

        // Ensure the supplied argument 'that' is not the same as *this, throws if not
        void assertNotSelf(Mdn2d& that, const std::string& description) const;


        // *** Lock functions
        // All these functions require the mutex to be locked first before calling

            // Set all entries to zero, clear metadata
            void locked_clear();

            // Clears all addressing and bounds data
            void locked_clearMetadata() const;

            // Clears all addressing and bounds data, and rebuilds it
            void locked_rebuildMetadata() const;

            // Add xy as a non-zero number position to the metadata
            void locked_insertAddress(const Coord& xy) const;

            // Check xy in terms of bounds, ensuring maxSpan is not exceeded
            //  * if exceeded with smaller magnitude, returns false (i.e. do not set value)
            //  * if exceeded with larger magnitude, drops low magnitude values until maxSpan is
            //      respected.
            // Returns:
            //  * true  - xy is still valid to hold a value
            //  * false - xy is too small for given numerical precision to hold any value
            bool locked_checkBounds(const Coord& xy);

            // Update the m_boundsMin and m_boundsMax variables based on the current values
            void locked_updateBounds();

// TODO
//            // Given another Mdn2d, copy the contents of *this into other.  Requires other's lock.
//            void locked_copyInto(Mdn2d& other);

            // Return value at given coord
            Digit locked_getValue(const Coord& xy) const;

            // Construct a vector of digits for a row at given y value
            std::vector<Digit> locked_getRow(int y) const;

            // Fill the provided vector of digits at the given y value
            void locked_getRow(int y, std::vector<Digit>& digits) const;

            // Construct a vector of digits for a column at given x value
            std::vector<Digit> locked_getCol(int x) const;

            // Fill the provided vector of digits for a column at given x value
            void locked_getCol(int x, std::vector<Digit>& digits) const;

            // Set the value at the given coordinates
            void locked_setValue(const Coord& xy, Digit value);

            // Addition: *this + rhs = ans, overwrites ans
            void locked_plus(const Mdn2d& rhs, Mdn2d& ans) const;

            // Subtraction: *this - rhs = ans, overwrites ans
            void locked_minus(const Mdn2d& rhs, Mdn2d& ans) const;

            // Multiplication: *this x rhs = ans, overwrites ans
            void locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const;

            // Division: *this / rhs = ans, overwrites ans
            void locked_divide(const Mdn2d& rhs, Mdn2d& ans) const;

            // Add the given number at xy, breaking into integer and fraxis operations
            void locked_add(const Coord& xy, double realNum, Fraxis fraxis);

            // Subtract the given number at xy, breaking into integer and fraxis operations
            void locked_subtract(const Coord& xy, double realNum, Fraxis fraxis);

            // Addition component: integer part, at xy with symmetric carryover
            void locked_add(const Coord& xy, Digit value);
            void locked_add(const Coord& xy, int value);
            void locked_add(const Coord& xy, long value);
            void locked_add(const Coord& xy, long long value);

            // Subtraction component: integer part, at xy with symmetric carryover
            void locked_subtract(const Coord& xy, Digit value);
            void locked_subtract(const Coord& xy, int value);
            void locked_subtract(const Coord& xy, long value);
            void locked_subtract(const Coord& xy, long long value);

            // Addition component: fractional part, at xy with assymmetric cascade
            void locked_addFraxis(const Coord& xy, double fraction, Fraxis fraxis);

            // Addition component: fractional part, at xy with assymmetric cascade along X
            void locked_addFraxisX(const Coord& xy, double fraction);

            // Addition component: fractional part, at xy with assymmetric cascade along Y
            void locked_addFraxisY(const Coord& xy, double fraction);

            // Subtraction component: fractional part, at xy with assymmetric cascade
            void locked_subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis);

            // Subtraction component: fractional part, at xy with assymmetric cascade along X
            void locked_subtractFraxisX(const Coord& xy, double fraction);

            // Subtraction component: fractional part, at xy with assymmetric cascade along Y
            void locked_subtractFraxisY(const Coord& xy, double fraction);

            // Element-wise multiply by value - int only, intended for use in x and / algorithms
            void locked_multiply(int value);

            // Create a multiline string representing this Mdn2d
            std::string locked_toString() const;

            // Create an array of strings, each a display of a column of this Mdn2d
            std::vector<std::string> locked_toStringCols() const;

            // Create an array of strings, each a display of a row of this Mdn2d
            std::vector<std::string> locked_toStringRows() const;

            // Perform a carryover at the given coordinate
            void locked_carryOver(const Coord& xy);

            // Shift all digits in the number nDigits to the right (higher magnitude on X)
            void locked_shiftRight(int nDigits);

            // Shift all digits in the number nDigits to the left (lower magnitude on X)
            void locked_shiftLeft(int nDigits);

            // Shift all digits in the number nDigits upwards (higher magnitude on Y)
            void locked_shiftUp(int nDigits);
            // Shift all digits in the number nDigits to the right (higher magnitude on X)
            void locked_shiftDown(int nDigits);

            // Swap X and Y coordinates for the entire number
            void locked_transpose();

// TODO - get rid of these, but their algorithm are important...
void locked_addInteger(const Coord& xy, int value);
void locked_addReal(const Coord& xy, double realNum, Fraxis fraxis);
void locked_setValue(const Coord& xy, int value);

        // Returns pointer to existing digit entry, if it exists
        Digit* getPtr(const Coord& xy);

        // Set the given coordinate to zero (i.e. delete), may already be zero
        void locked_setToZero(const Coord& xy);

        // Set the given coordinates to zero
        void locked_setToZero(const std::unordered_set<Coord>& purgeSet);

    //     // Accessor to modifiable digit at coordinate (x, y) - non-const access to raw data is only for
    //     // internal uses - such modifications require updating metadata
    //     Digit& operator()(int x, int y);
    //     Digit& operator()(const Coord& xy);


    // *** Private Member Data

        // Numeric base for digit calculations
        const int m_base;

        // Maximum distance from high magnitude to low magnitude, -1 for no limit
        // A measure of numerical precision.
        //
        //  Digit values:   1 0|0 0 1  <-- span == 4
        //  Digit indices: -2-1 0 1 2
        // maxOrderX - minMagnitudeX
        int m_maxSpan;

        // Sparse coordinate-to-digit mapping
        std::unordered_map<Coord, Digit> m_raw;

        // Addressing
        // Iterate with iterators:
        // for (auto it = m_xIndex.begin(); it != m_xIndex.end(); ++it) {
        //     int x = it->first;
        //     const std::unordered_set<Coord>& coords = it->second;
        //     // use x and coords
        // }
        //
        // Iterate with a ranged for loop:
        // for (const auto& pair : m_xIndex) {
        //     int x = pair.first;
        //     const std::unordered_set<Coord>& coords = pair.second;
        //     // use x and coords
        // }
        // Iterate with structured bindings:
        // for (const auto& [x, coords] : m_xIndex) {
        //     // x is the key, coords is the value
        //     // use x and coords directly
        // }
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
