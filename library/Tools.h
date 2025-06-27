#pragma once

#include <vector>
#include <string>

namespace mdn {

class Tools {

public:

    static std::string joinArray(
        const std::vector<std::string>& lines,
        const std::string& delimiter
    );

    static std::string joinArray(
        const std::vector<int>& array,
        const std::string& delimiter
    );

    template<typename T>
    static std::string joinArrayT(
        const std::vector<T>& array,
        const std::string& delimiter
    );


};  // end class Tools

} // end namespace mdn
