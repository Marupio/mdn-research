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

#include "Carryover.h"
#include "Coord.h"
#include "Digit.h"
#include "Fraxis.h"

namespace mdn {

// Represents a 2D Multi-Dimensional Number (MDN).
class Mdn2d {

    // Calculate minimum fraction value to add to a digit, that it will appear as a non-zero digit
    // within m_maxSpan (numerical precision)
    static double static_calculateEpsilon(int m_maxSpan, int m_base);


public:

    // Check for the type of carryover, given the pivot digit and x and y axial digits
    static Carryover static_checkCarryover(Digit p, Digit x, Digit y, Digit base);


    // *** Typedefs

    using WritableLock = std::unique_lock<std::shared_mutex>;
    using ReadOnlyLock = std::shared_lock<std::shared_mutex>;


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
                private: Digit locked_getValue(const Coord& xy) const; public:

                // Assembles the row at the given y index value, spanning the x bounds of full MDN
                std::vector<Digit> getRow(int y) const;
                private: std::vector<Digit> locked_getRow(int y) const; public:

                // Assembles the row at the given y index value, spanning the x bounds of full MDN
                void getRow(int y, std::vector<Digit>& digits) const;
                private: void locked_getRow(int y, std::vector<Digit>& digits) const; public:

                // Assembles the col at the given x index value, spanning the y bounds of full MDN
                std::vector<Digit> getCol(int x) const;
                private: std::vector<Digit> locked_getCol(int x) const; public:

                // Assembles the col at the given x index value, spanning the y bounds of full MDN
                void getCol(int x, std::vector<Digit>& digits) const;
                private: void locked_getCol(int x, std::vector<Digit>& digits) const; public:


            // *** Setters

                // Clears all digits in the MDN.
                void clear();
                private: void locked_clear(); public:

                // Set the value at xy to zero, returns false only if xy is below precision
                bool setToZero(const Coord& xy);
                private: bool locked_setToZero(const Coord& xy); public:

                // Set the value at coords to zero, returns number of digits changed
                int setToZero(const std::unordered_set<Coord>& coords);
                private: int locked_setToZero(const std::unordered_set<Coord>& purgeSet); public:

                // Changes the value at xy, returns false only if xy is confirmed below precision
                bool setValue(const Coord& xy, Digit value);
                bool setValue(const Coord& xy, int value);
                bool setValue(const Coord& xy, long value);
                bool setValue(const Coord& xy, long long value);
                private: bool locked_setValue(const Coord& xy, Digit value); public:


            // *** Full Mdn2d mathematical operations

                // Addition: *this + rhs = ans, overwrites ans
                void plus(const Mdn2d& rhs, Mdn2d& ans) const;
                private: void locked_plus(const Mdn2d& rhs, Mdn2d& ans) const; public:

                // Subtraction: *this - rhs = ans, overwrites ans
                void minus(const Mdn2d& rhs, Mdn2d& ans) const;
                private: void locked_minus(const Mdn2d& rhs, Mdn2d& ans) const; public:

                // Multiplication: *this x rhs = ans, overwrites ans
                void multiply(const Mdn2d& rhs, Mdn2d& ans) const;
                private: void locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const; public:

                // Division: *this / rhs = ans, overwrites ans
                void divide(const Mdn2d& rhs, Mdn2d& ans) const;
                private: void locked_divide(const Mdn2d& rhs, Mdn2d& ans) const; public:


            // *** Addition / subtraction

                // Add the given number at xy, breaking into integer and fraxis operations
                void add(const Coord& xy, float realNum, Fraxis fraxis);
                void add(const Coord& xy, double realNum, Fraxis fraxis);
                private: void locked_add(const Coord& xy, double realNum, Fraxis fraxis); public:

                // Subtract the given number at xy, breaking into integer and fraxis operations
                void subtract(const Coord& xy, float realNum, Fraxis fraxis);
                void subtract(const Coord& xy, double realNum, Fraxis fraxis);

                // Addition component: integer part, at xy with symmetric carryover
                void add(const Coord& xy, Digit value, Fraxis unused=Fraxis::Unknown);
                void add(const Coord& xy, int value, Fraxis unused=Fraxis::Unknown);
                private: void locked_add(const Coord& xy, int value); public:
                void add(const Coord& xy, long value, Fraxis unused=Fraxis::Unknown);
                private: void locked_add(const Coord& xy, long value); public:
                void add(const Coord& xy, long long value, Fraxis unused=Fraxis::Unknown);
                private: void locked_add(const Coord& xy, long long value); public:

