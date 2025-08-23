// Online C++ compiler to run C++ program online
#include <iostream>

using namespace std;

int* mrSix = new int(6);
int* fn() {
    static int count = 0;
    count += 1;
    if (count > 3) {
        return nullptr;
    }
    return mrSix;
}

int main() {
    for (int i = 0; i < 5; ++i) {
        if (int* s = fn()) {
            cout << "It's all good, s=" << (*s) << endl;
        } else {
            cout << "No good, man!" << endl;
        }
    }
   return 0;
}