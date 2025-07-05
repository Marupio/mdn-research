    // Carryovers
    //  There are carryovers that must be performed when a digit holds a value greater than base (or
    //  less that negative base).  This carryover function is unrelated to that.  In MDN theory, a
    //  number can have different equivalent states, reached by performing carryover operations.
    //  The carryover operations this function performs act on an MDN that has reached a valid
    //  resting state.
    //
    //  A  3    |   4
    //     0 -3 | -10 -2
    //
    //  B  0    |   1
    //     4 0  |  -6 1
    //
    //  C  0    |   1
    //     4 3  |  -6 4
    //
    //  D  0    |   1
    //     4 -3 |  -6 -2
    //
    //  E -2    |  -1
    //     4  3 |  -6  4
    //
    //  F  2    |   3
    //     4  3 |  -6  4
    //
    //  G -2    |  -1
    //     4 -3 |  -6 -2
    //
    //    |  p  |  x  |  y  | comment
    //    +-----+-----+-----+---------------------
    // A  |  0  |  ?  |  ?  | not possible

    // G  |  +  |  -  |  -  | forced
    // D  |  +  |  -  |  0  | ambiguous
    // E  |  +  |  +  |  -  | ambiguous
    // B  |  +  |  0  |  0  | invalid
    // C  |  +  |  +  |  0  | invalid
    // F  |  +  |  +  |  +  | invalid

    // M  |  -  |  +  |  +  | forced
    // J  |  -  |  +  |  0  | ambiguous
    // K  |  -  |  -  |  +  | ambiguous
    // H  |  -  |  0  |  0  | invalid
    // I  |  -  |  -  |  0  | invalid
    // L  |  -  |  -  |  -  | invalid


enum class CarryOver {
    Unknown,
    Invalid,
    Ambiguous,
    Forced
}

bool pos(int t) { return t > 0; }
bool neg(int t) { return t < 0; }
bool zero(int t) { return t == 0; }

CarryOver pseudo_code(int p, int x, int y) {
    if (pos(p)) {
        if (neg(x) && neg(y)) {
            return CarryOver::Forced;
        }
        else if (neg(x) || neg(x)) {
            return CarryOver::Ambiguous;
        }
    }
    else if (neg(p)) {
        if (pos(x) && pos(y)) {
            return CarryOver::Forced;
        }
        else if (pos(x) || pos(y)) {
            return CarryOver::Ambiguous;
        }
    }
    return CarryOver::Invalid;
}


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
