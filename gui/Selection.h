#pragma once

#include <vector>

#include "../library/Rect.h"


struct Selection {
    // Coordinate bounds of selection, inclusive: [x0..x1] × [y0..y1]
    mdn::Rect rect;

    // Tab copied from
    int originTabId{-1};

    bool isEmpty() const {
        return originTabId < 0 || !rect.isValid();
    }
};
