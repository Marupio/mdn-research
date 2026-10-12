#include <mdn/Mdn2d.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

// Exact text input, unlimited precision, explicit cascade depth and sign policy.
// Usage: base depth sign direction decimal-text [multiply|add direction decimal-text]
static void emit(mdn::Mdn2d& value) {
    std::cout << '[';
    bool comma = false;
    for (const auto& [coordinate, digit] : value.data_raw()) {
        if (comma) std::cout << ',';
        comma = true;
        std::cout << '[' << coordinate.x() << ',' << coordinate.y() << ','
                  << static_cast<int>(digit) << ']';
    }
    std::cout << ']';
}

static mdn::Fraxis direction(const std::string& text) {
    if (text == "X") return mdn::Fraxis::X;
    if (text == "Y") return mdn::Fraxis::Y;
    throw std::invalid_argument("Direction must be X or Y");
}

static void input(mdn::Mdn2d& number, const std::string& value, mdn::Fraxis axis) {
    const bool negative = !value.empty() && value.front() == '-';
    const std::string digits = negative ? value.substr(1) : value;
    const auto point = digits.find('.');
    const auto integer = digits.substr(0, point);
    const auto fraction = point == std::string::npos ? std::string() : digits.substr(point + 1);
    number.add(mdn::Coord(0, 0), negative, integer, fraction, false, axis);
}

int main(int argc, char** argv) {
    try {
        if (argc != 6 && argc != 9) {
            throw std::invalid_argument(
                "Expected base depth sign direction decimal-text [multiply|add direction decimal-text]");
        }
        mdn::Logger::instance().setEnabled(false);
        const int base = std::stoi(argv[1]), depth = std::stoi(argv[2]);
        const auto sign = static_cast<mdn::SignConvention>(std::stoi(argv[3]));
        const auto leftDirection = direction(argv[4]);
        mdn::Mdn2dConfig config(base, -1, sign, depth, leftDirection);
        mdn::Mdn2d left(config), right(config), result(config);
        input(left, argv[5], leftDirection);
        std::cout << "{\"left\":";
        emit(left);
        if (argc == 9) {
            input(right, argv[8], direction(argv[7]));
            std::cout << ",\"right\":";
            emit(right);
            if (std::string(argv[6]) == "multiply") left.multiply(right, result);
            else if (std::string(argv[6]) == "add") left.plus(right, result);
            else throw std::invalid_argument("Operation must be multiply or add");
            std::cout << ",\"result\":";
            emit(result);
        }
        std::cout << "}\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
