#include "Tools.h"
#include <sstream>


std::string mdn::Tools::joinArray(
    const std::vector<std::string>& array,
    const std::string& delimiter)
{
    std::ostringstream oss;
    for (const auto& line : array) {
        oss << line << delimiter;
    }
    return oss.str();
}


std::string mdn::Tools::joinArray(
    const std::vector<int>& array,
    const std::string& delimiter)
{
    if (array.size() == 0) {
        return "";
    }
    std::ostringstream oss;
    oss << array[0];
    for (size_t i = 1; i < array.size(); ++i) {
        oss << delimiter << array[i];
    }
    return oss.str();
}


// Overload for char delimiter (convenience)
std::string joinArray(const std::vector<int>& array, char delimiter) {
    return joinArray(array, std::string(1, delimiter));
}
