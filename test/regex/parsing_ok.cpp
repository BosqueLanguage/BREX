#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"

std::optional<std::u8string> tryParseForTestOk_Unicode(const std::u8string& str) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str);
    if(pr.first.has_value() && pr.second.empty()) {
        return std::make_optional(pr.first.value()->toBSQONFormat());
    }
    else {
        return std::nullopt;
    }
}

void checkAndReportOkUnicodeResult(const std::u8string& str, const std::u8string& expected) {
    if(str != expected) {
        std::cout << "Expected: " << std::string(expected.cbegin(), expected.cend()) << std::endl;
        std::cout << "Got: " << std::string(str.cbegin(), str.cend()) << std::endl;
    }

    BOOST_ASSERT(str == expected);
}

#define PARSE_TEST_UNICODE(RE, EX) { auto res = tryParseForTestOk_Unicode(RE); BOOST_CHECK(res.has_value()); checkAndReportOkUnicodeResult(res.value(), EX); }

std::optional<std::u8string> tryParseForTestOk_ASCII(const std::string& str) {
    auto pr = brex::RegexParser::parseASCIIRegex(str);
    if(pr.first.has_value() && pr.second.empty()) {
        return std::make_optional(pr.first.value()->toBSQONFormat());
    }
    else {
        return std::nullopt;
    }
}

void checkAndReportOkASCIIResult(const std::u8string& str, const std::u8string& expected) {
    if(str != expected) {
        std::cout << "Expected: " << std::string(expected.cbegin(), expected.cend()) << std::endl;
        std::cout << "Got: " << std::string(str.cbegin(), str.cend()) << std::endl;
    }

    BOOST_ASSERT(str == expected);
}

#define PARSE_TEST_ASCII(RE, EX) { auto res = tryParseForTestOk_ASCII(RE); BOOST_CHECK(res.has_value()); checkAndReportOkASCIIResult(res.value(), EX); }

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

BOOST_AUTO_TEST_CASE(escape) {
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
BOOST_AUTO_TEST_CASE(emoji) {
    PARSE_TEST_UNICODE(u8"/[🌵-%x1f336;]/", u8"/[🌵-🌶]/");
}
BOOST_AUTO_TEST_CASE(escape) {
    PARSE_TEST_UNICODE(u8"/[%x32;-%tick;]/", u8"/['-2]/");
    PARSE_TEST_UNICODE(u8"/[^%x32; %underscore;]/", u8"/[^2 _]/");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ASCII)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_ASCII("/[06a]/", u8"/[06a]/a");
    PARSE_TEST_ASCII("/[0-9]/", u8"/[0-9]/a");
    PARSE_TEST_ASCII("/[0^]/", u8"/[0^]/a");
}
BOOST_AUTO_TEST_CASE(combos) {
    PARSE_TEST_ASCII("/[0-9 +]/", u8"/[0-9 +]/a");
}
BOOST_AUTO_TEST_CASE(compliment) {
    PARSE_TEST_ASCII("/[^A-Z]/", u8"/[^A-Z]/a");
}
BOOST_AUTO_TEST_CASE(escape) {
    PARSE_TEST_ASCII("/[%x32;-%tick;]/", u8"/[%;-2]/a");
    PARSE_TEST_ASCII("/[^%x32; %underscore;]/", u8"/[^2 _]/a");
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Dot
BOOST_AUTO_TEST_SUITE(Dot)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/./", u8"/./");
    PARSE_TEST_UNICODE(u8"/[.]/", u8"/[.]/");
}
BOOST_AUTO_TEST_CASE(combos) {
    PARSE_TEST_UNICODE(u8"/.\"a\"./", u8"/.\"a\"./");
    PARSE_TEST_UNICODE(u8"/[0-9]./", u8"/[0-9]./");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ASCII)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_ASCII("/./", u8"/./a");
    PARSE_TEST_ASCII("/[.]/", u8"/[.]/a");
}
BOOST_AUTO_TEST_CASE(combos) {
    PARSE_TEST_ASCII("/.'a'./", u8"/.'a'./a");
    PARSE_TEST_ASCII("/[0-9]./", u8"/[0-9]./a");
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Parens
BOOST_AUTO_TEST_SUITE(Parens)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/(\"a\")/", u8"/\"a\"/");
    PARSE_TEST_UNICODE(u8"/((\"a\"))/", u8"/\"a\"/");

    PARSE_TEST_UNICODE(u8"/(\"a\")(\"b\")*/", u8"/\"a\"\"b\"*/");
    PARSE_TEST_UNICODE(u8"/(\"a\"\"b\")*/", u8"/(\"a\"\"b\")*/");
    PARSE_TEST_UNICODE(u8"/(\"ab\")*/", u8"/\"ab\"*/");

    PARSE_TEST_UNICODE(u8"/((\"a\")+)?/", u8"/(\"a\"+)?/");
}
BOOST_AUTO_TEST_SUITE_END()

////
//Star
BOOST_AUTO_TEST_SUITE(Star)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/\"a\"*/", u8"/\"a\"*/");
    PARSE_TEST_UNICODE(u8"/\"ab\"*/", u8"/\"ab\"*/");
    PARSE_TEST_UNICODE(u8"/\"a\"\"b\"*/", u8"/\"a\"\"b\"*/");

    PARSE_TEST_UNICODE(u8"/\"a\"   */", u8"/\"a\"*/");
}
BOOST_AUTO_TEST_SUITE_END()

