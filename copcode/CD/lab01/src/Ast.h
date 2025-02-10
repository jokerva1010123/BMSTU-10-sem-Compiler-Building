#pragma once

#include <string>
#include <string_view>

#include <AstNode.h>


class Ast {
public:
    explicit Ast(std::string_view expression);

    const AstNode* root() const { return root_; }

    std::string toDotFormat() const;

private:
    AstNode* root_;
};
