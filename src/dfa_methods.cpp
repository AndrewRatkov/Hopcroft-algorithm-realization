#include "dfa_class.h"

void DFA::init(uint32_t _alphabet_length, uint32_t _size, uint32_t _starting_node, std::vector<std::vector<uint32_t> > &table, std::vector<bool> &v_acc) {
    // size, alphabet length and index of starting are just given
    this->size = _size;
    this->alphabet_length = _alphabet_length;
    this->starting_node = _starting_node;

    // before we get the table for delta, we need to check that its sizes are size * alphabet_length
    assert(table.size() == _alphabet_length);
    for (uint32_t i = 0; i < _alphabet_length; ++i) {
        assert(table[i].size() == _size);
    }
    this->delta = table;

    // getting information about the states: acc or rej they are
    assert(v_acc.size() == size);
    states_info.assign(size, {UINT32_MAX, EMPTY_STATE, EMPTY_STATE});
    this->acc.assign(size, false);
    for (uint32_t i = 0; i < size; ++i) this->acc[i] = v_acc[i];

    // we don't need to color anything at the first time
    // k=0; // so number of colors is 0
    block2first_state_of_this_block={};
    block_and_char2first_node_of_this_color_and_char={};

    info_L={};
    L.assign(alphabet_length, {});

    addresses_for_reversed_delta={};
    reversed_delta_lengths = {};
    reversed_delta={};

    B_cap_lengths ={};
    blocks_need_to_be_separated={};
    block2index_of_new_block={};
    block2index_special={};

    sep_blocks = {}; // Blocks to separate
    sep_states = {}; // states which should be separated from the blocks they are in (states, which will change their color)

    empty_colors={};
    block_lengths={};

    next_B_cap={};
    prev_B_cap={};
}

void DFA::print_table() const {
    std::cout << "SIZE: " << size << "  LEN_ALPHABET: " << alphabet_length << " STARTING_NODE: " << starting_node << '\n';
    if (size > 50) { std::cout << "Too big dfa to print in stdout\n"; return; }
    uint32_t spaces_per_cell = 1;
    uint32_t max_state_amount = 10;
    while (max_state_amount < size) {
        spaces_per_cell++; max_state_amount *= 10;
    }
    spaces_per_cell += 2;
    if (spaces_per_cell < 4) spaces_per_cell =  4;

    // left upper corner --- empty
    std::cout << std::string(spaces_per_cell, ' ');

    for (uint32_t i=0; i<alphabet_length; ++i) {
        // counting spaces to write
        uint32_t i_copy = i;
        uint32_t spaces = spaces_per_cell - 2;
        while (i_copy >= 10) {i_copy /= 10; spaces--;}
        std::cout << "|" << std::string(spaces, ' ') << i << ' ';
    }
    std::cout << "| TYPE \n";
    std::cout << std::string(spaces_per_cell*(alphabet_length+1) + alphabet_length+7, '=') << '\n';

    for (uint32_t node=0; node<size; ++node) {
        // counting spaces to write
        uint32_t state_copy = node;
        uint32_t spaces_first_column = spaces_per_cell - 2;
        while (state_copy >= 10) {state_copy /= 10; spaces_first_column--;}
        std::cout << std::string(spaces_first_column, ' ') << node << ' ';

        for (uint32_t i = 0; i<alphabet_length; ++i) {
            // counting spaces to write
            uint32_t next_copy = delta[i][node];
            uint32_t spaces = spaces_per_cell - 2;
            while (next_copy >= 10) {next_copy /= 10; spaces--;}
            std::cout << "|" << std::string(spaces, ' ') << delta[i][node] << ' ';
        }
        std::cout << "| " << (acc[node] ? "ACC" : "REJ") << '\n';
        std::cout << std::string(spaces_per_cell*(alphabet_length+1) + alphabet_length+7, '=') << '\n';
    }
}



bool DFA::check_string(std::vector<uint32_t> &str) const {
        uint32_t q_cur = starting_node;
        for (uint32_t c: str) {
            q_cur = delta[c][q_cur];
        }
        return acc[q_cur];
}

