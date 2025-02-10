#include <iostream>
#include <string>

#include <Ast.h>
#include <Dfa.h>
#include <Graphviz.h>
#include <Nfa.h>


int main() {
    std::cout << "Ввод регулярного выражения >> ";
    std::string re;
    std::getline(std::cin, re);

    Ast ast{re};
    std::cout << "AST:\n" << generateLinkToGraphvizOnline(ast.toDotFormat()) << std::endl;

    Nfa nfa{ast};
    std::cout << "NFA:\n" << generateLinkToGraphvizOnline(nfa.toDotFormat()) << std::endl;

    Dfa dfa{nfa};
    std::cout << "DFA:\n" << generateLinkToGraphvizOnline(dfa.toDotFormat()) << std::endl;

    dfa.rename();
    std::cout << "DFA (renamed):\n" << generateLinkToGraphvizOnline(dfa.toDotFormat()) << std::endl;

    dfa.minimize();
    std::cout << "DFA (minimized):\n" << generateLinkToGraphvizOnline(dfa.toDotFormat()) << std::endl;

    std::string word;
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, word);
        if (word == "stop") {
            break;
        }
        std::cout << dfa.checkWord(word) << std::endl;
    }

}
