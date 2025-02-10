#include <functional>
#include <iostream>

#include <ContextFreeGrammar.h>

namespace {

using Callback = std::function<void(ContextFreeGrammar&)>;

bool sTest(const std::filesystem::path& input, const std::filesystem::path& output, const std::filesystem::path& expected, const Callback& callback) {
    auto grammar = ContextFreeGrammar::readFromFile(input);
    callback(grammar);
    grammar.writeToFile(output);
    auto expected_grammar = ContextFreeGrammar::readFromFile(expected);
    return grammar == expected_grammar;
}

bool sEliminateLeftRecursionTest(size_t n) {
    auto filename = [n](const std::string& name) {
        return "../tests/LEFT-" + std::to_string(n) + "-" + name + ".txt";
    };
    return sTest(filename("input"), filename("output"), filename("expected"), [](ContextFreeGrammar& grammar) {
        grammar.eliminateLeftRecursion();
    });
}

bool sEliminateEpsRulesTest(size_t n) {
    auto filename = [n](const std::string& name) {
        return "../tests/EPS-" + std::to_string(n) + "-" + name + ".txt";
    };
    return sTest(filename("input"), filename("output"), filename("expected"), [](ContextFreeGrammar& grammar) {
        grammar.eliminateEpsRules();
    });
}

}  // namespace


int main() {
    std::cout << "LEFT-1: " << (sEliminateLeftRecursionTest(1) ? "passed" : "failed") << std::endl;
    std::cout << "LEFT-2: " << (sEliminateLeftRecursionTest(2) ? "passed" : "failed") << std::endl;
    std::cout << " EPS-1: " << (sEliminateEpsRulesTest(1) ? "passed" : "failed") << std::endl;
    std::cout << " EPS-2: " << (sEliminateEpsRulesTest(2) ? "passed" : "failed") << std::endl;
    std::cout << " EPS-3: " << (sEliminateEpsRulesTest(3) ? "passed" : "failed") << std::endl;
}
