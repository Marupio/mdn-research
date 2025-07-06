#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <limits>


// #include "Mdn2d.h"

using namespace std;
// using namespace mdn;


// // const std::string m_boxArt_h = "-"; // u8"\u2500"; // ─
// // const std::string m_boxArt_v = "|"; // u8"\u2502"; // │
// // const std::string m_boxArt_x = "+"; // u8"\u253C"; // ┼
// const std::string m_boxArt_h = u8"\u2500"; // ─
// const std::string m_boxArt_v = u8"\u2502"; // │
// const std::string m_boxArt_x = u8"\u253C"; // ┼


// std::vector<string> write(int xx, int xn, int yx, int yn) {
//     int yStart = yn;
//     int yEnd = yx+1;
//     int xStart = xn;
//     int xEnd = xx+1;
//     int xCount = xEnd - xStart;
//     // cout << "x:(" << xStart << "," << xEnd << ")=" << xCount << endl;
//     // xDigLine - digit line appears before what index in 'digits' array below
//     //  xDigLine  < 0 : off screen, stage left
//     //  xDigLine == 0 : digit line appears first, then the digits
//     //  xDigLine == digits.size() : digit line appears after last digit
//     //  digLne   > digits.size() : off scren, stage right
//     int xDigLine = -xStart;
//     int yDigLine = -yStart;
//     int yCount = yEnd - yStart;
//     // cout << "y:(" << yStart << "," << yEnd << ")=" << yCount << endl;
//     // cout << "digLine:(" << xDigLine << "," << yDigLine << ")" << endl;
//     std::vector<std::string> out;
//     out.reserve(yCount);
//     std::vector<int> digits;

//     // cout << "reverse=" << reverse << ", y=" << yStart << ", yEnd=" << yEnd << ") " << endl;
//     // cout << "y>yEnd=" << (yStart > yEnd) << ", y<yEnd=" << (yStart < yEnd) << endl;
//     // cout << "r&&y>yEnd=" << (reverse && yStart > yEnd) << ", !r&&y<yEnd=" << (!reverse && yStart < yEnd) << endl;
//     for (int y = yStart; y < yEnd; ++y) {
//         // First, are we at the yDigit line?
//         // cout << "y=" << y << endl;
//         if (y == 0) {
//             int x;
//             std::ostringstream oss;
//             oss << "  : ";
//             for (x = 0; x < xDigLine && x < xCount; ++x) {
//                 oss << m_boxArt_h << m_boxArt_h;
//             }
//             if (x == xDigLine) {
//                 oss << m_boxArt_x;
//             }
//             for (; x < xCount; ++x) {
//                 oss << m_boxArt_h << m_boxArt_h;
//             }
//             out.push_back(oss.str());
//         }
//         // locked_fillRow(y, digits);
//         // assert(digits.size() == yCount && "Digits not the correct size");
//         std::ostringstream oss;
//         if (y < 0)
//             oss << y;
//         else
//             oss << "+" << y;
//         oss << ": ";
//         int x;
//         for (x = 0; x < xDigLine && x < xCount; ++x) {
//             if (y < 0)
//                 oss << y;
//             else
//                 oss << "+" << y;
//         }
//         if (x == xDigLine) {
//             oss << m_boxArt_v;
//         }
//         for (; x < xCount; ++x) {
//             if (y < 0)
//                 oss << y;
//             else
//                 oss << "+" << y;
//         }
//         out.push_back(oss.str());
//     } // end y loop
//     return out;
// }



