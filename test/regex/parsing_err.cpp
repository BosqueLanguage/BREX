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

void checkAndReportErrUnicodeResult(const std::u8string& str, const std::u8string& expected) {
    if(!str.starts_with(expected)) {
        std::cout << "Expected: " << std::string(expected.cbegin(), expected.cend()) << std::endl;
        std::cout << "Got: " << std::string(str.cbegin(), str.cend()) << std::endl;
    }

    BOOST_ASSERT(str.starts_with(expected));
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

void checkAndReportErrASCIIResult(const std::u8string& str, const std::u8string& expected) {
    if(!str.starts_with(expected)) {
        std::cout << "Expected: " << std::string(expected.cbegin(), expected.cend()) << std::endl;
        std::cout << "Got: " << std::string(str.cbegin(), str.cend()) << std::endl;
    }

    BOOST_ASSERT(str.starts_with(expected));
}

#define PARSE_TEST_UNICODE(RE, EX) { auto res = tryParseForTestError_Unicode(RE); BOOST_CHECK(res.has_value()); checkAndReportErrUnicodeResult(res.value().msg, EX); }
#define PARSE_TEST_ASCII(RE, EX) { auto res = tryParseForTestError_ASCII(RE); BOOST_CHECK(res.has_value()); checkAndReportErrASCIIResult(res.value().msg, EX); }

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

BOOST_AUTO_TEST_SUITE(ASCII)
BOOST_AUTO_TEST_CASE(abc) {
    PARSE_TEST_ASCII("/'abc/", u8"Unterminated regex literal");
    PARSE_TEST_ASCII("/abc'/", u8"Invalid regex component -- expected (, [, ', \", {, or . but found \"a\"");
}

BOOST_AUTO_TEST_CASE(escape) {
    PARSE_TEST_ASCII("/'%'/", u8"Escape sequence is missing terminal ';'");
    PARSE_TEST_ASCII("/'%x0'/", u8"Escape sequence is missing terminal ';'");
    PARSE_TEST_ASCII("/'%59;'/", u8"Invalid escape sequence -- unknown escape name '59'");
    PARSE_TEST_ASCII("/'%x8f3G;'/", u8"Hex escape sequence contains non-hex characters");
    PARSE_TEST_ASCII("/'%x100;'/", u8"Invalid hex escape sequence");
    PARSE_TEST_ASCII("/'%x7F;'/", u8"Invalid hex escape sequence");
    PARSE_TEST_ASCII("/'%x;'/", u8"Invalid escape sequence -- unknown escape name 'x'");
    PARSE_TEST_ASCII("/'%bob;'/", u8"Invalid escape sequence -- unknown escape name 'bob'");

    PARSE_TEST_ASCII("/'a🌵c'/", u8"Invalid ASCII encoding -- string contains a non-ASCII character");
    PARSE_TEST_ASCII("/'%a;'/", u8"Invalid escape sequence -- unknown escape name 'a'");
    PARSE_TEST_ASCII("/'%x7;'/", u8"Invalid hex escape sequence");
    PARSE_TEST_ASCII("/'%x127;'/", u8"Invalid hex escape sequence");
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Range
BOOST_AUTO_TEST_SUITE(Range)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/[06a/", u8"Missing ] in char range regex");
    PARSE_TEST_UNICODE(u8"/[0-9/", u8"Missing ] in char range regex");
    PARSE_TEST_UNICODE(u8"/0]/", u8"Invalid regex component");
}
BOOST_AUTO_TEST_CASE(escape) {
    PARSE_TEST_UNICODE(u8"/[%x32;-%tick]/", u8"Escape sequence is missing terminal ';'");
    PARSE_TEST_UNICODE(u8"/[^%x32; %undescore;]/", u8"Invalid escape sequence -- unknown escape name 'undescore'");
    PARSE_TEST_UNICODE(u8"/[^%x32-%undescore;]/", u8"Hex escape sequence contains non-hex characters -- x32-%undescore");
    PARSE_TEST_UNICODE(u8"/[^%32-%undescore;]/", u8"Invalid escape sequence -- unknown escape name '32-%undescore'");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ASCII)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_ASCII("/[06a/", u8"Missing ] in char range regex");
    PARSE_TEST_ASCII("/[0-9/", u8"Missing ] in char range regex");
    PARSE_TEST_ASCII("/0]/", u8"Invalid regex component");
}
BOOST_AUTO_TEST_CASE(escape) {
    PARSE_TEST_ASCII("/[%x32;-%tick]/", u8"Escape sequence is missing terminal ';'");
    PARSE_TEST_ASCII("/[^%x32; %undescore;]/", u8"Invalid escape sequence -- unknown escape name 'undescore'");
    PARSE_TEST_ASCII("/[^%x32-%undescore;]/", u8"Hex escape sequence contains non-hex characters -- x32-%undescore");
    PARSE_TEST_ASCII("/[^%32-%undescore;]/", u8"Invalid escape sequence -- unknown escape name '32-%undescore'");

    PARSE_TEST_ASCII("/[%a;]/", u8"Invalid escape sequence -- unknown escape name 'a'");
    PARSE_TEST_ASCII("/[^%x127;]/", u8"Hex escape sequence is not a valid ASCII character -- x127");
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()


////
//Parens
BOOST_AUTO_TEST_SUITE(Parens)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/(\"a\"/", u8"Missing ) in regex");
    PARSE_TEST_UNICODE(u8"/\"a\")/", u8"Invalid regex -- trailing characters after end of regex");
    PARSE_TEST_UNICODE(u8"/((\"a\")/", u8"Missing ) in regex");

    PARSE_TEST_UNICODE(u8"/(\"a\")\"b\")*/", u8"Invalid regex -- trailing characters after end of regex");
    PARSE_TEST_UNICODE(u8"/(\"a\")+)?/", u8"Invalid regex -- trailing characters after end of regex");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
