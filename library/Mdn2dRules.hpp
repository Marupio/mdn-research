#pragma once

#include "Carryover.hpp"
#include "GlobalConfig.hpp"
#include "Mdn2dBase.hpp"

namespace mdn {

// Basic rules layer of 2d multi dimensional numbers, establishes:
//  * carryover rules
//  * polymorphism
//  * equality operator, now available with polymorphism
//  * digit shifting
//  * transposing, because why not?
class MDN_API Mdn2dRules : public Mdn2dBase {

protected:

    // Polymorphic nodes and the last time it was calculated
    mutable CoordSet m_polymorphicNodes;
    mutable long long m_polymorphicNodes_event;


public:

    // Check for the type of carryover, given the pivot digit and x and y axial digits
    static Carryover static_checkCarryover(Digit p, Digit x, Digit y, Digit base);


    // *** Constructors

        // Construct null
        Mdn2dRules(std::string nameIn="");

        // Construct from a configuration
        Mdn2dRules(Mdn2dConfig config, std::string nameIn="");


        // *** Rule of five

            // Copy constructor
            Mdn2dRules(const Mdn2dRules& other, std::string nameIn="");

            // Assignment operator
            Mdn2dRules& operator=(const Mdn2dRules& other);

            // Move operator
            Mdn2dRules(Mdn2dRules&& other, std::string nameIn="") noexcept;

            // Move assignment operator
            Mdn2dRules& operator=(Mdn2dRules&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2dRules() {}


    // *** Member Functions

        // *** Config

            // Change the config - can lead to any of the Mdn2dConfigImpact effects
            // Overriding virtual fn from base for access to carryover functionality
            protected: void locked_setConfig(Mdn2dConfig newConfig) override; public:


        // *** Transformations

            // Return the Carryover type (enumareation) of xy
            Carryover checkCarryover(const Coord& xy) const;
            protected: Carryover locked_checkCarryover(const Coord& xy) const; public:

            // Perform a manual carry-over at coordinate (x, y), returns affected coords as a set
            CoordSet carryover(const Coord& xy);
            protected:
                // Internal function takes arg 'carry' to be added to root of carryover, xy
                CoordSet locked_carryover(const Coord& xy, int carry = 0);
            public:

            // Given set of suspicious coords, check if any need carryovers, and if so, do them
            //  Returns set of coordinates that actually have changed
            CoordSet carryoverCleanup(const CoordSet& coords);
            protected: CoordSet locked_carryoverCleanup(const CoordSet& coords); public:

            // Given all the non-zero coords, check if any need carryovers, and if so, do them
            //  Returns set of coordinates that actually have changed
            CoordSet carryoverCleanupAll();
            protected: CoordSet locked_carryoverCleanupAll(); public:

            // General shift interface
            void shift(int xDigits, int yDigits);
            void shift(const Coord& xy);
            protected:
                void locked_shift(const Coord& xy);
                void locked_shift(int xDigits, int yDigits);
            public:

            // Shift all digits in a direction (R=+X, L=-X, U=+Y, D=-Y)
            void shiftRight(int nDigits);
            protected: void locked_shiftRight(int nDigits); public:
            void shiftLeft(int nDigits);
            protected: void locked_shiftLeft(int nDigits); public:
            void shiftUp(int nDigits);
            protected: void locked_shiftUp(int nDigits); public:
            void shiftDown(int nDigits);
            protected: void locked_shiftDown(int nDigits); public:

            // Swap X and Y
            void transpose();
            protected: void locked_transpose(); public:

            // Returns polymorphic nodes, demand driven
            const CoordSet& getPolymorphicNodes() const;
            protected: const CoordSet& locked_getPolymorphicNodes() const; public:


protected:

    // *** Protected Member Functions

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

