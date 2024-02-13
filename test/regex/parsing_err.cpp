#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"

std::optional<brex::RegexParserError> tryParseForTestError_Unicode(const std::u8string& str) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str);
    if(pr.second.empty()) {
        return std::nullopt;
    }
    else {
        return std::make_optional(pr.second.front());
    }
}

std::optional<brex::RegexParserError> tryParseForTestError_ASCII(const std::string& str) {
    auto pr = brex::RegexParser::parseASCIIRegex(str);
    if(pr.second.empty()) {
        return std::nullopt;
    }
    else {
        return std::make_optional(pr.second.front());
    }
}

#define PARSE_TEST_UNICODE(RE, EX) { auto res = tryParseForTestError_Unicode(RE); BOOST_CHECK(res.has_value()); BOOST_CHECK(res.value().msg.starts_with(EX)); }
#define PARSE_TEST_ASCII(RE, EX) { auto res = tryParseForTestError_ASCII(RE); BOOST_CHECK(res.has_value()); BOOST_CHECK(res.value().msg.starts_with(EX)); }

BOOST_AUTO_TEST_SUITE(ParsingErr)

////
//Literal
BOOST_AUTO_TEST_SUITE(Literal)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(abc) {
    PARSE_TEST_UNICODE(u8"/\"abc/", u8"Unterminated regex literal");
    PARSE_TEST_UNICODE(u8"/abc\"/", u8"Invalid regex component -- expected (, [, ', \", {, or . but found \"a\"");
}

BOOST_AUTO_TEST_CASE(escape) {
    PARSE_TEST_UNICODE(u8"/\"%\"/", u8"Escape sequence is missing terminal ';'");
    PARSE_TEST_UNICODE(u8"/\"%x0\"/", u8"Escape sequence is missing terminal ';'");
    PARSE_TEST_UNICODE(u8"/\"%59;\"/", u8"Invalid escape sequence -- unknown escape name '59'");
    PARSE_TEST_UNICODE(u8"/\"%x8f3G;\"/", u8"Hex escape sequence contains non-hex characters");
    PARSE_TEST_UNICODE(u8"/\"%x100000000000;\"/", u8"Invalid hex escape sequence");
    PARSE_TEST_UNICODE(u8"/\"%x1f3335;\"/", u8"Invalid hex escape sequence");
    PARSE_TEST_UNICODE(u8"/\"%x;\"/", u8"Invalid escape sequence -- unknown escape name 'x'");
    PARSE_TEST_UNICODE(u8"/\"%bob;\"/", u8"Invalid escape sequence -- unknown escape name 'bob'");
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
