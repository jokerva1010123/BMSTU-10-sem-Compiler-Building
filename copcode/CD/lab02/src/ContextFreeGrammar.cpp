#include <ContextFreeGrammar.h>

#include <experimental/iterator>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>



namespace {

template <typename T1, typename T2>
using Pair = std::pair<T1, T2>;

template <typename T>
Set<T> sIntersection(const Set<T>& a, const Set<T>& b) {
    Set<T> result;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(result, result.begin()));
    return result;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Set<T>& set) {
    std::copy(set.begin(), set.end(), std::experimental::make_ostream_joiner(os, " "));
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Vector<T>& v) {
    std::copy(v.begin(), v.end(), std::experimental::make_ostream_joiner(os, " "));
    return os;
}

template <typename T>
T sSqr(const T& x) {
    return x * x;
}

Vector<Word> sGenerateAllPermutations(const Word& rule, const Alphabet& epsilon) {
    Vector<size_t> epsilon_in_rule;
    for (size_t i = 0; i < rule.size(); ++i) {
        if (epsilon.contains(rule[i])) {
            epsilon_in_rule.push_back(i);
        }
    }

    if (epsilon_in_rule.empty()) {
        return {};
    }

    Vector<Word> result;

    const auto permutations = sSqr(epsilon_in_rule.size());
    for (int n = 0; n < permutations; ++n) {
        auto s = rule;
        for (int i = 0; i < epsilon_in_rule.size(); ++i) {
            int bit = 1 << i;
            if (!(n & bit)) {
                s.erase(s.begin() + epsilon_in_rule[epsilon_in_rule.size() - 1 - i]);
            }
        }
        result.push_back(std::move(s));
    }

    return result;
}


}  // namespace


ContextFreeGrammar::ContextFreeGrammar(
    Alphabet N,
    Alphabet T,
    ProductionRules P,
    Symbol S
)
    : N_{std::move(N)}
    , T_{std::move(T)}
    , P_{std::move(P)}
    , S_{std::move(S)}
{
    if (!sIntersection(N_, T_).empty()) {
        throw std::invalid_argument("[ContextFreeGrammar::ContextFreeGrammar] expected different symbols in N and T");
    }

    for (const auto& A : N_) {
        if (A.empty()) {
            throw std::invalid_argument("[ContextFreeGrammar::ContextFreeGrammar] expected not empty symbol in N");
        }
    }
    for (const auto& a : T_) {
        if (a.empty()) {
            throw std::invalid_argument("[ContextFreeGrammar::ContextFreeGrammar] expected not empty symbol in T");
        }
    }

    for (const auto& [s, ws] : P_) {
        if (!N_.contains(s)) {
            throw std::invalid_argument("[ContextFreeGrammar::ContextFreeGrammar] expected non-terminal in the left side of production rule");
        }
        for (const auto& w : ws) {
            for (const auto& c : w) {
                if (!N_.contains(c) && !T_.contains(c)) {
                    throw std::invalid_argument("[ContextFreeGrammar::ContextFreeGrammar] expected grammar symbol in the right side of production rule");
                }
            }
        }
    }

    if (!N_.contains(S_) && !(N_.empty() && S_.empty())) {
        throw std::invalid_argument("[ContextFreeGrammar::ContextFreeGrammar] expected S to be in N");
    }
}

std::istream& operator>>(std::istream& is, ContextFreeGrammar& grammar) {
    Alphabet N;
    Alphabet T;
    ProductionRules P;
    Symbol S;

    size_t n = 0;
    is >> n;
    for (size_t i = 0; i < n; ++i) {
        Symbol A;
        is >> A;
        N.insert(std::move(A));
    }

    is >> n;
    for (size_t i = 0; i < n; ++i) {
        Symbol a;
        is >> a;
        T.insert(std::move(a));
    }

    is >> n;
    for (size_t i = 0; i < n; ++i) {
        Symbol key;
        is >> key;
        std::string arrow;
        is >> arrow;
        if (arrow != "->") {
            throw std::invalid_argument("[operator>>(std::istream&, ContextFreeGrammar&)] expected ->");
        }
        std::string line;
        std::getline(is, line);
        std::stringstream ss{line};
        Word word;
        Symbol alpha;
        while (ss >> alpha) {
            word.push_back(std::move(alpha));
        }

        P[key].insert(std::move(word));
    }

    is >> S;

    grammar = ContextFreeGrammar{std::move(N), std::move(T), std::move(P), std::move(S)};

    return is;
}


std::ostream& operator<<(std::ostream& os, const ContextFreeGrammar& grammar) {
    os << grammar.N_.size() << '\n';
    os << grammar.N_ << '\n';
    os << grammar.T_.size() << '\n';
    os << grammar.T_ << '\n';

    size_t counter = 0;

    for (const auto& [s, ws] : grammar.P_) {
        counter += ws.size();
    }

    os << counter << '\n';

    for (const auto& [s, ws] : grammar.P_) {
        for (const auto& w : ws) {
            os << s << " -> " << w << '\n';
        }
    }

    os << grammar.S_ << '\n';

    return os;
}

