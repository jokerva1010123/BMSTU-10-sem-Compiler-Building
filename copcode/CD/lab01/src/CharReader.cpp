#include <CharReader.h>

#include <stdexcept>


void CharReader::load_(std::string_view expression) {
    expression_ = expression;
}

char CharReader::peek_() const {
    return expression_[0];
}

char CharReader::next_() {
    char c = peek_();
    eat_();
    return c;
}

void CharReader::eat_() {
    expression_ = std::string_view{expression_.begin() + 1, expression_.end()};
}

void CharReader::eat_(char item) {
    if (peek_() != item) {
        throw std::invalid_argument("[CharReader::eat_] peek_ not equal to item");
    }
    eat_();
}

bool CharReader::empty_() const {
    return expression_.empty();
}
