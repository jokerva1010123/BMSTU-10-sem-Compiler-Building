#pragma once

#include <filesystem>
#include <istream>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>


template <typename T>
using Set = std::set<T>;

template <typename Key, typename T>
using UMap = std::unordered_map<Key, T>;

template <typename T>
using Vector = std::vector<T>;

using Symbol = std::string;
using Alphabet = Set<Symbol>;
using Word = Vector<Symbol>;

using ProductionRules = UMap<Symbol, Set<Word>>;

const auto kEpsilon = Word{};


class ContextFreeGrammar {
public:
    ContextFreeGrammar(
        Alphabet N = {},
        Alphabet T = {},
        ProductionRules P = {},
        Symbol S = {}
    );

    friend std::istream& operator>>(std::istream& is, ContextFreeGrammar& grammar);
    friend std::ostream& operator<<(std::ostream& os, const ContextFreeGrammar& grammar);

    static ContextFreeGrammar readFromFile(const std::filesystem::path& path);
    void writeToFile(const std::filesystem::path& path) const;

    bool operator==(const ContextFreeGrammar& grammar) const = default;

    void eliminateLeftRecursion();
    void eliminateEpsRules();


private:
    Alphabet N_;
    Alphabet T_;
    ProductionRules P_;
    Symbol S_;

    void eliminateImmediateLeftRecursion_(const Symbol& A);
    Alphabet findEpsGeneratingNonTerminals_() const;
};
