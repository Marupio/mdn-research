#include <iostream>
#include <string>
#include <cctype>
#include <cstdlib>

int main() {
    std::string working = "10123";
    int nChar = working.size();
    if (!isdigit(working[nChar-1])) {
        std::cout << "Early exit, none found" << std::endl;
        return 0;
    }
    int cursor = nChar - 1;
    while (cursor >= 0 && isdigit(working[cursor])) {cursor--;}
    cursor++;
    std::cout << "cursor = " << cursor << std::endl;
    std::string substr = working.substr(cursor, nChar - cursor);
    std::cout << "working.substr(" << cursor << ", " << (nChar-cursor) << ")" << std::endl;
    int val = std::stoi(substr);
    std::cout << "working=[" << working << "], cursor=" << cursor << ", substr=[" << substr << "], int=" << val << std::endl;
    return 0;
}