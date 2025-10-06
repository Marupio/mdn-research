#pragma once

#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "CoordTypes.hpp"
#include "GlobalConfig.hpp"
#include "LockTracker.hpp"
#include "Mdn2dConfig.hpp"
#include "Mdn2dConfigImpact.hpp"
#include "MdnObserver.hpp"
#include "PrecisionStatus.hpp"
#include "Rect.hpp"
#include "TextOptions.hpp"

namespace mdn {

class Mdn2d;
struct TextWriteOptions;

// Digit layer of 2d multi dimensional numbers, establishes:
//  * 2-dimensional digit representation
//  * sparse storage and addressing
//  * bounds metadata
class MDN_API Mdn2dBase {

    // *** Friends

    // This guy handles input and output, mostly text-related
    friend class Mdn2dIO;


protected:

    // *** Local variables

    // Configuration settings for this Mdn2dBase
    Mdn2dConfig m_config;

    // Thread safety
    mutable std::shared_mutex m_mutex;

    // Counts active read/write locks taken via lockReadOnly/lockWriteable
    mutable mdn::LockTracker m_lockTracker;

    // Empty CoordSet for functions that return references, but no reference exists for null
    const CoordSet m_nullCoordSet;

    // *** Data & addressing

    // Name of this number
    std::string m_name;

    // Sparse coordinate-to-digit mapping
    std::unordered_map<Coord, Digit> m_raw;

    // Addressing
    //  m_xIndex[x value] = CoordSet of non-zeroes in this column
    //  m_yIndex[y value] = Coordset of non-zeroes in this row
    mutable std::map<int, CoordSet> m_xIndex;
    mutable std::map<int, CoordSet> m_yIndex;

    // Full index
    mutable CoordSet m_index;

    // Observers
    mutable std::unordered_map<int, MdnObserver*> m_observers;


    // *** Metadata

        // Bounding box for non-zero digits
        mutable Rect m_bounds;

        // When true, increment m_event once operation is complete
        bool m_modified;

        // Event number for tracking derived, demand-driven data
        long long m_event;

        // Coordinates that have changed during the current operation (only applicable in overwrite
        //  mode)
        mutable CoordSet m_affected;


public:

    // *** Public data types

    using WritableLock = mdn::LockTracker::WritableLock;
    using ReadOnlyLock = mdn::LockTracker::ReadOnlyLock;


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
            protected: Mdn2dBase& locked_operatorEquals(const Mdn2dBase& other); public:

            // Move operator
            Mdn2dBase(Mdn2dBase&& other) noexcept;

