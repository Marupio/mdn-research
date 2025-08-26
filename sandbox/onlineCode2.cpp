// Online C++ compiler to run C++ program online
#include <iostream>

int main() {
    std::string ref = "        |Mdn2dBase.cpp:598,setValue";
    int i=0,j,k;
    int phase = 0;
    while (i<ref.size()) {
        std::cout << "r[i]=[" << ref[i] << "]" << std::endl;
        if (ref[i++] == '|') {
            std::cout << "got it" << std::endl;
            ++phase;
            break;
        }
    }
    if (phase == 0) {
        std::cout << "bad bad bad" << std::endl;
    }
    j = i;
    while (j<ref.size()) {
        std::cout << "r[j]=[" << ref[j] << "]" << std::endl;
        if (ref[j++] == ':') {
            std::cout << "got it" << std::endl;
            ++phase;
            break;
        }
    }
    if (phase == 1) {
        std::cout << "bad bad bad bad" << std::endl;
    }
    k = j;
    while (k<ref.size()) {
        std::cout << "r[k]=[" << ref[k] << "]" << std::endl;
        if (ref[k++] == ',') {
            std::cout << "got it" << std::endl;
            ++phase;
            break;
        }
    }
    if (phase == 2) {
        std::cout << "bad bad bad bad bad" << std::endl;
    } else {
        std::string result(
            ref.substr(i, j-i) +
            ref.substr(k, ref.size()-k)
        );
        std::cout << "result = [" << result << "]" << std::endl;
    }
//    std::string ref = "        |Mdn2dBase.cpp:598,setValue";
    return 0;
}