void DFA::delete_unreachable_states() {
    // we will color all states in 3 colors:
    // 0 if it isn't visited yet
    // 1 if it is visited, but it's neighbours
    // (states where we can go from current state) are not visited yet
    // 2 if it is already visited and it's neighbours too
    std::vector<char> colors(size, 0);
    std::queue<uint32_t> q; // queue of states of color 1. When it becomes empty the algorithm finishes
    colors[starting_node] = 1;
    q.push(starting_node);


    uint32_t new_size = 0; // size of DFA after deleting unreachable states = number of states popped from q
    while (!q.empty()) {
        uint32_t cur_node = q.front();
        q.pop();
        colors[cur_node] = 2; ++new_size;
        for (uint32_t i = 0; i < alphabet_length; ++i) {
            if (colors[delta[i][cur_node]] == 0) { 
                colors[delta[i][cur_node]] = 1;
                q.push(delta[i][cur_node]);
            }
        }
    }

    if (new_size == size) return; // we have nothing to delete

    std::vector<uint32_t> node2new_idx(size); // each state will have new index
    uint32_t cur = 0;
    for (uint32_t i = 0; i < size; ++i) {
        if (colors[i] == 2) {
            node2new_idx[i] = cur;
            cur++;
        }
    }

    // new delta function and in
    std::vector<std::vector<uint32_t> > new_delta(alphabet_length, std::vector<uint32_t>(new_size));
    std::vector<bool> new_v_acc(new_size);


    for (uint32_t i = 0; i < size; ++i) {
        if (colors[i] == 2) { // if state is visited
            new_v_acc[node2new_idx[i]] = acc[i];
            for (uint32_t a=0; a < alphabet_length; a++) {
                new_delta[a][node2new_idx[i]] = node2new_idx[delta[a][i]];
            }
        }
    }

    // rewrite new DFA
    init(alphabet_length, new_size, node2new_idx[starting_node], new_delta, new_v_acc);

}


void DFA::construct_reversed_delta() {
    // reversed delta will be a vector of size*alphabet_length (instead of vector<vector<vector>>>)
    // for each state s and char a reversed_delta_lengths[a][s] is number of states t: delta(t, a) = s
    // reversed_delta[addresses_for_reversed_delta[a][s]], reversed_delta[addresses_for_reversed_delta[a][s] + 1], ...
    // ..., reversed_delta[addresses_for_reversed_delta[a][s] + reversed_delta_lengths[a][s] - 1] are all states t
    // such as delta(t, a) = s
    addresses_for_reversed_delta = std::vector<std::vector<uint32_t> >(alphabet_length, std::vector<uint32_t>(size, 0));
    reversed_delta_lengths = std::vector<std::vector<uint32_t> >(alphabet_length, std::vector<uint32_t>(size, 0));
    reversed_delta.assign(size * alphabet_length, 0);

    for (uint32_t s = 0; s < size; s++) {
        for (uint32_t a = 0; a < alphabet_length; a++) {
            reversed_delta_lengths[a][delta[a][s]]++;
        }
    }

    for (uint32_t s = 0; s < size; s++) {
        for (uint32_t a = 0; a < alphabet_length; a++) {
            if (!a) { // a == 0
                if (!s) continue; // s == 0
                else {
                    addresses_for_reversed_delta[a][s] = addresses_for_reversed_delta[alphabet_length - 1][s - 1] + reversed_delta_lengths[alphabet_length - 1][s - 1];
                }
            } else {
                addresses_for_reversed_delta[a][s] = addresses_for_reversed_delta[a - 1][s] + reversed_delta_lengths[a - 1][s];
            }
        }
    }

    std::vector<std::vector<uint32_t> > addresses_for_put = addresses_for_reversed_delta;

    for (uint32_t s = 0; s < size; s++) {
        for (uint32_t a = 0; a < alphabet_length; a++) {
            uint32_t address_to_put = addresses_for_put[a][delta[a][s]];
            reversed_delta[address_to_put] = s;
            addresses_for_put[a][delta[a][s]]++;
        }
    }

// // debug for reversed delta
//        std::cout << "DECODE REVERSED_DELTA: (for small DFAs)\n";
//        for (uint32_t s = 0; s < size; s++) {
//            for (uint32_t a = 0; a < alphabet_length; a++) {
//                std::cout << "s: " << s << ", a: " << a << '\n';
//                uint32_t indexer = addresses_for_reversed_delta[a][s];
//                for (uint32_t i = 0; i < reversed_delta_lengths[a][s]; i++) {
//                    std::cout << reversed_delta[indexer + i] << ' '; // indexer + 0LL + i
//                }
//                std::cout << '\n';
//            }
//        }

}


