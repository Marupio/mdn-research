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

//
//Identify all carryovers
//    List of optional carryovers
//    List of required carryovers
//
//Go through all digits and check for carryovers
//




namespace mdn {

enum class Carryover {
    Invalid,    // Valid MDN --> carryover --> Invalid MDN
    Optional,   // Valid MDN --> carryover --> Valid MDN
    Required    // Invalid MDN --> carryover --> Valid MDN
};

} // end namespace