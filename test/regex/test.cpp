#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"
#include "../../src/regex/brex_compiler.h"

std::optional<brex::UnicodeRegexExecutor*> tryParseForTest(const std::u8string& str) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return std::nullopt;
    }

    std::map<std::string, const brex::RegexOpt*> emptymap;
    std::vector<brex::RegexCompileError> compileerror;
    auto executor = brex::RegexCompiler::compileUnicodeRegexToExecutor(pr.first.value(), emptymap, nullptr, nullptr, compileerror);
    if(!compileerror.empty()) {
        return std::nullopt;
    }

    return std::make_optional(executor);
}

#define ACCEPTS_TEST(RE, STR, ACCEPT) {auto uustr = brex::UnicodeString(STR); auto accepts = executor->test(&uustr); BOOST_CHECK(accepts == ACCEPT); }

BOOST_AUTO_TEST_SUITE(Test)

BOOST_AUTO_TEST_SUITE(Literal)
BOOST_AUTO_TEST_CASE(abc) {
    auto texecutor = tryParseForTest(u8"/\"abc\"/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST(executor, u8"abc", true);
    ACCEPTS_TEST(executor, u8"ab", false);
    ACCEPTS_TEST(executor, u8"", false);
}

BOOST_AUTO_TEST_CASE(eps) {
    auto texecutor = tryParseForTest(u8"/\"\"/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST(executor, u8"abc", false);
    ACCEPTS_TEST(executor, u8"", true);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