ContextFreeGrammar ContextFreeGrammar::readFromFile(const std::filesystem::path& path) {
    std::ifstream stream{path};
    ContextFreeGrammar grammar;
    stream >> grammar;
    return grammar;
}

void ContextFreeGrammar::writeToFile(const std::filesystem::path& path) const {
    std::ofstream stream{path};
    stream << *this;
}

void ContextFreeGrammar::eliminateLeftRecursion() {
    auto N = N_;
    for (auto i = N.begin(); i != N.end(); ++i) {
        const auto& Ai = *i;
        for (auto j = N.begin(); j != i; ++j) {
            const auto& Aj = *j;

            auto PAi = P_[Ai];
            for (const auto& Aj_gamma : P_[Ai]) {
                if (Aj_gamma.empty() || Aj_gamma.front() != Aj) {
                    continue;
                }
                PAi.erase(Aj_gamma);

                Word gamma(Aj_gamma.begin() + 1, Aj_gamma.end());
                for (const auto& delta : P_[Aj]) {
                    auto delta_gamma = delta;
                    delta_gamma.insert(delta_gamma.end(), gamma.begin(), gamma.end());
                    PAi.insert(delta_gamma);
                }
            }
            P_[Ai] = std::move(PAi);
        }
        eliminateImmediateLeftRecursion_(Ai);
    }
}

void ContextFreeGrammar::eliminateImmediateLeftRecursion_(const Symbol& A) {
    bool is_break = false;
    for (const auto& word : P_[A]) {
        if (!word.empty() && word.front() == A) {
            is_break = true;
            break;
        }
    }

    if (!is_break) {
        return;
    }

    auto A1 = A + '\'';
    while (N_.contains(A1)) {
        A1 += '\'';
    }

    N_.insert(A1);

    auto PA = P_[A];
    for (const auto& word : P_[A]) {
        PA.erase(word);
        if (!word.empty() && word.front() == A) {
            const auto& A_alpha = word;
            Word alpha_A1(A_alpha.begin() + 1, A_alpha.end());
            alpha_A1.push_back(A1);
            P_[A1].insert(alpha_A1);
        } else {
            const auto& beta = word;
            auto beta_A1 = beta;
            beta_A1.push_back(A1);
            PA.insert(beta_A1);
        }
    }
    P_[A] = std::move(PA);
    P_[A1].insert(kEpsilon);
}

void ContextFreeGrammar::eliminateEpsRules() {
    auto eps_generating = findEpsGeneratingNonTerminals_();
    auto start_is_epsilon = eps_generating.contains(S_);

    auto P1 = P_;
    for (auto&& [A, rules] : P_) {
        for (auto&& rule : rules) {
            auto permutations = sGenerateAllPermutations(rule, eps_generating);
            P1[A].insert(permutations.begin(), permutations.end());
        }
    }

    P_ = P1;
    for (auto&& [A, rules] : P_) {
        for (auto&& rule : rules) {
            if (rule.empty()) {
                P1[A].erase(rule);
                if (P1[A].empty()) {
                    P1.erase(A);
                }
            }
        }
    }

    P_ = std::move(P1);

    if (start_is_epsilon) {
        auto S1 = S_ + '\'';
        while (N_.contains(S1)) {
            S1 += '\'';
        }
        N_.insert(S1);
        P_[S1] = {{S_}, kEpsilon};
    }
}

Alphabet ContextFreeGrammar::findEpsGeneratingNonTerminals_() const {
    // множество eps-порождающих нетерминалов
    Alphabet is_epsilon;

    // пронумерованные правила
    Vector<Pair<Symbol, Word>> rules;
    for (const auto& [s, ws] : P_) {
        for (const auto& w : ws) {
            bool is_break = false;
            for (const auto& c : w) {
                if (T_.contains(c)) {
                    is_break = true;
                    break;
                }
            }
            if (!is_break) {
                rules.emplace_back(s, w);
            }
        }
    }

    // для каждого нетерминала храним список номеров тех правил, в правой части которых он встречается
    UMap<Symbol, Set<size_t>> concerned_rules;

    // для каждого правила храним счетчик количества нетерминалов в правой части, которые еще не помечены eps-порождающими
    Vector<size_t> counter(rules.size());

    // очередь нетерминалов, помеченных eps-порождающими, но еще не обработанных
    Alphabet q;

    for (size_t i = 0; i != rules.size(); ++i) {
        const auto& [left, right] = rules[i];
        Alphabet unique;
        for (const auto& symbol : right) {
            if (N_.contains(symbol)) {
                unique.insert(symbol);
                concerned_rules[symbol].insert(i);
            }
        }
        counter[i] = unique.size();
        if (counter[i] == 0) {
            q.insert(left);
            is_epsilon.insert(left);
        }
    }

    while (!q.empty()) {
        auto A = *q.begin();
        q.erase(q.begin());

        for (auto i : concerned_rules[A]) {
            --counter[i];
            if (counter[i] == 0) {
                if (!is_epsilon.contains(rules[i].first)) {
                    is_epsilon.insert(rules[i].first);
                    q.insert(rules[i].first);
                }
            }
        }
    }

    return is_epsilon;
}
