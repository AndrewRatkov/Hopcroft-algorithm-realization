# Учебная практика. Реализация алгоритма Хопкрофта.
## Структура проекта
Алгоритм Хопкрофта -- алгоритм для минимизации DFA, работяющий с аcимптотикой O(n*log(n)). Реализован в текущем проекте.
```
Hopcroft-algorithm-realization
├╼ hopcroft_tex/    ← documentation files (.tex, .bib, .pdf, etc)
├╼ include/         ← header files (*.h)
|  ├╼ dfa_class.h   ← structure of dfa class
|  ╰╼ nfa_class.h   ← structure of nfa class
├╼ minimizer        ← compiled program (will be created after you run make)
├╼ Makefile         ← build rules
├╼ obj/             ← object files *.o (will be created after you run make)
├╼ test/            ← some programs for test 
├╼ README.md        ← this file ;)
╰╼ src/             ← source files (*.cpp)
   ├╼ main.cpp
   ├╼ dfa_methods.cpp
   ├╼ dfa_build.cpp
   ╰╼ nfa_methods.cpp
```
Документация (отчёт) находится в файле ```hopcroft_tex/hopcroft.pdf```. Там же инструкции по запуску ```minimizer``` для минимизации DFA.
