#include "Logger.hpp"

#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <regex>
#include <iomanip>


std::ofstream* mdn::Logger::m_ossPtr;


std::filesystem::path mdn::Logger::m_debugLog;


void mdn::Logger::setOutputToFile(std::filesystem::path debugFile) {
    namespace fs = std::filesystem;

    if (m_ossPtr) {
        Log_Warn("Attempting to open debug file '" << debugFile
                 << "', but '" << m_debugLog << "' is already in use.");
        m_ossPtr->close();
        delete m_ossPtr;
        m_ossPtr = nullptr;
    }

    if (debugFile.empty()) {
        debugFile = defaultPath();
    }

    fs::path candidate = debugFile;
    const fs::path dir = candidate.parent_path();
    const std::string stem = candidate.stem().string();
    const std::string ext  = candidate.extension().string();

    // up to 3 digits max
    std::smatch m;
    std::regex re(R"(^(.*?)(\d{1,3})$)");
    if (std::regex_match(stem, m, re)) {
        std::string base = m[1].str();
        int num = std::stoi(m[2].str());
        while (fs::exists(candidate) && num < 1000) {
            std::ostringstream oss;
            oss << base << std::setw(3) << std::setfill('0') << num++;
            candidate = dir / (oss.str() + ext);
        }
    } else {
        int num = 0;
        do {
            std::ostringstream oss;
            oss << stem << "-" << std::setw(3) << std::setfill('0') << num++;
            candidate = dir / (oss.str() + ext);
        } while (fs::exists(candidate) && num < 1000);
    }

    if (fs::exists(candidate)) {
        Log_Warn("Failed to acquire a suitable log file from '" << debugFile.string() << "'");
        return;
    }

    auto* f = new std::ofstream(candidate, std::ios::out | std::ios::trunc);
    if (!f || !(*f)) {
        Log_Warn("Failed to open debug file '" << candidate.string() << "'");
        delete f;
        return;
    }

    m_ossPtr = f;
    m_debugLog = candidate;
    Log_Info("Debug log opened at '" << m_debugLog.string() << "'");
}


void mdn::Logger::setOutputToFileLegacy(std::filesystem::path debugFile) {
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
