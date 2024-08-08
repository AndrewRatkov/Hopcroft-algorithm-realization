#ifndef DFA_CLASS_H
#define DFA_CLASS_H

#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <cassert>
#include <queue>
#include <cassert>
#include <chrono>

struct request_check{
    bool accepted;
    std::string error;
};

request_check correctness_of_dfa_input(char* command, char* dfa_str);

char integer2char(uint32_t x);
uint32_t char2integer(char c);

// I'll assume that dfa has <= 4294967295 states before minimization
const uint32_t EMPTY_STATE = UINT32_MAX;

// I'm going to color the states (put them in different blocks) sometimes while minimizing the dfa
// I will need the flowing information for each state
struct StateInfo {
    uint32_t color=UINT32_MAX; // when minimising we color the states
    uint32_t next_state_of_same_color=EMPTY_STATE; // number of previous state of the same color
    uint32_t prev_state_of_same_color=EMPTY_STATE; // number of the next state of the same color

    // bool acc=false; // is the current state final or not
};

class DFA{

private:
    uint32_t alphabet_length=0; // length of the alphabet
    uint32_t size=0; // number of states in DFA
    std::vector<std::vector<uint32_t> > delta; // delta function
    std::vector<bool> acc;
    uint32_t starting_node=0;
    std::vector<StateInfo> states_info={}; // information about colors and acc/rej of all states

    // uint32_t k=0; // number of colors
    std::vector<uint32_t> block2first_state_of_this_block={}; // if we color states we need to remember index of first state of each color
    std::vector<std::vector<uint32_t> > block_and_char2first_node_of_this_color_and_char={};

    std::vector<std::vector<bool> > info_L = {};
    std::vector<std::vector<uint32_t> > L={};

    // for each char a in alphabet if state is reachable by a then we need
    // to know the next state of same color which is also reachable by a
    std::vector<std::vector<uint32_t> > next_B_cap={};

    // for each char a in alphabet if state is reachable by a then we need
    // to know the previous state of same color which is also reachable by a
    std::vector<std::vector<uint32_t> > prev_B_cap={};

    // all this is used for reversed_delta
    std::vector<std::vector<uint32_t> > addresses_for_reversed_delta = {}; // each element <= n*I
    std::vector<std::vector<uint32_t> > reversed_delta_lengths = {}; // each element <= n
    std::vector<uint32_t> reversed_delta={}; // each element <= n

    std::vector<std::vector<uint32_t> > B_cap_lengths = {};

    std::vector<bool> blocks_need_to_be_separated={};
    std::vector<uint32_t> block2index_of_new_block={};

    std::vector<uint32_t> sep_blocks={}; // Blocks to separate
    std::vector<uint32_t> sep_states={}; // states which should be separated from the blocks they are in (states, which will change their color)

    std::queue<uint32_t> empty_colors={};
    std::vector<uint32_t> block_lengths={};
    std::vector<uint32_t> block2index_special={};


public:
    void init(uint32_t _alphabet_length, uint32_t _size, uint32_t _starting_node, std::vector<std::vector<uint32_t> > &table, std::vector<bool> &v_acc);


    void print_table();

    bool check_string(std::vector<uint32_t> &str);

    // we can delete unreachable states (for example, at the start of the algorithm)
    void delete_unreachable_states();


    void construct_reversed_delta();


    // When starting we might try to color all states in to colors:
    // acceptable states in one color, rejectable - in another
    // acc is 0; rej is 1
    void color_acc_and_rej_in_2_colors();

    // true, if algorithm terminates
    // false if algorithm is not finished yet
    bool minimize_iteration();

    void minimization(bool no_debug);

    void print_current_classes_of_equality(bool finished, bool debug);

    void print_L();

    void print_B_caps();

    uint32_t get_size() const {
        return size;
    }

    uint32_t get_alphabet_length() const {
        return alphabet_length;
    }

    int save_to_file(char* filename);

    // DFA constructor [works the same way as init method]
    DFA(uint32_t _alphabet_length, uint32_t _size, uint32_t _starting_node, std::vector<std::vector<uint32_t> > &_delta, std::vector<bool> &_v_acc) {
        init(_alphabet_length, _size, _starting_node, _delta, _v_acc);
    }

    // DFA constructor from 2 arguments (for small automata)
    explicit DFA(char* command, char* dfa_str);

    bool operator==(const DFA& other);

    // explicit DFA(std::string &special_type, std::vector<uint32_t> &parameters);

};

#endif
