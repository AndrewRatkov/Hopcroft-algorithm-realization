#include "nfa_class.h"

NFA::NFA(char* s) { // I assume that string s is valid
    char* ptr = s;
    uint32_t blocks = 0;
    bool opened_bracket = false;
    bool exists_initial_sign = false;

    while (*ptr != '-' && *ptr != '+') {
        if (*ptr == '{') opened_bracket = true;
        else if (*ptr == '}') opened_bracket = false;
        else if (*ptr == '>') exists_initial_sign = true;
        if (!opened_bracket && *ptr != '>') blocks++;
        ptr++;
    }

    uint32_t _size = 0;
    char* end_ptr = ptr;
    while (*ptr != '\0') {_size++; ptr++;}


    uint32_t _alphabet_length = blocks / _size;
    std::vector<bool> _v_acc(_size);
    for (uint32_t i = 0; i < _size; i++) _v_acc[i] = (end_ptr[i] == '+');

    std::vector<uint32_t> _starting_nodes={};
    std::vector<std::vector<std::vector<uint32_t> > > _delta(_size, std::vector<std::vector<uint32_t> >(_alphabet_length, std::vector<uint32_t>(0)));

    ptr = s;
    int read_blocks = 0; // how many blocks are already read
    opened_bracket = false;
    while (*ptr != '-' && *ptr != '+') {
        if (*ptr == '>') _starting_nodes.push_back(read_blocks / _alphabet_length);
        else if (*ptr == '{') opened_bracket = true;
        else if (*ptr == '}') opened_bracket = false;
        if (*ptr != '{' && *ptr != '>') {
            if (*ptr != '}') {
                _delta[read_blocks / _alphabet_length][read_blocks % _alphabet_length].push_back(char2integer(*ptr));
            }
            if (!opened_bracket) read_blocks++;
        }
        ptr++;
    }    

    if (!exists_initial_sign) {
        _starting_nodes.push_back(0);
    }

    init(_alphabet_length, _size, _delta, _starting_nodes, _v_acc);
}

void NFA::init(uint32_t _alphabet_length, uint32_t _size, std::vector<std::vector<std::vector<uint32_t> > > &_delta, std::vector<uint32_t> &_starting_nodes, std::vector<bool> &_v_acc) {
    alphabet_length = _alphabet_length;
    size = _size;
    starting_nodes = _starting_nodes;
    for (uint32_t starting_node : starting_nodes) assert(starting_node != EMPTY_STATE && starting_node < size);
    delta = _delta;
    assert(delta.size() == size);
    for(uint32_t node = 0; node < size; ++node) {
        assert(delta[node].size() == alphabet_length);
        for (uint32_t a = 0; a < alphabet_length; ++a) {
            for (uint32_t next_node: delta[node][a]) assert(next_node != EMPTY_STATE && next_node < size);
        }
    }
    v_acc = _v_acc;
    assert(v_acc.size() == size);
}

void NFA::print() {
    std::cout << "alp_size = " << alphabet_length << "; size = " << size << '\n';
    std::cout << "starting_nodes:";
    for (uint32_t starting_node: starting_nodes) { std::cout << starting_node << ' '; }
    std::cout << '\n';
    for (uint32_t node = 0; node < size; ++node) {
        std::cout << node << ":: ";
        for (uint32_t a = 0; a < alphabet_length; ++a) {
            std::cout << "(" << a << ":";
            for (uint32_t next_node : delta[node][a]) std::cout << ' ' << next_node;
            std::cout << ") ";
        }
        std::cout << '\n';
    }
}

DFA NFA::convert2dfa() { // O(2^n * n^2)
    assert(size <= 31); // I'm not sure that more than 2^31 nodes will be okay
    uint32_t dfa_size = (1 << size);
    std::vector<std::vector<uint32_t> > dfa_delta(alphabet_length, std::vector<uint32_t>(dfa_size));

    uint32_t dfa_starting_node = 0;
    for (uint32_t node: starting_nodes) {
        assert(!((dfa_starting_node >> node) & 1));
        dfa_starting_node += (1 << node);
    }

    for (uint32_t node = 0; node < dfa_size; ++node) {
        for (uint32_t a = 0; a < alphabet_length; ++a) {
            for (uint32_t nfa_node = 0; nfa_node < size; ++nfa_node) {
                if (node & (1 << nfa_node)) {
                    for (auto next_nfa_node : delta[nfa_node][a]) {dfa_delta[a][node] |= (1 << next_nfa_node);}
                }
            }
        }
    }

    std::vector<bool> dfa_v_acc(dfa_size, false);
    for (uint32_t nfa_node = 0; nfa_node < size; ++nfa_node) {
        if (v_acc[nfa_node]) {
            for (uint32_t node = 0; node < dfa_size; ++node) {
                if (node & (1 << nfa_node)) {
                    dfa_v_acc[node] = true;
                }
            }
        }
    }

    return DFA(alphabet_length, dfa_size, dfa_starting_node, dfa_delta, dfa_v_acc);
}
