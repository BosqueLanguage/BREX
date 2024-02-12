#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"

std::optional<std::u8string> tryParseForTest_Unicode(const std::u8string& str) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str);
    if(pr.first.has_value() && pr.second.empty()) {
        return std::make_optional(pr.first.value()->toBSQONFormat());
    }
    else {
        return std::nullopt;
    }
}

void checkAndReportUnicodeResult(const std::u8string& str, const std::u8string& expected) {
    if(str != expected) {
        std::cout << "Expected: " << std::string(expected.cbegin(), expected.cend()) << std::endl;
        std::cout << "Got: " << std::string(str.cbegin(), str.cend()) << std::endl;
    }

    BOOST_ASSERT(str == expected);
}

#define PARSE_TEST_UNICODE(RE, EX) { auto res = tryParseForTest_Unicode(RE); BOOST_CHECK(res.has_value()); checkAndReportUnicodeResult(res.value(), EX); }

std::optional<std::u8string> tryParseForTest_ASCII(const std::string& str) {
    auto pr = brex::RegexParser::parseASCIIRegex(str);
    if(pr.first.has_value() && pr.second.empty()) {
        return std::make_optional(pr.first.value()->toBSQONFormat());
    }
    else {
        return std::nullopt;
    }
}

void checkAndReportASCIIResult(const std::u8string& str, const std::u8string& expected) {
    if(str != expected) {
        std::cout << "Expected: " << std::string(expected.cbegin(), expected.cend()) << std::endl;
        std::cout << "Got: " << std::string(str.cbegin(), str.cend()) << std::endl;
    }

    BOOST_ASSERT(str == expected);
}

#define PARSE_TEST_ASCII(RE, EX) { auto res = tryParseForTest_ASCII(RE); BOOST_CHECK(res.has_value()); checkAndReportASCIIResult(res.value(), EX); }

BOOST_AUTO_TEST_SUITE(ParsingOk)

////
//Literal
BOOST_AUTO_TEST_SUITE(Literal)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(abc) {
    PARSE_TEST_UNICODE(u8"/\"abc\"/", u8"/\"abc\"/");
}

BOOST_AUTO_TEST_CASE(eps) {
    PARSE_TEST_UNICODE(u8"/\"\"/", u8"/\"\"/");
}

BOOST_AUTO_TEST_CASE(literal) {
    PARSE_TEST_UNICODE(u8"/\"a🌵c\"/", u8"/\"a🌵c\"/");
}

BOOST_AUTO_TEST_CASE(escape) {
    PARSE_TEST_UNICODE(u8"/\"%x0;\"/", u8"/\"%NUL;\"/");
    PARSE_TEST_UNICODE(u8"/\"%x59;\"/", u8"/\"Y\"/");
    PARSE_TEST_UNICODE(u8"/\"%x1f335;\"/", u8"/\"🌵\"/");

    PARSE_TEST_UNICODE(u8"/\"%;\"/", u8"/\"%;\"/");
    PARSE_TEST_UNICODE(u8"/\"%%;\"/", u8"/\"%%;\"/");
    PARSE_TEST_UNICODE(u8"/\"%n;\"/", u8"/\"%n;\"/");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ASCII)
BOOST_AUTO_TEST_CASE(abc) {
    PARSE_TEST_ASCII("/'abc'/", u8"/'abc'/a");
}

BOOST_AUTO_TEST_CASE(eps) {
    PARSE_TEST_ASCII("/''/", u8"/''/a");
}

BOOST_AUTO_TEST_CASE(asciiescape) {
    PARSE_TEST_ASCII("/'%x59;'/", u8"/'Y'/a");

    PARSE_TEST_ASCII("/'%;'/", u8"/'%;'/a");
    PARSE_TEST_ASCII("/'%%;'/", u8"/'%%;'/a");
    PARSE_TEST_ASCII("/'%n;'/", u8"/'%n;'/a");
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()


////
//Range
BOOST_AUTO_TEST_SUITE(Range)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/[06a]/", u8"/[06a]/");
    PARSE_TEST_UNICODE(u8"/[0-9]/", u8"/[0-9]/");
    PARSE_TEST_UNICODE(u8"/[0^]/", u8"/[0^]/");
}
BOOST_AUTO_TEST_CASE(combos) {
    PARSE_TEST_UNICODE(u8"/[0-9 +]/", u8"/[0-9 +]/");
}
BOOST_AUTO_TEST_CASE(compliment) {
    PARSE_TEST_UNICODE(u8"/[^A-Z]/", u8"/[^A-Z]/");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ASCII)
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
