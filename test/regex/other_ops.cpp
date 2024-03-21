#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"
#include "../../src/regex/brex_compiler.h"

std::optional<brex::UnicodeRegexExecutor*> tryParseForUnicodeOtherOp(const std::u8string& str) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str, false);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return std::nullopt;
    }

    std::map<std::string, const brex::RegexOpt*> namemap;
    std::map<std::string, const brex::LiteralOpt*> envmap;
    std::vector<brex::RegexCompileError> compileerror;
    auto executor = brex::RegexCompiler::compileUnicodeRegexToExecutor(pr.first.value(), namemap, envmap, false, nullptr, nullptr, compileerror);
    if(!compileerror.empty()) {
        return std::nullopt;
    }

    return std::make_optional(executor);
}

BOOST_AUTO_TEST_SUITE(OtherOps)

BOOST_AUTO_TEST_SUITE(StartsWith)
BOOST_AUTO_TEST_CASE(abc) {
    brex::ExecutorError err;
    auto texecutor = tryParseForUnicodeOtherOp(u8"/\"abc\"/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    auto ustr = brex::UnicodeString(u8"abcdef");
    BOOST_CHECK(executor->testFront(&ustr, err));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(EndsWith)
BOOST_AUTO_TEST_CASE(abc) {
    brex::ExecutorError err;
    auto texecutor = tryParseForUnicodeOtherOp(u8"/\"def\"/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    auto ustr = brex::UnicodeString(u8"abcdef");
    BOOST_CHECK(executor->testBack(&ustr, err));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Contains)
BOOST_AUTO_TEST_CASE(abc) {
    brex::ExecutorError err;
    auto texecutor = tryParseForUnicodeOtherOp(u8"/[cd]/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    auto ustr = brex::UnicodeString(u8"abcdef");
    BOOST_CHECK(executor->testContains(&ustr, err));
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(StartsMatch)
BOOST_AUTO_TEST_CASE(repeat09) {
    brex::ExecutorError err;
    auto texecutor = tryParseForUnicodeOtherOp(u8"/[0-9]+/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    auto ustr = brex::UnicodeString(u8"123a456");
    auto rr = executor->matchFront(&ustr, err);

    BOOST_CHECK(rr.has_value() && rr.value() == 2);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(EndsMatch)
BOOST_AUTO_TEST_CASE(repeat09) {
    brex::ExecutorError err;
    auto texecutor = tryParseForUnicodeOtherOp(u8"/[0-9]+/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    auto ustr = brex::UnicodeString(u8"123a456");
    auto rr = executor->matchBack(&ustr, err);

    BOOST_CHECK(rr.has_value() && rr.value() == 4);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ContainsMatch)
BOOST_AUTO_TEST_CASE(repeata09) {
    brex::ExecutorError err;
    auto texecutor = tryParseForUnicodeOtherOp(u8"/\"a\"[0-9]/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    auto ustr = brex::UnicodeString(u8"123a456");
    auto rr = executor->matchContainsFirst(&ustr, err);

    BOOST_CHECK(rr.has_value() && rr.value().first == 3 && rr.value().second == 4);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()