// int main() {
//     // Mdn2d m(2);
//     // m.addInteger(1, 1, 1);
//     // cout << "Hello world" << endl;
//     // {
//     //     vector<string> rows = write(5,-2,4,-2);
//     //     for (auto row : rows) {
//     //         cout << row << endl;
//     //     }
//     //     cout << "In reverse" << endl;
//     //     for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
//     //         std::cout << *it << std::endl;
//     //     }
//     // }
//     // cout << "\nNEXT" << endl;
//     // {
//     //     vector<string> rows = write(5,0,4,1);
//     //     for (auto row : rows) {
//     //         cout << row << endl;
//     //     }
//     //     cout << "In reverse" << endl;
//     //     for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
//     //         std::cout << *it << std::endl;
//     //     }
//     // }
//     // cout << "\nNEXT" << endl;
//     // {
//     //     vector<string> rows = write(10,1,6,0);
//     //     for (auto row : rows) {
//     //         cout << row << endl;
//     //     }
//     //     cout << "In reverse" << endl;
//     //     for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
//     //         std::cout << *it << std::endl;
//     //     }
//     // }
//     // cout << "\nNEXT" << endl;
//     // {
//     //     vector<string> rows = write(-10,-15,6,1);
//     //     for (auto row : rows) {
//     //         cout << row << endl;
//     //     }
//     //     cout << "In reverse" << endl;
//     //     for (auto it = rows.rbegin(); it != rows.rend(); ++it) {
//     //         std::cout << *it << std::endl;
//     //     }
//     // }

//     // cout << "In reverse again" << endl;
//     // for (auto rit : std::views::reverse(rows)) {
//     //     std::cout << rit << std::endl;
//     // }

//     double val = 3.141592635;
//     double a, b;
//     a = modf(val, &b);
//     cout << "a = modf(val, &b), " << a << " = modf(" << val << ", " << b << ")" << endl;

//     unordered_map<int, int> mrmap;
//     mrmap[0] = 1;
//     mrmap[1] = 2;
//     mrmap[2] = 3;
//     mrmap[10] = 5;

//     int result = mrmap.erase(9);
//     cout << "result = " << result;
//     result = mrmap.erase(2);
//     cout << "result = " << result;

//     cout << endl;

//     const int m_base = 8;
//     double pi = 3.1415926359;
//     int8_t d0(pi);
//     double pi0 = pi;
//     pi0 -= d0;
//     cout << "pi = " << pi << ", d0 = " << d0 << ", pi0 = " << pi0 << endl;
//     double pi1 = pi0;
//     pi1 *= m_base;
//     int8_t d1(pi1);
//     double pi2 = pi1;
//     pi2 -= d1;
//     cout << "pi1 = " << pi1 << ", d1 = " << d1 << ", pi2 = " << pi2 << endl;

//     cout << endl << endl;

//     cout << "Numeric limits:" << endl;
//     cout << "float" << endl;
//     cout << "   epsilon = " << numeric_limits<float>::epsilon() << endl;
//     cout << "   max = " << numeric_limits<float>::max() << endl;
//     cout << "   min = " << numeric_limits<float>::min() << endl;
//     cout << "   round_error = " << numeric_limits<float>::round_error() << endl;
//     cout << "double" << endl;
//     cout << "   epsilon = " << numeric_limits<double>::epsilon() << endl;
//     cout << "   max = " << numeric_limits<double>::max() << endl;
//     cout << "   min = " << numeric_limits<double>::min() << endl;
//     cout << "   round_error = " << numeric_limits<double>::round_error() << endl;
//     float ff = 0.123456789112345678921234567893;
//     double dd = 0.123456789112345678921234567893;
//     cout << std::fixed << std::setprecision(40);
//     cout << "ff = " << ff << endl;
//     cout << "dd = " << dd << endl;
//     {
//         float unity = 1.0;
//         for (int i = 0; i < 100; ++i){
//             float num = pow(10, -i);
//             cout << i << ":" << num << flush;
//             float inv = unity / num;
//             cout << " inv = " << inv << endl;
//         }
//     }
//     {
//         double unity = 1.0;
//         for (int i = 0; i < 400; ++i){
//             double num = pow(10, -i);
//             cout << i << ":" << num << flush;
//             double inv = unity / num;
//             cout << " inv = " << inv << endl;
//         }
//     }
//     {
//         double unity = 1.0;
//         double small = 1e-12;
//         for (int i = 0; i < 400; ++i){
//             double num = pow(10, i);
//             cout << i << ":" << num << flush;
//             double inv = num / small;
//             cout << " inv = " << inv << endl;
//         }
//     }

