#include <Ast.h>

#include <iomanip>
#include <sstream>

#include <RecursiveDescentParser.h>


namespace {

std::string sToDotFormat(AstNode* node) {
    std::stringstream stream;
    stream << "\t\"" << std::hex << reinterpret_cast<const void*>(node) << "\" [label=\"" << node->data << "\"]\n";

    if (node->left || node->right) {
        stream << "\t\"" << std::hex << reinterpret_cast<const void*>(node) << "\" -- {\n";
        for (auto* child : {node->left, node->right}) {
            if (child) {
                stream << "\t\t\"" << std::hex << reinterpret_cast<const void*>(child) << "\"\n";
            }
        }
        stream << "\t}\n\n";
        for (auto* child : {node->left, node->right}) {
            if (child) {
                stream << sToDotFormat(child);
            }
        }
    }
    return stream.str();

}

}  // namespace


Ast::Ast(std::string_view expression)
    : root_{RecursiveDescentParser{}.parse(expression)}
{
}

std::string Ast::toDotFormat() const {
    return "graph AST {" + sToDotFormat(root_) + "}\n";
}
