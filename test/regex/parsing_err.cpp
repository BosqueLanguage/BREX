#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"

std::optional<brex::RegexParserError> tryParseForTestError_Unicode(const std::u8string& str) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str, false);
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

std::optional<brex::RegexParserError> tryParseForTestError_C(const std::string& str) {
    auto pr = brex::RegexParser::parseCRegex(str, false);
    if(pr.second.empty()) {
        return std::nullopt;
    }
    else {
        return std::make_optional(pr.second.front());
    }
}

void checkAndReportErrCResult(const std::u8string& str, const std::u8string& expected) {
    if(!str.starts_with(expected)) {
        std::cout << "Expected: " << std::string(expected.cbegin(), expected.cend()) << std::endl;
        std::cout << "Got: " << std::string(str.cbegin(), str.cend()) << std::endl;
    }

    BOOST_ASSERT(str.starts_with(expected));
}

#define PARSE_TEST_UNICODE(RE, EX) { auto res = tryParseForTestError_Unicode(RE); BOOST_CHECK(res.has_value()); checkAndReportErrUnicodeResult(res.value().msg, EX); }
#define PARSE_TEST_C(RE, EX) { auto res = tryParseForTestError_C(RE); BOOST_CHECK(res.has_value()); checkAndReportErrCResult(res.value().msg, EX); }

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

BOOST_AUTO_TEST_SUITE(C)
BOOST_AUTO_TEST_CASE(abc) {
    PARSE_TEST_C("/'abc/c", u8"Unterminated regex literal");
    PARSE_TEST_C("/abc'/c", u8"Invalid regex component -- expected (, [, ', \", {, or . but found \"a\"");
}

BOOST_AUTO_TEST_CASE(escape) {
    PARSE_TEST_C("/'%'/c", u8"Escape sequence is missing terminal ';'");
    PARSE_TEST_C("/'%x0'/c", u8"Escape sequence is missing terminal ';'");
    PARSE_TEST_C("/'%59;'/c", u8"Invalid escape sequence -- unknown escape name '59'");
    PARSE_TEST_C("/'%x8f3G;'/c", u8"Hex escape sequence contains non-hex characters");
    PARSE_TEST_C("/'%x100;'/c", u8"Invalid hex escape sequence");
    PARSE_TEST_C("/'%x7F;'/c", u8"Invalid hex escape sequence");
    PARSE_TEST_C("/'%x;'/c", u8"Invalid escape sequence -- unknown escape name 'x'");
    PARSE_TEST_C("/'%bob;'/c", u8"Invalid escape sequence -- unknown escape name 'bob'");

    PARSE_TEST_C("/'a🌵c'/c", u8"Invalid Char encoding -- string contains a non-char character");
    PARSE_TEST_C("/'%a;'/c", u8"Invalid escape sequence -- unknown escape name 'a'");
    PARSE_TEST_C("/'%x7;'/c", u8"Invalid hex escape sequence");
    PARSE_TEST_C("/'%x127;'/c", u8"Invalid hex escape sequence");
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

BOOST_AUTO_TEST_SUITE(C)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_C("/[06a/c", u8"Missing ] in char range regex");
    PARSE_TEST_C("/[0-9/c", u8"Missing ] in char range regex");
    PARSE_TEST_C("/0]/c", u8"Invalid regex component");
}
BOOST_AUTO_TEST_CASE(escape) {
    PARSE_TEST_C("/[%x32;-%tick]/c", u8"Escape sequence is missing terminal ';'");
    PARSE_TEST_C("/[^%x32; %undescore;]/c", u8"Invalid escape sequence -- unknown escape name 'undescore'");
    PARSE_TEST_C("/[^%x32-%undescore;]/c", u8"Hex escape sequence contains non-hex characters -- x32-%undescore");
    PARSE_TEST_C("/[^%32-%undescore;]/c", u8"Invalid escape sequence -- unknown escape name '32-%undescore'");

    PARSE_TEST_C("/[%a;]/c", u8"Invalid escape sequence -- unknown escape name 'a'");
    PARSE_TEST_C("/[^%x127;]/c", u8"Hex escape sequence is not a valid char character -- x127");
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Parens
BOOST_AUTO_TEST_SUITE(Parens)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/(\"a\"/", u8"Missing ) in regex");
    PARSE_TEST_UNICODE(u8"/\"a\")/", u8"Empty regex sequence");
    PARSE_TEST_UNICODE(u8"/((\"a\")/", u8"Missing ) in regex");

    PARSE_TEST_UNICODE(u8"/(\"a\")\"b\")*/", u8"Empty regex sequence");
    PARSE_TEST_UNICODE(u8"/(\"a\")+)?/", u8"Empty regex sequence");
}
BOOST_AUTO_TEST_SUITE_END()