void DFA::color_acc_and_rej_in_2_colors() {
    // size >= 2
    next_B_cap=std::vector<std::vector<uint32_t> >(alphabet_length, std::vector<uint32_t>(size, EMPTY_STATE));
    prev_B_cap=std::vector<std::vector<uint32_t> >(alphabet_length, std::vector<uint32_t>(size, EMPTY_STATE));
    
    block2first_state_of_this_block.assign(size, EMPTY_STATE); // 0 --> EMPTY_STATE; 1 --> EMPTY_STATE
    block_and_char2first_node_of_this_color_and_char.assign(alphabet_length, std::vector<uint32_t>(size, EMPTY_STATE));

    // for color v and char a B_cap_lengths[a][v] is number of states s,
    // such as s is colored in v-th color and is reachable by symbol a (exists state t: delta(t, a) = s)
    B_cap_lengths = std::vector<std::vector<uint32_t> >(alphabet_length, std::vector<uint32_t>(size, 0));

    uint32_t last_acc = EMPTY_STATE; uint32_t last_rej = EMPTY_STATE; // last acc and rej states during iteration
    block_lengths.assign(size, 0);

    std::vector<uint32_t> last0(alphabet_length, EMPTY_STATE), last1(alphabet_length, EMPTY_STATE); // for B_cap(B(0), a) and B_cap(B(1), a) for all a in alphabet
    for (uint32_t s = 0; s < size; ++s) {
        if (acc[s]) {
            block_lengths[0]++;
            for (uint32_t a = 0; a < alphabet_length; a++) {
                if (reversed_delta_lengths[a][s]) { // possible to get to s by a: reversed_delta_lengths[a][s] > 0
                    B_cap_lengths[a][0]++; // counter of acc states in which it's possible to get with a
                    if (block_and_char2first_node_of_this_color_and_char[a][0] == EMPTY_STATE) {
                        block_and_char2first_node_of_this_color_and_char[a][0] = s;
                        prev_B_cap[a][s] = EMPTY_STATE;
                    } else {
                        next_B_cap[a][last0[a]] = s;
                        prev_B_cap[a][s] = last0[a];
                    }
                    last0[a] = s;
                }
            }

            if (last_acc == EMPTY_STATE) {
                block2first_state_of_this_block[0] = s;
                states_info[s].prev_state_of_same_color = EMPTY_STATE;
            }
            else {
                states_info[last_acc].next_state_of_same_color = s;
                states_info[s].prev_state_of_same_color = last_acc;
            }
            states_info[s].color = 0; // acc have color 0
            last_acc = s;
        } else {
            
            block_lengths[1]++;
            for (uint32_t a = 0; a < alphabet_length; a++) {
                if (reversed_delta_lengths[a][s]) { // possible to get to s by a: reversed_delta_lengths[a][s] > 0
                    B_cap_lengths[a][1]++;
                    if (block_and_char2first_node_of_this_color_and_char[a][1] == EMPTY_STATE) {
                        block_and_char2first_node_of_this_color_and_char[a][1] = s;
                        prev_B_cap[a][s] = EMPTY_STATE;
                        
                    } else {
                        next_B_cap[a][last1[a]] = s;
                        prev_B_cap[a][s] = last1[a];
                    }
                    last1[a] = s;
                }
            }
            if (last_rej == EMPTY_STATE) {
                block2first_state_of_this_block[1] = s;
                states_info[s].prev_state_of_same_color = EMPTY_STATE;
            }
            else {
                states_info[last_rej].next_state_of_same_color = s;
                states_info[s].prev_state_of_same_color = last_rej;
            }
            states_info[s].color = 1;
            last_rej = s;
        }
    }


    if (last_acc != EMPTY_STATE) states_info[last_acc].next_state_of_same_color = EMPTY_STATE; // at least one acc found
    if (last_rej != EMPTY_STATE) states_info[last_rej].next_state_of_same_color = EMPTY_STATE; // at least one rej found
    
    info_L = std::vector<std::vector<bool> >(alphabet_length, std::vector<bool>(size, false));
    for (uint32_t a = 0; a < alphabet_length; a++) {
        if (B_cap_lengths[a][0] <= B_cap_lengths[a][1]) {L[a].push_back(0); info_L[a][0] = true;}
        else {L[a].push_back(1); info_L[a][1] = true; }
    }
    
    blocks_need_to_be_separated.assign(size, false);
    block2index_of_new_block.assign(size, EMPTY_STATE);
    block2index_special.assign(size, EMPTY_STATE);

    for (uint32_t i = 2; i < size; ++i) {
        empty_colors.push(i);
    }

}


