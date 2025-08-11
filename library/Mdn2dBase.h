#pragma once

#include <map>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "CoordSet.h"
#include "GlobalConfig.h"
#include "Mdn2dConfig.h"
#include "Mdn2dConfigImpact.h"
#include "PrecisionStatus.h"
#include "Rect.h"

namespace mdn {

class Mdn2d;
class Project;

// Digit layer of 2d multi dimensional numbers, establishes:
//  * 2-dimensional digit representation
//  * sparse storage and addressing
//  * bounds metadata
class MDN_API Mdn2dBase {

    // *** Friends
    friend class Project;


protected:

    // Static name generation

    // Thread safety at static layer
    static std::shared_mutex m_static_mutex;

    // Used to generate the next defaulted name
    static int m_nextNameSeed;

    // Creates a new Mdn2d name, acquires static lock first
    static std::string static_generateNextName();

    // Creates a new Mdn2d name, assumes lock already acquired
    static std::string locked_generateNextName();

    // Create a 'copy' name from given nameIn (e.g. nameIn_copy0), acquires static lock first
    static std::string static_generateCopyName(const std::string& nameIn);

    // Create a 'copy' name from given nameIn (e.g. nameIn_copy0), assumes lock already acquired
    static std::string locked_generateCopyName(const std::string& nameIn);


    // Configuration settings for this Mdn2dBase
    Mdn2dConfig m_config;

    // Thread safety
    mutable std::shared_mutex m_mutex;


    // *** Data & addressing

    // Name of this number
    std::string m_name;

    // Sparse coordinate-to-digit mapping
    std::unordered_map<Coord, Digit> m_raw;

    // Addressing
    mutable std::map<int, CoordSet> m_xIndex;
    mutable std::map<int, CoordSet> m_yIndex;

    // Full index
    mutable CoordSet m_index;

    // *** Metadata

        // Bounding box for non-zero digits
        mutable Rect m_bounds;

        // When true, increment m_event once operation is complete
        bool m_modified;

        // Event number for tracking derived, demand-driven data
        long long m_event;


public:

    // *** Typedefs

    using WritableLock = std::unique_lock<std::shared_mutex>;
    using ReadOnlyLock = std::shared_lock<std::shared_mutex>;


    // *** Static functions


    // Create a fully-realised Mdn2d instance, accessible from downstream layers
    static Mdn2d NewInstance(
        Mdn2dConfig config=Mdn2dConfig::static_defaultConfig(),
        std::string nameIn=""
    );
    static Mdn2d Duplicate(const Mdn2d& other, std::string nameIn="");


    // *** Constructors

        // Construct from a configuration, or default
        Mdn2dBase(std::string nameIn="");

        Mdn2dBase(Mdn2dConfig config, std::string nameIn="");


        // *** Rule of five

            // Copy constructor
            Mdn2dBase(const Mdn2dBase& other, std::string nameIn="");

            // Assignment operator
            Mdn2dBase& operator=(const Mdn2dBase& other);

            // Move operator
            Mdn2dBase(Mdn2dBase&& other, std::string nameIn="") noexcept;

            // Move assignment operator
            Mdn2dBase& operator=(Mdn2dBase&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2dBase() {}


    // *** Member Functions

        // *** Config

            // Return the Mdn2dConfig for this number
            const Mdn2dConfig& getConfig() const;
            protected: const Mdn2dConfig& locked_getConfig() const; public:

            // Assess the impact of changing the config to the supplied newConfig
            Mdn2dConfigImpact assessConfigChange(const Mdn2dConfig& newConfig);
            protected: Mdn2dConfigImpact locked_assessConfigChange(const Mdn2dConfig& newConfig); public:

            // Change the config - can lead to any of the Mdn2dConfigImpact effects
            void setConfig(Mdn2dConfig& newConfig);
            // Locked version *copies* config - everyone has their own copy
            protected: virtual void locked_setConfig(Mdn2dConfig newConfig); public:


        // *** Identity

            // Return name
            const std::string& getName() const;
            protected: const std::string& locked_getName() const; public:

            // Set this number's 'name', deferring to framework for approval
            void setName(const std::string& nameIn);
            protected: void locked_setName(const std::string& nameIn); public:


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

            // Returns true if m_bounds are both valid, finite numbers
            bool hasBounds() const;
            protected: bool locked_hasBounds() const; public:

            // Retuns bounds of non zero entries in m_raw
            const Rect& bounds() const;
            protected: const Rect& locked_getBounds() const; public:


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

            // Set the m_modified flag to trigger housekeeping with derived data when operations are
            // complete

            // Use when the data has changed, but operation may not yet be complete
            void internal_modified();

            // Use when the operation is complete, but the data may not have changed
            void internal_operationComplete();

            // Use when the data has changed AND the operation is complete, performs housekeeping:
            //  * clear derived data
            //  * increment m_event flag
            void internal_modifiedAndComplete();


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
                Digit baseDigit = m_config.baseDigit();

                if (Log_Showing_Debug4) {
                    Log_N_Debug4(
                        "Checking value " << static_cast<int>(value) << " against base "
                        << int(baseDigit)
                    );
                }
                if (value >= baseDigit || value <= -baseDigit) {
                    OutOfRange err(xy, static_cast<int>(value), baseDigit);
                    Log_N_Error(err.what());
                    throw err;
                }
            }

            // Purge any digits that exceed the precision window, return the number of purged digits
            int internal_purgeExcessDigits();

            // Update the m_bounds based on the current values
            void internal_updateBounds();

};

} // end namespace mdn
