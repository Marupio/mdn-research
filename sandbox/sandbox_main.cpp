#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <limits>


// #include "Mdn2d.h"

using namespace std;
// using namespace mdn;


// const std::string m_boxArt_h = "-"; // u8"\u2500"; // ─
// const std::string m_boxArt_v = "|"; // u8"\u2502"; // │
// const std::string m_boxArt_x = "+"; // u8"\u253C"; // ┼
const std::string m_boxArt_h = u8"\u2500"; // ─
const std::string m_boxArt_v = u8"\u2502"; // │
const std::string m_boxArt_x = u8"\u253C"; // ┼


std::vector<string> write(int xx, int xn, int yx, int yn) {
    int yStart = yn;
    int yEnd = yx+1;
    int xStart = xn;
    int xEnd = xx+1;
    int xCount = xEnd - xStart;
    // cout << "x:(" << xStart << "," << xEnd << ")=" << xCount << endl;
    // xDigLine - digit line appears before what index in 'digits' array below
    //  xDigLine  < 0 : off screen, stage left
    //  xDigLine == 0 : digit line appears first, then the digits
    //  xDigLine == digits.size() : digit line appears after last digit
    //  digLne   > digits.size() : off scren, stage right
    int xDigLine = -xStart;
    int yDigLine = -yStart;
    int yCount = yEnd - yStart;
    // cout << "y:(" << yStart << "," << yEnd << ")=" << yCount << endl;
    // cout << "digLine:(" << xDigLine << "," << yDigLine << ")" << endl;
    std::vector<std::string> out;
    out.reserve(yCount);
    std::vector<int> digits;

    // cout << "reverse=" << reverse << ", y=" << yStart << ", yEnd=" << yEnd << ") " << endl;
    // cout << "y>yEnd=" << (yStart > yEnd) << ", y<yEnd=" << (yStart < yEnd) << endl;
    // cout << "r&&y>yEnd=" << (reverse && yStart > yEnd) << ", !r&&y<yEnd=" << (!reverse && yStart < yEnd) << endl;
    for (int y = yStart; y < yEnd; ++y) {
        // First, are we at the yDigit line?
        // cout << "y=" << y << endl;
        if (y == 0) {
            int x;
            std::ostringstream oss;
            oss << "  : ";
            for (x = 0; x < xDigLine && x < xCount; ++x) {
                oss << m_boxArt_h << m_boxArt_h;
            }
            if (x == xDigLine) {
                oss << m_boxArt_x;
            }
            for (; x < xCount; ++x) {
                oss << m_boxArt_h << m_boxArt_h;
            }
            out.push_back(oss.str());
        }
        // locked_fillRow(y, digits);
        // assert(digits.size() == yCount && "Digits not the correct size");
        std::ostringstream oss;
        if (y < 0)
            oss << y;
        else
            oss << "+" << y;
        oss << ": ";
        int x;
        for (x = 0; x < xDigLine && x < xCount; ++x) {
            if (y < 0)
                oss << y;
            else
                oss << "+" << y;
        }
        if (x == xDigLine) {
            oss << m_boxArt_v;
        }
        for (; x < xCount; ++x) {
            if (y < 0)
                oss << y;
            else
                oss << "+" << y;
        }
        out.push_back(oss.str());
    } // end y loop
    return out;
}


int main() {
    // Mdn2d m(2);
    // m.addInteger(1, 1, 1);
    // cout << "Hello world" << endl;
    // {
    //     vector<string> rows = write(5,-2,4,-2);
    //     for (auto row : rows) {
    //         cout << row << endl;
    //     }
    //     cout << "In reverse" << endl;
    //     for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
    //         std::cout << *it << std::endl;
    //     }
    // }
    // cout << "\nNEXT" << endl;
    // {
    //     vector<string> rows = write(5,0,4,1);
    //     for (auto row : rows) {
    //         cout << row << endl;
    //     }
    //     cout << "In reverse" << endl;
    //     for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
    //         std::cout << *it << std::endl;
    //     }
    // }
    // cout << "\nNEXT" << endl;
    // {
    //     vector<string> rows = write(10,1,6,0);
    //     for (auto row : rows) {
    //         cout << row << endl;
    //     }
    //     cout << "In reverse" << endl;
    //     for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
    //         std::cout << *it << std::endl;
    //     }
    // }
    // cout << "\nNEXT" << endl;
    // {
    //     vector<string> rows = write(-10,-15,6,1);
    //     for (auto row : rows) {
    //         cout << row << endl;
    //     }
    //     cout << "In reverse" << endl;
    //     for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
    //         std::cout << *it << std::endl;
    //     }
    // }

    // cout << "In reverse again" << endl;
    // for (auto rit : std::views::reverse(rows)) {
    //     std::cout << rit << std::endl;
    // }

    double val = 3.141592635;
    double a, b;
    a = modf(val, &b);
    cout << "a = modf(val, &b), " << a << " = modf(" << val << ", " << b << ")" << endl;

    unordered_map<int, int> mrmap;
    mrmap[0] = 1;
    mrmap[1] = 2;
    mrmap[2] = 3;
    mrmap[10] = 5;

    int result = mrmap.erase(9);
    cout << "result = " << result;
    result = mrmap.erase(2);
    cout << "result = " << result;

    cout << endl;

    const int m_base = 8;
    double pi = 3.1415926359;
    int8_t d0(pi);
    double pi0 = pi;
    pi0 -= d0;
    cout << "pi = " << pi << ", d0 = " << d0 << ", pi0 = " << pi0 << endl;
    double pi1 = pi0;
    pi1 *= m_base;
    int8_t d1(pi1);
    double pi2 = pi1;
    pi2 -= d1;
    cout << "pi1 = " << pi1 << ", d1 = " << d1 << ", pi2 = " << pi2 << endl;

    cout << endl << endl;

    cout << "Numeric limits:" << endl;
    cout << "float" << endl;
    cout << "   epsilon = " << numeric_limits<float>::epsilon() << endl;
    cout << "   max = " << numeric_limits<float>::max() << endl;
    cout << "   min = " << numeric_limits<float>::min() << endl;
    cout << "   round_error = " << numeric_limits<float>::round_error() << endl;
    cout << "double" << endl;
    cout << "   epsilon = " << numeric_limits<double>::epsilon() << endl;
    cout << "   max = " << numeric_limits<double>::max() << endl;
    cout << "   min = " << numeric_limits<double>::min() << endl;
    cout << "   round_error = " << numeric_limits<double>::round_error() << endl;
    float ff = 0.123456789112345678921234567893;
    double dd = 0.123456789112345678921234567893;
    cout << std::fixed << std::setprecision(40);
    cout << "ff = " << ff << endl;
    cout << "dd = " << dd << endl;
    {
        float unity = 1.0;
        for (int i = 0; i < 100; ++i){
            float num = pow(10, -i);
            cout << i << ":" << num << flush;
            float inv = unity / num;
            cout << " inv = " << inv << endl;
        }
    }
    {
        double unity = 1.0;
        for (int i = 0; i < 400; ++i){
            double num = pow(10, -i);
            cout << i << ":" << num << flush;
            double inv = unity / num;
            cout << " inv = " << inv << endl;
        }
    }
    {
        double unity = 1.0;
        double small = 1e-12;
        for (int i = 0; i < 400; ++i){
            double num = pow(10, i);
            cout << i << ":" << num << flush;
            double inv = num / small;
            cout << " inv = " << inv << endl;
        }
    }
    return 0;

}