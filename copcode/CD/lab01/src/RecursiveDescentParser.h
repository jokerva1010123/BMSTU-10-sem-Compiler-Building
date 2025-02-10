#pragma once

#include <AstNode.h>
#include <CharReader.h>


/*
 * A parser to be constructed each time
 * a regular expression needs to be parsed.
 */
class RecursiveDescentParser : public CharReader {
public:
    AstNode* parse(std::string_view expression);

private:
    /* Regular expression term types. */
    AstNode* regex_();
    AstNode* term_();
    AstNode* factor_();
    AstNode* base_();
};
