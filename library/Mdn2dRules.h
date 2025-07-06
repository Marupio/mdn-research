#pragma once

#include "Carryover.h"
#include "Mdn2dBase.h"

namespace mdn {

// Basic rules layer of 2d multi dimensional numbers, establishes:
//  * carryover rules
//  * polymorphism
//  * equality operator, now available with polymorphism
//  * digit shifting
//  * transposing, because why not?
class Mdn2dRules : public Mdn2dBase {

    // Polymorphic nodes and the last time it was calculated
    mutable std::unordered_set<Coord> m_polymorphicNodes;
    mutable long m_polymorphicNodes_event;


public:

    // Check for the type of carryover, given the pivot digit and x and y axial digits
    static Carryover static_checkCarryover(Digit p, Digit x, Digit y, Digit base);


    // *** Constructors

        Mdn2dRules(Mdn2dConfig config=Mdn2dConfig::static_defaultConfig());


        // *** Rule of five

            // Copy constructor
            Mdn2dRules(const Mdn2dRules& other);

            // Assignment operator
            Mdn2dRules& operator=(const Mdn2dRules& other);

            // Move operator
            Mdn2dRules(Mdn2dRules&& other) noexcept;

            // Move assignment operator
            Mdn2dRules& operator=(Mdn2dRules&& other) noexcept;


    // *** Member Functions

        // *** Transformations

            // Return the Carryover type (enumareation) of xy
            Carryover checkCarryover(const Coord& xy) const;
            private: Carryover locked_checkCarryover(const Coord& xy) const; public:

            // Perform a manual carry-over at coordinate (x, y), returns affected coords as a set
            std::unordered_set<Coord> carryover(const Coord& xy);
            private:
                // Internal function takes arg 'carry' to be added to root of carryover, xy
                std::unordered_set<Coord> locked_carryover(const Coord& xy, int carry = 0);
            public:

            // Given set of suspicious coords, check if any need carryovers, and if so, do them
            //  Returns set of coordinates that actually have changed
            std::unordered_set<Coord> carryoverCleanup(const std::unordered_set<Coord>& coords);
            private:
                std::unordered_set<Coord> locked_carryoverCleanup(
                    const std::unordered_set<Coord>& coords
                );
            public:

            // Given all the non-zero coords, check if any need carryovers, and if so, do them
            //  Returns set of coordinates that actually have changed
            std::unordered_set<Coord> carryoverCleanupAll();
            private: std::unordered_set<Coord> locked_carryoverCleanupAll(); public:

            // General shift interface
            void shift(int xDigits, int yDigits);
            void shift(const Coord& xy);
            private:
                void locked_shift(const Coord& xy);
                void locked_shift(int xDigits, int yDigits);
            public:

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

            // Returns polymorphic nodes, demand driven
            const std::unordered_set<Coord>& getPolymorphicNodes() const;
            private: const std::unordered_set<Coord>& locked_getPolymorphicNodes() const; public:

            // // Take on polymorphism state x0 (all p-nodes are negative)
            // void polymorphism_x0();
            // private: void locked_polymorphism_x0(); public:
            //
            // // Take on polymorphism state y0 (all p-nodes are positive)
            // void polymorphism_y0();
            // private: void locked_polymorphism_y0(); public:


        // *** Other functionality


        // Equality comparison
        //  The rules layer brings carryovers, allowing us to find equivalence between different
        //  states of polymorphism.  But for now, equivalence only works with a default sign
        //  convention (Mdn2dConfig)
        bool operator==(const Mdn2dRules& rhs) const;

        // Inequality comparison.
        bool operator!=(const Mdn2dRules& rhs) const;


protected:

    // *** Protected Member Functions

            // New scan for carryovers, perform all 'Required' carryovers, enforce the default sign
            // convention
            void mdn::Mdn2dRules::internal_updatePolymorphism();

            // Scan for carryovers, perform all 'Required' carryovers, return list of optional c/o
            void internal_polymorphicScanAndFix();

            // Find all the 'Optional' carryovers, create m_polymorphicNodes data
            void internal_polymorphicScan() const;

            // Perform a blind single carryover at xy without any checks
            void internal_oneCarryover(const Coord& xy);

            // Perform a carryover during math operations (xy magnitude must exceed base)
            void internal_ncarryover(const Coord& xy);

            // Clear all derived data, including polymorphism-related data
            virtual void internal_clearMetadata() const override;

};

} // end namespace mdn

