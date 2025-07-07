#pragma once

#include <map>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "CoordSet.h"
#include "Mdn2dConfig.h"
#include "PrecisionStatus.h"

namespace mdn {

class Mdn2d;

// Digit layer of 2d multi dimensional numbers, establishes:
//  * 2-dimensional digit representation
//  * sparse storage and addressing
//  * bounds metadata
class Mdn2dBase {

protected:

    // Configuration settings for this Mdn2dBase
    Mdn2dConfig m_config;

    // Thread safety
    mutable std::shared_mutex m_mutex;


    // *** Data & addressing

    // Sparse coordinate-to-digit mapping
    std::unordered_map<Coord, Digit> m_raw;

    // Addressing
    mutable std::map<int, CoordSet> m_xIndex;
    mutable std::map<int, CoordSet> m_yIndex;

    // Full index
    mutable CoordSet m_index;

    // *** Metadata

        // Bounding box for non-zero digits
        mutable Coord m_boundsMin;
        mutable Coord m_boundsMax;

        // Event number for tracking derived, demand-driven data
        long m_event;


public:

    // *** Typedefs

    using WritableLock = std::unique_lock<std::shared_mutex>;
    using ReadOnlyLock = std::shared_lock<std::shared_mutex>;


    // *** Create a fully-realised Mdn2d instance
    static Mdn2d NewInstance(Mdn2dConfig config=Mdn2dConfig::static_defaultConfig());
    static Mdn2d Duplicate(const Mdn2d& other);


    // *** Constructors

        // Construct from a configuration, or default
        Mdn2dBase();

        Mdn2dBase(Mdn2dConfig config);


        // *** Rule of five

            // Copy constructor
            Mdn2dBase(const Mdn2dBase& other);

            // Assignment operator
            Mdn2dBase& operator=(const Mdn2dBase& other);

            // Move operator
            Mdn2dBase(Mdn2dBase&& other) noexcept;

            // Move assignment operator
            Mdn2dBase& operator=(Mdn2dBase&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2dBase() {}


    // *** Member Functions

        // *** Getters

            // Retrieves the value at coordinate (x, y), or 0 if not present.
            Digit getValue(const Coord& xy) const;
            protected: Digit locked_getValue(const Coord& xy) const; public:

            // Assembles the row at the given y index value, spanning the x bounds of full MDN
            std::vector<Digit> getRow(int y) const;
            protected: std::vector<Digit> locked_getRow(int y) const; public:

            // Assembles the row at the given y index value, spanning the x bounds of full MDN
            void getRow(int y, std::vector<Digit>& digits) const;
            protected: void locked_getRow(int y, std::vector<Digit>& digits) const; public:

            // Assembles the col at the given x index value, spanning the y bounds of full MDN
            std::vector<Digit> getCol(int x) const;
            protected: std::vector<Digit> locked_getCol(int x) const; public:

            // Assembles the col at the given x index value, spanning the y bounds of full MDN
            void getCol(int x, std::vector<Digit>& digits) const;
            protected: void locked_getCol(int x, std::vector<Digit>& digits) const; public:


        // *** Setters

            // Clears all digits in the MDN.
            void clear();
            protected: void locked_clear(); public:

            // Set the value at xy to zero
            //  Returns true if carryover status might change:
            //      * Value goes from zero to non-zero
            //      * Value changes sign
            bool setToZero(const Coord& xy);
            protected: bool locked_setToZero(const Coord& xy); public:

            // Set the value at coords to zero, returns subset containing those whose values changed
            CoordSet setToZero(const CoordSet& coords);
            protected: CoordSet locked_setToZero(const CoordSet& purgeSet); public:

            // Changes the value at xy
            //  Returns true if carryover status might change:
            //      * Value goes from zero to non-zero
            //      * Value changes sign
            bool setValue(const Coord& xy, Digit value);
            bool setValue(const Coord& xy, int value);
            bool setValue(const Coord& xy, long value);
            bool setValue(const Coord& xy, long long value);
            protected:
                bool locked_setValue(const Coord& xy, Digit value);
                bool locked_setValue(const Coord& xy, int value);
                bool locked_setValue(const Coord& xy, long value);
                bool locked_setValue(const Coord& xy, long long value);
            public:


        // *** Conversion / display

            // Converts the MDN to a human-readable string.
            std::string toString() const;
            protected: std::string locked_toString() const; public:

            // Converts the MDN to an array of strings, representing rows
            std::vector<std::string> toStringRows() const;
            protected: std::vector<std::string> locked_toStringRows() const; public:

            // Converts the MDN to an array of strings, representing columns
            std::vector<std::string> toStringCols() const;
            protected: std::vector<std::string> locked_toStringCols() const; public:


        // *** Transformations

            // Clears all addressing and bounds data, and rebuilds it
            void rebuildMetadata() const;
            protected: void locked_rebuildMetadata() const; public:

            // Returns true if m_boundsMin and m_boundsMax are both valid, finite numbers
            bool hasBounds() const;
            protected: bool locked_hasBounds() const; public:

            // Retuns bounds of non zero entries in m_raw
            std::pair<Coord, Coord> getBounds() const;
            protected: std::pair<Coord, Coord> locked_getBounds() const; public:


        // *** Other functionality

            int getPrecision() const;
            protected: int locked_getPrecision() const; public:

            // Change the setting for m_precision, returns the number of dropped digits
            int setPrecision(int newMaxSpan);
            protected: int locked_setPrecision(int newMaxSpan); public:

            // Query the precision status of xy to ensure precision is not exceeded
            // Returns:
            //  * PrecisionStatus::Below  - above precision window
            //  * PrecisionStatus::Inside - within precision window
            //  * PrecisionStatus::Above  - below precision window
            PrecisionStatus checkPrecisionWindow(const Coord& xy) const;
            protected: PrecisionStatus locked_checkPrecisionWindow(const Coord& xy) const; public:

            // Clear all derived data, update event number
            void modified();


protected:

    // *** Private Member Functions

        // Lock m_mutex for writeable reasons (unique_lock)
        WritableLock lockWriteable() const;

        // Lock m_mutex for read-only reasons (shareable_lock)
        ReadOnlyLock lockReadOnly() const;

        // Ensure the supplied argument 'that' is not the same as *this, throws if not
        void assertNotSelf(Mdn2dBase& that, const std::string& description) const;


        // *** Internal functions
        // All these functions require the mutex to be locked first before calling

            // Clears all addressing and bounds data
            virtual void internal_clearMetadata() const;

            // Sets value at xy without checking in range of base
            //  Returns true if carryover status might change:
            //      * Value goes from zero to non-zero
            //      * Value changes sign
            bool internal_setValueRaw(const Coord& xy, Digit value);

            // Add xy as a non-zero number position to the metadata
            void internal_insertAddress(const Coord& xy) const;

            // Checks if value is within +/- (m_base-1).  If not, throws or returns false.
            template <class Type>
            void internal_checkDigit(const Coord& xy, Type value) const {
                Digit dbase = m_config.dbase();
                if (value >= dbase || value <= -dbase) {
                    throw OutOfRange(xy, static_cast<int>(value), dbase);
                }
            }

            // Purge any digits that exceed the precision window, return the number of purged digits
            int internal_purgeExcessDigits();

            // Update the m_boundsMin and m_boundsMax variables based on the current values
            void internal_updateBounds();

};

} // end namespace mdn
