#include <RecursiveDescentParser.h>


AstNode* RecursiveDescentParser::parse(std::string_view expression) {
    load_(expression);
    return regex_();
}

// For regex_() method, we know that we must parse at least one term,
// and whether we parse another depends only on what we find afterward
AstNode* RecursiveDescentParser::regex_() {
    auto* term = term_();

    if (!empty_() && peek_() == '|') {
        eat_();  // '|'
        auto* regex = regex_() ;
        return new AstNode{'|', term, regex};
    }
    return term;
}

// term_() has to check that it has not reached the boundary of a term or the end of the input
AstNode* RecursiveDescentParser::term_() {
    if (empty_() || peek_() == ')' || peek_() == '|') {
        return new AstNode{'E'};
    }

    auto* factor = factor_();

    while (!empty_() && peek_() != ')' && peek_() != '|') {
        auto* nextFactor = factor_() ;
        factor = new AstNode{'&', factor, nextFactor};
    }
    return factor;
}

// To implement factor, we parse a base and then any number of Kleene stars
AstNode* RecursiveDescentParser::factor_() {
    auto* base = base_() ;

    while (!empty_() && peek_() == '*') {
        eat_();  // '*'
        base = new AstNode{'*', base};
    }
    return base;
}

// The implementation of base_() checks to see which of the three cases it has encountered
AstNode* RecursiveDescentParser::base_() {
    if (peek_() == '(') {
        eat_();  // '('
        auto *r = regex_();
        eat_(')');
        return r;
    }
    return new AstNode{next_()};
}