////
//Range
BOOST_AUTO_TEST_SUITE(Range)
BOOST_AUTO_TEST_CASE(single) {
    PARSE_TEST_UNICODE(u8"/\"a\"5}/", u8"Invalid regex component -- expected");
    PARSE_TEST_UNICODE(u8"/\"a\"{5/", u8"Missing comma (possibly) in range repeat");
    PARSE_TEST_UNICODE(u8"/\"a\"{0}/", u8"Invalid range repeat bounds -- both min and max are 0 so the repeat is empty");
    PARSE_TEST_UNICODE(u8"/\"a\"{1}/", u8"Invalid range repeat bounds -- min and max are both 1 so the repeat is redundant");

    PARSE_TEST_UNICODE(u8"/\"a\"{-1}/", u8"Invalid range repeat bound -- cannot have negative bound");
    PARSE_TEST_UNICODE(u8"/\"a\"{x}/", u8"Missing } in range repeat");
}
BOOST_AUTO_TEST_CASE(lower) {
    PARSE_TEST_UNICODE(u8"/\"a\"{5,a/", u8"Missing } in range repeat");
}
BOOST_AUTO_TEST_CASE(upper) {
    PARSE_TEST_UNICODE(u8"/\"a\"{,5b/", u8"Missing } in range repeat");
    PARSE_TEST_UNICODE(u8"/\"a\"{,5b}/", u8"Missing } in range repeat");
}
BOOST_AUTO_TEST_CASE(both) {
    PARSE_TEST_UNICODE(u8"/\"a\"{3 4}/", u8"Missing comma (possibly) in range repeat");
    PARSE_TEST_UNICODE(u8"/\"a\"{4,0}/", u8"Invalid range repeat bounds -- max is less than min");

    PARSE_TEST_UNICODE(u8"/\"a\"{0,0}/", u8"Invalid range repeat bounds -- both min and max are 0 so the repeat is empty");
    PARSE_TEST_UNICODE(u8"/\"a\"{1,1}/", u8"Invalid range repeat bounds -- min and max are both 1 so the repeat is redundant");
}
BOOST_AUTO_TEST_SUITE_END()

////
//Opt
BOOST_AUTO_TEST_SUITE(Opt)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/\"a\"|/", u8"Empty regex sequence");
    PARSE_TEST_UNICODE(u8"/|\"b\"/", u8"Empty regex sequence");
}
BOOST_AUTO_TEST_SUITE_END()

////
//All
BOOST_AUTO_TEST_SUITE(All)
BOOST_AUTO_TEST_CASE(ii) {
    PARSE_TEST_UNICODE(u8"/[0-9] && (\"5\"|\"6\")/", u8"Invalid regex -- && is not a valid regex operator");
    PARSE_TEST_UNICODE(u8"/[0-9] &/", u8"Empty regex sequence");
    PARSE_TEST_UNICODE(u8"/& [0-9]/", u8"Empty regex sequence");
}
BOOST_AUTO_TEST_SUITE_END()

////
//Negative
BOOST_AUTO_TEST_SUITE(Negative)
BOOST_AUTO_TEST_CASE(notbob) {
    PARSE_TEST_UNICODE(u8"/^!\"bob\"/", u8"Invalid regex -- negation is not allowed inside anchor");
}
BOOST_AUTO_TEST_SUITE_END()


////
//StartsAnchor
BOOST_AUTO_TEST_SUITE(StartsAnchor)
BOOST_AUTO_TEST_CASE(notbob) {
    PARSE_TEST_UNICODE(u8"/.+ & ^^(\"bob\"|\"sally\")/", u8"Empty regex sequence");
    PARSE_TEST_UNICODE(u8"/^\"bob\"|\"sally\"/", u8"Invalid regex -- all top-level components are front or back checks");
    PARSE_TEST_UNICODE(u8"/.+ & !^(\"bob\"|\"sally\")$/", u8"Invalid regex -- front and back checks cannot be used together");
}
BOOST_AUTO_TEST_SUITE_END()

////
//EndsAnchor
BOOST_AUTO_TEST_SUITE(EndsAnchor)
BOOST_AUTO_TEST_CASE(notbob) {
    PARSE_TEST_UNICODE(u8"/.+ & (\"bob\"|\"sally\")$$/", u8"Empty regex sequence");
    PARSE_TEST_UNICODE(u8"/\"bob\"|\"sally\" $/", u8"Invalid regex -- all top-level components are front or back checks");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
