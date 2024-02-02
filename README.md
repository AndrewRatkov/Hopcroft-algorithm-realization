# Учебная практика. Реализация алгоритма Хопкрофта.
## Структура проекта
Алгоритм Хопкрофта -- алгоритм для минимизации DFA, работяющий с аcимптотикой O(n*log(n)). Реализован в текущем проекте.
```
hopcroft8
├╼ hopcroft_tex/    ← documentation files (.tex, .bib, .pdf, etc)
├╼ include/         ← header files (*.h)
|  ├╼ dfa_class.h   ← structure of dfa class
|  ╰╼ nfa_class.h   ← structure of nfa class
├╼ minimizer        ← compiled program
├╼ Makefile         ← build rules
├╼ obj/             ← object files *.o
├╼ README.md        ← этот файл ;)
╰╼ src/             ← source files (*.cpp)
   ├╼ main.cpp
   ├╼ dfa_methods.cpp
   ├╼ dfa_build.cpp
   ╰╼ nfa_methods.cpp
```
Документация (отчёт) находится в файле ```hopcroft_tex/hopcroft.pdf```
