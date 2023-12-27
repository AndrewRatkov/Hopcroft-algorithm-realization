#include "nfa_class.h"

NFA::NFA(std::string &s) {
    uint32_t _size, idx=0, underlines=0;
    std::vector<bool> _v_acc = {};
    std::vector<uint32_t> _starting_nodes={0};
    while (s[idx] != '_') {
        _v_acc.push_back(s[idx] == '1');
        ++idx;
    }
    _size = idx;
    for (char c: s) underlines += (c == '_');
    uint32_t _alphabet_length = underlines / _size;
    std::cout << _size << ' ' << _alphabet_length << '\n'; // need to be commented???
    std::vector<std::vector<std::vector<uint32_t> > > _delta(_size, std::vector<std::vector<uint32_t> >(_alphabet_length, std::vector<uint32_t>(0)));
    uint32_t q=0, a=0;
    while (idx < s.size() - 1) {
        ++idx;
        if (s[idx] == '_') {
            ++a;
            if (a == _alphabet_length) {
                a = 0;
                ++q;
            }
        }
        else {
            _delta[q][a].push_back((int)(s[idx] - '0'));
        }
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
    assert(size <= 30); // I'm not sure that more than 2^30 nodes will be okay
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
