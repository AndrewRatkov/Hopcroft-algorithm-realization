#include "dfa_class.h"

void DFA::init(uint32_t _alphabet_length, uint32_t _size, uint32_t _starting_node,
               std::vector<std::vector<uint32_t> > &table, std::vector<bool> &v_acc) {
    // size, alphabet length and index of starting are just given
    this->size = _size;
    this->alphabet_length = _alphabet_length;
    this->starting_node = _starting_node;

    // before we get the table for delta, we need to check that it's sizes are size * alphabet_length
    assert(table.size() == _alphabet_length);
    for (uint32_t i = 0; i < _alphabet_length; ++i) {
        assert(table[i].size() == _size);
    }
    this->delta = table;

    // getting information about the states: acc or rej they are
    assert(v_acc.size() == this->size);
    this->acc = v_acc;
}

void DFA::print_table() const {
    std::cout << "SIZE: " << this->size << "  LEN_ALPHABET: " << this->alphabet_length << " STARTING_NODE: " << this->starting_node << '\n';
    if (size > 50) { std::cout << "Too big dfa to print in stdout\n"; return; }

    uint32_t spaces_per_cell = 1;
    uint32_t max_state_amount = 10;
    while (max_state_amount < size) {
        ++spaces_per_cell; max_state_amount *= 10;
    }
    spaces_per_cell += 2;
    if (spaces_per_cell < 4) spaces_per_cell = 4;

    // left upper corner --- empty
    std::cout << std::string(spaces_per_cell, ' ');

    for (uint32_t i = 0; i < this->alphabet_length; ++i) {
        // counting spaces to write
        uint32_t i_copy = i;
        uint32_t spaces = spaces_per_cell - 2;
        while (i_copy >= 10) { i_copy /= 10; --spaces; }
        std::cout << "|" << std::string(spaces, ' ') << i << ' ';
    }
    std::cout << "| TYPE \n";
    std::cout << std::string(spaces_per_cell * (this->alphabet_length + 1) + this->alphabet_length + 7, '=') << '\n';

    for (uint32_t node = 0; node < size; ++node) {
        // counting spaces to write
        uint32_t state_copy = node;
        uint32_t spaces_first_column = spaces_per_cell - 2;
        while (state_copy >= 10) {state_copy /= 10; --spaces_first_column;}
        std::cout << std::string(spaces_first_column, ' ') << node << ' ';

        for (uint32_t i = 0; i < this->alphabet_length; ++i) {
            // counting spaces to write
            uint32_t next_copy = this->delta[i][node];
            uint32_t spaces = spaces_per_cell - 2;
            while (next_copy >= 10) {next_copy /= 10; --spaces;}
            std::cout << "|" << std::string(spaces, ' ') << this->delta[i][node] << ' ';
        }
        std::cout << "| " << (acc[node] ? "ACC" : "REJ") << '\n';
        std::cout << std::string(spaces_per_cell * (this->alphabet_length + 1) + this->alphabet_length + 7, '=') << '\n';
    }
}


bool DFA::check_string(std::vector<uint32_t> &str) const {
        uint32_t q_cur = this->starting_node;
        for (uint32_t c: str) {
            q_cur = this->delta[c][q_cur];
        }
        return this->acc[q_cur];
}

void DFA::delete_unreachable_states() {
    // we will color all states in 3 colors:
    // 0 if it isn't visited yet
    // 1 if it is visited, but it's neighbours
    // (states where we can go from current state) are not visited yet
    // 2 if it is already visited and it's neighbours too
    std::vector<char> colors(size, 0);
    std::queue<uint32_t> q; // queue of states of color 1. When it becomes empty the algorithm finishes
    colors[this->starting_node] = 1;
    q.push(this->starting_node);


    uint32_t new_size = 0; // size of DFA after deleting unreachable states = number of states popped from q
    while (!q.empty()) {
        uint32_t cur_node = q.front();
        q.pop();
        colors[cur_node] = 2; ++new_size;
        for (uint32_t i = 0; i < this->alphabet_length; ++i) {
            uint32_t next_node = this->delta[i][cur_node];
            if (colors[next_node] == 0) { 
                colors[next_node] = 1;
                q.push(next_node);
            }
        }
    }

    if (new_size == this->size) return; // we have nothing to delete

    std::vector<uint32_t> node2new_idx(this->size); // each state will have new index
    uint32_t cur = 0;
    for (uint32_t i = 0; i < this->size; ++i) {
        if (colors[i] == 2) {
            node2new_idx[i] = cur;
            ++cur;
        }
    }

    // new delta function and in
    std::vector<std::vector<uint32_t> > new_delta(this->alphabet_length, std::vector<uint32_t>(new_size));
    std::vector<bool> new_v_acc(new_size);


    for (uint32_t i = 0; i < size; ++i) {
        if (colors[i] == 2) { // if state is visited
            new_v_acc[node2new_idx[i]] = this->acc[i];
            for (uint32_t a = 0; a < this->alphabet_length; ++a) {
                new_delta[a][node2new_idx[i]] = node2new_idx[this->delta[a][i]];
            }
        }
    }

    this->size = new_size;
    this->starting_node = node2new_idx[starting_node];
    this->delta = std::move(new_delta);
    this->acc = std::move(new_v_acc);
}


