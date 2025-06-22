#ifndef DIGIT_H
#define DIGIT_H

// #include <stdexcept>
// #include <sstream>
#include <cstdint>

namespace mdn {
    using Digit = int8_t;
}


// class Digit {
//     int8_t raw;

// public:
//     Digit(int value = 0) { set(value); }

//     void set(int value) {
//         #ifdef MDN_DEBUG
//         if (value < -32 || value > 32)
//             std::ostringstream oss;
//             oss << "Digit out of range: " << value
//                 << " (expected between -32 and 32)";
//             throw std::out_of_range(oss.str());
//         #endif
//         raw = static_cast<int8_t>(value);
//     }

//     int value() const { return static_cast<int>(raw); }

//     // Implicit conversion to int
//     operator int() const { return value(); }

//     Digit& operator+=(int x) {
//         set(value() + x);
//         return *this;
//     }

//     Digit& operator-=(int x) {
//         set(value() - x);
//         return *this;
//     }

//     Digit& operator*=(int x) {
//         set(value() * x);
//         return *this;
//     }

//     Digit& operator/=(int x) {
//         set(value() / x);
//         return *this;
//     }

// };

#endif // DIGIT_H
