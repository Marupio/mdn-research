#pragma once

#include <memory>

#include "MainWindow.h"
#include "Project.h"
#include "../library/Mdn2d.h"

namespace mdn {

class GuiDriver {

    MainWindow m_mainWindow;
    std::shared_ptr<Project>* m_projectPtr;

};


} // end namespace mdn