bool DFA::minimize_iteration() {
    if (empty_colors.empty()) return true; // number of blocks == size => nothing to minimize

    // finding a and i such as i-th class is in L[a]
    uint32_t a = 0;
    while (a < alphabet_length && L[a].empty()) {
        a++;
    }
    if (a == alphabet_length) return true; // algorithm terminates (all L[a] are empty)

    // extracting i
    uint32_t i = L[a].back();
    info_L[a][i] = false;
    L[a].pop_back();

    uint32_t state_i = block_and_char2first_node_of_this_color_and_char[a][i];
    uint32_t added_blocks = 0;
    while (state_i != EMPTY_STATE) {
        for (uint32_t t = addresses_for_reversed_delta[a][state_i]; t < addresses_for_reversed_delta[a][state_i] + reversed_delta_lengths[a][state_i]; t++) {
            uint32_t sep_state = reversed_delta[t]; // delta(state, a) in B(i)
            uint32_t sep_state_color = states_info[sep_state].color;


            if (block_lengths[sep_state_color] == 1) { // if block consists only from 1 state, we don't need to separate it
                continue;
            }

            sep_states.push_back(sep_state);
            if (!blocks_need_to_be_separated[sep_state_color]) {
                blocks_need_to_be_separated[sep_state_color] = true;
                sep_blocks.push_back(sep_state_color);
                block2index_of_new_block[sep_state_color] = empty_colors.front();
                empty_colors.pop();
                block2index_special[sep_state_color] = added_blocks;
                added_blocks++;
            }
        }
        state_i = next_B_cap[a][state_i];
    }

    //std::cout << "BLOCKS_TO_SEPARATE:\n";
    //for (auto color: colors_j) std::cout << color << ' ';
    //std::cout << '\n';

    //std::cout << "sep_states: ";
    //for (auto sep_st: sep_states) std::cout << sep_st << ' ';
    //std::cout << '\n';
    //std::cout << " added_blocks: " << added_blocks << '\n';

    std::vector<std::vector<uint32_t> > last_states_with_new_color_and_char(alphabet_length, std::vector<uint32_t>(added_blocks, EMPTY_STATE)); // reformat!!!
    std::vector<uint32_t> last_states_of_new_blocks(added_blocks, EMPTY_STATE);

    for (auto t: sep_states) {
        uint32_t new_color = block2index_of_new_block[states_info[t].color];
        uint32_t old_color = states_info[t].color;
        
        block_lengths[new_color]++;
        block_lengths[old_color]--;
        if (block_lengths[old_color] == 0) empty_colors.push(old_color);
        

        if (states_info[t].prev_state_of_same_color == EMPTY_STATE) {
            block2first_state_of_this_block[old_color] = states_info[t].next_state_of_same_color;
        } else {
            states_info[states_info[t].prev_state_of_same_color].next_state_of_same_color = states_info[t].next_state_of_same_color;
        }

        if (states_info[t].next_state_of_same_color != EMPTY_STATE) {
            states_info[states_info[t].next_state_of_same_color].prev_state_of_same_color = states_info[t].prev_state_of_same_color;
        }
        
        uint32_t new_block_number = block2index_special[old_color];
        states_info[t].next_state_of_same_color = EMPTY_STATE;
        states_info[t].prev_state_of_same_color = last_states_of_new_blocks[new_block_number];
        
        if (last_states_of_new_blocks[new_block_number] == EMPTY_STATE) {
            block2first_state_of_this_block[new_color] = t;
        } else {
            states_info[last_states_of_new_blocks[new_block_number]].next_state_of_same_color = t;
        }
        
        last_states_of_new_blocks[new_block_number] = t;
        states_info[t].color = new_color;

        
        for (uint32_t c = 0; c < alphabet_length; c++) {
            if (reversed_delta_lengths[c][t] > 0) { // we can get to t by c

                if (prev_B_cap[c][t] == EMPTY_STATE) {
                    block_and_char2first_node_of_this_color_and_char[c][old_color] = next_B_cap[c][t];
                } else {
                    next_B_cap[c][prev_B_cap[c][t]] = next_B_cap[c][t];
                }

                if (next_B_cap[c][t] != EMPTY_STATE) {
                    prev_B_cap[c][next_B_cap[c][t]] = prev_B_cap[c][t];
                }

                next_B_cap[c][t] = EMPTY_STATE;
                prev_B_cap[c][t] = last_states_with_new_color_and_char[c][new_block_number];

                if (last_states_with_new_color_and_char[c][new_block_number] == EMPTY_STATE) {
                    block_and_char2first_node_of_this_color_and_char[c][new_color] = t;
                } else {
                    next_B_cap[c][last_states_with_new_color_and_char[c][new_block_number]] = t;
                }

                last_states_with_new_color_and_char[c][new_block_number] = t;
                B_cap_lengths[c][old_color]--;
                B_cap_lengths[c][new_color]++;
            }

        }
    }

    last_states_with_new_color_and_char.clear();
    last_states_of_new_blocks.clear();

    for (uint32_t c = 0; c < alphabet_length; c++) {
        for (auto j : sep_blocks) {
            uint32_t new_color = block2index_of_new_block[j];
            if (info_L[c][j] && (block_lengths[j] == 0)) {
                if (!info_L[c][new_color]) {
                    L[c].push_back(new_color);
                }
                info_L[c][new_color] = true;
            } else if (block_lengths[j] == 0 || B_cap_lengths[c][j] == 0) { // вообще тут должно быть просто второе)
                continue;
            }

            // both not empty
            if (info_L[c][j]) {
                L[c].push_back(new_color);
                info_L[c][new_color] = true;
            } else {
                if (B_cap_lengths[c][new_color] <= B_cap_lengths[c][j]) {
                    L[c].push_back(new_color);
                    info_L[c][new_color] = true;
                }
                else {
                    L[c].push_back(j);
                    info_L[c][j] = true;
                }
            }
        }
    }

    for (auto j: sep_blocks) {
        block2index_of_new_block[j] = EMPTY_STATE;
        blocks_need_to_be_separated[j] = false;
    }

    sep_blocks.clear();
    sep_states.clear();

    return false;
}

