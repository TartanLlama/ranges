#include <tl/getlines.hpp>
#include <catch2/catch.hpp>
#include <tl/to.hpp>
#include <vector>
#include <sstream>

TEST_CASE("getlines") {
	std::stringstream ss;
	ss << R"(hello
there
I
am
a
big
fan
of
cake
personally)";

	std::vector<std::string_view> result = {
		"hello",
	"there",
	"I",
	"am",
	"a",
	"big",
	"fan",
	"of",
	"cake",
	"personally"
	};

	auto lines = tl::views::getlines(ss) | tl::to<std::vector>();
	auto equal = std::ranges::equal(lines, result);
	REQUIRE(equal);
}

TEST_CASE("comma") {
	std::stringstream ss;
	ss << "hello,there,I,am,a,big,fan,of,cake,personally";

	std::vector<std::string_view> result = {
		"hello",
	"there",
	"I",
	"am",
	"a",
	"big",
	"fan",
	"of",
	"cake",
	"personally"
	};

	auto lines = tl::views::getlines(ss,',') | tl::to<std::vector>();
	auto equal = std::ranges::equal(lines, result);
	REQUIRE(equal);
}