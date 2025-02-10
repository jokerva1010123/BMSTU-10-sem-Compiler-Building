#include <Nfa.h>

#include <stdexcept>

namespace {

Nfa sTraverse(const AstNode* node) {
    if (!node->left && !node->right) {
        if (node->data == 'E') {
            return Nfa::baseEpsilon();
        }
        return Nfa::baseSymbol(node->data);
    }

    if (node->data == '|') {
        return sTraverse(node->left) | sTraverse(node->right);
    }
    if (node->data == '&') {
        return sTraverse(node->left) & sTraverse(node->right);
    }
    if (node->data == '*') {
        return Nfa::kleene(sTraverse(node->left));
    }

    throw std::invalid_argument("[sTraverse] unexpected node->data: " + std::string{node->data});
}

}  // namespace




Nfa::Nfa(States states,
         Alphabet alphabet,
         Transitions transitions,
         State initialState,
         States acceptStates)
    : states_{std::move(states)}
    , alphabet_{std::move(alphabet)}
    , transitions_{std::move(transitions)}
    , initialState_{std::move(initialState)}
    , acceptStates_{std::move(acceptStates)}
{
    for (const auto& [from, mapSymbolToStates] : transitions_) {
        checkState_(from);

        for (const auto& [symbol, toStates] : mapSymbolToStates) {
            checkSymbol_(symbol);

            for (const auto& state : toStates) {
                checkState_(state);
            }
        }
    }

    if (!states_.empty()) {
        checkState_(initialState_);
    }

    for (const auto& acceptState : acceptStates_) {
        checkState_(acceptState);
    }
}

Nfa::Nfa(const Ast& ast)
    : Nfa{sTraverse(ast.root())}
{
}

void Nfa::checkState_(const State& state) {
    if (!states_.contains(state)) {
        throw std::invalid_argument("[Nfa::checkState_] there is no such state in states_: \"" + state + "\"");
    }
}

void Nfa::checkSymbol_(Symbol symbol) {
    if (symbol != 'E' && !alphabet_.contains(symbol)) {
        throw std::invalid_argument("[Nfa::checkSymbol_] there is no such symbol in alphabet_: \'" + std::string{symbol} + "\'");
    }
}

States Nfa::transition(const State& from, Symbol symbol) const {
    if (auto itFrom = transitions_.find(from); itFrom != transitions_.end()) {
        if (auto it = itFrom->second.find(symbol); it != itFrom->second.end()) {
            return it->second;
        }
    }
    return {};
}

Nfa Nfa::baseEpsilon() {
    return Nfa{
            {"0", "1"},
            {},
            {{"0", {{'E', {"1"}}}}},
            "0",
            {"1"},
    };
}

Nfa Nfa::baseSymbol(Symbol a) {
    return Nfa{
            {"0", "1"},
            {a},
            {{"0", {{a, {"1"}}}}},
            "0",
            {"1"},
    };
}

Nfa operator|(const Nfa& lhs, const Nfa& rhs) {
    Nfa nfa;
    auto left = lhs.renamed(1);
    auto right = rhs.renamed(left.states_.size() + 1);

    nfa.initialState_ = "0";
    auto acceptState = std::to_string(left.states_.size() + right.states_.size() + 1);
    nfa.acceptStates_ = {acceptState};

    nfa.states_= {nfa.initialState_, acceptState};
    nfa.states_.insert(left.states_.begin(), left.states_.end());
    nfa.states_.insert(right.states_.begin(), right.states_.end());

    nfa.alphabet_ = std::move(left.alphabet_);
    nfa.alphabet_.insert(right.alphabet_.begin(), right.alphabet_.end());

    nfa.transitions_ = std::move(left.transitions_);
    nfa.transitions_.insert(right.transitions_.begin(), right.transitions_.end());
    nfa.transitions_[nfa.initialState_]['E'] = {left.initialState_, right.initialState_};
    for (const auto& state : left.acceptStates_) {
        nfa.transitions_[state]['E'].insert(acceptState);
    }
    for (const auto& state : right.acceptStates_) {
        nfa.transitions_[state]['E'].insert(acceptState);
    }

    return nfa;
}

Nfa operator&(const Nfa& lhs, const Nfa& rhs) {
    Nfa nfa;
    auto left = lhs.renamed();
    auto right = rhs.renamed(left.states_.size());

    nfa.initialState_ = left.initialState_;
    nfa.acceptStates_ = right.acceptStates_;

    nfa.states_.insert(left.states_.begin(), left.states_.end());
    nfa.states_.insert(right.states_.begin(), right.states_.end());

    nfa.alphabet_ = std::move(left.alphabet_);
    nfa.alphabet_.insert(right.alphabet_.begin(), right.alphabet_.end());

    nfa.transitions_ = std::move(left.transitions_);
    nfa.transitions_.insert(right.transitions_.begin(), right.transitions_.end());
    for (const auto& state : left.acceptStates_) {
        nfa.transitions_[state]['E'].insert(right.initialState_);
    }

    return nfa;
}