void DFA::minimization(bool no_debug) {
    if (!no_debug) std::cout << "DELETING UNREACHABLE STATES...\n";
    delete_unreachable_states();
    if (size < 2) return;
    if (!no_debug) std::cout << "MINIMIZATION STARTED...\n";
    construct_reversed_delta();
    color_acc_and_rej_in_2_colors();


    bool finish = false; uint32_t it = 1;
    while (!finish) {
        finish = minimize_iteration();
        it++;
    }
    if (!no_debug) {
        std::cout << "MINIMIZATION FINISHED SUCCESSFULLY\n";
        std::cout << it - 1 << " iterations happened\n";
    }

    std::vector<uint32_t> new_color2block(size), block2new_color(size);
    uint32_t next_empty_color = 0;
    for (uint32_t c = 0; c < size; c++) {
        if (block2first_state_of_this_block[c] != EMPTY_STATE) {
            new_color2block[next_empty_color] = c;
            block2new_color[c] = next_empty_color;
            next_empty_color++;
        }
    }
    uint32_t new_size = next_empty_color;
    std::vector<bool> new_v_acc(new_size);
    uint32_t new_starting_node = block2new_color[states_info[starting_node].color];
    std::vector<std::vector<uint32_t> > new_delta(alphabet_length, std::vector<uint32_t>(new_size));
    for (uint32_t s = 0; s < new_size; s++) {
        new_v_acc[s] = acc[block2first_state_of_this_block[new_color2block[s]]];
        for (uint32_t a = 0; a < alphabet_length; a++) {
            new_delta[a][s] = block2new_color[states_info[delta[a][block2first_state_of_this_block[new_color2block[s]]]].color];
        }
    }
    init(alphabet_length, new_size, new_starting_node, new_delta, new_v_acc);
    if (!no_debug) {
        std::cout << "DFA UPDATED\n";
        std::cout << "It has " << size << " states now\n";
    }
}


