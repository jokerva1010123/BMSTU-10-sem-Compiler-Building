#include <array>
#include <iostream>
#include <stdexcept>

#include <graphviz.hpp>


namespace {

bool sTest(size_t i, const std::string& input, bool expected_tree_has_value, bool expected_str_empty) {
	std::cout.setstate(std::ios_base::badbit);
	auto&& [tree, str] = Parser::accept(input);
	std::cout.clear();

	constexpr auto printExpected = [](std::string_view what, bool expected) {
		std::cout << " -- Expected " << what << " == " << std::boolalpha << expected << ", got " << !expected << std::endl;
	};

	bool pass = true;
	if (tree.has_value() != expected_tree_has_value) {
		printExpected("tree.has_value()", expected_tree_has_value);
		pass = false;
	}
	if (str.empty() != expected_str_empty) {
		printExpected("str.empty()", expected_str_empty);
		pass = false;
	}

	std::cout << "Test " << std::to_string(i) << (pass ? " passed" : " failed") << std::endl;

	return pass;
}

const std::array<std::tuple<std::string, bool, bool>, 10> cTests = {{
	{"{a = 1 < 2}", true, true},
	{"{a = 1 < 2;}", false, false},
	{"{a = 1 < 2; a = b * a == 0}", true, true},
	{"{a = (b) * a <> 3}", true, true},
	{"{a = 1 >= 2; {a = -a < 0}}", true, true},
	{"{a = 1 <> a - a}", true, true},
	{"{a = 1 <> a; {b = -1 > a}}", true, true},
	{"{a = a / b % c == 9}", true, true},
	{"{a = a / b % c == 9; {a = +b <= 1}}", true, true},
	{"{}", false, false},
}};

void sRunTests() {
	size_t i = 1;
	size_t passed = 0;
	for (const auto& [input, expected_tree_has_value, expected_str_empty] : cTests) {
		passed += sTest(i, input, expected_tree_has_value, expected_str_empty);
		++i;
	}
	std::cout << "Passed " << passed << "/" << cTests.size() << std::endl;
}

}  // namespace


int main() {
	sRunTests();

	std::string s = "{ a = -b + 1 < (-a + b) * b / a; {a = a <> a; {c = 2 > a;};};}";
	auto&& [tree, str] = Parser::accept(s);
	std::cout << str << std::endl;

	if (tree && str.empty()) {
		Graphviz::saveDot(*tree, "../tree.dot");
		auto link = Graphviz::generateLinkToGraphvizOnline("../tree.dot");
		std::cout << link << std::endl;
	}
}
