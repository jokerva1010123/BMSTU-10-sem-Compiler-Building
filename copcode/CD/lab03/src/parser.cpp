#include <parser.hpp>

#include <algorithm>
#include <array>
#include <iostream>


namespace {

using namespace std::string_view_literals;

constexpr std::array kSignAdditionalOperators{"+"sv, "-"sv};
constexpr std::array kSignMultiplicationOperators{"*"sv, "/"sv, "%"sv};
constexpr std::array kSignRelationalOperators = {"=="sv, "<>"sv, "<="sv, ">="sv, "<"sv, ">"sv};
constexpr std::array kIdentifiers{"a"sv, "b"sv, "c"sv};
constexpr std::array kNumbers{"0"sv, "1"sv, "2"sv, "3"sv, "4"sv, "5"sv, "6"sv, "7"sv, "8"sv, "9"sv};

using Parser::Node;
using Parser::ReturnType;

struct GrammarElement {
	virtual ReturnType accept(std::string_view str, size_t depth) = 0;
};

template <typename T>
ReturnType accept(std::string_view str, size_t depth)
{
	if constexpr (std::derived_from<T, GrammarElement>) {
		return T{}.accept(str, depth);
	} else {
		return {};
	}
}

struct ArithmeticExpression;
struct ArithmeticExpression1;
struct Expression;
struct Factor;
struct Factor1;
struct Identifier;
struct Number;
struct Operator;
struct OperatorsList;
struct PrimaryExpression;
struct SignAdditionalOperation;
struct SignMultiplicationOperation;
struct SignRelationOperation;
struct Tail;
struct Term1;
struct Term;

struct Block : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "Block: " << str << std::endl;

		Node tree{"Block", {}};
		if (str.starts_with('{') && str.ends_with('}')) {
			if (auto&& [node, sv] = ::accept<OperatorsList>(str.substr(1), depth + 1); node) {
				if (sv.starts_with('}')) {
					tree.children.push_back({"{", {}});
					tree.children.push_back(std::move(*node));
					tree.children.push_back({"}", {}});
					return {tree, sv.substr(1)};
				}
			}
		}

		return {std::nullopt, str};
	}
};

struct OperatorsList : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "OperatorList: " << str << std::endl;

		Node tree{"OperatorsList", {}};
		if (auto&& [node1, sv1] = ::accept<Operator>(str, depth + 1); node1) {
            if (str.starts_with(';')) {
                if (auto&&[node2, sv2] = ::accept <Tail> (sv1.substr(1), depth + 2); node2) {
                    tree.children.push_back(std::move(*node1));
                    tree.children.push_back(std::move(*node2));
                    return {tree, sv2};
                }
            }
		}

		return {std::nullopt, str};
	}
};

struct Tail : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "Tail: " << str << std::endl;

		Node tree{"Tail", {}};
		if (auto&& [node1, sv1] = ::accept<Operator>(str, depth + 1); node1) {
			if (str.starts_with(';')) {
				if (auto&& [node2, sv2] = ::accept<Tail>(sv1.substr(1), depth + 2); node2) {
					tree.children.push_back({";", {}});
					tree.children.push_back(std::move(*node1));
					tree.children.push_back(std::move(*node2));
					return {tree, sv2};
				}
			}
		} else {
			tree.children.push_back({"ε", {}});
			return {tree, str};
		}

		return {std::nullopt, str};
	}
};

struct Operator : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "Operator: " << str << std::endl;

		Node tree{"Operator", {}};
		if (auto&& [node1, sv1] = ::accept<Identifier>(str, depth + 1); node1) {
			if (sv1.starts_with('=')) {
				if (auto&& [node2, sv2] = ::accept<Expression>(sv1.substr(1), depth + 2); node2) {
					tree.children.push_back(std::move(*node1));
					tree.children.push_back({"=", {}});
					tree.children.push_back(std::move(*node2));
					return {tree, sv2};
				}
			}
		}

		if (auto&& [node, sv] = ::accept<Block>(str, depth + 1); node) {
			tree.children.push_back(std::move(*node));
			return {tree, sv};
		}

		return {std::nullopt, str};
	}
};

