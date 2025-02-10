#pragma once

#include <string_view>


class CharReader {
protected:
    void load_(std::string_view expression);

    // returns the next_ item of input without consuming it
    char peek_() const;

    // returns the next_ item of input and consumes it
    char next_();

    // consumes the next_ item of input
    void eat_();

    // consumes the next_ item of input, failing if not equal to item
    void eat_(char item);

    bool empty_() const;

private:
    std::string_view expression_;
};