Nfa Nfa::kleene(const Nfa& lhs) {
    Nfa nfa;
    auto left = lhs.renamed(1);

    nfa.initialState_ = "0";
    auto acceptState = std::to_string(left.states_.size() + 1);
    nfa.acceptStates_ = {acceptState};

    nfa.states_= {nfa.initialState_, acceptState};
    nfa.states_.insert(left.states_.begin(), left.states_.end());

    nfa.alphabet_ = std::move(left.alphabet_);

    nfa.transitions_ = std::move(left.transitions_);
    nfa.transitions_[nfa.initialState_]['E'] = {left.initialState_, acceptState};
    for (const auto& state : left.acceptStates_) {
        nfa.transitions_[state]['E'].insert(left.initialState_);
        nfa.transitions_[state]['E'].insert(acceptState);
    }
    return nfa;
}

void Nfa::rename(size_t start) {
    States newStates;
    UMap<State, State> map;
    for (const auto& state : states_) {
        auto newState = std::to_string(start);
        newStates.insert(newState);
        map.emplace(state, std::move(newState));
        ++start;
    }
    states_ = std::move(newStates);

    Transitions newTransitions;
    for (const auto& [from, mupSymbolToStates] : transitions_) {
        const auto& newFrom = map[from];
        for (const auto& [symbol, toStates] : mupSymbolToStates) {
            for (const auto& state : toStates) {
                 const auto& newState = map[state];
                 newTransitions[newFrom][symbol].insert(newState);
            }
        }
    }
    transitions_ = std::move(newTransitions);

    initialState_ = map[initialState_];

    States newAcceptStates;
    for (const auto& state : acceptStates_) {
        newAcceptStates.insert(map[state]);
    }
    acceptStates_ = std::move(newAcceptStates);
}

Nfa Nfa::renamed(size_t start) const {
    Nfa newNfa = *this;
    newNfa.rename(start);
    return newNfa;
}

void Nfa::deleteState(const State& state) {
    auto it = states_.find(state);
    if (it == states_.end()) {
        return;
    }

    states_.erase(it);

    if (initialState_ == state) {
        States initialStates;
        for (auto&& [symbol, toStates] : transitions_[state]) {
            initialStates.insert(toStates.begin(), toStates.end());
        }
        if (!initialStates.empty()) {
            createInitialState_(initialStates);
        }
    }

    transitions_.erase(state);

    for (auto&& [from, mapSymbolToStates] : transitions_) {
        for (auto&& [symbol, toStates] : mapSymbolToStates) {
            if (auto it = toStates.find(state); it != toStates.end()) {
                toStates.erase(it);
            }
        }
    }

    acceptStates_.erase(state);
}


std::string Nfa::toDotFormat() const {
    std::string dot =
            "digraph FA {\n"
            "\trankdir=\"LR\"\n"
            "\t\"\" [shape=none]\n\n";

    dot += "\t\"\" -> \"" + initialState_ + "\"\n";

    for (auto&& state : acceptStates_) {
        dot += "\t\"" + state + "\" [peripheries=2]\n";
    }
    dot += "\n";

    for (auto&& [from, mapSymbolToStates] : transitions_) {
        const auto fromStr = "\t\"" + from + "\" -> \"";
        for (auto&& [symbol, toStates] : mapSymbolToStates) {
            const auto symbolStr = "\" [label=\"" + (symbol == 'E' ? "ε" : std::string{symbol}) + "\"]\n";
            for (auto&& to : toStates) {
                dot += fromStr;
                dot += to;
                dot += symbolStr;
            }
        }
    }
    dot += "}\n";

    return dot;
}

void Nfa::createInitialState_(const States& initialStates) {
    initialState_ = "s0";
    while (states_.contains(initialState_)) {
        initialState_ += "0";
    }

    states_.insert(initialState_);
    for (auto&& state : initialStates) {
        transitions_[initialState_]['L'].insert(state);
    }
}

void Nfa::mergeStates_(const State& a, const State& b) {
    states_.erase(b);

    Transitions newTransitions;
    for (auto&& [from, mapSymbolToStates] : transitions_) {
        const auto& mergedFrom = from == b ? a : from;
        for (auto&& [symbol, toStates] : mapSymbolToStates) {
            for (auto&& to : toStates) {
                const auto& mergedTo = to == b ? a : to;
                if (mergedFrom == a && mergedTo == a) {
                    auto tr = transition(b, symbol);
                    tr.erase(a);
                    tr.erase(b);
                    if (!tr.empty()) {
                        continue;
                    }
                }
                newTransitions[mergedFrom][symbol].insert(mergedTo);
            }
        }
    }

    std::swap(newTransitions, transitions_);

    if (initialState_ == b) {
        initialState_ = a;
    }

    if (auto it = acceptStates_.find(b); it != acceptStates_.end()) {
        acceptStates_.erase(it);
        acceptStates_.insert(a);
    }
}
