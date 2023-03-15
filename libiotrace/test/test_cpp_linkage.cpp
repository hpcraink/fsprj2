/**
 * C++ program 4 testing linkage w/ libiotrace
 * (generates random output which should be traceable via libiotrace)
 */
#include <random>
#include <map>

#include <iostream>
#include <cstdio>


int main() {
    (void)printf("C++ linkage test\n\n");


    std::random_device rand;
    std::uniform_int_distribution<int> dist(0, 9);

    std::map<int, int> hist;
    for (int n = 0; n != 20000; ++n) {
        ++hist[dist(rand)];
    }

    for (auto [x, y] : hist) {
        std::cout << x << " : " << std::string(y / 100, '*') << '\n';
    }


    return 0;
}
