// Online C++ compiler to run C++ program online
#include <iostream>

using namespace std;

string fn(char in) {
    string ret = "+";
    return ret + in;
}

int main() {
    cout << "fn('h') = [" << fn('h') << "]" << endl;
    {
    int test = 5;
    int k = 10;
    int tens = test / k;
    int ones = test % k;
    cout << "tens=[" << tens << "], ones=[" << ones << "]" << endl;
    }

    {
    int test = 25;
    int k = 10;
    int tens = test / k;
    int ones = test % k;
    cout << "tens=[" << tens << "], ones=[" << ones << "]" << endl;
    }

    return 0;
}