struct Expression : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "Expression: " << str << std::endl;

		Node tree{"Expression", {}};
		if (auto&& [node1, sv1] = ::accept<ArithmeticExpression>(str, depth + 1); node1) {
			if (auto&& [node2, sv2] = ::accept<SignRelationOperation>(sv1, depth + 2); node2) {
				if (auto&& [node3, sv3] = ::accept<ArithmeticExpression>(sv2, depth + 3); node3) {
					tree.children.push_back(std::move(*node1));
					tree.children.push_back(std::move(*node2));
					tree.children.push_back(std::move(*node3));
					return {tree, sv3};
				}
			}
		}

		return {std::nullopt, str};
	}
};

struct ArithmeticExpression : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "ArithmeticExpression: " << str << std::endl;

		Node tree{"ArithmeticExpression", {}};
		if (auto&& [node1, sv1] = ::accept<Term>(str, depth + 1); node1) {
			if (auto&& [node2, sv2] = ::accept<ArithmeticExpression1>(sv1, depth + 2); node2) {
				tree.children.push_back(std::move(*node1));
				tree.children.push_back(std::move(*node2));
				return {tree, sv2};
			}
		}

		if (auto&& [node1, sv1] = ::accept<SignAdditionalOperation>(str, depth + 1); node1) {
			if (auto&& [node2, sv2] = ::accept<Term>(sv1, depth + 2); node2) {
				if (auto&& [node3, sv3] = ::accept<ArithmeticExpression1>(sv2, depth + 3); node3) {
					tree.children.push_back(std::move(*node1));
					tree.children.push_back(std::move(*node2));
					tree.children.push_back(std::move(*node3));
					return {tree, sv3};
				}
			}
		}

		return {std::nullopt, str};
	}
};

struct Term : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "Term: " << str << std::endl;

		Node tree{"Term", {}};
		if (auto&& [node1, sv1] = ::accept<Factor>(str, depth + 1); node1) {
			if (auto&& [node2, sv2] = ::accept<Term1>(sv1, depth + 2); node2) {
				tree.children.push_back(std::move(*node1));
				tree.children.push_back(std::move(*node2));
				return {tree, sv2};
			}
		}

		return {std::nullopt, str};
	}
};

struct Factor : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "Factor: " << str << std::endl;

		Node tree{"Factor", {}};
		if (auto&& [node1, sv1] = ::accept<PrimaryExpression>(str, depth + 1); node1) {
			if (auto&& [node2, sv2] = ::accept<Factor1>(sv1, depth + 2); node2) {
				tree.children.push_back(std::move(*node1));
				tree.children.push_back(std::move(*node2));
				return {tree, sv2};
			}
		}

		return {std::nullopt, str};
	}
};

struct PrimaryExpression : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "PrimaryExpression: " << str << std::endl;

		Node tree{"PrimaryExpression", {}};
		if (auto&& [node, sv] = ::accept<Number>(str, depth + 1); node) {
			tree.children.push_back(std::move(*node));
			return {tree, sv};
		}
		if (auto&& [node, sv] = ::accept<Identifier>(str, depth + 1); node) {
			tree.children.push_back(std::move(*node));
			return {tree, sv};
		}

		if (str.starts_with('(')) {
			if (auto&& [node, sv] = ::accept<ArithmeticExpression>(str.substr(1), depth + 1); node) {
				if (sv.starts_with(')')) {
					tree.children.push_back({"(", {}});
					tree.children.push_back(std::move(*node));
					tree.children.push_back({")", {}});
					return {tree, sv.substr(1)};
				}
			}
		}

		return {std::nullopt, str};
	}
};

struct SignAdditionalOperation : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "SignAdditionalOperation: " << str << std::endl;

		Node tree{"SignAdditionalOperation", {}};
		for (auto&& op: kSignAdditionalOperators) {
			if (str.starts_with(op)) {
				tree.children.push_back({std::string(op), {}});
				return {tree, str.substr(op.size())};
			}
		}

		return {std::nullopt, str};
	}
};

