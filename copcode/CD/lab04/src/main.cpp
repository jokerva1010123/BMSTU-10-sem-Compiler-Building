#include <array>
#include <iostream>
#include <tuple>

#include <parser.hpp>


namespace {

bool sTest(size_t i, const std::vector<std::string>& input, bool f_expected, const std::string& r_expected) {
	auto&& [f, r] = parse(input);

	constexpr auto printExpected = [](std::string_view what, auto& expected, auto& got) {
		std::cout << " -- Expected " << what << " == " << std::boolalpha << expected << ", got " << got << std::endl;
	};

	bool pass = true;
	if (f != f_expected) {
		printExpected("[f, _]", f_expected, f);
		pass = false;
	}
	if (r != r_expected) {
		printExpected("[_, r]", r_expected, r);
		pass = false;
	}

	std::cout << "Test " << std::to_string(i) << (pass ? " passed" : " failed") << std::endl;

	return pass;
}

const std::array<std::tuple<std::vector<std::string>, bool, std::string>, 10> cTests = {{
	{{"a", "<", "b"}, true, "a b <"},
	{{"a", "b", "c"}, false, ""},
	{{"(", "(", "(", "a", "/", "b", ")", ")", ")"}, true, "a b /"},
	{{"(", "a", "%", "b", ")", "^", "c"}, true, "a b % c ^"},
	{{"a", "<", "c", "=", "(", "b", ">", "c", ")", "<", "1"}, true, "a c < b c > = 1 <"},
	{{"a", "+", "b", "*", "(", "a", "-", "c", ")"}, true, "a b a c - * +"},
	{{"(", "a", "*", "2", ")", "/", "c", "+", "("}, false, ""},
	{{"(", "a", "*", "2", ")", "/", "(", "c", "+", "2", ")"}, true, "a 2 * c 2 + /"},
	{{"(", "a", "b", "*", "c", ")"}, false, ""},
}};

void sRunTests() {
	size_t i = 1;
	size_t passed = 0;
	for (const auto& [input, f_expected, r_expected] : cTests) {
		passed += sTest(i, input, f_expected, r_expected);
		++i;
	}
	std::cout << "Passed " << passed << "/" << cTests.size() << std::endl;
}

}  // namespace


int main() {
	sRunTests();

	const std::vector<std::string> tokens = {"a", "+", "c", "*", "(", "b", "-", "c", ")", "/", "1"};
	auto&& [f, r] = parse(tokens);
	std::cout << std::endl << "Input: ";
	for (auto& token : tokens) {
		std::cout << token << " ";
	}
	std::cout << "\nResult: " << std::boolalpha << f << " :: " << r << std::endl;
}