void DFA::print_current_classes_of_equality(bool finished, bool debug) const {
    if (!finished) {return;}
    uint32_t not_empty_blocks = 0;
    for (uint32_t c = 0; c < size; c++) {
        if (debug) std::cout << "Block ";
        if (block2first_state_of_this_block[c] == EMPTY_STATE) {
            if (debug) std::cout << c << ": No states with this color\n";
            continue;
        }
        not_empty_blocks++;
        if (!debug) continue;
        uint32_t s = block2first_state_of_this_block[c];
        std::cout << c << ": ";
        while (s != EMPTY_STATE) {
            std::cout << s << ' ';
            s = states_info[s].next_state_of_same_color;
            // the last state with this color has a pointer to EMPTY_STATE
        }
        std::cout << '\n';
        for (uint32_t a = 0; a < alphabet_length; a++) {
            std::cout << a << "-reachable: ";
            uint32_t s_it = block_and_char2first_node_of_this_color_and_char[a][c];
            while (s_it != EMPTY_STATE) {
                std::cout << s_it << ' ';
                // std::cout << "(" << prev_B_cap[a][s_it] << ") ";
                s_it = next_B_cap[a][s_it];
            }
            std::cout << '\n';
        }
    }
    std::cout << not_empty_blocks << " blocks are not empty\n";
    std::cout << "+++++++++++++++++++++\n";
}

void DFA::print_L() const {
    assert(L.size() == alphabet_length);
    std::cout << "L:\n";
    for (uint32_t a = 0; a < alphabet_length; a++) {
        std::cout << "info_L[" << a << "]: ";
        for (uint32_t u = 0; u < size; u++) std::cout << info_L[a][u] << ' ';
        std::cout << '\n';
        std::cout << a << ": { ";
        for (auto class_of_equality : L[a]) {
            std::cout << class_of_equality << ' ';
        }
        std::cout << "}\n";
    }
}

void DFA::print_B_caps() const {
    for (uint32_t color = 0; color < size; color++) {
        for (uint32_t a = 0; a < alphabet_length; a++) {
            std::cout << "color: " << color << "; char: " << a << '\n';
            uint32_t s = block_and_char2first_node_of_this_color_and_char[a][color];
            while (s != EMPTY_STATE) {
                std::cout << "(" << s << ", " << prev_B_cap[a][s] << ", " << next_B_cap[a][s] << "); ";
                s = next_B_cap[a][s];
            }
            std::cout << "\n";
        }
    }
}

int DFA::save_to_file(char* filename) const {
    FILE* file = fopen(filename, "wb");
    if (file == nullptr) {
        return 1; // nothing to save
    }
    int size_writing = fwrite(&size, sizeof(uint32_t), 1, file);
    int alphabet_writing = fwrite(&alphabet_length, sizeof(uint32_t), 1, file);
    int starting_node_writing = fwrite(&starting_node, sizeof(uint32_t), 1, file);
    if (size_writing != 1 || alphabet_writing != 1 || starting_node_writing != 1) {
        fclose(file);
        return 1;
    }

    for (uint32_t a = 0; a < alphabet_length; a++) {
        for (uint32_t i = 0; i < size; i++) {
            int delta_writing = fwrite(&delta[a][i], sizeof(uint32_t), 1, file);
            if (delta_writing != 1) { fclose(file); return 1; }
        }
    }

    
    for (uint32_t i = 0; i < size; i += 8) {
        unsigned char bools = 0;
        for (uint32_t j = 0; j < 8; j++) {
            if (i + j < size && acc[i + j]) {
                bools |= (1 << j);
            }
        }
        int bools_writing = fwrite(&bools, sizeof(unsigned char), 1, file);
        if (bools_writing != 1) { fclose(file); return 1; }
    }
    fclose(file);
    return 0;
}


