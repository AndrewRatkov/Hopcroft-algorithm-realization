#ifndef NFA_CLASS_H
#define NFA_CLASS_H
#include "dfa_class.h"

class NFA {

private:
    uint32_t alphabet_length=0;
    uint32_t size=0;
    std::vector<std::vector<std::vector<uint32_t> > > delta; // Q x \sigma -> 2^Q
    std::vector<uint32_t> starting_nodes;
    std::vector<bool> v_acc;

public:
    NFA(uint32_t _alphabet_length, uint32_t _size, std::vector<std::vector<std::vector<uint32_t> > > &_delta, std::vector<uint32_t> &_starting_nodes, std::vector<bool> &_v_acc) {
        init(_alphabet_length, _size, _delta, _starting_nodes, _v_acc);
    }

    explicit NFA(char* s);

    void init(uint32_t _alphabet_length, uint32_t _size, std::vector<std::vector<std::vector<uint32_t> > > &_delta, std::vector<uint32_t> &_starting_nodes, std::vector<bool> &_v_acc);

    void print();

    DFA convert2dfa(); // O(2^n * n^2)
    
    uint32_t get_size() const {
        return size;
    }

    bool line_complicated_initial_states() {
        return starting_nodes.size() >= 2 || starting_nodes[0] != 0;
    }

    char state_to_printable_character(int x) {
        if (x < 10) {
            return '0' + (char)x;
        } else if (x < 10 + 26) {
            return 'a' + (char)(x - 10);
        } else {
            return 'A' + (char)(x - 36);
        }
    }

    std::string longline() {
        std::string s = "";
        s += std::to_string(size) + ":" + std::to_string(alphabet_length) + ":";
        for (uint32_t state : starting_nodes) {
            s += std::to_string(state) + " ";
        }
        s += ":";
        for (uint32_t i = 0; i < size; i++) {
            for (uint32_t a = 0; a < alphabet_length; a++) {
                for (size_t idx = 0; idx < delta[i][a].size(); idx++) {
                    s += std::to_string(delta[i][a][idx]) + " ";
                }
                s += ":";
            }
        }
        return s;
    }

    std::string line() {
        if (size > 62) {
            return longline();
        }

        bool complicated_initial_states = line_complicated_initial_states();
        std::vector<bool> v_init(size, false);
        for (uint32_t state : starting_nodes) {
            v_init[state] = true;
        }
        std::string w;

        for (uint32_t i = 0; i < size; i++) {
                if (complicated_initial_states && v_init[i]) {
                    w += '>';
                }
            for (uint32_t a = 0; a < alphabet_length; a++) {
                if (delta[i][a].size() == 1) {
                    w += state_to_printable_character(delta[i][a][0]);
                } else {
                    w += '{';
                    for (uint32_t t = 0; t < delta[i][a].size(); t++) {
                        w += state_to_printable_character(delta[i][a][t]);
                    }
                    w += '}';
                }
            }
        }

        for (uint32_t i = 0; i < size; i++) {
            w += (v_acc[i] ? '+' : '-');
        }

        return w;
    }

};


#endif
