#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"
#include "../../src/regex/brex_compiler.h"

std::optional<brex::UnicodeRegexExecutor*> tryParseForUnicodeTest(const std::u8string& str) {
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

std::optional<brex::CRegexExecutor*> tryParseForCTest(const std::string& str) {
    auto pr = brex::RegexParser::parseCRegex(str, false);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return std::nullopt;
    }

    std::map<std::string, const brex::RegexOpt*> namemap;
    std::map<std::string, const brex::LiteralOpt*> envmap;
    std::vector<brex::RegexCompileError> compileerror;
    auto executor = brex::RegexCompiler::compileCRegexToExecutor(pr.first.value(), namemap, envmap, false, nullptr, nullptr, compileerror);
    if(!compileerror.empty()) {
        return std::nullopt;
    }

    return std::make_optional(executor);
}

#define ACCEPTS_TEST_UNICODE(RE, STR, ACCEPT) {auto uustr = brex::UnicodeString(STR); brex::ExecutorError err; auto accepts = executor->test(&uustr, err); BOOST_CHECK(err == brex::ExecutorError::Ok); BOOST_CHECK(accepts == ACCEPT); }

#define ACCEPTS_TEST_C(RE, STR, ACCEPT) {auto uustr = brex::CString(STR); brex::ExecutorError err; auto accepts = executor->test(&uustr, err); BOOST_CHECK(err == brex::ExecutorError::Ok); BOOST_CHECK(accepts == ACCEPT); }

BOOST_AUTO_TEST_SUITE(Test)

////
//Range
BOOST_AUTO_TEST_SUITE(Literal)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(abc) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"abc\"/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"abc", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);

    ACCEPTS_TEST_UNICODE(executor, u8"abcd", false);
    ACCEPTS_TEST_UNICODE(executor, u8"xab", false);
}

BOOST_AUTO_TEST_CASE(eps) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"\"/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"abc", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}

BOOST_AUTO_TEST_CASE(literal) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a🌵c\"/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a🌵c", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaa", false);
}

