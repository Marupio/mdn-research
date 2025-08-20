#pragma once

#include <memory>

#include "MainWindow.hpp"
#include "Project.hpp"

#include "../library/GlobalConfig.hpp"
#include "../library/Mdn2d.hpp"

namespace mdn {

class GuiDriver {

    MainWindow m_mainWindow;
    std::shared_ptr<Project>* m_projectPtr;

};


} // end namespace mdn