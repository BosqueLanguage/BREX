#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"
#include "../../src/regex/brex_compiler.h"

std::optional<brex::UnicodeRegexExecutor*> tryParseForUnicodeCPPIRTest(const std::u8string& str) {
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

std::string fnresolvecppir(const std::string& name, brex::NameResolverState s) {
    return name;
}

bool tryParseIntoNameMapCPPIR(const std::string& name, const std::u8string& str, std::map<std::string, const brex::RegexOpt*>& nmap) {
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

std::optional<brex::UnicodeRegexExecutor*> tryParseForNameSubTestCPPIR(const std::u8string& str, std::map<std::string, const brex::RegexOpt*>& nmap) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str, false);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return std::nullopt;
    }

    std::map<std::string, const brex::LiteralOpt*> envmap;
    std::vector<brex::RegexCompileError> compileerror;
    auto executor = brex::RegexCompiler::compileUnicodeRegexToExecutor(pr.first.value(), nmap, envmap, false, nullptr, fnresolvecppir, compileerror);
    if(!compileerror.empty()) {
        return std::nullopt;
    }

    return std::make_optional(executor);
}

std::optional<brex::CRegexExecutor*> tryParseForCDocsTestCPPIR(const std::string& str) {
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

BOOST_AUTO_TEST_SUITE(CPPIR_OUTPUT)
BOOST_AUTO_TEST_CASE(thisisaliteral) {
    auto texecutor = tryParseForUnicodeCPPIRTest(u8"/\"this is a literal\"*/");
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/\"this is a literal\"*/");
    BOOST_CHECK_EQUAL(cppirinfo.second, "(this is a literal)*");
}
BOOST_AUTO_TEST_CASE(thisisaliteralpepper) {
    auto texecutor = tryParseForUnicodeCPPIRTest(u8"/\"unicode literal 🌶\"*/");
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/\"unicode literal %x1f336;\"*/");
    BOOST_CHECK_EQUAL(cppirinfo.second, "(unicode literal \\u{1f336})*");
}
BOOST_AUTO_TEST_CASE(thisisaliteralc) {
    auto texecutor = tryParseForCDocsTestCPPIR("/'char literals %x59;'/c");
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/'char literals Y'/c");
    BOOST_CHECK_EQUAL(cppirinfo.second, "char literals Y");
}
BOOST_AUTO_TEST_CASE(nameddigit) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMapCPPIR("Digit", u8"/[0-9]/", nmap));

    auto texecutor = tryParseForNameSubTestCPPIR(u8"/[+-]${Digit}+/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/[+-][0-9]+/");
    BOOST_CHECK_EQUAL(cppirinfo.second, "[+\\-][0-9]+");
}
BOOST_AUTO_TEST_CASE(aeiou) {
    auto texecutor = tryParseForUnicodeCPPIRTest(u8"/\"h\"[aeiou]+/");
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/\"h\"[aeiou]+/");
    BOOST_CHECK_EQUAL(cppirinfo.second, "h[aeiou]+");
}
BOOST_AUTO_TEST_CASE(aeiouc) {
    auto texecutor = tryParseForCDocsTestCPPIR("/'h'[aeiou]+/c");
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/'h'[aeiou]+/c");
    BOOST_CHECK_EQUAL(cppirinfo.second, "h[aeiou]+");
}
BOOST_AUTO_TEST_CASE(pepper) {
    auto texecutor = tryParseForUnicodeCPPIRTest(u8"/\"🌶\" %*unicode pepper*%/");
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/\"%x1f336;\"/");
    BOOST_CHECK_EQUAL(cppirinfo.second, "\\u{1f336}");
}
BOOST_AUTO_TEST_CASE(repeatsnumber) {
    auto texecutor = tryParseForUnicodeCPPIRTest(u8"/[+-]? (\"0\" | [1-9][0-9]+)/");
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/[+-]?(\"0\"|([1-9][0-9]+))/");
    BOOST_CHECK_EQUAL(cppirinfo.second, "[+\\-]?(0|([1-9][0-9]+))");
}
BOOST_AUTO_TEST_CASE(repeatsfilename) {
    auto texecutor = tryParseForUnicodeCPPIRTest(u8"/[a-zA-Z0-9_]+ \".\" [a-zA-Z0-9]{1,3}/");
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/[a-zA-Z0-9_]+\".\"[a-zA-Z0-9]{1,3}/");
    BOOST_CHECK_EQUAL(cppirinfo.second, "[a-zA-Z0-9_]+\\.[a-zA-Z0-9]{1,3}");
}
BOOST_AUTO_TEST_CASE(namednumber) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMapCPPIR("NonZeroDigit", u8"/[1-9]/", nmap));
    BOOST_CHECK(tryParseIntoNameMapCPPIR("Digit", u8"/[0-9]/", nmap));

    auto texecutor = tryParseForNameSubTestCPPIR(u8"/[+-]? (\"0\" | ${NonZeroDigit}${Digit}+)/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/[+-]?(\"0\"|([1-9][0-9]+))/");
    BOOST_CHECK_EQUAL(cppirinfo.second, "[+\\-]?(0|([1-9][0-9]+))");
}
BOOST_AUTO_TEST_CASE(sccopedname) {
    std::map<std::string, const brex::RegexOpt*> nmap;
    BOOST_CHECK(tryParseIntoNameMapCPPIR("Main::re2", u8"/[a-zA-Z0-9_]+/", nmap));

    auto texecutor = tryParseForNameSubTestCPPIR(u8"/${Main::re2}/", nmap);
    BOOST_CHECK(texecutor.has_value());

    auto cppirinfo = texecutor.value()->getCPPIRInfo();
    BOOST_CHECK_EQUAL(cppirinfo.first, "/[a-zA-Z0-9_]+/");
    BOOST_CHECK_EQUAL(cppirinfo.second, "[a-zA-Z0-9_]+");
}

BOOST_AUTO_TEST_SUITE_END()
