#include "Logger.h"


std::ofstream* mdn::Logger::m_ossPtr;


std::filesystem::path mdn::Logger::m_debugLog;


void mdn::Logger::setOutputToFile(std::filesystem::path debugFile) {
    if (m_ossPtr) {
        std::ostringstream oss;
        oss << "Attempting to open debug file '" << debugFile << "', but '"
            << m_debugLog << "' is already in use.";
        log(LogLevel::Warning, oss.str());
        m_ossPtr->close();
    }
    if (debugFile.empty()) {
        debugFile = defaultPath();
    }
    // if (std::filesystem::exists(debugFile)) {
    //     debugFile = debugFile + "0";
    //     while (std::filesystem::exists(debugFile)) {
    //         debugFile = debugFile + "0";
    //     }
    // }
    if (!std::filesystem::exists(debugFile)) {
        m_ossPtr = new std::ofstream(debugFile);
        if (!m_ossPtr) {
            std::ostringstream oss;
            oss << "Failed to open debug file '" << debugFile << "'";
            log(LogLevel::Warning, oss.str());
        } else {
            m_debugLog = debugFile;
        }
    }
}


const std::filesystem::path& mdn::Logger::defaultPath() {
    static const std::filesystem::path path = "debug";
    return path;
}
