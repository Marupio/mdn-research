// Online C++ compiler to run C++ program online
#include <iostream>
#include <string>

int unsafe_alphaToDigit(char c) {
    if (c <= '9' && c >= '0') {
        return int(c - '0');
    }
    return int(c - 'A' + 10);
}

int main() {
    std::string hw = "hello world";
    hw.erase(0, 1);
    std::cout << "hw = [" << hw << "]" << std::endl;
    int size = 10;
    for (int cursor = size-1; cursor >= 0; --cursor) {
        std::cout << "cursor=" << cursor << std::endl;
    }
    std::cout << "\n----- NEXT -----\n" << std::endl;
    int maxIndex = size - 1;
    for (int i = 0; i <= maxIndex; ++i) {
        int pos = maxIndex - i;
        std::cout << i << "," << pos << std::endl;
    }

    std::cout << "\n----- NEXT -----\n" << std::endl;
    std::string test = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZa";
    int line = 0;
    for (char c : test) {
        int ci = unsafe_alphaToDigit(c);
        std::cout << line << ":" << c << " = " << ci << std::endl;
        line++;
    }

    return 0;
}