#pragma once

// Carryovers
//  Some carryovers are required, such as when a digit magnitude exceeds its base.  Other times a
//  carryover is optional - some MDNs have multiple valid states.  Switching between these states is
//  achieved using carryovers.  And some carryovers bring a number to an invalid state and should
//  not be performed.
//
//  In general, there are three types of carryovers:
//
//      * Invalid:    Valid MDN --> carryover --> Invalid MDN
//      * Optional:   Valid MDN --> carryover --> Valid MDN
//      * Required: Invalid MDN --> carryover --> Valid MDN
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
//
// G  |  +  |  -  |  -  | required
// D  |  +  |  -  |  0  | optional
// E  |  +  |  +  |  -  | optional
// B  |  +  |  0  |  0  | invalid
// C  |  +  |  +  |  0  | invalid
// F  |  +  |  +  |  +  | invalid
//
// M  |  -  |  +  |  +  | required
// J  |  -  |  +  |  0  | optional
// K  |  -  |  -  |  +  | optional
// H  |  -  |  0  |  0  | invalid
// I  |  -  |  -  |  0  | invalid
// L  |  -  |  -  |  -  | invalid

// Interactions between polymorphic molecules
//
//  a  3    |   3      a'
//     0 -3 |   1 -3
//     2 -3 |  -8 -2
//
// In [a] above:
//  * the [2] is a polymorphic node, and
//  * the [0] is an invalid carryover
// However, in [a'] above:
//  * the [-8] is a polymorphic node (still), but
//  * the [1] is now a newly created polymorphic node
//
// Let's consider the [0] in [a] to be a dormant polymorphic node.  A polymorphic carryover can
// awaken a dormant polymorphic node.
//
//  b  3    |   3      b'
//     0 -3 |   1 -3
//     2 -3 |  -8 -2
//
//
// Further testing shows that a carryover can alter the neighbouring digit's carryover status, at
// any time.

#include "Coord.h"

namespace mdn {

enum class Carryover {
    Invalid,            // Valid MDN --> carryover --> Invalid MDN
    OptionalPositive,   // Valid MDN --> carryover --> Valid MDN, current value positive
    OptionalNegative,   // Valid MDN --> carryover --> Valid MDN, current value negative
    Required            // Invalid MDN --> carryover --> Valid MDN
};

} // end namespace