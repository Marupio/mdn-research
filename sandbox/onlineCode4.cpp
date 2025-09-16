// Online C++ compiler to run C++ program online
#include <iostream>
#include <string>
#include <sstream>

class Dummy {};

using namespace std;

int main() {
    stringstream ss;
    string s = "Test";
    int i = -8;
    ss << "{" << s << ", " << s << "}-(" << i << ")";
    cout << ss.str() << endl;
    char clsw, ccom, crsw, cneg, clpr, crpr;
    string sa, sb;
    int ia;
    ss >> clsw;
    char l;
    ss >> l;
    do {
        sa += l;
        ss >> l;
    } while (l != ',');

    ss >> l;
    do {
        sb += l;
        ss >> l;
    } while (l != '}');

    ss >> cneg >> clpr >> ia >> crpr;

    cout << "clsw: [" << clsw << "]" << endl;
    cout << "sa: [" << sa << "]" << endl;
    cout << "ccom: [,] (forced)" << endl;
    cout << "sb: [" << sb << "]" << endl;
    cout << "crsw: [" << crsw << "]" << endl;
    cout << "cneg: [" << cneg << "]" << endl;
    cout << "clpr: [" << clpr << "]" << endl;
    cout << "ia: [" << ia << "]" << endl;
    cout << "crpr: [" << crpr << "]" << endl;


    return 0;
}