BOOST_AUTO_TEST_CASE(escape) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"%%;%underscore;%x32;\"/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"%_2", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"%_aa", false);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(C)
BOOST_AUTO_TEST_CASE(abc) {
    auto texecutor = tryParseForCTest("/'abc'/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "abc", true);
    ACCEPTS_TEST_C(executor, "ab", false);
    ACCEPTS_TEST_C(executor, "", false);

    ACCEPTS_TEST_C(executor, "abcd", false);
    ACCEPTS_TEST_C(executor, "xab", false);
}

BOOST_AUTO_TEST_CASE(eps) {
    auto texecutor = tryParseForCTest("/''/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "abc", false);
    ACCEPTS_TEST_C(executor, "", true);
}

BOOST_AUTO_TEST_CASE(escape) {
    auto texecutor = tryParseForCTest("/'%%;%underscore;%x32;'/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "%_2", true);
    ACCEPTS_TEST_C(executor, "aaa", false);
    ACCEPTS_TEST_C(executor, "%_aa", false);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Range
BOOST_AUTO_TEST_SUITE(Range)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(opts3) {
    auto texecutor = tryParseForUnicodeTest(u8"/[06a]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"6", true);
    ACCEPTS_TEST_UNICODE(executor, u8"1", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(optsrng) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0-9]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE(executor, u8"3", true);
    ACCEPTS_TEST_UNICODE(executor, u8"9", true);
    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(optshat) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0^]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE(executor, u8"^", true);
    ACCEPTS_TEST_UNICODE(executor, u8"1", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(combos) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0-9 +]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE(executor, u8"5", true);
    ACCEPTS_TEST_UNICODE(executor, u8" ", true);
    ACCEPTS_TEST_UNICODE(executor, u8"+", true);
    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
}
BOOST_AUTO_TEST_CASE(compliment) {
    auto texecutor = tryParseForUnicodeTest(u8"/[^A-Z]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"0", true);
    ACCEPTS_TEST_UNICODE(executor, u8"A", false);
    ACCEPTS_TEST_UNICODE(executor, u8"Q", false);
}
BOOST_AUTO_TEST_CASE(complimet2) {
    auto texecutor = tryParseForUnicodeTest(u8"/[^A-Z0a-c]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"5", true);
    ACCEPTS_TEST_UNICODE(executor, u8" ", true);
    ACCEPTS_TEST_UNICODE(executor, u8"^", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌵", true);
    ACCEPTS_TEST_UNICODE(executor, u8"0", false);
    ACCEPTS_TEST_UNICODE(executor, u8"b", false);
}
BOOST_AUTO_TEST_CASE(emoji) {
    auto texecutor = tryParseForUnicodeTest(u8"/[🌵-🌶]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"🌵", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌶", true);
    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"🌽", false);
}
BOOST_AUTO_TEST_CASE(complimentemoji) {
    auto texecutor = tryParseForUnicodeTest(u8"/[^🌵-🌶]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌽", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌵", false);
    ACCEPTS_TEST_UNICODE(executor, u8"🌶", false);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(C)
BOOST_AUTO_TEST_CASE(opts3) {
    auto texecutor = tryParseForCTest("/[06a]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "0", true);
    ACCEPTS_TEST_C(executor, "a", true);
    ACCEPTS_TEST_C(executor, "6", true);
    ACCEPTS_TEST_C(executor, "1", false);
    ACCEPTS_TEST_C(executor, "", false);
}
BOOST_AUTO_TEST_CASE(optsrng) {
    auto texecutor = tryParseForCTest("/[0-9]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "0", true);
    ACCEPTS_TEST_C(executor, "3", true);
    ACCEPTS_TEST_C(executor, "9", true);
    ACCEPTS_TEST_C(executor, "a", false);
    ACCEPTS_TEST_C(executor, "", false);
}
BOOST_AUTO_TEST_CASE(optshat) {
    auto texecutor = tryParseForCTest("/[0^]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "0", true);
    ACCEPTS_TEST_C(executor, "^", true);
    ACCEPTS_TEST_C(executor, "1", false);
    ACCEPTS_TEST_C(executor, "", false);
}
BOOST_AUTO_TEST_CASE(combos) {
    auto texecutor = tryParseForCTest("/[0-9 +]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "0", true);
    ACCEPTS_TEST_C(executor, "5", true);
    ACCEPTS_TEST_C(executor, " ", true);
    ACCEPTS_TEST_C(executor, "+", true);
    ACCEPTS_TEST_C(executor, "a", false);
}
BOOST_AUTO_TEST_CASE(compliment) {
    auto texecutor = tryParseForCTest("/[^A-Z]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "0", true);
    ACCEPTS_TEST_C(executor, "A", false);
    ACCEPTS_TEST_C(executor, "Q", false);
}
BOOST_AUTO_TEST_CASE(complimet2) {
    auto texecutor = tryParseForCTest("/[^A-Z0a-c]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "5", true);
    ACCEPTS_TEST_C(executor, " ", true);
    ACCEPTS_TEST_C(executor, "^", true);
    ACCEPTS_TEST_C(executor, "0", false);
    ACCEPTS_TEST_C(executor, "b", false);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Dot
BOOST_AUTO_TEST_SUITE(Dot)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(simple) {
    auto texecutor = tryParseForUnicodeTest(u8"/./");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8".", true);
    ACCEPTS_TEST_UNICODE(executor, u8" ", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌶", true);

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(dotrng) {
    auto texecutor = tryParseForUnicodeTest(u8"/[.b]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8".", true);
    ACCEPTS_TEST_UNICODE(executor, u8"b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"🌶", false);

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(combobe) {
    auto texecutor = tryParseForUnicodeTest(u8"/.\"b\"./");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8".b.", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bbx", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ab", false);
}
BOOST_AUTO_TEST_CASE(comborng) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0-9]./");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"9b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"4🌶", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ab", false);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(C)
BOOST_AUTO_TEST_CASE(simple) {
    auto texecutor = tryParseForCTest("/./");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "a", true);
    ACCEPTS_TEST_C(executor, ".", true);
    ACCEPTS_TEST_C(executor, " ", true);

    ACCEPTS_TEST_C(executor, "", false);
}
BOOST_AUTO_TEST_CASE(dotrng) {
    auto texecutor = tryParseForCTest("/[.b]/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "a", false);
    ACCEPTS_TEST_C(executor, ".", true);
    ACCEPTS_TEST_C(executor, "b", true);

    ACCEPTS_TEST_C(executor, "", false);
}
BOOST_AUTO_TEST_CASE(combobe) {
    auto texecutor = tryParseForCTest("/.'b'./");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, ".b.", true);
    ACCEPTS_TEST_C(executor, "bbx", true);
    ACCEPTS_TEST_C(executor, "ab", false);
}
BOOST_AUTO_TEST_CASE(comborng) {
    auto texecutor = tryParseForCTest("/[0-9]./");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_C(executor, "9b", true);
    ACCEPTS_TEST_C(executor, "ab", false);
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Star
BOOST_AUTO_TEST_SUITE(Star)
BOOST_AUTO_TEST_CASE(a) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"*/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(ab) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"ab\"*/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", true);

    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(absplit) {
    auto texecutor = tryParseForUnicodeTest(u8"/(\"a\"\"b\")*/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", true);

    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(abs) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"\"b\"*/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abb", true);

    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(nested) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"ab\"**/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abb", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", true);

    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(stacked) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"ab\"*\"a\"*/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", true);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", true);

    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(range) {
    auto texecutor = tryParseForUnicodeTest(u8"/([0-9].)*/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    
    ACCEPTS_TEST_UNICODE(executor, u8"ab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"0a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"0091", true);

    ACCEPTS_TEST_UNICODE(executor, u8"9", false);
    ACCEPTS_TEST_UNICODE(executor, u8"1234567890", true);
    ACCEPTS_TEST_UNICODE(executor, u8"123456789", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_SUITE_END()

////
//Plus
BOOST_AUTO_TEST_SUITE(Plus)
BOOST_AUTO_TEST_CASE(a) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"+/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(ab) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"ab\"+/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", true);

    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(absplit) {
    auto texecutor = tryParseForUnicodeTest(u8"/(\"a\"\"b\")+/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", true);

    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(abs) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"\"b\"+/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abb", true);

    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(nested) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"ab\"++/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abb", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", true);

    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(stacked) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"ab\"+\"a\"+/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    
    ACCEPTS_TEST_UNICODE(executor, u8"ab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", true);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"ababaaaaa", true);

    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(range) {
    auto texecutor = tryParseForUnicodeTest(u8"/([0-9].)+/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    
    ACCEPTS_TEST_UNICODE(executor, u8"ab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"0a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"0091", true);

    ACCEPTS_TEST_UNICODE(executor, u8"9", false);
    ACCEPTS_TEST_UNICODE(executor, u8"1234567890", true);
    ACCEPTS_TEST_UNICODE(executor, u8"123456789", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_SUITE_END()

////
//Optional
BOOST_AUTO_TEST_SUITE(Optional)
BOOST_AUTO_TEST_CASE(a) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"?/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(ab) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"ab\"?/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(absplit) {
    auto texecutor = tryParseForUnicodeTest(u8"/(\"a\"\"b\")?/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(abs) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"\"b\"?/");
    BOOST_CHECK(texecutor.has_value());

    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abb", false);

    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", false);
}
BOOST_AUTO_TEST_CASE(nested) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"ab\"? ?/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", false);

    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(stacked) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"ab\"?\"a\"?/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aba", true);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", false);

    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_CASE(range) {
    auto texecutor = tryParseForUnicodeTest(u8"/([0-9].)?/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    
    ACCEPTS_TEST_UNICODE(executor, u8"ab", false);
    ACCEPTS_TEST_UNICODE(executor, u8"0a", true);

    ACCEPTS_TEST_UNICODE(executor, u8"9", false);
    ACCEPTS_TEST_UNICODE(executor, u8"", true);
}
BOOST_AUTO_TEST_SUITE_END()

////
//Range
BOOST_AUTO_TEST_SUITE(Range)
BOOST_AUTO_TEST_CASE(single) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"{5}/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aaaa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aaaaa", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaaaaa", false);
}
BOOST_AUTO_TEST_CASE(lower) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"{3,}/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aaa", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaaaaa", true);
}
BOOST_AUTO_TEST_CASE(upper) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"{,3}/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaa", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaaa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aaaaaa", false);
}
BOOST_AUTO_TEST_CASE(both) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0-9]{1,3}/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
    ACCEPTS_TEST_UNICODE(executor, u8"5", true);
    ACCEPTS_TEST_UNICODE(executor, u8"123", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aa", false);
    ACCEPTS_TEST_UNICODE(executor, u8"1234", false);
}
BOOST_AUTO_TEST_SUITE_END()

////
//Opt
BOOST_AUTO_TEST_SUITE(Opt)
BOOST_AUTO_TEST_CASE(simpleab) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"|\"b\"/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"c", false);
}
BOOST_AUTO_TEST_CASE(simpleabstar) {
    auto texecutor = tryParseForUnicodeTest(u8"/(\"a\"|\"b\")*/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", true);
    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaa", true);
    ACCEPTS_TEST_UNICODE(executor, u8"abab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bbbb", true);
}
BOOST_AUTO_TEST_CASE(simple09b) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0-9]|\"b\"/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"3", true);
    ACCEPTS_TEST_UNICODE(executor, u8"b", true);
}
BOOST_AUTO_TEST_CASE(multiple) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"|\"b\"|\"c\"/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"c", true);
}
BOOST_AUTO_TEST_CASE(abplusfollow) {
    auto texecutor = tryParseForUnicodeTest(u8"/(\"a\"|\"b\")+[0-9]./");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
    ACCEPTS_TEST_UNICODE(executor, u8"a", false);
    ACCEPTS_TEST_UNICODE(executor, u8"b01", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaa5!", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ababc1", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abab1a", true);
}
BOOST_AUTO_TEST_CASE(mixeddown1) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"*|\"b\"?|\"c\"+/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaa", true);
    ACCEPTS_TEST_UNICODE(executor, u8"b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bc", false);
    ACCEPTS_TEST_UNICODE(executor, u8"c", true);
    ACCEPTS_TEST_UNICODE(executor, u8"cc", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ccd", false);
}
BOOST_AUTO_TEST_CASE(mixeddownalt1) {
    auto texecutor = tryParseForUnicodeTest(u8"/\"a\"*|(\"b\"?|\"c\"+)/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", true);
    ACCEPTS_TEST_UNICODE(executor, u8"aaa", true);
    ACCEPTS_TEST_UNICODE(executor, u8"b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bc", false);
    ACCEPTS_TEST_UNICODE(executor, u8"c", true);
    ACCEPTS_TEST_UNICODE(executor, u8"cc", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ccd", false);
}
BOOST_AUTO_TEST_CASE(mixedup) {
    auto texecutor = tryParseForUnicodeTest(u8"/(\"a\"|\"b\")+|\"c\"/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"a", true);
    ACCEPTS_TEST_UNICODE(executor, u8"b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"c", true);
    ACCEPTS_TEST_UNICODE(executor, u8"cc", false);
    ACCEPTS_TEST_UNICODE(executor, u8"aaab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bbba", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bbaac", false);
}
BOOST_AUTO_TEST_SUITE_END()

////
//Sequence
BOOST_AUTO_TEST_SUITE(Sequence)
BOOST_AUTO_TEST_CASE(overlap) {
    auto texecutor = tryParseForUnicodeTest(u8"/(\"a\"|\"b\")+\"b\"/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bbb", true);
    ACCEPTS_TEST_UNICODE(executor, u8"acb", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abaa", false);
}
BOOST_AUTO_TEST_CASE(overlapempty) {
    auto texecutor = tryParseForUnicodeTest(u8"/(\"a\"|\"b\")*\"b\"/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
    ACCEPTS_TEST_UNICODE(executor, u8"b", true);
    ACCEPTS_TEST_UNICODE(executor, u8"ab", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bbb", true);
    ACCEPTS_TEST_UNICODE(executor, u8"acb", false);
    ACCEPTS_TEST_UNICODE(executor, u8"abaa", false);
}
BOOST_AUTO_TEST_CASE(taggednumber) {
    auto texecutor = tryParseForUnicodeTest(u8"/[+-]?[0-9]+(\"n\"|\"i\")/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"", false);
    ACCEPTS_TEST_UNICODE(executor, u8"123i", true);
    ACCEPTS_TEST_UNICODE(executor, u8"0n", true);
    ACCEPTS_TEST_UNICODE(executor, u8"-123i", true);
    ACCEPTS_TEST_UNICODE(executor, u8"+-123i", false);
    ACCEPTS_TEST_UNICODE(executor, u8"+i", false);
    ACCEPTS_TEST_UNICODE(executor, u8"123", false);
}
BOOST_AUTO_TEST_SUITE_END()

////
//All
BOOST_AUTO_TEST_SUITE(All)
BOOST_AUTO_TEST_CASE(ii) {
    auto texecutor = tryParseForUnicodeTest(u8"/[0-9]&(\"5\"|\"6\")/");
    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"5", true);
    ACCEPTS_TEST_UNICODE(executor, u8"6", true);
    ACCEPTS_TEST_UNICODE(executor, u8"3", false);
}
BOOST_AUTO_TEST_SUITE_END()

////
//Negative
BOOST_AUTO_TEST_SUITE(Negative)
BOOST_AUTO_TEST_CASE(notbob) {
    auto texecutor = tryParseForUnicodeTest(u8"/!(\"bob\"|\"sally\")/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"5", true);
    ACCEPTS_TEST_UNICODE(executor, u8"6", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bob", false);
    ACCEPTS_TEST_UNICODE(executor, u8"sally", false);
}
BOOST_AUTO_TEST_SUITE_END()

////
//StartsAnchor
BOOST_AUTO_TEST_SUITE(StartsAnchor)
BOOST_AUTO_TEST_CASE(bob) {
    auto texecutor = tryParseForUnicodeTest(u8"/.+ & ^(\"bob\"|\"sally\")/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();

    ACCEPTS_TEST_UNICODE(executor, u8"bob xyz", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bo", false);
    ACCEPTS_TEST_UNICODE(executor, u8"5 bob", false);
}
BOOST_AUTO_TEST_CASE(notbob) {
    auto texecutor = tryParseForUnicodeTest(u8"/.+ & !^(\"bob\"|\"sally\")/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"bob xyz", false);
    ACCEPTS_TEST_UNICODE(executor, u8"bo", true);
    ACCEPTS_TEST_UNICODE(executor, u8"5 bob", true);
}
BOOST_AUTO_TEST_SUITE_END()

////
//EndsAnchor
BOOST_AUTO_TEST_SUITE(EndsAnchor)
BOOST_AUTO_TEST_CASE(bob) {
    auto texecutor = tryParseForUnicodeTest(u8"/.+ & (\"bob\"|\"sally\")$/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"bob xyz", false);
    ACCEPTS_TEST_UNICODE(executor, u8"bo", false);
    ACCEPTS_TEST_UNICODE(executor, u8"5 bob", true);
}
BOOST_AUTO_TEST_CASE(notbob) {
    auto texecutor = tryParseForUnicodeTest(u8"/.+ & !(\"bob\"|\"sally\")$/");

    BOOST_CHECK(texecutor.has_value());
    
    auto executor = texecutor.value();
    ACCEPTS_TEST_UNICODE(executor, u8"bob xyz", true);
    ACCEPTS_TEST_UNICODE(executor, u8"bo", true);
    ACCEPTS_TEST_UNICODE(executor, u8"5 bob", false);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
