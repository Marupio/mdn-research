// Online C++ compiler to run C++ program online
#include <iostream>
#include <string>

int main() {
    enum class FrontBack {
        Behind,
        BackEdge,
        Inside,
        FrontEdge,
        InFront
    };

    for (int i = 0; i < 5; ++i) {
        FrontBack ia = static_cast<FrontBack>(i);
        for (int j = 0; j < 5; ++j) {
            FrontBack ja = static_cast<FrontBack>(j);
            switch(ja) {
                case FrontBack::Behind: {
                    std::cout << "Behind, ";
                    break;
                }
                case FrontBack::BackEdge: {
                    std::cout << "BackEdge, ";
                    break;
                }
                case FrontBack::Inside: {
                    std::cout << "Inside, ";
                    break;
                }
                case FrontBack::FrontEdge: {
                    std::cout << "FrontEdge, ";
                    break;
                }
                case FrontBack::InFront: {
                    std::cout << "InFront, ";
                    break;
                }
            }
            switch(ia) {
                case FrontBack::Behind: {
                    std::cout << "Behind" << std::endl;
                    break;
                }
                case FrontBack::BackEdge: {
                    std::cout << "BackEdge" << std::endl;
                    break;
                }
                case FrontBack::Inside: {
                    std::cout << "Inside" << std::endl;
                    break;
                }
                case FrontBack::FrontEdge: {
                    std::cout << "FrontEdge" << std::endl;
                    break;
                }
                case FrontBack::InFront: {
                    std::cout << "InFront" << std::endl;
                    break;
                }
            }
        }
    }

    return 0;
}