void DFA::construct_reversed_delta() {
    // reversed delta will be a vector of size*alphabet_length (instead of vector<vector<vector>>>)
    // for each state s and char a reversed_delta_lengths[a][s] is number of states t: delta(t, a) = s
    // reversed_delta[addresses_for_reversed_delta[a][s]], reversed_delta[addresses_for_reversed_delta[a][s] + 1], ...
    // ..., reversed_delta[addresses_for_reversed_delta[a][s] + reversed_delta_lengths[a][s] - 1] are all states t
    // such as delta(t, a) = s
    this->addresses_for_reversed_delta.assign(this->alphabet_length, std::vector<uint32_t>(this->size, 0));
    this->reversed_delta.assign(this->size * this->alphabet_length, 0);
    std::vector<std::vector<uint32_t> > reversed_delta_lengths(this->alphabet_length, std::vector<uint32_t>(this->size, 0));

    for (uint32_t a = 0; a < this->alphabet_length; ++a) {
        for (uint32_t s = 0; s < this->size; ++s) {
            ++reversed_delta_lengths[a][this->delta[a][s]];
        }
    }

    uint32_t cur = 0;
    for (uint32_t a = 0; a < this->alphabet_length; ++a) {
        for (uint32_t s = 0; s < this->size; ++s) {
            this->addresses_for_reversed_delta[a][s] = cur;
            cur += reversed_delta_lengths[a][s];
        }
    }

    std::vector<std::vector<uint32_t> > addresses_for_put = this->addresses_for_reversed_delta;

    for (uint32_t s = 0; s < this->size; ++s) {
        for (uint32_t a = 0; a < this->alphabet_length; ++a) {
            const uint32_t address_to_put = addresses_for_put[a][this->delta[a][s]];
            this->reversed_delta[address_to_put] = s;
            ++addresses_for_put[a][delta[a][s]];
        }
    }

}


void DFA::color_acc_and_rej_in_2_colors() {
    // size >= 2
    this->next_B_cap.assign(this->alphabet_length, std::vector<uint32_t>(this->size, EMPTY_STATE));
    this->prev_B_cap.assign(this->alphabet_length, std::vector<uint32_t>(this->size, EMPTY_STATE));
    
    this->block2first_state_in_it.assign(this->size, EMPTY_STATE); // 0 --> EMPTY_STATE; 1 --> EMPTY_STATE
    this->block_and_char2first_B_cap.assign(this->alphabet_length, std::vector<uint32_t>(this->size, EMPTY_STATE));

    // for color v and char a B_cap_lengths[a][v] is number of states s,
    // such as s is colored in v-th color and is reachable by symbol a (exists state t: delta(t, a) = s)
    this->B_cap_lengths.assign(this->alphabet_length, std::vector<uint32_t>(this->size, 0));
    this->states_info.assign(this->size, {UINT32_MAX, EMPTY_STATE, EMPTY_STATE});

    uint32_t last_acc = EMPTY_STATE; uint32_t last_rej = EMPTY_STATE; // last acc and rej states during iteration
    this->block_lengths.assign(size, 0);

    std::vector<uint32_t> last0(this->alphabet_length, EMPTY_STATE), last1(this->alphabet_length, EMPTY_STATE); // for B_cap(B(0), a) and B_cap(B(1), a) for each a in alphabet
    for (uint32_t s = 0; s < this->size; ++s) {
        if (this->acc[s]) {
            ++this->block_lengths[0];
            for (uint32_t a = 0; a < this->alphabet_length; ++a) {
                if (get_reversed_delta_length(a, s)) { // possible to get to s by a
                    ++this->B_cap_lengths[a][0]; // counter of acc states in which it's possible to get with a
                    this->prev_B_cap[a][s] = last0[a];
                    if (last0[a] == EMPTY_STATE) {
                        this->block_and_char2first_B_cap[a][0] = s;
                    } else {
                        this->next_B_cap[a][last0[a]] = s;
                    }
                    last0[a] = s;
                }
            }

            this->states_info[s].prev_state_of_same_block = last_acc;
            if (last_acc == EMPTY_STATE) {
                this->block2first_state_in_it[0] = s;
            } else {
                this->states_info[last_acc].next_state_of_same_block = s;
            }
            this->states_info[s].block = 0; // acc have color 0
            last_acc = s;
        } else {
            ++this->block_lengths[1];
            for (uint32_t a = 0; a < this->alphabet_length; ++a) {
                if (get_reversed_delta_length(a, s)) { // possible to get to s by a:
                    ++this->B_cap_lengths[a][1];
                    this->prev_B_cap[a][s] = last1[a];
                    if (last1[a] == EMPTY_STATE) {
                        this->block_and_char2first_B_cap[a][1] = s;
                    } else {
                        this->next_B_cap[a][last1[a]] = s;
                    }
                    last1[a] = s;
                }
            }

            this->states_info[s].prev_state_of_same_block = last_rej;
            if (last_rej == EMPTY_STATE) {
                this->block2first_state_in_it[1] = s;
            } else {
                this->states_info[last_rej].next_state_of_same_block = s;                
            }
            this->states_info[s].block = 1;
            last_rej = s;
        }
    }

    if (last_acc != EMPTY_STATE) this->states_info[last_acc].next_state_of_same_block = EMPTY_STATE; // at least one acc found
    if (last_rej != EMPTY_STATE) this->states_info[last_rej].next_state_of_same_block = EMPTY_STATE; // at least one rej found
    
    this->info_L.assign(this->alphabet_length, std::vector<bool>(this->size, false));
    for (uint32_t a = 0; a < this->alphabet_length; ++a) {
        if (this->B_cap_lengths[a][0] <= this->B_cap_lengths[a][1]) {
            this->L.push(std::make_pair(a, 0));
            this->info_L[a][0] = true;
        } else {
            this->L.push(std::make_pair(a, 1));
            this->info_L[a][1] = true;
        }
    }
    
    this->colors = 2;
}