                // Subtraction component: integer part, at xy with symmetric carryover
                void subtract(const Coord& xy, Digit value, Fraxis unused=Fraxis::Unknown);
                void subtract(const Coord& xy, int value, Fraxis unused=Fraxis::Unknown);
                void subtract(const Coord& xy, long value, Fraxis unused=Fraxis::Unknown);
                void subtract(const Coord& xy, long long value, Fraxis unused=Fraxis::Unknown);

                // Addition component: fractional part, at xy with assymmetric cascade
                void addFraxis(const Coord& xy, float fraction, Fraxis fraxis);
                void addFraxis(const Coord& xy, double fraction, Fraxis fraxis);
                private: void locked_addFraxis(
                    const Coord& xy, double fraction, Fraxis fraxis
                ); public:

                // Subtract a fractional value cascading along the fraxis
                void subtractFraxis(const Coord& xy, float fraction, Fraxis fraxis);
                void subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis);


            // *** Multiplication / divide

                // Multiply the full Mdn2d by an integer
                void multiply(Digit value);
                void multiply(int value);
                void multiply(long value);
                void multiply(long long value);
                private: void locked_multiply(int value); public:
                private: void locked_multiply(long value); public:
                private: void locked_multiply(long long value); public:


            // *** Conversion / display

                // Converts the MDN to a human-readable string.
                std::string toString() const;

                // Converts the MDN to an array of strings, representing rows
                std::vector<std::string> toStringRows() const;

                // Converts the MDN to an array of strings, representing columns
                std::vector<std::string> toStringCols() const;


        // *** Transformations

            // Return the Carryover type (enumareation) of xy
            Carryover checkCarryover(const Coord& xy) const;
            private: Carryover locked_checkCarryover(const Coord& xy) const; public:

            // Perform a manual carry-over at coordinate (x, y)
            void carryover(const Coord& xy);
            private: void locked_carryover(const Coord& xy); public:

            // General shift interface
            void shift(int xDigits, int yDigits);
            void shift(const Coord& xy);
            private: void locked_shift(const Coord& xy); public:
            private: void locked_shift(int xDigits, int yDigits); public:

            // Shift all digits in a direction (R=+X, L=-X, U=+Y, D=-Y)
            void shiftRight(int nDigits);
            private: void locked_shiftRight(int nDigits); public:
            void shiftLeft(int nDigits);
            private: void locked_shiftLeft(int nDigits); public:
            void shiftUp(int nDigits);
            private: void locked_shiftUp(int nDigits); public:
            void shiftDown(int nDigits);
            private: void locked_shiftDown(int nDigits); public:

            // Swap X and Y
            void transpose();
            private: void locked_transpose(); public:

            // Clears all addressing and bounds data, and rebuilds it
            void rebuildMetadata() const;
            private: void locked_rebuildMetadata() const; public:

            // Returns true if m_boundsMin and m_boundsMax are both valid, finite numbers
            bool hasBounds() const;
            private: bool locked_hasBounds() const; public:

            // Retuns bounds of non zero entries in m_raw
            std::pair<Coord, Coord> getBounds() const;
            private: std::pair<Coord, Coord> locked_getBounds() const; public:

            // Returns polymorphic nodes, demand driven
            const std::unordered_set<Coord>& getPolymorphicNodes() const;
            private: const std::unordered_set<Coord>& locked_getPolymorphicNodes() const; public:

            // Take on polymorphism state x0 (all p-nodes are negative)
            void polymorphism_x0();
            private: void locked_polymorphism_x0(); public:

            // Take on polymorphism state y0 (all p-nodes are positive)
            void polymorphism_y0();
            private: void locked_polymorphism_y0(); public:


        // *** Other functionality

            int getPrecision() const;
            private: int locked_getPrecision() const; public:

            // Change the setting for m_maxSpan, returns the number of dropped digits
            int setPrecision(int newMaxSpan);
            private: int locked_setPrecision(int newMaxSpan); public:

            // Query the precision status of xy to ensure maxSpan is not exceeded
            // Returns:
            //  * PrecisionStatus::Below  - above precision window
            //  * PrecisionStatus::Inside - within precision window
            //  * PrecisionStatus::Above  - below precision window
            PrecisionStatus checkPrecisionWindow(const Coord& xy) const;
            private: PrecisionStatus locked_checkPrecisionWindow(const Coord& xy) const; public:

            // Clear all derived data, update event number
            void modified();


    // *** Member Operators

        // Accessor to read-only digit at coordinate (x, y).
        Digit operator()(int x, int y) const;
        Digit operator()(const Coord& xy) const;

