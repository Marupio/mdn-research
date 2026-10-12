#include <mdn/Mdn2d.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

static void emit(const char* name, mdn::Mdn2d& value, bool& separator) {
    if (separator) std::cout << ',';
    separator = true;
    std::cout << '"' << name << "\":[";
    bool comma = false;
    for (const auto& [position, digit] : value.data_raw()) {
        if (comma) std::cout << ',';
        comma = true;
        std::cout << '[' << position.x() << ',' << position.y() << ','
                  << static_cast<int>(digit) << ']';
    }
    std::cout << ']';
}

int main(int argc, char** argv) {
    try {
        if (argc != 4 && argc != 5) {
            throw std::invalid_argument("Expected X-depth a c [Y-depth]; base10, Positive, precision=-1");
        }
        mdn::Logger::instance().setEnabled(false);
        const int n = std::stoi(argv[1]);
        const int a = std::stoi(argv[2]), c = std::stoi(argv[3]);
        const int m = argc == 5 ? std::stoi(argv[4]) : n;
        if (n < 0 || m < 0 || a < 0 || c < 0 || a+c > 12) {
            throw std::invalid_argument("Require nonnegative depths/exponents and a+c<=12");
        }
        const mdn::Coord origin(0, 0);
        const auto sign = static_cast<mdn::SignConvention>(2);
        const mdn::Mdn2dConfig leftConfig(10, -1, sign, n, mdn::Fraxis::X);
        const mdn::Mdn2dConfig rightConfig(10, -1, sign, m, mdn::Fraxis::Y);
        mdn::Mdn2d left(leftConfig), right(rightConfig);
        left.add(origin, false, "0", "1", false, mdn::Fraxis::X);
        right.add(origin, false, "0", "1", false, mdn::Fraxis::Y);
        auto product = [&](bool reverse) {
            mdn::Mdn2d value(leftConfig);
            value.add(origin, 1);
            for (int group = 0; group < 2; ++group) {
                const bool useRight = (group == 1) != reverse;
                const int count = useRight ? c : a;
                const auto& factor = useRight ? right : left;
                for (int index = 0; index < count; ++index) {
                    mdn::Mdn2d next(leftConfig);
                    value.multiply(factor, next);
                    value = std::move(next);
                }
            }
            return value;
        };
        auto forward = product(false);
        auto reverse = product(true);
        bool separator = false;
        std::cout << '{';
        emit("L", left, separator); emit("R", right, separator);
        emit("product", forward, separator); emit("reverse", reverse, separator);
        std::cout << "}\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
