#include <mdn/Mdn2d.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

// Emit the actual grid; the Python tests independently evaluate it with exact
// rational arithmetic, rather than relying on the backend's scalar conversion.
int main(int argc, char** argv) {
    try {
        if (argc != 11) {
            throw std::invalid_argument(
                "Expected base depth precision sign direction mode value digits x y");
        }
        const int base = std::stoi(argv[1]);
        const int depth = std::stoi(argv[2]);
        const int precision = std::stoi(argv[3]);
        const auto sign = static_cast<mdn::SignConvention>(std::stoi(argv[4]));
        const auto direction = std::string(argv[5]) == "X"
            ? mdn::Fraxis::X : mdn::Fraxis::Y;
        const std::string mode(argv[6]);
        const std::string value(argv[7]);
        const int digits = std::stoi(argv[8]);
        const mdn::Coord origin(std::stoi(argv[9]), std::stoi(argv[10]));
        mdn::Mdn2dConfig config(base, precision, sign, depth, direction);
        mdn::Mdn2d number(config), answer(config);

        auto addText = [&](bool overwrite, bool negate) {
            bool negative = !value.empty() && value.front() == '-';
            const std::string unsignedValue = negative ? value.substr(1) : value;
            const auto point = unsignedValue.find('.');
            const auto integer = unsignedValue.substr(0, point);
            const auto fraction = point == std::string::npos
                ? std::string() : unsignedValue.substr(point + 1);
            number.add(origin, negative != negate, integer, fraction, overwrite, direction);
        };

        if (mode == "carry-positive" || mode == "carry-negative") {
            const int polarity = mode == "carry-positive" ? 1 : -1;
            number.setValue(origin, polarity);
            number.setValue(origin.translatedX(1), polarity * (base - 1));
            number.setValue(origin.translatedY(1), -polarity);
            number.carryover(origin);
        } else if (mode == "double") {
            number.add(origin, std::stod(value), digits, false, direction);
        } else if (mode == "fraction-default") {
            number.addFraxis(origin, std::stod(value), digits);
        } else if (mode == "subtract") {
            number.subtract(origin, std::stod(value), digits, false, direction);
        } else {
            addText(false, false);
            if (mode == "repeat") addText(false, false);
            if (mode == "cancel") addText(false, true);
            if (mode == "overwrite") addText(true, false);
        }

        mdn::Mdn2d* result = &number;
        if (mode == "square") {
            number.multiply(number, answer);
            result = &answer;
        } else if (mode == "times-base") {
            mdn::Mdn2d multiplier(config);
            multiplier.add(mdn::Coord(0, 0), base);
            number.multiply(multiplier, answer);
            result = &answer;
        }

        std::cout << '[';
        bool separator = false;
        for (const auto& [coord, digit] : result->data_raw()) {
            if (separator) std::cout << ',';
            separator = true;
            std::cout << '[' << coord.x() << ',' << coord.y() << ','
                      << static_cast<int>(digit) << ']';
        }
        std::cout << "]\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