        // Assignment addition, *this += rhs
        Mdn2d& operator+=(const Mdn2d& rhs);
        private: Mdn2d& locked_plusEquals(const Mdn2d& rhs); public:

        // Assignment subtraction, *this -= rhs
        Mdn2d& operator-=(const Mdn2d& rhs);
        private: Mdn2d& locked_minusEquals(const Mdn2d& rhs); public:

        // Assignment multiplication, *this *= rhs
        Mdn2d& operator*=(const Mdn2d& rhs);
        private: Mdn2d& locked_timesEquals(const Mdn2d& rhs); public:

        // Placeholder for MDN division.
        Mdn2d& operator/=(const Mdn2d& rhs);

        // Scalar multiplication.
        Mdn2d& operator*=(int scalar);

        // Scalar multiplication.
        Mdn2d& operator*=(long scalar);

        // Scalar multiplication.
        Mdn2d& operator*=(long long scalar);

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


        // *** Internal functions
        // All these functions require the mutex to be locked first before calling

            // Clears all addressing and bounds data
            // * Has no public 'clearMetadata() function
            void internal_clearMetadata() const;

            // Add xy as a non-zero number position to the metadata
            // * Has no public 'insertAddress() function
            void internal_insertAddress(const Coord& xy) const;

            // Checks if value is within +/- (m_base-1).  If not, throws or returns false.
            bool internal_checkDigit(const Coord& xy, Digit value) const;

            // Purge any digits that exceed the precision window, return the number of purged digits
            int internal_purgeExcessDigits();

            // Update the m_boundsMin and m_boundsMax variables based on the current values
            void internal_updateBounds();

            // Execute the fraxis propagation algorithm
            //  dX, dY, c - constants to guide propagation:
            //      x Direction: -1, 0, -1
            //      y Direction: 0, -1, 1
            void internal_fraxis(const Coord& xy, double f, int dX, int dY, int c);
            void internal_fraxisCascade(const Coord& xy, Digit d, int c);

            // // plusEquals variant: *this += rhs x scalar, used in mdn x mdn algorithm
            // Mdn2d& internal_plusEquals(const Mdn2d& rhs, int scalar);

            // Creates a copy of *this and performs a multiply and shift, used in mdn x mdn
            //  return = (*this x value).shift(xy)
            Mdn2d& internal_copyMultiplyAndShift(int value, const Coord& shiftXY) const;

            // Scan for carryovers, perform all 'Required' carryovers, return list of optional c/o
            void internal_polymorphicScanAndFix();

            // Find all the 'Optional' carryovers, create m_polymorphicNodes data
            void internal_polymorphicScan() const;

            // Perform a blind single carryover at xy without any checks
            void internal_oneCarryover(const Coord& xy);

            // Perform a carryover during math operations (xy magnitude must exceed base)
            void internal_ncarryover(const Coord& xy);


// TODO
//            // Given another Mdn2d, copy the contents of *this into other.  Requires other's lock.
//            void locked_copyInto(Mdn2d& other);


            // Element-wise multiply by value - int only, intended for use in x and / algorithms
            private: void locked_multiply(int value); public:

            // Create a multiline string representing this Mdn2d
            private: std::string locked_toString() const; public:

            // Create an array of strings, each a display of a column of this Mdn2d
            private: std::vector<std::string> locked_toStringCols() const; public:

            // Create an array of strings, each a display of a row of this Mdn2d
            private: std::vector<std::string> locked_toStringRows() const; public:


    //     // Accessor to modifiable digit at coordinate (x, y) - non-const access to raw data is
    //      // only for internal uses - such modifications require updating metadata
    //     Digit& operator()(int x, int y);
    //     Digit& operator()(const Coord& xy);


    // *** Private Member Data

        // Numeric base for digit calculations
        const int m_base;
        const Digit m_dbase;

        // Maximum distance from high magnitude to low magnitude, -1 for no limit
        // A measure of numerical precision.
        //
        //  Digit values:   1 0|0 0 1  <-- span == 4
        //  Digit indices: -2-1 0 1 2
        // maxOrderX - minMagnitudeX
        int m_maxSpan;

        // Smallest value added to a digit that can cascade to a non-zero value within the precision
        //  window
        mutable double m_epsilon;

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

            // Polymorphic nodes and the last time it was calculated
            mutable std::unordered_set<Coord> m_polymorphicNodes;
            mutable long m_polymorphicNodes_event;

            // Event number for tracking when something is out-of-date
            long m_event;
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
