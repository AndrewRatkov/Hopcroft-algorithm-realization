#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <chrono>
#include "dfa_class.h"
#include "nfa_class.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cout << "argc must be 5!\n";
        return 0;
    }

    DFA new_dfa1(argv[1], argv[2]); /// ???
    if (strcmp(argv[1], "from_nfa_string") == 0) {
        NFA new_nfa1(argv[2]);
        new_dfa1 = new_nfa1.convert2dfa();
    }
    new_dfa1.minimization(true);

    DFA new_dfa2(argv[3], argv[4]); /// ???
    if (strcmp(argv[3], "from_nfa_string") == 0) {
        NFA new_nfa2(argv[4]);
        new_dfa2 = new_nfa2.convert2dfa();
    }
    new_dfa2.minimization(true);

    if (new_dfa1 == new_dfa2) std::cout << "equal!\n";
    else std::cout << "different!\n";
    return 0;
}
