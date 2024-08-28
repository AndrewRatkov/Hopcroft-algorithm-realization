#include "dfa_class.h"
#include "nfa_class.h"

char integer2char(const uint32_t x) { // 0 <= x < 62
    assert(x < 62);
    if (x < 10) {
        return ('0' + (char)x);
    } else if (x < 36) {
        return ('a' + (char)(x - 10));
    } else {
        return ('A' + (char)(x - 36));
    }
}

uint32_t char2integer(const char c) {
    assert(('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'));
    if ('0' <= c && c <= '9') {
        return (int)(c - '0');
    } else if ('a' <= c && c <= 'z') {
        return (int)(c - 'a') + 10;
    } else {
        return (int)(c - 'A') + 36;
    }
}

bool correctness_of_dfa_string(char* dfa_str) {
    char* ptr = dfa_str;
    while (*ptr != '\0' && *ptr != '_') {
        if (*ptr != '1' && *ptr != '0') return false;
        ptr++;
    }
    if (*ptr == '\0') return false; // dfa must_have one '_'
    uint32_t size = (uint32_t)(ptr - dfa_str);
    ptr++;
    while (*ptr != '\0') {
        if (('0' <= *ptr && *ptr <= '9') || ('a' <= *ptr && *ptr <= 'z') || ('A' <= *ptr && *ptr <= 'Z')) {
            if (char2integer(*ptr) >= size) return false;
            ptr++;
        } else {
            return false;
        }
    }
    if ((strlen(dfa_str) - 1) % size != 0) return false;
    return true;
}


bool correctness_of_nfa_string(char* nfa_str) {
    bool opened_bracket = false; // true if last '{' wasn't closed
    char* ptr = nfa_str;
    int blocks = 0;
    uint32_t size = 0;
    while (*ptr != '-' && *ptr != '+') {
        if (*ptr == '\0') return false; // pluses and minuses should be at the end
        else if (*ptr == '{') {
            if (opened_bracket) return false; // cannot be to '{' without '}' between them
            opened_bracket = true;
        } else if (*ptr == '}') {
            if (!opened_bracket) return false; // too much '}' somewhere
            opened_bracket = false;
        } else if (!(('0' <= *ptr && *ptr <= '9') || ('a' <= *ptr && *ptr <= 'z') || ('A' <= *ptr && *ptr <= 'Z') || (*ptr == '>'))) {
            return false; // unknown symbol
        }
        if (!opened_bracket) blocks++;
        ptr++;
    }
    if (opened_bracket) return false;

    while (*ptr != '\0') {
        if (*ptr != '-' && *ptr != '+') return false; // only '+' and '-' at the end
        size++;
        ptr++;
    }

    if (blocks % size != 0) return false;

    int alphabet_size = blocks / size;
    ptr = nfa_str;
    int read_blocks = 0;
    bool last_read_was_initial_symbol = false;

    while (*ptr != '-' && *ptr != '+') {
        if (*ptr == '>') {
            if (last_read_was_initial_symbol) return false; // cannot be ">>" in string
            if (opened_bracket) return false; // cannot be > inside of block
            if (read_blocks % alphabet_size != 0) return false;
            last_read_was_initial_symbol = true;
        } else {
            last_read_was_initial_symbol = false;
        }
        if (*ptr == '{') opened_bracket = true;
        else if (*ptr == '}') { opened_bracket = false; read_blocks++;}
        if (('0' <= *ptr && *ptr <= '9') || ('a' <= *ptr && *ptr <= 'z') || ('A' <= *ptr && *ptr <= 'Z')) {
            if (char2integer(*ptr) >= size) return false; // too big numbers of states
            if (!opened_bracket) read_blocks++;
        }
        ptr++;
    }

    return true;
}


request_check correctness_of_dfa_input(char* command, char* dfa_str) {
    if (strcmp(command, "from_dfa_string") == 0) { // from_dfa_string 000010000100001_123456789abcdef0
        bool b = correctness_of_dfa_string(dfa_str);
        if (b) return {true, ""};
        else return {false, "Error: Incorrect input of dfa string (2nd argument)"};
    } else if (strcmp(command, "bamboo") == 0 || strcmp(command, "circle") == 0) { // bamboo 1000000,2 ; circle 500,1
        // dfa_str = "{size},{alphabet_length}"
        uint32_t _size, _alphabet_length;
        int read_vars = sscanf(dfa_str, "%d,%d", &_size, &_alphabet_length);
        if (read_vars != 2) return {false, "Error: Incorrect input of \"{size},{alphabet_length}\" in 2nd argument"};
        if (_size <= 1) return {false, "Error: too small (" + std::to_string(_size) + " states) dfa"};
        if (_alphabet_length == 0) return {false, "Alphabet length mustn't be equal to 0"};
        return {true, ""};

    } else if (strcmp(command, "repeated_cycle") == 0) { // repeated_cycle 100000,5
        // dfa_str = "{size},{cycle_size}", size % cycle_size = 0
        uint32_t _size, _cycle_size;
        int read_vars = sscanf(dfa_str, "%d,%d", &_size, &_cycle_size);
        if (read_vars != 2) return {false, "Error: Incorrect input of \"{size},{cycle_size}\" in 2nd argument"};
        if (_cycle_size == 0) return {false, "Error: cycle_size cannot be equal to 0"};
        if (_size % _cycle_size != 0) return {false, "Error: size % cycle_size must be equal to 0"};
        return {true, ""};

    } else if (strcmp(command, "from_nfa_string") == 0) {
        bool b = correctness_of_nfa_string(dfa_str);
        if (b) return {true, ""};
        else return {false, "Error: Incorrect input of nfa string (2nd argument)"};

    } else if (strcmp(command, "from_bin_file") == 0) {
        FILE* file = fopen(dfa_str, "rb");
        if (file == nullptr) {
            return {false, "Error: when openning file"};
        }
        uint32_t _size, _alphabet_length, _starting_node;
        int size_reading = fread(&_size, sizeof(uint32_t), 1, file);
        int alphabet_length_reading = fread(&_alphabet_length, sizeof(uint32_t), 1, file);
        int starting_node_reading = fread(&_starting_node, sizeof(uint32_t), 1, file);

        if (size_reading != 1 || alphabet_length_reading != 1 || starting_node_reading != 1) {
            fclose(file);
            return {false, "Error: while reading first 3 bytes"};
        }

        for (uint32_t i = 0; i < _alphabet_length * _size; i++) {
            uint32_t x;
            int x_reading = fread(&x, sizeof(uint32_t), 1, file);
            if (x_reading != 1) { fclose(file); return {false, "Error: when readind delta table"}; }
            if (x >= _size) { fclose(file); return {false, "Error: too big value in table"}; }
        }
        for (uint32_t i = 0; i < _size; i += 8) {
            unsigned char c;
            int c_reading = fread(&c, sizeof(unsigned char), 1, file);
            if (c_reading != 1) {
                fclose(file);
                return {false, "Error: when reading end information (about acc. and rej. states)"};
            }
        }

        fclose(file);
        return {true, ""};

    } else {
        return {false, "Error: Incorrect 1st argument"};
    }
}

