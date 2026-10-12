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
        if (argc != 7 && argc != 9) {
            throw std::invalid_argument("Expected base sign N M P Q [U V]");
        }
        mdn::Logger::instance().setEnabled(false);
        const int base = std::stoi(argv[1]);
        const auto sign = static_cast<mdn::SignConvention>(std::stoi(argv[2]));
        const int n = std::stoi(argv[3]), m = std::stoi(argv[4]);
        const int p = std::stoi(argv[5]), q = std::stoi(argv[6]);
        if (base < 3 || base > 16 || n < 0 || m < 0 || p < 0 || q < 0) {
            throw std::invalid_argument("Require 3<=base<=16 and nonnegative depths");
        }
        const mdn::Coord origin(0, 0);
        const mdn::Mdn2dConfig cn(base, -1, sign, n, mdn::Fraxis::X);
        const mdn::Mdn2dConfig cm(base, -1, sign, m, mdn::Fraxis::Y);
        const mdn::Mdn2dConfig cp(base, -1, sign, p, mdn::Fraxis::X);
        const mdn::Mdn2dConfig cq(base, -1, sign, q, mdn::Fraxis::Y);
        mdn::Mdn2d left(cn), right(cm), extraLeft(cp), extraRight(cq);
        left.add(origin, false, "0", "1", false, mdn::Fraxis::X);
        right.add(origin, false, "0", "1", false, mdn::Fraxis::Y);
        extraLeft.add(origin, false, "0", "1", false, mdn::Fraxis::X);
        extraRight.add(origin, false, "0", "1", false, mdn::Fraxis::Y);
        mdn::Mdn2d fuzzy(cn), fl(cn), fr(cn), square(cn);
        left.multiply(right, fuzzy);
        fuzzy.multiply(extraLeft, fl);
        fuzzy.multiply(extraRight, fr);
        fuzzy.multiply(fuzzy, square);

        bool separator = false;
        std::cout << '{';
        emit("L_N", left, separator); emit("R_M", right, separator);
        emit("L_P", extraLeft, separator); emit("R_Q", extraRight, separator);
        emit("F", fuzzy, separator); emit("FL", fl, separator);
        emit("FR", fr, separator); emit("F2", square, separator);
        if (argc == 9) {
            const int u = std::stoi(argv[7]), v = std::stoi(argv[8]);
            if (u < 0 || v < 0) throw std::invalid_argument("U,V must be nonnegative");
            const mdn::Mdn2dConfig cu(base, -1, sign, u, mdn::Fraxis::X);
            const mdn::Mdn2dConfig cv(base, -1, sign, v, mdn::Fraxis::Y);
            mdn::Mdn2d lu(cu), rv(cv), other(cn), mixed(cn);
            lu.add(origin, false, "0", "1", false, mdn::Fraxis::X);
            rv.add(origin, false, "0", "1", false, mdn::Fraxis::Y);
            lu.multiply(rv, other);
            fuzzy.multiply(other, mixed);
            emit("L_U", lu, separator); emit("R_V", rv, separator);
            emit("G", other, separator); emit("FG", mixed, separator);
        }
        std::cout << "}\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