////
//Plus
BOOST_AUTO_TEST_SUITE(Plus)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/\"a\"+/", u8"/\"a\"+/");
    PARSE_TEST_UNICODE(u8"/\"ab\"+/", u8"/\"ab\"+/");
    PARSE_TEST_UNICODE(u8"/\"a\"\"b\"+/", u8"/\"a\"\"b\"+/");

    PARSE_TEST_UNICODE(u8"/\"a\" +/", u8"/\"a\"+/");
}
BOOST_AUTO_TEST_SUITE_END()

////
//Question
BOOST_AUTO_TEST_SUITE(Question)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/\"a\"?/", u8"/\"a\"?/");
    PARSE_TEST_UNICODE(u8"/\"ab\"?/", u8"/\"ab\"?/");
    PARSE_TEST_UNICODE(u8"/\"a\"\"b\"?/", u8"/\"a\"\"b\"?/");

    PARSE_TEST_UNICODE(u8"/\"a\"    ?/", u8"/\"a\"?/");
}
BOOST_AUTO_TEST_SUITE_END()

////
//Range
BOOST_AUTO_TEST_SUITE(Range)
BOOST_AUTO_TEST_CASE(single) {
    PARSE_TEST_UNICODE(u8"/\"a\"{5}/", u8"/\"a\"{5}/");

    PARSE_TEST_UNICODE(u8"/\"a\"{5 }/", u8"/\"a\"{5}/");
    PARSE_TEST_UNICODE(u8"/\"a\"{ 5}/", u8"/\"a\"{5}/");
    PARSE_TEST_UNICODE(u8"/\"a\"    { 5 }/", u8"/\"a\"{5}/");
}
BOOST_AUTO_TEST_CASE(lower) {
    PARSE_TEST_UNICODE(u8"/\"a\"{5,}/", u8"/\"a\"{5,}/");
    PARSE_TEST_UNICODE(u8"/\"a\"{0,}/", u8"/\"a\"*/");
    PARSE_TEST_UNICODE(u8"/\"a\"{1,}/", u8"/\"a\"+/");

    PARSE_TEST_UNICODE(u8"/\"a\"{5 ,}/", u8"/\"a\"{5,}/");
    PARSE_TEST_UNICODE(u8"/\"a\"{5, }/", u8"/\"a\"{5,}/");
}
BOOST_AUTO_TEST_CASE(upper) {
    PARSE_TEST_UNICODE(u8"/\"a\"{,5}/", u8"/\"a\"{,5}/");
    PARSE_TEST_UNICODE(u8"/\"a\"{,1}/", u8"/\"a\"?/");

    PARSE_TEST_UNICODE(u8"/\"a\"{ ,5}/", u8"/\"a\"{,5}/");
    PARSE_TEST_UNICODE(u8"/\"a\"{, 5}/", u8"/\"a\"{,5}/");
    PARSE_TEST_UNICODE(u8"/\"a\"{  , 5}/", u8"/\"a\"{,5}/");
}
BOOST_AUTO_TEST_CASE(both) {
    PARSE_TEST_UNICODE(u8"/\"a\"{3,4}/", u8"/\"a\"{3,4}/");
    PARSE_TEST_UNICODE(u8"/\"a\"{0,4}/", u8"/\"a\"{,4}/");
    PARSE_TEST_UNICODE(u8"/\"a\"{4,4}/", u8"/\"a\"{4}/");

    PARSE_TEST_UNICODE(u8"/\"a\"{3 , 4}/", u8"/\"a\"{3,4}/");
    PARSE_TEST_UNICODE(u8"/\"a\"{3, 4}/", u8"/\"a\"{3,4}/");
    PARSE_TEST_UNICODE(u8"/\"a\"{ 3,4 }/", u8"/\"a\"{3,4}/");
}
BOOST_AUTO_TEST_SUITE_END()

////
//Opt
BOOST_AUTO_TEST_SUITE(Opt)
BOOST_AUTO_TEST_CASE(simple) {
    PARSE_TEST_UNICODE(u8"/\"a\"|\"b\"/", u8"/\"a\"|\"b\"/");
    PARSE_TEST_UNICODE(u8"/\"a\" | \"b\"/", u8"/\"a\"|\"b\"/");
    PARSE_TEST_UNICODE(u8"/[0-9]|\"b\"/", u8"/[0-9]|\"b\"/");
}
BOOST_AUTO_TEST_CASE(multiple) {
    PARSE_TEST_UNICODE(u8"/\"a\"|\"b\"|\"c\"/", u8"/\"a\"|\"b\"|\"c\"/");
}
BOOST_AUTO_TEST_CASE(mixeddown) {
    PARSE_TEST_UNICODE(u8"/\"a\"*|\"b\"?|\"c\"+/", u8"/(\"a\"*)|(\"b\"?)|(\"c\"+)/");
    PARSE_TEST_UNICODE(u8"/\"a\"*|(\"b\"?|\"c\"+)/", u8"/(\"a\"*)|((\"b\"?)|(\"c\"+))/");
}
BOOST_AUTO_TEST_CASE(mixedup) {
    PARSE_TEST_UNICODE(u8"/(\"a\"|\"b\")+|\"c\"/", u8"/((\"a\"|\"b\")+)|\"c\"/");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