            // Move assignment operator
            Mdn2dBase& operator=(Mdn2dBase&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2dBase();


    // *** Member Functions

        // *** Config

            // Return the Mdn2dConfig for this number
            const Mdn2dConfig& config() const;
            protected: const Mdn2dConfig& locked_config() const; public:

            // Assess the impact of changing the config to the supplied newConfig
            Mdn2dConfigImpact assessConfigChange(const Mdn2dConfig& newConfig) const;
            protected:
                Mdn2dConfigImpact locked_assessConfigChange(const Mdn2dConfig& newConfig) const;
            public:

            // Change the config - can lead to any of the Mdn2dConfigImpact effects
            void setConfig(const Mdn2dConfig& newConfig);
            // Locked version *copies* config - everyone has their own copy
            protected: virtual void locked_setConfig(const Mdn2dConfig& newConfig); public:


        // *** Observers

            // Register a new observer
            void registerObserver(MdnObserver* obs) const;
            protected: void locked_registerObserver(MdnObserver* obs) const; public:

            // Unregister the owner (does not delete observer)
            void unregisterObserver(MdnObserver* obs) const;
            protected: void locked_unregisterObserver(MdnObserver* obs) const; public:


        // *** Identity

            // Return name
            const std::string& name() const;
            protected: const std::string& locked_name() const; public:

            // Set this number's 'name', deferring to framework for approval
            std::string setName(const std::string& nameIn);
            protected: std::string locked_setName(const std::string& nameIn); public:


        // *** Queries

            // Returns true if xy contains a non-zero digit
            bool nonZero(const Coord& xy) const;
            protected: bool locked_nonZero(const Coord& xy) const; public:

            // Returns non-zero coordinates along the given row
            const CoordSet& nonZeroOnRow(const Coord& xy) const;
            protected: const CoordSet& locked_nonZeroOnRow(const Coord& xy) const; public:

            // Returns non-zero coordinates along the given row
            const CoordSet& nonZeroOnCol(const Coord& xy) const;
            protected: const CoordSet& locked_nonZeroOnCol(const Coord& xy) const; public:


        // *** Navigation

            // Give the Coord position after jumping from xy in direction cd
            //  * if xy is non-zero, finds first zero Coord in cd direction
            //  * if xy is zero, finds first non-zero Coord in cd direction
            //  Returns the position that is on the non-zero Coord at the boundary between zero and
            //  non-zero.  This is the behaviour of the cursor in spreadsheet apps, when moved with
            //  [ctrl]+[direction].
            Coord jump(const Coord& xy, CardinalDirection cd) const;
            protected: Coord locked_jump(const Coord& xy, CardinalDirection cd) const; public:


        // *** Value Getters

            // Retrieves the value at coordinate (x, y), or 0 if not present.
            Digit getValue(const Coord& xy) const;
            protected: Digit locked_getValue(const Coord& xy) const; public:

            // Get overall value - sum all row signed values
            long double getTotalValue() const;
            protected: long double locked_getTotalValue() const; public:

            // Get overall value - sum all row absolute magnitudes
            long double getTotalMagnitude() const;
            protected: long double locked_getTotalMagnitude() const; public:


        // *** Row Getters

            long double getRowValue(const Coord& xy) const;
            protected: long double locked_getRowValue(const Coord& xy) const; public:

            // If all the rows calculated their total value, account for negative digits, this
            //  function finds the one with the largest absolute magnitude (positive or negative)
            //  xy gets set to the position of the head of the row (furthest right, x+ digit), and
            //  val contains the value (signed).  Tie breaker - prefers higher y values.
            //  Returns false if no non-zeroes.
            bool getRowMagMax(Coord& xy, long double& val) const;
            protected: bool locked_getRowMagMax(Coord& xy, long double& val) const; public:

            // Assembles the row at the given y index value, spanning the x bounds of full MDN
            VecDigit getRow(int y) const;
            protected: VecDigit locked_getRow(int y) const; public:

            // Assembles a full row of digits at the y-indexed row
            void getRow(int y, VecDigit& out) const;
            protected: void locked_getRow(int y, VecDigit& out) const; public:

            // Assembles a full row of digits, xy coordinate gives the row, x component is ignored
            void getRow(const Coord& xy, VecDigit& out) const;
            protected: void locked_getRow(const Coord& xy, VecDigit& out) const; public:

            // Write out a continuous range for a row, starting at xy, and spanning width digits
            // out.size() is width
            void getRow(const Coord& xy, int width, VecDigit& out) const;
            protected:
            void locked_getRow(const Coord& xy, int width, VecDigit& out) const;
            public:

            // Write out a continuous range for a rows covering an area from Coord c0 to Coord c1
            //  Returns bounds of output digits
            Rect getAreaRows(VecVecDigit& out) const;

            // Write out a continuous range for a rows covering an area from Coord c0 to Coord c1
            //  Returns bounds of output digits
            void getAreaRows(const Rect& window, VecVecDigit& out) const;
            protected:
            void locked_getAreaRows(const Rect& window, VecVecDigit& out) const;
            public:


        // *** Column Getters

            long double getColValue(const Coord& xy) const;
            protected: long double locked_getColValue(const Coord& xy) const; public:

            // If all the columns calculated their total value, account for negative digits, this
            //  function finds the one with the largest absolute magnitude (positive or negative)
            //  xy gets set to the position of the head of the column (furthest right, x+ digit),
            //  and val contains the value (signed).  Tie breaker - prefers higher x values.
            //  Returns false if no non-zeroes.
            bool getColMagMax(Coord& xy, long double& val) const;
            protected: bool locked_getColMagMax(Coord& xy, long double& val) const; public:

            // Assembles the col at the given x index value, spanning the y bounds of full MDN
            VecDigit getCol(int x) const;
            protected: VecDigit locked_getCol(int x) const; public:

            // Assembles the col at the given x index value, spanning the y bounds of full MDN
            void getCol(int x, VecDigit& out) const;
            protected: void locked_getCol(int x, VecDigit& out) const; public:

            // Write out a continuous range for a column, from xy, at height.
            void getCol(const Coord& xy, int height, VecDigit& out) const;
            protected:
            void locked_getCol(const Coord& xy, int height, VecDigit& out) const;
            public:


            // Return the set of non-zero coordinates inside 'window'
            CoordSet getNonZeroes(const Rect& window) const;
            protected: CoordSet locked_getNonZeroes(const Rect& window) const; public:


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

            // Set to zero all digits within the given window
            CoordSet setToZero(const Rect& window);
            protected: CoordSet locked_setToZero(const Rect& window); public:

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

            // Write a contiguous row back in one call. Interprets 0 as "setToZero".  y is the row,
            //  x0 is the column.  Why is it backwards?  Shouldn't it be x,y?  Well, I mean, we also
            //  have getRow with y,x0,x1.
            void setRow(const Coord& xy, const VecDigit& row);
            protected:
                void locked_setRow(const Coord& xy, const VecDigit& row);
            public:


        // *** Mdn2dIO functionality hooks

        // Stream ops delegate to Mdn2dIO (defined in free operators in Mdn2dIO.h)

        // Convenience: return lines for pretty/utility output via IO options
        // (declared here; defined in Mdn2dBase.cpp to avoid circular includes)
        std::vector<std::string> toStringRows() const;
        std::vector<std::string> toStringRows(const TextWriteOptions& opt) const;
        std::vector<std::string> toStringCols() const;
        std::vector<std::string> toStringCols(const TextWriteOptions& opt) const;


        // *** Conversion to string
        //  These functions convert the Mdn2d or portions of it to human-readable ascii text.  Some
        //  common parameters include:
        //      * hDelim: horizontal delimiter, for use at the 0 digit line, e.g. ─
        //      * vDelim: vertical delimiter, for use at the 0 digit vertical line, e.g. │
        //      * xDelim: cross delimiter, for use at the origin, where the two lines meet, e.g. ┼
        //      * enableAlphaNumerics: when true, converts all digits to a single alphanumeric
        //          character, 0-9, a, b, c ..., see Tools::digitToAlpha
        //      * window: the rectangular area to include in the view, padding with zeros when it
        //          extends beyond the bounds of this number
        //
        //  Possible user-facing settings, and their associated full parameter list
        //      * Delimiter - Comma / Tab / Space
        //      * WideNegatives
        //      * AxesVisible
        //      * Alphanumeric
        //
        //  Standard use cases:
        //      * pretty (human readable, not too usefulin calculation)
        //              │─a─3 2
        //              │ 6 d 8
        //              ┼──────
        //          Delimiter = Space
        //              no delimiter between numbers, apart from sign: positive ' ', negative '─'
        //          WideNegatives = true
        //              negatives are box art horizontal lines, not just '-'
        //          AxesVisible = true
        //              axes are visible, use default box art
        //          Alphanumeric = true
        //              Alphanumerics used for digits that exceed 9, e.g. 0-9,a,b,c,...
        //      * utility (easier to use for calculation, not as pretty)
        //              -10,-3,2    or -10\t-3\t2
        //              6,13,8         6\t13\t8
        //          Delimiter = Comma or Tab
        //              standard CSV or TSV format, digits may visually get out-of-alignment
        //          AxesVisible = false
        //              no axes are visible, position inferred from bounds
        //          Alphanumeric = false
        //              no alphanumerics, single digits can be multiple characters

            // Write out number using 'pretty' settings above
            std::vector<std::string> saveTextPrettyRows(
                bool wideNegatives, bool alphanumeric
            ) const;
            std::vector<std::string> saveTextPrettyRows(
                Rect& window, bool wideNegatives, bool alphanumeric
            ) const;
            protected:
                std::vector<std::string> locked_saveTextPrettyRows(
                    Rect& window, bool wideNegatives, bool alphanumeric
                ) const;
            public:

            // Write out number using 'utility' settings above
            std::vector<std::string> saveTextUtilityRows(CommaTabSpace delim) const;
            std::vector<std::string> saveTextUtilityRows(Rect& window, CommaTabSpace delim) const;
            protected:
                std::vector<std::string> locked_saveTextUtilityRows(
                    Rect& window, CommaTabSpace delim
                ) const;
            public:


        // *** Save / load

            // Save in text format
            void saveTextPretty(std::ostream& os, bool wideNegatives, bool alphanumeric) const;
            protected: void locked_saveTextPretty(
                std::ostream& os,
                bool wideNegatives,
                bool alphanumeric
            ) const; public:
            void saveTextUtility(std::ostream& os, CommaTabSpace delim) const;
            protected:
                void locked_saveTextUtility(std::ostream& os, CommaTabSpace delim) const;
            public:

            // Load in text format - Automatically detects pretty / utility format
            void loadText(std::istream& is);
            protected: void locked_loadText(std::istream& is); public:

            // Save in binary format
            void saveBinary(std::ostream& os) const;
            protected: void locked_saveBinary(std::ostream& os) const; public:

            // Load in binary format
            void loadBinary(std::istream& is);
            protected: void locked_loadBinary(std::istream& is); public:


        // *** Transformations

            // Clears all addressing and bounds data, and rebuilds it
            void rebuildMetadata() const;
            protected: void locked_rebuildMetadata() const; public:

            // Returns true if m_bounds are both valid, finite numbers
            bool hasBounds() const;
            protected: bool locked_hasBounds() const; public:

            // Retuns bounds of non zero entries in m_raw
            const Rect& bounds() const;
            protected: const Rect& locked_bounds() const; public:


        // *** Direct access to underlying data

        const std::unordered_map<Coord, Digit>&  data_raw();
        const std::unordered_map<Coord, Digit>&  locked_data_raw();
        const std::map<int, CoordSet>&  data_xIndex();
        const std::map<int, CoordSet>&  locked_data_xIndex();
        const std::map<int, CoordSet>&  data_yIndex();
        const std::map<int, CoordSet>&  locked_data_yIndex();
        const CoordSet&  data_index();
        const CoordSet&  locked_data_index();
        const std::unordered_map<int, MdnObserver*>&  data_observers();
        const std::unordered_map<int, MdnObserver*>&  locked_data_observers();


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

            protected:
                // Helper - given a order of magnitude, returns base^orderOfMagnitude
                long double internal_baseFactor(long double base, int orderOfMagnitude) const;
            public:

            // Use when the data has changed, but operation may not yet be complete
            void internal_modified();

            // Use when the operation is complete, but the data may not have changed
            void internal_operationComplete();

            // Use when the data has changed AND the operation is complete, performs housekeeping:
            //  * clear derived data
            //  * increment m_event flag
            void internal_modifiedAndComplete();

            // Access mutex counter data
            int activeWriterCount() const noexcept {
                return m_lockTracker.activeWriterCount();
            }
            int activeReaderCount() const noexcept {
                return m_lockTracker.activeReaderCount();
            }


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

            // Fraxis x: place the given value at xy.y(), whose x position is driven by the decimal
            // Fraxis y: place the given value at xy.x(), whose y position is driven by the decimal
            CoordSet internal_emplace(const Coord& xy, long double val, Fraxis fraxis);

            // Checks if value is within +/- (m_base-1).  If not, throws or returns false.
            template <class Type>
            void internal_checkDigit(const Coord& xy, Type value) const {
                Digit baseDigit = m_config.baseDigit();

                If_Log_Showing_Debug4 (
                    Log_N_Debug4(
                        "Checking value " << static_cast<int>(value) << " against base "
                        << int(baseDigit)
                    );
                );
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
