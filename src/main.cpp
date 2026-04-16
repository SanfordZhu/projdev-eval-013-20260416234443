#include "map.hpp"
#include <iostream>

using namespace sjtu;

int main() {
    map<int, int> m;
    m[1] = 10;
    m[2] = 20;
    m[3] = 30;
    
    std::cout << m[1] << " " << m[2] << " " << m[3] << std::endl;
    
    return 0;
}