void DFA::extract_state_to_new_block(const uint32_t s, const uint32_t new_block) {
    const uint32_t old_block = this->states_info[s].block;
    const uint32_t old_prev = this->states_info[s].prev_state_of_same_block;
    const uint32_t old_next = this->states_info[s].next_state_of_same_block;
    const uint32_t new_next = this->block2first_state_in_it[new_block];

    if (old_prev != EMPTY_STATE) {
        this->states_info[old_prev].next_state_of_same_block = old_next;
    } else {
        this->block2first_state_in_it[old_block] = old_next;
    }

    if (old_next != EMPTY_STATE) {
        this->states_info[old_next].prev_state_of_same_block = old_prev;
    }

    this->block2first_state_in_it[new_block] = s;
    this->states_info[s].prev_state_of_same_block = EMPTY_STATE;
    this->states_info[s].next_state_of_same_block = new_next;
    
    if (new_next != EMPTY_STATE) {
        this->states_info[new_next].prev_state_of_same_block = s;
    }
    
    for (uint32_t c = 0; c < this->alphabet_length; ++c) {
        if (get_reversed_delta_length(c, s)) {
            --this->B_cap_lengths[c][old_block];
            ++this->B_cap_lengths[c][new_block];

            if (this->prev_B_cap[c][s] != EMPTY_STATE) {
                this->next_B_cap[c][this->prev_B_cap[c][s]] = this->next_B_cap[c][s];
            } else {
                this->block_and_char2first_B_cap[c][old_block] = this->next_B_cap[c][s];
            }

            if (this->next_B_cap[c][s] != EMPTY_STATE) {
                this->prev_B_cap[c][this->next_B_cap[c][s]] = this->prev_B_cap[c][s];
            }

            this->next_B_cap[c][s] = this->block_and_char2first_B_cap[c][new_block];
            this->prev_B_cap[c][s] = EMPTY_STATE;
            
            if (this->block_and_char2first_B_cap[c][new_block] != EMPTY_STATE) {
                this->prev_B_cap[c][this->block_and_char2first_B_cap[c][new_block]] = s;
            }

            this->block_and_char2first_B_cap[c][new_block] = s;
        }
    }

    this->states_info[s].block = new_block;                        
}


