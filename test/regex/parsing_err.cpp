#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"

std::optional<brex::RegexParserError> tryParseForTest_Fail(const std::u8string& str) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str);
    if(pr.second.empty()) {
        return std::nullopt;
    }
    else {
        return std::make_optional(pr.second.front());
    }
}

#define PARSE_TEST(RE, EX) { auto res = tryParseForTest_Fail(RE); BOOST_CHECK(res.has_value()); BOOST_CHECK(res.value().msg.starts_with(EX)); }

BOOST_AUTO_TEST_SUITE(ParsingErr)

////
//Literal
BOOST_AUTO_TEST_SUITE(Literal)
BOOST_AUTO_TEST_CASE(abc) {
    PARSE_TEST(u8"/\"abc/", u8"Unterminated regex literal");
    PARSE_TEST(u8"/abc\"/", u8"Invalid regex component -- expected (, [, ', \", {, or . but found \"a\"");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
