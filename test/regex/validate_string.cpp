#include <boost/test/unit_test.hpp>

#include "../../src/common.h"

std::pair<std::optional<brex::UnicodeString>, std::optional<std::u8string>> validateString_Unicode(const std::u8string& str) {
    return brex::unescapeUnicodeString((uint8_t*)str.c_str(), str.size());
}

std::pair<std::optional<brex::UnicodeString>, std::optional<std::u8string>> validateString_Unicode_Multiline(const std::u8string& str) {
    return brex::unescapeUnicodeStringLiteralInclMultiline((uint8_t*)str.c_str(), str.size());
}

#define VALIDATE_TEST_UNICODE(RE, EX) { auto res = validateString_Unicode(RE); BOOST_CHECK(res.first.has_value()); BOOST_ASSERT(res.first.value() == EX); }
#define VALIDATE_TEST_UNICODE_MULTILINE(RE, EX) { auto res = validateString_Unicode_Multiline(RE); BOOST_CHECK(res.first.has_value()); BOOST_ASSERT(res.first.value() == EX); }
#define VALIDATE_TEST_UNICODE_FAIL(RE, MSG) { auto res = validateString_Unicode(RE); BOOST_CHECK(res.second.has_value()); std::cout << std::string(res.second.value().cbegin(), res.second.value().cend()) << std::endl; BOOST_ASSERT(res.second.value() == MSG); }

std::pair<std::optional<brex::CString>, std::optional<std::u8string>> validateString_CString(const std::string& str) {
    return brex::unescapeCString((uint8_t*)str.c_str(), str.size());
}

std::pair<std::optional<brex::CString>, std::optional<std::u8string>> validateString_CString_Multiline(const std::string& str) {
    return brex::unescapeCStringLiteralInclMultiline((uint8_t*)str.c_str(), str.size());
}

#define VALIDATE_TEST_CSTRING(RE, EX) { auto res = validateString_CString(RE); BOOST_CHECK(res.first.has_value()); BOOST_ASSERT(res.first.value() == EX); }
#define VALIDATE_TEST_CSTRING_MULTILINE(RE, EX) { auto res = validateString_CString_Multiline(RE); BOOST_CHECK(res.has_value()); BOOST_ASSERT(res.first.value() == EX); }
#define VALIDATE_TEST_CSTRING_FAIL(RE, MSG) { auto res = validateString_CString(RE); BOOST_CHECK(res.second.has_value()); std::cout << std::string(res.second.value().cbegin(), res.second.value().cend()) << std::endl; BOOST_ASSERT(res.second.value() == MSG); }

BOOST_AUTO_TEST_SUITE(ValidateString)

////
//Basic literal strings (ok)
BOOST_AUTO_TEST_SUITE(Literal)
BOOST_AUTO_TEST_SUITE(Unicode)
BOOST_AUTO_TEST_CASE(abc) {
    VALIDATE_TEST_UNICODE(u8"abc", u8"abc");
}

BOOST_AUTO_TEST_CASE(eps) {
    VALIDATE_TEST_UNICODE(u8"", u8"");
}

BOOST_AUTO_TEST_CASE(literal) {
    VALIDATE_TEST_UNICODE(u8"a🌵c", u8"a🌵c");
}

BOOST_AUTO_TEST_CASE(escape) {
    VALIDATE_TEST_UNICODE(u8"%x0;", std::u8string({'\0'})); //o/w embedded null gets lost
    VALIDATE_TEST_UNICODE(u8"%x59;", u8"Y");
    VALIDATE_TEST_UNICODE(u8"%x1f335;", u8"🌵");

    VALIDATE_TEST_UNICODE(u8"%;", u8"\"");
    VALIDATE_TEST_UNICODE(u8"%%;", u8"%");
    VALIDATE_TEST_UNICODE(u8"%n;", u8"\n");
    VALIDATE_TEST_UNICODE(u8"\t", u8"\t");
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(C)
BOOST_AUTO_TEST_CASE(abc) {
    VALIDATE_TEST_CSTRING("abc", "abc");
}

BOOST_AUTO_TEST_CASE(eps) {
    VALIDATE_TEST_CSTRING("", "");
}

BOOST_AUTO_TEST_CASE(escape) {
    VALIDATE_TEST_CSTRING("%x59;", "Y");

    VALIDATE_TEST_CSTRING("%;", "'");
    VALIDATE_TEST_CSTRING("%%;", "%");
    VALIDATE_TEST_CSTRING("%n;", "\n");
    VALIDATE_TEST_CSTRING("\t", "\t");
    VALIDATE_TEST_CSTRING("\n", "\n");
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Basic literal strings (err)
BOOST_AUTO_TEST_SUITE(Literal_ERR)
BOOST_AUTO_TEST_SUITE(Unicode_ERR)
BOOST_AUTO_TEST_CASE(escape_ERR) {
    VALIDATE_TEST_UNICODE_FAIL(u8"%", u8"Unterminated escape sequence -- missing ;");
    VALIDATE_TEST_UNICODE_FAIL(u8"%x0", u8"Unterminated escape sequence -- missing ;");
    VALIDATE_TEST_UNICODE_FAIL(u8"%59;", u8"Invalid escape name -- %59;");
    VALIDATE_TEST_UNICODE_FAIL(u8"%x8f3G;", u8"err4");
    VALIDATE_TEST_UNICODE_FAIL(u8"%x100000000000;", u8"err5");
    VALIDATE_TEST_UNICODE_FAIL(u8"%x1f3335;", u8"err6");
    VALIDATE_TEST_UNICODE_FAIL(u8"%x;", u8"err7");
    VALIDATE_TEST_UNICODE_FAIL(u8"%bob;", u8"err8");
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(C_ERR)
BOOST_AUTO_TEST_CASE(chars_ERR) {
    VALIDATE_TEST_CSTRING_FAIL("\3", u8"Invalid character in string");
    VALIDATE_TEST_CSTRING_FAIL("\177", u8"err11");
    VALIDATE_TEST_CSTRING_FAIL("\v", u8"err12");
}

BOOST_AUTO_TEST_CASE(escape_ERR) {
    VALIDATE_TEST_CSTRING_FAIL("%", u8"Unterminated escape sequence -- missing ;");
    VALIDATE_TEST_CSTRING_FAIL("%x0", u8"Unterminated escape sequence -- missing ;");
    VALIDATE_TEST_CSTRING_FAIL("%59;", u8"Invalid escape name -- %59;");
    VALIDATE_TEST_CSTRING_FAIL("%x8f3G;", u8"err16");
    VALIDATE_TEST_CSTRING_FAIL("%x100;", u8"err17");
    VALIDATE_TEST_CSTRING_FAIL("%x7F;", u8"err18");
    VALIDATE_TEST_CSTRING_FAIL("%x;", u8"err19");
    VALIDATE_TEST_CSTRING_FAIL("%bob;", u8"err20");

    VALIDATE_TEST_CSTRING_FAIL("a🌵c", u8"err21");
    VALIDATE_TEST_CSTRING_FAIL("%a;", u8"err22");
    VALIDATE_TEST_CSTRING_FAIL("%x7;", u8"err23");
    VALIDATE_TEST_CSTRING_FAIL("%x127;", u8"err24");
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

////
//Strings with multiple lines

////
//Strings with alignment


BOOST_AUTO_TEST_SUITE_END()