struct SignMultiplicationOperation : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "SignMultiplicationOperation: " << str << std::endl;

		Node tree{"SignMultiplicationOperation", {}};
		for (auto&& op: kSignMultiplicationOperators) {
			if (str.starts_with(op)) {
				tree.children.push_back({std::string(op), {}});
				return {tree, str.substr(op.size())};
			}
		}

		return {std::nullopt, str};
	}
};

struct SignRelationOperation : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "SignRelationOperation: " << str << std::endl;

		Node tree{"SignRelationOperation", {}};
		for (auto&& op: kSignRelationalOperators) {
			if (str.starts_with(op)) {
				tree.children.push_back({std::string(op), {}});
				return {tree, str.substr(op.size())};
			}
		}

		return {std::nullopt, str};
	}
};

struct Identifier : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "Identifier: " << str << std::endl;

		Node tree{"Identifier", {}};
		for (auto&& identifier: kIdentifiers) {
			if (str.starts_with(identifier)) {
				tree.children.push_back({std::string(identifier), {}});
				return {tree, str.substr(identifier.size())};
			}
		}

		return {std::nullopt, str};
	}
};

struct Number : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "Number: " << str << std::endl;

		Node tree{"Number", {}};
		for (auto&& number: kNumbers) {
			if (str.starts_with(number)) {
				tree.children.push_back({std::string(number), {}});
				return {tree, str.substr(number.size())};
			}
		}

		return {std::nullopt, str};
	}
};

struct ArithmeticExpression1 : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "ArithmeticExpression': " << str << std::endl;

		Node tree{"ArithmeticExpression'", {}};
		if (auto&& [node1, sv1] = ::accept<SignAdditionalOperation>(str, depth + 1); node1) {
			if (auto&& [node2, sv2] = ::accept<Term>(sv1, depth + 2); node2) {
				if (auto&& [node3, sv3] = ::accept<ArithmeticExpression1>(sv2, depth + 3); node3) {
					tree.children.push_back(std::move(*node1));
					tree.children.push_back(std::move(*node2));
					tree.children.push_back(std::move(*node3));
					return {tree, sv3};
				}
			}
		}

		tree.children.push_back({"ε", {}});
		return {tree, str};
	}
};

struct Term1 : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "Term': " << str << std::endl;

		Node tree{"Term'", {}};
		if (auto&& [node1, sv1] = ::accept<SignMultiplicationOperation>(str, depth + 1); node1) {
			if (auto&& [node2, sv2] = ::accept<Factor>(sv1, depth + 2); node2) {
				if (auto&& [node3, sv3] = ::accept<Term1>(sv2, depth + 3); node3) {
					tree.children.push_back(std::move(*node1));
					tree.children.push_back(std::move(*node2));
					tree.children.push_back(std::move(*node3));
					return {tree, sv3};
				}
			}
		}

		tree.children.push_back({"ε", {}});
		return {tree, str};
	}
};

struct Factor1 : GrammarElement {
	ReturnType accept(std::string_view str, size_t depth) override {
		std::cout << std::string(depth, '\t') << "Factor': " << str << std::endl;

		Node tree{"Factor'", {}};
		if (str.starts_with('^')) {
			if (auto&& [node1, sv1] = ::accept<PrimaryExpression>(str.substr(1), depth + 1); node1) {
				if (auto&& [node2, sv2] = ::accept<Factor1>(sv1, depth + 2); node2) {
					tree.children.push_back({"^", {}});
					tree.children.push_back(std::move(*node1));
					tree.children.push_back(std::move(*node2));
					return {tree, sv2};
				}
			}
		}

		tree.children.push_back({"ε", {}});
		return {tree, str};
	}
};

}  // namespace


namespace Parser {

ReturnType accept(std::string str) {
	str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
	return ::accept<Block>(str, 0);
}

}  // namespace Parser
