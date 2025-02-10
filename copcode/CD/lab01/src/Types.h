#pragma once

#include <set>
#include <string>
#include <unordered_map>


template <typename T>
using Set = std::set<T>;

template <typename Key, typename T>
using UMap = std::unordered_map<Key, T>;

using State = std::string;
using States = Set<State>;

using Symbol = char;
using Alphabet = Set<Symbol>;

using Transitions = UMap<State, UMap<Symbol, States>>;
