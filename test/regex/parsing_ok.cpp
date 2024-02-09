#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"

std::optional<std::u8string> tryParseForTest_Ok(const std::u8string& str) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str);
    if(pr.first.has_value() && pr.second.empty()) {
        return std::make_optional(pr.first.value()->toBSQONFormat());
    }
    else {
        return std::nullopt;
    }
}

#define PARSE_TEST(RE, EX) { auto res = tryParseForTest_Ok(RE); BOOST_CHECK(res.has_value()); BOOST_CHECK(res.value() == EX); }

BOOST_AUTO_TEST_SUITE(ParsingOk)

BOOST_AUTO_TEST_CASE(literalXokabc) {
    PARSE_TEST(u8"/\"abc\"/", u8"/\"abc\"/");
}

BOOST_AUTO_TEST_CASE(literalXokeps) {
    PARSE_TEST(u8"/\"\"/", u8"/\"\"/");
}

BOOST_AUTO_TEST_SUITE_END()