//     enum class Flurples {
//         Flompy,
//         Flurgen,
//         Florkabork
//     };
//     std::vector FlurplesNames( {
//         "Flompy",
//         "Flurgen",
//         "Florkabork"
//     });
//     for (int i = 0; i < FlurplesNames.size(); ++i) {
//         Flurples myEnum(static_cast<Flurples>(i));
//         switch (myEnum) {
//             case Flurples::Flompy:
//                 cout << "I is 'Flompy'" << endl;
//             case Flurples::Flurgen:
//                 cout << "I is 'Flurgen'" << endl;
//             case Flurples::Florkabork:
//                 cout << "I is 'Florkabork'" << endl;
//         };
//         cout << "Name = " << FlurplesNames[i] << ", enum = " << int(Flurples(i)) << endl;
//     }

//     return 0;
// }

// class MrBase {
// public:
//     virtual void VF() { cout << "MrBase::VF" << endl; }
//     virtual void VF1() { cout << "MrBase::VF1" << endl; }
//     virtual void VF2() { cout << "MrBase::VF2" << endl; }
//     virtual void VF3() { cout << "MrBase::VF3" << endl; }

//     void F() { cout << "MrBase::F" << endl; }
//     void F1() { cout << "MrBase::F1" << endl; }
//     void F2() { cout << "MrBase::F2" << endl; VF2();}
//     void F3() { cout << "MrBase::F3" << endl; }
// };

// class MrDerived : public MrBase {
// public:
//     virtual void VF() { cout << "MrDerived::VF" << endl; }
//     virtual void VF1() { cout << "MrDerived::VF1" << endl; MrBase::VF1(); }
//     virtual void VF2() { cout << "MrDerived::VF2" << endl; }
//     virtual void VF3() { cout << "MrDerived::VF3" << endl; }

//     void F() { cout << "MrDerived::F" << endl; }
//     void F1() { cout << "MrDerived::F1" << endl; MrBase::F1(); }
//     void F2() { cout << "MrDerived::F2" << endl;  }
//     void F3() { cout << "MrDerived::F3" << endl; }
// };