bool DFA::minimize_iteration() {
    if (this->colors == this->size) return true; // number of blocks == size => nothing to minimize

    if (this->L.empty()) return true;
    const std::pair<uint32_t, uint32_t>& extracted_pair = this->L.front();
    this->L.pop();
    const uint32_t a = extracted_pair.first;
    const uint32_t i = extracted_pair.second;

    
    uint32_t state_i = this->block_and_char2first_B_cap[a][i];
    while (state_i != EMPTY_STATE) {
        for (uint32_t t = this->addresses_for_reversed_delta[a][state_i]; t < next_address_for_reversed_delta(a, state_i); ++t) {
            uint32_t sep_state = this->reversed_delta[t]; // delta(sep_state, a) in B(i)
            this->sep_states.push_back(sep_state);
            
            uint32_t sep_state_color = this->states_info[sep_state].block;
            if (this->blocks_info.find(sep_state_color) == this->blocks_info.end()) {
                this->blocks_info[sep_state_color] = {1, EMPTY_STATE};
            } else {
                ++this->blocks_info[sep_state_color].states2extract;
            }
        }

        state_i = this->next_B_cap[a][state_i];
    }

    for (auto& block_info : this->blocks_info) {
        const uint32_t block = block_info.first;
        const uint32_t new_block_size = block_info.second.states2extract;
        uint32_t& new_block = block_info.second.new_color;
        if (new_block_size == this->block_lengths[block]) { // block does not divide into 2 new blocks
            continue;
        } else {
            new_block = this->colors;
            ++this->colors;

            if (this->block_lengths[block] < 2 * new_block_size) {
                uint32_t s = this->block2first_state_in_it[block];
                while (s != EMPTY_STATE) {
                    uint32_t next_state = this->states_info[s].next_state_of_same_block;
                    if (this->states_info[this->delta[a][s]].block != i) {
                        extract_state_to_new_block(s, new_block);
                    }
                    s = next_state;            
                }

                this->block_lengths[new_block] = block_lengths[block] - new_block_size;
                this->block_lengths[block] = new_block_size;
                new_block = EMPTY_STATE;
            } else {
                this->block_lengths[block] -= new_block_size;
                this->block_lengths[new_block] = new_block_size;
                this->sep_blocks.push_back(block);
            }
        }
    }

    for (const uint32_t sep_state: this->sep_states) {
        const uint32_t new_block = this->blocks_info[this->states_info[sep_state].block].new_color;
        if (new_block != EMPTY_STATE) {
            extract_state_to_new_block(sep_state, new_block);
        }
    }


    for (uint32_t c = 0; c < this->alphabet_length; ++c) {
        for (auto j : this->sep_blocks) {
            uint32_t new_color = this->blocks_info[j].new_color;

            if (this->info_L[c][j]) {
                this->L.push(std::make_pair(c, new_color));
                this->info_L[c][new_color] = true;
            } else {
                if (this->B_cap_lengths[c][new_color] <= this->B_cap_lengths[c][j]) {
                    this->L.push(std::make_pair(c, new_color));
                    this->info_L[c][new_color] = true;
                } else {
                    this->L.push(std::make_pair(c, j));
                    this->info_L[c][j] = true;
                }
            }
        }
    }

    this->sep_blocks.clear();
    this->sep_states.clear();
    this->blocks_info.clear();

    return false;
}

void DFA::minimization(bool no_debug) {
    if (!no_debug) std::cout << "DELETING UNREACHABLE STATES...\n";
    delete_unreachable_states();
    if (size < 2) return;
    if (!no_debug) std::cout << "MINIMIZATION STARTED...\n";
    construct_reversed_delta();
    color_acc_and_rej_in_2_colors();


    bool finish = false; uint32_t it = 0;
    while (!finish) {
        finish = minimize_iteration();
        ++it;
    }
    if (!no_debug) {
        std::cout << "MINIMIZATION FINISHED SUCCESSFULLY\n";
        std::cout << it << " iterations happened\n";
    }

    std::vector<std::vector<uint32_t> > new_delta(this->alphabet_length, std::vector<uint32_t>(this->colors));
    std::vector<bool> new_acc(this->colors);

    for (uint32_t a = 0; a < this->alphabet_length; ++a) {
        for (uint32_t s = 0; s < this->colors; ++s) {
            new_delta[a][s] = this->states_info[this->delta[a][this->block2first_state_in_it[s]]].block;
        }
    }

    for (uint32_t s = 0; s < this->colors; ++s) {
        new_acc[s] = this->acc[this->block2first_state_in_it[s]];
    }

    init(this->alphabet_length, this->colors, this->states_info[this->starting_node].block, new_delta, new_acc);

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
        if (block2first_state_in_it[c] == EMPTY_STATE) {
            if (debug) std::cout << c << ": No states with this color\n";
            continue;
        }
        not_empty_blocks++;
        if (!debug) continue;
        uint32_t s = block2first_state_in_it[c];
        std::cout << c << ": ";
        while (s != EMPTY_STATE) {
            std::cout << s << ' ';
            s = states_info[s].next_state_of_same_block;
            // the last state with this color has a pointer to EMPTY_STATE
        }
        std::cout << '\n';
        for (uint32_t a = 0; a < alphabet_length; a++) {
            std::cout << a << "-reachable: ";
            uint32_t s_it = block_and_char2first_B_cap[a][c];
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

void DFA::print_B_caps() const {
    for (uint32_t color = 0; color < size; color++) {
        for (uint32_t a = 0; a < alphabet_length; a++) {
            std::cout << "color: " << color << "; char: " << a << '\n';
            uint32_t s = block_and_char2first_B_cap[a][color];
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
