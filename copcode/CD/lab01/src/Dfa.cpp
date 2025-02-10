#include <Dfa.h>

#include <algorithm>
#include <deque>
#include <experimental/iterator>
#include <ostream>
#include <set>
#include <sstream>

namespace {

template <typename T>
std::ostream& operator<<(std::ostream& os, const Set<T>& set) {
    os << "{";
    std::copy(set.begin(), set.end(), std::experimental::make_ostream_joiner(os, ", "));
    os << "}";
    return os;
}

template <typename T>
std::string sToString(const Set<T>& set) {
    std::stringstream ss;
    ss << set;
    return ss.str();
}

template <typename T>
Set<T> sIntersection(const Set<T>& a, const Set<T>& b) {
    Set<T> result;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(result, result.begin()));
    return result;
}

template <typename T>
Set<T> sDifference(const Set<T>& a, const Set<T>& b) {
    Set<T> result;
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(result, result.end()));
    return result;
}

States sEClosure(const Nfa& nfa, const States& T) {
    States stack = T;
    States closure = T;
    while (!stack.empty()) {
        auto t = *stack.begin();
        stack.erase(stack.begin());
        for (auto&& u : nfa.transition(t, 'E')) {
            if (!closure.contains(u)) {
                closure.insert(u);
                stack.insert(u);
            }
        }
    }
    return closure;
}

States sMove(const Nfa& nfa, const States& T, Symbol a) {
    States states;
    for (const auto& state : T) {
        auto toStates = nfa.transition(state, a);
        states.insert(toStates.begin(), toStates.end());
    }
    return states;
}

std::pair<States, States> sSplit(const Dfa& dfa, const States& R, const States& C, Symbol a) {
    States R1, R2;
    for (auto&& r : R) {
        if (auto c = dfa.transition(r, a); c.size() == 1 && C.contains(*c.begin())) {
            R1.insert(r);
        } else {
            R2.insert(r);
        }
    }
    return {std::move(R1), std::move(R2)};
}

} // namespace


Dfa::Dfa(const Nfa& nfa) {
    alphabet_ = nfa.alphabet();

    auto initialState = sEClosure(nfa, {nfa.initialState()});
    initialState_ = sToString(initialState);
    states_.insert(initialState_);
    Set<States> nonMarked = {initialState};
    if (!sIntersection(initialState, nfa.acceptStates()).empty()) {
        acceptStates_.insert(initialState_);
    }
    while (!nonMarked.empty()) {
        auto T = *nonMarked.begin();
        nonMarked.erase(nonMarked.begin());
        for (auto a : alphabet_) {
            auto U = sEClosure(nfa, sMove(nfa, T, a));
            auto strU = sToString(U);
            if (!states_.contains(strU)) {
                states_.insert(strU);
                nonMarked.insert(U);
                if (!sIntersection(U, nfa.acceptStates()).empty()) {
                    acceptStates_.insert(strU);
                }
            }
            transitions_[sToString(T)][a].insert(strU);
        }
    }

    deleteState("{}");
}

void Dfa::minimize() {
    Set<States> P{acceptStates_, sDifference(states_, acceptStates_)};
    std::deque<std::pair<States, Symbol>> S;

    for (auto c : alphabet_) {
        for (auto&& p : P) {
            S.emplace_back(p, c);
        }
    }

    while (!S.empty()) {
        auto [C, a] = S.front();
        S.pop_front();

        auto new_P = P;
        for (auto&& R : P) {
            auto [R1, R2] = sSplit(*this, R, C, a);
            if (!R1.empty() && !R2.empty()) {
                new_P.erase(R);  // replace R with R1 and R2
                new_P.insert(R1);
                new_P.insert(R2);

                if (auto it = std::find(S.begin(), S.end(), std::pair{R, a}); it != S.end()) {
                    S.erase(it);
                    S.emplace_back(R1, a);
                    S.emplace_back(R2, a);
                } else {
                    if (R1.size() <= R2.size()) {
                        S.emplace_back(R1, a);
                    } else {
                        S.emplace_back(R2, a);
                    }
                }
            }
        }

        std::swap(P, new_P);
    }

    for (auto&& states : P) {
        if (states.size() > 1) {
            const auto& origin = *states.begin();
            for (auto it = std::next(states.begin()); it != states.end(); ++it) {
                mergeStates_(origin, *it);}
        }
    }
}

bool Dfa::checkWord(std::string_view word) const {
    auto s = initialState_;
    for (auto c : word) {
        auto t = transition(s, c);
        if (t.empty()) {
            return false;
        }
        if (t.size() > 1) {
            throw std::invalid_argument("[Dfa::checkWord] FA is not deterministic");
        }
        s = *t.begin();
    }
    return acceptStates_.contains(s);
}
