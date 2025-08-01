#pragma once

#include "GlobalConfig.h"
#include "Mdn2dRules.h"

namespace mdn {

// Represents a 2D Multi-Dimensional Number (MDN).
class MDN_API Mdn2d : public Mdn2dRules {

protected:

public:

    // *** Constructors

        // Construct null
        Mdn2d(std::string nameIn="");

        // Construct from a configuration
        Mdn2d(Mdn2dConfig config, std::string nameIn="");


        // *** Rule of five

            // Copy constructor
            Mdn2d(const Mdn2d& other, std::string nameIn="");

            // Assignment operator
            Mdn2d& operator=(const Mdn2d& other);

            // Move operator
            Mdn2d(Mdn2d&& other, std::string nameIn="") noexcept;

            // Move assignment operator
            Mdn2d& operator=(Mdn2d&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2d() {}


    // *** Member Functions

        // *** Full Mdn2d mathematical operations

            // Addition: *this + rhs = ans, overwrites ans
            void plus(const Mdn2d& rhs, Mdn2d& ans) const;
            protected: CoordSet locked_plus(const Mdn2d& rhs, Mdn2d& ans) const; public:

            // Subtraction: *this - rhs = ans, overwrites ans
            void minus(const Mdn2d& rhs, Mdn2d& ans) const;
            protected: CoordSet locked_minus(const Mdn2d& rhs, Mdn2d& ans) const; public:

            // Multiplication: *this x rhs = ans, overwrites ans
            void multiply(const Mdn2d& rhs, Mdn2d& ans) const;
            protected: CoordSet locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const; public:

            // Division: *this / rhs = ans, overwrites ans
            void divide(const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis=Fraxis::Default) const;
            protected: CoordSet locked_divide(
                const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis=Fraxis::Default
            ) const; public:
            protected: CoordSet locked_divideX(const Mdn2d& rhs, Mdn2d& ans) const; public:
            protected: CoordSet locked_divideY(const Mdn2d& rhs, Mdn2d& ans) const; public:


        // *** Addition / subtraction

            // Add the given number at xy, breaking into integer and fraxis operations
            void add(const Coord& xy, float realNum, Fraxis fraxis=Fraxis::Default);
            void add(const Coord& xy, double realNum, Fraxis fraxis=Fraxis::Default);
            protected: CoordSet locked_add(
                const Coord& xy, double realNum, Fraxis fraxis=Fraxis::Default
            ); public:

            // Subtract the given number at xy, breaking into integer and fraxis operations
            void subtract(const Coord& xy, float realNum, Fraxis fraxis=Fraxis::Default);
            void subtract(const Coord& xy, double realNum, Fraxis fraxis=Fraxis::Default);

            // Addition component: integer part, at xy with symmetric carryover
            void add(const Coord& xy, Digit value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, Digit value); public:
            void add(const Coord& xy, int value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, int value); public:
            void add(const Coord& xy, long value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, long value); public:
            void add(const Coord& xy, long long value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, long long value); public:

            // Subtraction component: integer part, at xy with symmetric carryover
            void subtract(const Coord& xy, Digit value, Fraxis unused=Fraxis::Default);
            void subtract(const Coord& xy, int value, Fraxis unused=Fraxis::Default);
            void subtract(const Coord& xy, long value, Fraxis unused=Fraxis::Default);
            void subtract(const Coord& xy, long long value, Fraxis unused=Fraxis::Default);

            // Addition component: fractional part, at xy with assymmetric cascade
            void addFraxis(const Coord& xy, float fraction, Fraxis fraxis=Fraxis::Default);
            void addFraxis(const Coord& xy, double fraction, Fraxis fraxis=Fraxis::Default);
            protected: CoordSet locked_addFraxis(
                const Coord& xy, double fraction, Fraxis fraxis
            ); public:

            // Subtract a fractional value cascading along the fraxis
            void subtractFraxis(const Coord& xy, float fraction, Fraxis fraxis=Fraxis::Default);
            void subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis=Fraxis::Default);


        // *** Multiplication / divide

            // Multiply the full Mdn2d by an integer
            void multiply(Digit value);
            void multiply(int value);
            void multiply(long value);
            void multiply(long long value);
            protected: CoordSet locked_multiply(int value); public:
            protected: CoordSet locked_multiply(long value); public:
            protected: CoordSet locked_multiply(long long value); public:


    // *** Member Operators

        // Equality comparison
        //  The rules layer brings carryovers, allowing us to find equivalence between different
        //  states of polymorphism.  But for now, equivalence only works with a default sign
        //  convention (Mdn2dConfig)
        bool operator==(const Mdn2d& rhs) const;

        // Inequality comparison.
        bool operator!=(const Mdn2d& rhs) const;

        // Assignment addition, *this += rhs
        Mdn2d& operator+=(const Mdn2d& rhs);
        protected:
            // I'm different from unlocked: I return the changed set, but the standard += return is
            //  just *this
            CoordSet locked_plusEquals(const Mdn2d& rhs);
        public:

        // Assignment subtraction, *this -= rhs
        Mdn2d& operator-=(const Mdn2d& rhs);
        protected:
            // I'm different from unlocked: I return the changed set, but the standard -= return is
            //  just *this
            CoordSet locked_minusEquals(const Mdn2d& rhs);
        public:

        // Assignment multiplication, *this *= rhs
        Mdn2d& operator*=(const Mdn2d& rhs);
        protected:
            // I'm different from unlocked: I return the changed set, but the standard -= return is
            //  just *this
            CoordSet locked_timesEquals(const Mdn2d& rhs);
        public:

        // Placeholder for MDN division.
        Mdn2d& operator/=(const Mdn2d& rhs);

        // Scalar multiplication.
        Mdn2d& operator*=(int scalar);

        // Scalar multiplication.
        Mdn2d& operator*=(long scalar);

        // Scalar multiplication.
        Mdn2d& operator*=(long long scalar);


protected:

        // *** Internal functions
        // All these functions require the mutex to be locked first before calling

            // Apply default to fraxis as required
            void internal_checkFraxis(Fraxis& fraxis) const;

            // Execute the fraxis propagation algorithm
            //  dX, dY, c - constants to guide propagation:
            //      x Direction: -1, 0, -1
            //      y Direction: 0, -1, 1
            CoordSet internal_fraxis(const Coord& xy, double f, int dX, int dY, int c);
            CoordSet internal_fraxisCascade(const Coord& xy, Digit d, int c);

            // // plusEquals variant: *this += rhs x scalar, used in mdn x mdn algorithm
            // Mdn2d& internal_plusEquals(const Mdn2d& rhs, int scalar);

            // Creates a copy of *this and performs a multiply and shift, used in mdn x mdn
            //  return = (*this x value).shift(xy)
            Mdn2d internal_copyMultiplyAndShift(int value, const Coord& shiftXY) const;

};

// Arithmetic binary operators
inline Mdn2d operator+(Mdn2d lhs, const Mdn2d& rhs) { return lhs += rhs; }
inline Mdn2d operator-(Mdn2d lhs, const Mdn2d& rhs) { return lhs -= rhs; }
inline Mdn2d operator*(Mdn2d lhs, const Mdn2d& rhs) { return lhs *= rhs; }
inline Mdn2d operator/(Mdn2d lhs, const Mdn2d& rhs) { return lhs /= rhs; }

inline Mdn2d operator*(Mdn2d lhs, int scalar) { return lhs *= scalar; }

} // end namespace mdn
