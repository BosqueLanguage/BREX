#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"
#include "../../src/regex/brex_compiler.h"

std::optional<brex::UnicodeRegexExecutor*> tryParseForUnicodeBSQIRTest(const std::u8string& str) {
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

std::string fnresolvebsqir(const std::string& name, brex::NameResolverState s) {
    return name;
}

bool tryParseIntoNameMapBSQIR(const std::string& name, const std::u8string& str, std::map<std::string, const brex::RegexOpt*>& nmap) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str, false);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return false;
    }

    if(pr.first.value()->preanchor != nullptr || pr.first.value()->postanchor != nullptr) {
        return false;
    }

    if(pr.first.value()->re->tag != brex::RegexComponentTag::Single) {
        return false;
    }

    auto sre = static_cast<const brex::RegexSingleComponent*>(pr.first.value()->re);
    if(sre->entry.isFrontCheck || sre->entry.isBackCheck || sre->entry.isNegated) {
        return false;
    }

    return nmap.insert({ name, sre->entry.opt }).second;
}

std::optional<brex::UnicodeRegexExecutor*> tryParseForNameSubTestBSQIR(const std::u8string& str, std::map<std::string, const brex::RegexOpt*>& nmap) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str, false);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return std::nullopt;
    }

    std::map<std::string, const brex::LiteralOpt*> envmap;
    std::vector<brex::RegexCompileError> compileerror;
    auto executor = brex::RegexCompiler::compileUnicodeRegexToExecutor(pr.first.value(), nmap, envmap, false, nullptr, fnresolvebsqir, compileerror);
    if(!compileerror.empty()) {
        return std::nullopt;
    }

    return std::make_optional(executor);
}

std::optional<brex::CRegexExecutor*> tryParseForCDocsTestBSQIR(const std::string& str) {
    auto pr = brex::RegexParser::parseCRegex(std::u8string(str.cbegin(), str.cend()), false);
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

BOOST_AUTO_TEST_SUITE(BSQIR_OUTPUT)
BOOST_AUTO_TEST_CASE(thisisaliteral) {
    auto texecutor = tryParseForUnicodeBSQIRTest(u8"/\"this is a literal\"*/");
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/\"this is a literal\"*/");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(re.* (str.to.re \"this is a literal\"))");
}
BOOST_AUTO_TEST_CASE(thisisaliteralpepper) {
    auto texecutor = tryParseForUnicodeBSQIRTest(u8"/\"unicode literal 🌶\"*/");
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/\"unicode literal %x1f336;\"*/");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(re.* (str.to.re \"unicode literal \\u{1f336}\"))");
}
BOOST_AUTO_TEST_CASE(thisisaliteralc) {
    auto texecutor = tryParseForCDocsTestBSQIR("/'char literals %x59;'/c");
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/'char literals Y'/c");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(str.to.re \"char literals Y\")");
}
BOOST_AUTO_TEST_CASE(nameddigit) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMapBSQIR("Digit", u8"/[0-9]/", nmap));

    auto texecutor = tryParseForNameSubTestBSQIR(u8"/[+-]${Digit}+/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/[+-][0-9]+/");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(re.++ (re.union (str.to.re \"+\") (str.to.re \"-\")) (re.+ (re.range \"0\" \"9\")))");
}
BOOST_AUTO_TEST_CASE(aeiou) {
    auto texecutor = tryParseForUnicodeBSQIRTest(u8"/\"h\"[aeiou]+/");
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/\"h\"[aeiou]+/");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(re.++ (str.to.re \"h\") (re.+ (re.union (str.to.re \"a\") (str.to.re \"e\") (str.to.re \"i\") (str.to.re \"o\") (str.to.re \"u\"))))");
}
BOOST_AUTO_TEST_CASE(aeiouc) {
    auto texecutor = tryParseForCDocsTestBSQIR("/'h'[aeiou]+/c");
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/'h'[aeiou]+/c");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(re.++ (str.to.re \"h\") (re.+ (re.union (str.to.re \"a\") (str.to.re \"e\") (str.to.re \"i\") (str.to.re \"o\") (str.to.re \"u\"))))");
}
BOOST_AUTO_TEST_CASE(pepper) {
    auto texecutor = tryParseForUnicodeBSQIRTest(u8"/\"🌶\" %*unicode pepper*%/");
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/\"%x1f336;\"/");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(str.to.re \"\\u{1f336}\")");
}
BOOST_AUTO_TEST_CASE(repeatsnumber) {
    auto texecutor = tryParseForUnicodeBSQIRTest(u8"/[+-]? (\"0\" | [1-9][0-9]+)/");
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/[+-]?(\"0\"|([1-9][0-9]+))/");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(re.++ (re.opt (re.union (str.to.re \"+\") (str.to.re \"-\"))) (re.union (str.to.re \"0\") (re.++ (re.range \"1\" \"9\") (re.+ (re.range \"0\" \"9\")))))");
}
BOOST_AUTO_TEST_CASE(repeatsfilename) {
    auto texecutor = tryParseForUnicodeBSQIRTest(u8"/[a-zA-Z0-9_]+ \".\" [a-zA-Z0-9]{1,3}/");
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/[a-zA-Z0-9_]+\".\"[a-zA-Z0-9]{1,3}/");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(re.++ (re.+ (re.union (re.range \"a\" \"z\") (re.range \"A\" \"Z\") (re.range \"0\" \"9\") (str.to.re \"_\"))) (str.to.re \".\") ((_ re.loop 1 3) (re.union (re.range \"a\" \"z\") (re.range \"A\" \"Z\") (re.range \"0\" \"9\"))))");
}
BOOST_AUTO_TEST_CASE(namednumber) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMapBSQIR("NonZeroDigit", u8"/[1-9]/", nmap));
    BOOST_CHECK(tryParseIntoNameMapBSQIR("Digit", u8"/[0-9]/", nmap));

    auto texecutor = tryParseForNameSubTestBSQIR(u8"/[+-]? (\"0\" | ${NonZeroDigit}${Digit}+)/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/[+-]?(\"0\"|([1-9][0-9]+))/");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(re.++ (re.opt (re.union (str.to.re \"+\") (str.to.re \"-\"))) (re.union (str.to.re \"0\") (re.++ (re.range \"1\" \"9\") (re.+ (re.range \"0\" \"9\")))))");
}
BOOST_AUTO_TEST_CASE(sccopedname) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMapBSQIR("Main::re2", u8"/[a-zA-Z0-9_]+/", nmap));

    auto texecutor = tryParseForNameSubTestBSQIR(u8"/${Main::re2}/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto bsqirinfo = texecutor.value()->getBSQIRInfo();
    BOOST_CHECK_EQUAL(bsqirinfo.first, "/[a-zA-Z0-9_]+/");
    BOOST_CHECK_EQUAL(bsqirinfo.second, "(re.+ (re.union (re.range \"a\" \"z\") (re.range \"A\" \"Z\") (re.range \"0\" \"9\") (str.to.re \"_\")))");
}

BOOST_AUTO_TEST_SUITE_END()
