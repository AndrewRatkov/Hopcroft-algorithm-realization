#ifndef DFA_CLASS_H
#define DFA_CLASS_H

#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <cassert>
#include <queue>
#include <unordered_map>
#include <cassert>
#include <chrono>

struct request_check{
    bool accepted;
    std::string error;
};

request_check correctness_of_dfa_input(char* command, char* dfa_str);

char integer2char(const uint32_t x);
uint32_t char2integer(const char c);

// I'll assume that dfa has <= 4294967295 states before minimization
const uint32_t EMPTY_STATE = UINT32_MAX;

// I'm going to color the states (put them in different blocks) sometimes while minimizing the dfa
// I will need the flowing information for each state
struct StateInfo {
    uint32_t block=UINT32_MAX; // when minimising we color the states
    uint32_t next_state_of_same_block=EMPTY_STATE; // number of previous state of the same color
    uint32_t prev_state_of_same_block=EMPTY_STATE; // number of the next state of the same color
};

class DFA{

private:
    uint32_t alphabet_length=0; // length of the alphabet
    uint32_t size=0; // number of states in DFA
    std::vector<std::vector<uint32_t> > delta; // delta function
    std::vector<bool> acc;
    uint32_t starting_node=0;

    std::vector<StateInfo> states_info={}; // information about colors and acc/rej of all states

    std::vector<uint32_t> block2first_state_in_it={}; // if we color states we need to remember index of first state of each color
    std::vector<std::vector<uint32_t> > block_and_char2first_B_cap={};

    std::vector<std::vector<bool> > info_L = {};
    std::queue<std::pair<uint32_t, uint32_t> > L={};

    // for each char a in alphabet if state is reachable by a then we need
    // to know the next state of same color which is also reachable by a
    std::vector<std::vector<uint32_t> > next_B_cap={};

    // for each char a in alphabet if state is reachable by a then we need
    // to know the previous state of same color which is also reachable by a
    std::vector<std::vector<uint32_t> > prev_B_cap={};

    // all this is used for reversed_delta
    std::vector<std::vector<uint32_t> > addresses_for_reversed_delta = {}; // each element <= n*I
    std::vector<uint32_t> reversed_delta={}; // each element <= n

    std::vector<std::vector<uint32_t> > B_cap_lengths = {};

    std::vector<uint32_t> sep_blocks={}; // Blocks to separate
    std::vector<uint32_t> sep_states={}; // states which should be separated from the blocks they are in (states, which will change their block)

    std::vector<uint32_t> block_lengths={};

    bool deleted_unreachable_states = false;
    bool constructed_reversed_delta = false;
    bool minimized = false;

    struct info {
        uint32_t states2extract;
        uint32_t new_color;
    };

    std::unordered_map<uint32_t, info> blocks_info;

    uint32_t colors=0;

    uint32_t next_address_for_reversed_delta(const uint32_t& a, const uint32_t& state) {
        assert(a < this->alphabet_length && state < this->size);
        if (state < this->size - 1) { 
            return addresses_for_reversed_delta[a][state + 1];
        } else if (a < this->alphabet_length - 1) {
            return addresses_for_reversed_delta[a + 1][0];
        } else { // (a, state) is the last pair
            return this->alphabet_length * this->size;
        }
    }

    uint32_t get_reversed_delta_length(const uint32_t& a, const uint32_t& state) {
        return next_address_for_reversed_delta(a, state) - addresses_for_reversed_delta[a][state];
    }

    void extract_state_to_new_block(const uint32_t s, const uint32_t new_block);


public:
    void init(uint32_t _alphabet_length, uint32_t _size, uint32_t _starting_node, std::vector<std::vector<uint32_t> > &table, std::vector<bool> &v_acc);

    void print_table() const;

    bool check_string(std::vector<uint32_t> &str) const;

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

    void print_current_classes_of_equality() const;

    uint32_t get_size() const noexcept {
        return this->size;
    }

    uint32_t get_alphabet_length() const noexcept {
        return this->alphabet_length;
    }

    uint32_t get_starting_node() const noexcept {
        return this->starting_node;
    }

    int save_to_file(char* filename) const;

    // DFA constructor [works the same way as init method]
    DFA(uint32_t _alphabet_length, uint32_t _size, uint32_t _starting_node, std::vector<std::vector<uint32_t> > &_delta, std::vector<bool> &_v_acc) {
        init(_alphabet_length, _size, _starting_node, _delta, _v_acc);
    }

    // DFA constructor from 2 arguments (for small automata)
    explicit DFA(char* command, char* dfa_str);

    bool operator==(const DFA& other) const;

    // explicit DFA(std::string &special_type, std::vector<uint32_t> &parameters);

};

#endif