int main() {
    // MrBase b;
    // MrDerived d;
    // MrBase* b_ptrTo_d(&d);
    // MrDerived* d_ptrTo_d(&d);

    // cout << "\nb.F2():" << endl;
    // b.F2();
    // cout << "\nd.MrBase::F2():" << endl;
    // d.MrBase::F2();

    // cout << "\nb_ptrTo_d->F2():" << endl;
    // b_ptrTo_d->F2();
    // cout << "\nd_ptrTo_d->MrBase::F2():" << endl;
    // d_ptrTo_d->MrBase::F2();

    // cout << "-------------------------------" << endl;

    // cout << "\nb.F():" << endl;
    // b.F();
    // cout << "\nb.F1():" << endl;
    // b.F1();
    // cout << "\nb.VF():" << endl;
    // b.VF();
    // cout << "\nb.VF1():" << endl;
    // b.VF1();

    // cout << "\nd.F():" << endl;
    // d.F();
    // cout << "\nd.F1():" << endl;
    // d.F1();
    // cout << "\nd.VF():" << endl;
    // d.VF();
    // cout << "\nd.VF1():" << endl;
    // d.VF1();

    // cout << "\nb_ptrTo_d->F():" << endl;
    // b_ptrTo_d->F();
    // cout << "\nb_ptrTo_d->F1():" << endl;
    // b_ptrTo_d->F1();
    // cout << "\nb_ptrTo_d->VF():" << endl;
    // b_ptrTo_d->VF();
    // cout << "\nb_ptrTo_d->VF1():" << endl;
    // b_ptrTo_d->VF1();

    // cout << "\nd_ptrTo_d->F():" << endl;
    // d_ptrTo_d->F();
    // cout << "\nd_ptrTo_d->F1():" << endl;
    // d_ptrTo_d->F1();
    // cout << "\nd_ptrTo_d->VF():" << endl;
    // d_ptrTo_d->VF();
    // cout << "\nd_ptrTo_d->VF1():" << endl;
    // d_ptrTo_d->VF1();

    // unordered_set<int> mySet({1,2,3,4,6});
    // unordered_set<int> aSet({1,2,8,9,10});
    // // unordered_set<int> bSet({20,21,22});
    // unordered_set<int> bSet = mySet;

    // cout << "mySet:" << endl;
    // cout << "size=" << mySet.size() << ", elems = ";
    // for (int i : mySet) {
    //     cout << i << ", ";
    // }
    // cout << "\naSet:" << endl;
    // cout << "size=" << aSet.size() << ", elems = ";
    // for (int i : aSet) {
    //     cout << i << ", ";
    // }
    // cout << "\nbSet:" << endl;
    // cout << "size=" << bSet.size() << ", elems = ";
    // for (int i : bSet) {
    //     cout << i << ", ";
    // }
    // cout << "\n" << endl;
    // aSet.merge(mySet);
    // mySet.clear();

    // cout << "aSet.merge(mySet)" << endl;
    // cout << "mySet.clear()" << endl;
    // cout << endl;

    // cout << "mySet:" << endl;
    // cout << "size=" << mySet.size() << ", elems = ";
    // for (int i : mySet) {
    //     cout << i << ", ";
    // }
    // cout << "\naSet:" << endl;
    // cout << "size=" << aSet.size() << ", elems = ";
    // for (int i : aSet) {
    //     cout << i << ", ";
    // }
    // cout << "\nbSet:" << endl;
    // cout << "size=" << bSet.size() << ", elems = ";
    // for (int i : bSet) {
    //     cout << i << ", ";
    // }

        // workingSet = buffer;
        // affectedCoords.merge(buffer);
        // buffer.clear();
        // if (!workingSet.size()) {
        //     achievedGreatness = true;
        // }

    // cout << "mySet:" << endl;
    // for (int i : mySet) {
    //     cout << i << ", ";
    // }
    // cout << "\naSet:" << endl;
    // cout << "size=" << aSet.size() << endl;
    // for (int i : aSet) {
    //     cout << i << ", ";
    // }
    // cout << "\nbSet:" << endl;
    // cout << "size=" << bSet.size() << endl;
    // for (int i : bSet) {
    //     cout << i << ", ";
    // }
    // cout << endl;
    // // mySet.insert(mySet.end(), aSet.begin(), aSet.end());
    // mySet.merge(aSet);
    // cout << "After merge setA into mySet\nmySet=" << endl;
    // cout << "size=" << mySet.size() << endl;
    // for (int i : mySet) {
    //     cout << i << ", ";
    // }
    // cout << endl;
    // mySet.merge(bSet);
    // cout << "After merge setB into mySet\nmySet=" << endl;
    // cout << "size=" << mySet.size() << endl;
    // for (int i : mySet) {
    //     cout << i << ", ";
    // }
    // cout << endl;
    // cout << "Do the original aSet and bSet still have numbers?" << endl;
    // cout << "\naSet:" << endl;
    // cout << "size=" << aSet.size() << endl;
    // for (int i : aSet) {
    //     cout << i << ", ";
    // }
    // cout << "\nbSet:" << endl;
    // cout << "size=" << bSet.size() << endl;
    // for (int i : bSet) {
    //     cout << i << ", ";
    // }
    // cout << endl;
    // int a = 4;
    // int b = 4;
    // switch (a) {
    //     case 0:
    //         cout << "0" << endl;
    //         break;
    //     case 4:
    //         cout << "4" << endl;
    //         break;
    //     case b:
    //         cout << "b value" << endl;
    //         break;
    // }

    std::string testStr;
    char h = 'h';
    char e = 'e';
    char l = 'l';
    char o = 'o';
    testStr += h;
    testStr += e;
    testStr += l;
    testStr += l;
    testStr += o;
    cout << "Test string = [" << testStr << "]" << endl;
    return 0;

}