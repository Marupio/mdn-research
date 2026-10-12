#include <mdn/Mdn2d.hpp>

#include <cstdlib>
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
        if (argc != 4) throw std::invalid_argument("Expected depth sine-tenths cosine-tenths");
        mdn::Logger::instance().setEnabled(false);
        const int depth = std::stoi(argv[1]);
        const int sine = std::stoi(argv[2]), cosine = std::stoi(argv[3]);
        if (depth < 0 || std::abs(sine) > 10 || std::abs(cosine) > 10)
            throw std::invalid_argument("Require depth>=0 and numerators in [-10,10]");
        const mdn::Coord origin(0, 0);
        const auto positive = static_cast<mdn::SignConvention>(2);
        const mdn::Mdn2dConfig config(10, -1, positive, depth, mdn::Fraxis::X);
        auto input = [&](int numerator, mdn::Fraxis fraxis) {
            mdn::Mdn2d value(config);
            const int magnitude = std::abs(numerator);
            value.add(origin, numerator < 0, std::to_string(magnitude/10),
                      std::to_string(magnitude%10), false, fraxis);
            return value;
        };
        auto L = input(1, mdn::Fraxis::X), R = input(1, mdn::Fraxis::Y);
        auto oneX = input(10, mdn::Fraxis::X), oneY = input(10, mdn::Fraxis::Y);
        auto A = input(sine, mdn::Fraxis::X);
        auto B = input(cosine, mdn::Fraxis::Y);
        auto C = input(sine, mdn::Fraxis::Y);
        auto D = input(cosine, mdn::Fraxis::X);
        auto vector = A+B;
        auto cross = A*C+D*B;
        auto squares = A*A+B*B;
        auto leftFactor = A+B, rightFactor = C+D;
        auto factored = leftFactor*rightFactor;
        auto expanded = A*C+A*D+B*C+B*D;
        auto omitted = A*D+B*C;
        auto tenL = L*10, tenR = R*10;
        auto reflectedVector = C+D, signflip = A-B, reflectedSignflip = C-D;
        auto normPair = vector*reflectedVector+signflip*reflectedSignflip;
        auto Wleft = input(-cosine, mdn::Fraxis::X);
        auto Wright = input(sine, mdn::Fraxis::Y);
        auto reflectedWleft = input(-cosine, mdn::Fraxis::Y);
        auto reflectedWright = input(sine, mdn::Fraxis::X);
        auto orthogonal = Wleft+Wright;
        auto reflectedOrthogonal = reflectedWleft+reflectedWright;
        auto reflectedOrthogonalSignflip = reflectedWleft-reflectedWright;
        auto dotPair = vector*reflectedOrthogonal+signflip*reflectedOrthogonalSignflip;
        auto determinantPair = vector*reflectedOrthogonal-reflectedVector*orthogonal;
        bool separator = false;
        std::cout << '{';
        emit("L", L, separator); emit("R", R, separator);
        emit("oneX", oneX, separator); emit("oneY", oneY, separator);
        emit("A", A, separator); emit("B", B, separator);
        emit("C", C, separator); emit("D", D, separator);
        emit("vector", vector, separator); emit("cross", cross, separator);
        emit("squares", squares, separator);
        emit("factored", factored, separator); emit("expanded", expanded, separator);
        emit("omitted", omitted, separator);
        emit("tenL", tenL, separator); emit("tenR", tenR, separator);
        emit("reflectedVector", reflectedVector, separator);
        emit("signflip", signflip, separator);
        emit("reflectedSignflip", reflectedSignflip, separator);
        emit("normPair", normPair, separator);
        emit("orthogonal", orthogonal, separator);
        emit("reflectedOrthogonal", reflectedOrthogonal, separator);
        emit("reflectedOrthogonalSignflip", reflectedOrthogonalSignflip, separator);
        emit("dotPair", dotPair, separator);
        emit("determinantPair", determinantPair, separator);
        std::cout << "}\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
