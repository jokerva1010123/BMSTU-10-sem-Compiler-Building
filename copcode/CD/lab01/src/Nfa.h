#pragma once

#include <Ast.h>
#include <Types.h>


class Nfa {
public:
    explicit Nfa(States states = {},
                 Alphabet alphabet = {},
                 Transitions transitions = {},
                 State initialState = {},
                 States acceptStates = {});

    explicit Nfa(const Ast& ast);

    const States& states() const { return states_; }
    const Alphabet& alphabet() const { return alphabet_; }
    const Transitions& transitions() const { return transitions_; }
    const State& initialState() const { return initialState_; }
    const States& acceptStates() const { return acceptStates_; }

    States transition(const State& from, Symbol symbol) const;

    static Nfa baseEpsilon();
    static Nfa baseSymbol(Symbol a);
    friend Nfa operator|(const Nfa& lhs, const Nfa& rhs);
    friend Nfa operator&(const Nfa& lhs, const Nfa& rhs);
    static Nfa kleene(const Nfa& nfa);

    void rename(size_t start = 0);
    Nfa renamed(size_t start = 0) const;

    void deleteState(const State& state);

    std::string toDotFormat() const;

protected:
    States states_;
    Alphabet alphabet_;
    Transitions transitions_;
    State initialState_;
    States acceptStates_;

    void checkState_(const State& state);
    void checkSymbol_(Symbol symbol);

    void createInitialState_(const States& initialStates);
    void mergeStates_(const State& a, const State& b);
};
