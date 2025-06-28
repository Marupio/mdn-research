#include <iostream>
#include <sstream>
#include <vector>

// #include "Mdn2d.h"

using namespace std;
// using namespace mdn;


const std::string m_boxArt_h = "-"; // u8"\u2500"; // ─
const std::string m_boxArt_v = "|"; // u8"\u2502"; // │
const std::string m_boxArt_x = "+"; // u8"\u253C"; // ┼


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
    cout << "Hello world" << endl;
    {
        vector<string> rows = write(5,-2,4,-2);
        for (auto row : rows) {
            cout << row << endl;
        }
        cout << "In reverse" << endl;
        for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
            std::cout << *it << std::endl;
        }
    }
    cout << "\nNEXT" << endl;
    {
        vector<string> rows = write(5,0,4,1);
        for (auto row : rows) {
            cout << row << endl;
        }
        cout << "In reverse" << endl;
        for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
            std::cout << *it << std::endl;
        }
    }
    cout << "\nNEXT" << endl;
    {
        vector<string> rows = write(10,1,6,0);
        for (auto row : rows) {
            cout << row << endl;
        }
        cout << "In reverse" << endl;
        for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
            std::cout << *it << std::endl;
        }
    }
    cout << "\nNEXT" << endl;
    {
        vector<string> rows = write(-10,-15,6,1);
        for (auto row : rows) {
            cout << row << endl;
        }
        cout << "In reverse" << endl;
        for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
            std::cout << *it << std::endl;
        }
    }

    // cout << "In reverse again" << endl;
    // for (auto rit : std::views::reverse(rows)) {
    //     std::cout << rit << std::endl;
    // }



    return 0;

}