bool DFA::operator==(const DFA& other) const {
    // TODO: add `minimized` field
    if (this->size != other.size) return false;
    if (this->alphabet_length != other.alphabet_length) return false;
    
    std::vector<uint32_t> dict(this->size, EMPTY_STATE); // map state->state
    dict[this->starting_node] = other.starting_node;

    std::queue<uint32_t> nodes2check;
    nodes2check.push(this->starting_node);

    while (!nodes2check.empty()) {
        uint32_t cur = nodes2check.front();
        nodes2check.pop();
        
        for (uint32_t a = 0; a < this->alphabet_length; ++a) {
            if (dict[this->delta[a][cur]] == EMPTY_STATE) {
                nodes2check.push(this->delta[a][cur]);
                dict[this->delta[a][cur]] = other.delta[a][dict[cur]];
            } else if (dict[this->delta[a][cur]] != other.delta[a][dict[cur]]) return false;
        }
    }

    for (uint32_t i = 0; i < this->size; ++i) {
        if (dict[i] == EMPTY_STATE) return false;
        if (this->acc[i] != other.acc[dict[i]]) return false;
        // for (uint32_t a = 0; a < this->alphabet_length; ++a) { /// think about it: maybe it is useless
        //     if (dict[this->delta[a][i]] != other.delta[a][dict[i]]) return false;
        // }
    }

    return true;
}


/*DFA::DFA(std::string &special_type, std::vector<uint32_t> &parameters) {
    if (special_type == "bamboo") {
        assert(!parameters.empty() && parameters[0] >= 1);
        uint32_t _size = parameters[0];
        std::vector<bool> _v_acc(_size, false);
        uint32_t _alphabet_length = (parameters.size() >= 2 && parameters[1] >= 1 ? parameters[1] : 1);
        _v_acc[_size - 1] = true;
        std::vector<std::vector<uint32_t> > _delta(_alphabet_length, std::vector<uint32_t>(_size));
        for (uint32_t s = 0; s < _size - 1; s++) {
            for (uint32_t a = 0; a < _alphabet_length; a++) {
                _delta[a][s] = s + 1;
            }
        }
        for (uint32_t a = 0; a < _alphabet_length; a++) {
            _delta[a][_size - 1] = _size - 1;
        }
        init(_alphabet_length, _size,  0, _delta, _v_acc);
    }
    if (special_type == "tiger_cycle") {
        assert(parameters.size() >= 2);
        uint32_t _size = parameters[0], small_cycle = parameters[1];
        assert(_size >= 1 && small_cycle >= 1 && _size % small_cycle == 0);
        std::vector<std::vector<uint32_t> > _delta(1, std::vector<uint32_t>(_size, 0));
        std::vector<bool> _v_acc(_size, false);
        for (uint32_t s = 0; s < _size - 1; s++) {
            _v_acc[s] = ((s + 1) % small_cycle == 0);
            _delta[0][s] = s + 1;
        }
        _v_acc[_size - 1] = true;
        init(1, _size, 0, _delta, _v_acc);
    }
    if (special_type == "alternation_tiger_cycle") {
        assert(parameters.size() >= 2);
        uint32_t _size = parameters[0], small_cycle = parameters[1];
        assert(_size >= 1 && small_cycle >= 1 && _size % small_cycle == 0 && small_cycle % 2 == 0);
        std::vector<std::vector<uint32_t> > _delta(2, std::vector<uint32_t>(_size));
        std::vector<bool> _v_acc(_size, false);

        for (uint32_t s = 0; s < _size; s++) {
            _v_acc[s] = ((s + 1) % small_cycle == 0);
            if (s % 2 == 0) {
                _delta[0][s] = s + 1;
                _delta[1][s] = s;
            } else {
                if (s == _size - 1) {
                    _delta[0][s] = s;
                    _delta[1][s] = 0;
                } else {
                    _delta[0][s] = s;
                    _delta[1][s] = s + 1;
                }
            }
        }
        init(2, _size, 0, _delta, _v_acc);
    }
    if (special_type == "special_cycle") {
        assert(parameters.size() >= 2);
        uint32_t _size = parameters[0], small_cycle = parameters[1];
        assert(_size >= 1 && small_cycle >= 1 && _size % small_cycle == 0 && small_cycle % 2 == 0);
        assert(parameters.size() >= 2 + small_cycle);
        std::vector<std::vector<uint32_t>> _delta(1, std::vector<uint32_t>(_size));
        std::vector<bool> _v_acc(_size, false);
        for (uint32_t s = 0; s < _size; s++) {
            _delta[0][s] = (s == _size - 1 ? 0 : s + 1);
            _v_acc[s] = parameters[2 + (s % small_cycle)];
        }
        init(1, _size, 0, _delta, _v_acc);
    }

}*/