DFA::DFA(char* command, char* s) {
    if (strcmp(command, "from_dfa_string") == 0) {
        this->acc = {};
        this->delta = {};

        uint32_t idx = 0;
        while (s[idx] != '_') {
            this->acc.push_back(s[idx] != '0');
            ++idx;
        }

        this->size = idx;
        uint32_t string_length = strlen(s);
        this->alphabet_length  = (string_length - 1) / this->size - 1;
        ++idx;
        this->delta.assign(this->alphabet_length, std::vector<uint32_t>(this->size));

        while (idx < string_length) {
            this->delta[(idx - this->size - 1) % this->alphabet_length][(idx - this->size - 1) / this->alphabet_length] = char2integer(s[idx]);
            ++idx;
        }
        this->starting_node = 0;
    } else if (strcmp(command, "bamboo") == 0  || strcmp(command, "circle") == 0) {
        sscanf(s, "%d,%d", &(this->size), &(this->alphabet_length));
        this->acc.assign(this->size, false);
        this->delta.assign(this->alphabet_length, std::vector<uint32_t>(this->size));
        this->acc[this->size - 1] = true;
        for (uint32_t s = 0; s < this->size - 1; ++s) {
            for (uint32_t a = 0; a < this->alphabet_length; ++a) {
                this->delta[a][s] = s + 1;
            }
        }
        for (uint32_t a = 0; a < this->alphabet_length; ++a) {
            this->delta[a][this->size - 1] = (strcmp(command, "circle") ? this->size - 1 : 0);
        }
        this->starting_node = 0;
    } else if (strcmp(command, "repeated_cycle") == 0) {
        this->alphabet_length = 1;
        uint32_t _cycle_size;
        sscanf(s, "%d,%d", &(this->size), &_cycle_size);
        this->acc.assign(this->size, false);
        this->delta.assign(this->alphabet_length, std::vector<uint32_t>(this->size));
        for (uint32_t s = 0; s < this->size - 1; ++s) {
            this->acc[s] = ((s + 1) % _cycle_size == 0);
            this->delta[0][s] = s + 1;
        }
        this->acc[this->size - 1] = true;
        this->starting_node = 0;
    } else if (strcmp(command, "from_bin_file") == 0) {
        FILE* file = fopen(s, "rb");
        if (file == NULL) {
            std::cerr << "couldn't open file\n";
            return;
        }

        if (fread(&(this->size), sizeof(uint32_t), 1, file) != 1) goto error_happened;
        if (fread(&(this->alphabet_length), sizeof(uint32_t), 1, file) != 1) goto error_happened;
        if (fread(&(this->starting_node), sizeof(uint32_t), 1, file) != 1) goto error_happened;

        this->delta.assign(this->alphabet_length, std::vector<uint32_t>(this->size));

        for (uint32_t a = 0; a < this->alphabet_length; ++a) {
            for (uint32_t i = 0; i < this->size; ++i) {
                if (fread(&(this->delta[a][i]), sizeof(uint32_t), 1, file) != 1) {
                    goto error_happened;
                }
            }
        }

        this->acc.assign(this->size, false);
        for (uint32_t i = 0; i < this->size; i += 8) {
            unsigned char c;
            if (fread(&c, sizeof(unsigned char), 1, file) != 1) goto error_happened;
            for (uint32_t j = 0; j < 8; ++j) {
                if (i + j < this->size) {
                    this->acc[i + j] = ((c & (1 << j)) ? true : false);
                }
            }
        }

        fclose(file);
        return;

        error_happened:
            fclose(file);
            std::cerr << "error while reading happened\n";
            this->alphabet_length = 1;
            this->size = 1;
            this->starting_node = 0;
            this->acc = {true};
            this->delta = {{0}};

    } else {
        this->alphabet_length = 1;
        this->size = 1;
        this->starting_node = 0;
        this->acc = {true};
        this->delta = {{0}};
    }
}
