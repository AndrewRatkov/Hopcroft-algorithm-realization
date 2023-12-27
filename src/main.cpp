#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <queue>
#include <cassert>
#include <chrono>
#include "dfa_class.h"
#include "nfa_class.h"





int main() {
    // Start the timer
    auto start = std::chrono::high_resolution_clock::now();


    uint32_t _size = 1000000;
    std::vector<std::vector<uint32_t> > _delta(1, std::vector<uint32_t>(_size));
    std::vector<bool> _v_acc(_size, false);
    _v_acc[_size - 1] = true;
    for (uint32_t s = 0; s < _size; s++) {
        _delta[0][s] = (s == _size - 1 ? s : s + 1);
    }
    DFA new_dfa(1, _size, 0, _delta, _v_acc);
    // std::string d = "00011111_1234353145742654";
    // DFA new_dfa(d);
    new_dfa.minimization();

    // End the timer
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Execution time: " << duration.count() << " seconds." << std::endl;
//    new_dfa.print_table();
    return 0;
}
