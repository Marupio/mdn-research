#pragma once

namespace mdn {

struct ClipboardData {

    int versionMajor;
    int versionMinor;

    QString sourceMdnName;

    Rect dataBounds;

    std::vector<std::vector<Digit>> data;

};

} // end namespace mdn