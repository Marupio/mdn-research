#pragma once

#include <vector>
#include <unordered_map>

#include "MainWindow.h"
#include "../library/Mdn2d.h"
#include "../library/Mdn2dFramework.h"

namespace mdn {

class Project: public Mdn2dFramework {

    // Null construction creates these zero-valued Mdn2d named tabs, in this order
    static const std::vector<std::string> m_defaultMdn2dNames;

    // References to the constituent Mdn2d data
    std::unordered_map<int, Mdn2d> m_data;


public:

    // Construct a null project
    Project();



};


} // end namespace mdn