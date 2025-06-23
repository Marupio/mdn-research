#include <iostream>
#include "Mdn2d.h"

using namespace std;
using namespace mdn;

int main() {
    Mdn2d m(2);
    m.addValueAt(1, 1, 1);
    cout << "m = " << m.toString() << endl;
    m.carryOver(1, 1); // assuming this exists
    return 0;

}