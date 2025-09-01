// guiTest.cpp
#include <QApplication>
#include <QDebug>

#include "LoggerConfigurator.hpp"
#include "Project.hpp"
#include "QtLoggingBridge.hpp"

#include "../library/GlobalConfig.hpp"
#include "../library/Logger.hpp"
#include "../library/Mdn2d.hpp"
#include "../library/Tools.hpp"

using namespace mdn;
using namespace mdn::gui;

static void printToc(const Project& p, const char* label)
{
    Log_Info("toc dispatch: " << label);
    std::vector<std::string> names = p.toc();
    std::string tocStr = mdn::Tools::vectorToString<std::string>(names, ", ", false);
    Log_Info("toc result: " << tocStr);
}

int main(int argc, char** argv)
{
    mdn_installQtMessageHandler();  // forward Qt -> Logger
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("mdn_guiTest");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    // Parse CLI / optional JSON and configure logger (no hard-coded defaults here)
    mdn::cli::LoggerConfigurator logCfg("MDN GUI headless Project smoke-test");
    logCfg.addStandardOptions();
    if (!logCfg.process(app)) {
        return EXIT_FAILURE;
    }

    // 1) Create a project with 3 starting tabs
    Project proj(nullptr, "guiTest-project", 3);  // parentless; no windows
    Log_Info("Created Project name='" << proj.name() << "' with " << proj.size() << " tabs");

    printToc(proj, "initial");

    // 2) Rename the first three tabs to A, B, C
    if (proj.contains(0)) proj.renameMdn(0, "A");
    if (proj.contains(1)) proj.renameMdn(1, "B");
    if (proj.contains(2)) proj.renameMdn(2, "C");
    printToc(proj, "after-rename");

    // 3) Verify addressing maps via indexOfMdn/nameOfMdn
    Log_Info("indexOfMdn('B') = " << proj.indexOfMdn("B"));
    Log_Info("nameOfMdn(0)     = '" << proj.nameOfMdn(0) << "'");

    // 4) Insert a new Mdn after tab 1
    {
        const std::string newName = "D";
        Mdn2d m(proj.config(), newName);
        const int insertAt = 1 + 1; // after "B"
        proj.insertMdn(m, insertAt);
        Log_Info("Inserted '" << newName << "' at " << insertAt);
    }
    printToc(proj, "after-insert");

    // 5) Set active tab by name and by index
    proj.setActiveMdn(std::string("C"));
    Log_Info("Active set to 'C' (index " << proj.indexOfMdn("C") << ")");
    proj.setActiveMdn(0);
    Log_Info("Active set to index 0 ('" << proj.nameOfMdn(0) << "')");

    // 6) Basic contains() checks
    Log_Info("contains('A')? " << (proj.contains(std::string("A")) ? "yes" : "no"));
    Log_Info("contains(99)?  " << (proj.contains(99) ? "yes" : "no"));

    // 7) Quick data access smoke test
    if (auto* pair = proj.at("A")) {
        auto& mdnA = pair->first;
        auto& selA = pair->second;
        Log_Info("Accessed 'A': bounds=" << selA.rect()); // relies on Selection’s ostream<<
        // Touch a digit (origin) to ensure model mutability works in this context:
        mdnA.setValue(COORD_ORIGIN, 7);
        Log_Info("Set 'A'(0,0)=7; now value=" << (int)mdnA.getValue(COORD_ORIGIN));
    }

    // 8) Deletion test
    Log_Info("Attempting to delete 'A'");
    proj.deleteMdn("A");
    printToc(proj, "After deleting 'A'");
    Log_Info("contains('A')? " << (proj.contains(std::string("A")) ? "yes" : "no"));

    // No need for an event loop; we’re headless here.
    return 0;
}
