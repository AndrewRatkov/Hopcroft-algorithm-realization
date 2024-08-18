#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <chrono>
#include "dfa_class.h"
#include "nfa_class.h"



int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << "no input\n";
        return 0;
    } else if (argc < 3) {
        std::cout << "at least 2 parameters required\n";
    }
    
    request_check input_check = correctness_of_dfa_input(argv[1], argv[2]);
    if (!input_check.accepted) {
        std::cout << input_check.error << '\n';
        return 0;
    }

    DFA new_dfa(argv[1], argv[2]); /// ???
    if (strcmp(argv[1], "from_nfa_string") == 0) {
        NFA new_nfa(argv[2]);
        new_dfa = new_nfa.convert2dfa();
    }

    bool need_to_save = false;
    if (argc >= 5 && strcmp(argv[3], "save_to_bin_file") == 0) {
        need_to_save = true;
    }

    bool debug_flag = false; // debug gives some more information about minimizing
    bool time_counter_flag = false;
    bool print_table_at_the_end = true;
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--time") == 0) time_counter_flag = true;
        else if (strcmp(argv[i], "-nd") == 0 || strcmp(argv[i], "--no-debug") == 0) debug_flag = false;
        else if (strcmp(argv[i], "-np") == 0 || strcmp(argv[i], "--no-print") == 0) print_table_at_the_end = false;
    }

    // Start the timer
    auto start = std::chrono::high_resolution_clock::now();

    new_dfa.minimization(debug_flag);

    // // End the timer
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    
    if (time_counter_flag) {
        std::cout << "Execution time: " << duration.count() << " seconds." << std::endl;
    }

    if (print_table_at_the_end) new_dfa.print_table();

    if (need_to_save) {
        int saving = new_dfa.save_to_file(argv[4]);
        std::cout << (saving == 0 ? "Saved successfully to binary file" : "Error happened when saving") << '\n';
    }

    return 0;
}
