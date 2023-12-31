.PHONY: all clean

CPPFLAGS = -Iinclude -Wall -Wextra -Werror -c
CPPFLAGS_FOR_MAIN_FILE = -Iinclude -Wall -Wextra -Werror

all: mkobj minimizer

mkobj: # if obj directory doesn't exist, create it
	mkdir -p obj 

minimizer: src/main.cpp include/dfa_class.h include/nfa_class.h obj/dfa.o obj/nfa.o obj/dfa_build.o src/dfa_methods.cpp src/nfa_methods.cpp src/dfa_build.cpp
	g++ $(CPPFLAGS_FOR_MAIN_FILE) obj/dfa.o obj/nfa.o obj/dfa_build.o src/main.cpp -o minimizer

obj/dfa.o: src/dfa_methods.cpp
	g++ $(CPPFLAGS) src/dfa_methods.cpp -o obj/dfa.o

obj/dfa_build.o: src/dfa_build.cpp
	g++ $(CPPFLAGS) src/dfa_build.cpp -o obj/dfa_build.o

obj/nfa.o: src/nfa_methods.cpp
	g++ $(CPPFLAGS) src/nfa_methods.cpp -o obj/nfa.o

clean:
	rm -rf obj/ minimizer
