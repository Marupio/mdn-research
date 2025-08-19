#pragma once

/*
See also Carryover.h

This file is me trying to figure out ways to fully capture all polymorphic states of an MDN.
The current library does not use any of this.  Consider this just a collection of notes.

Polymorphic data structure (pmds)
We start with an Mdn in a valid state.  All required c/o's have been performed.  The polymorphic
data is probably going to be a tree.  Top level nodes are all optional c/o's.

[0,0] [2,0] [0,4] [1,5] ...
  |     |     |     |

Below each one are downstream c/o's that may be altered / awakened / removed, for example:

[0,0]
    * [0,1] - from Optional --> Invalid
    * [1,0] - from Invalid --> Required

Any newly created 'Required' c/o's must be actuated during the construction of the pmds.

[0,0]
    * [0,1] -Optional
    * [1,0] +Required --> [2,0] no change (invalid in this example)
                      \-> [1,1] +Optional

Some 'optional' nodes can appear more than once.

Static representation
    Coord
        Toggles [c0, c1, ...]


Node:

    Coord (x, y)
        The coordinates of the polymorphic carryover / optional carryover
    Downstream  [c0, c1, ...]
        The downstream required c/o's
    Activates [c0, c1, ...]
        What carryovers it activates (original coord and downstream)
    Deactivates [c0, c1, ...]
        What carryovers it deactivates

    Map (Coord)
        How to get to carryover this digit, if at all
    Activates [[c0, c1, ...],[c0, c1, ...]]
        Each list of coords is a list of all necessary c/o's needed before reaching Coord
        Multiple lists because sometimes more than one route leads to the same c/o

    Straight up, honest tree
    C0-,-C1-C3
       '-C2
*/

#include <unordered_map>
#include <utility>

#include "Coord.hpp"

namespace mdn {

struct PolymorphicTree {
    std::unordered_map<Coord, PolymorphicTreeNode> m_tree;
    std::unordered_set<Coord> m_currentIndex;
    std::vector<PolymorphicTreeNode> m_breadcrumbs;

    void clear() {
        m_tree.clear();
        m_currentIndex.clear();
        m_breadcrumbs.clear();
    }
};

struct PolymorphicTreeNode {
    Coord m_root;
    std::vector<PolymorphicTreeNode> m_deactivations;
    std::vector<PolymorphicTreeNode> m_activations;
};



enum class Carryover {
    Invalid,            // Valid MDN --> carryover --> Invalid MDN
    OptionalPositive,   // Valid MDN --> carryover --> Valid MDN, current value positive
    OptionalNegative,   // Valid MDN --> carryover --> Valid MDN, current value negative
    Required            // Invalid MDN --> carryover --> Valid MDN
};

} // end namespace