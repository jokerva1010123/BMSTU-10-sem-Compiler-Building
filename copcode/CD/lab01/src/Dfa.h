#pragma once

#include <Nfa.h>


class Dfa : public Nfa {
public:
    explicit Dfa(const Nfa& nfa);

    void minimize();
    bool checkWord(std::string_view word) const;
};
