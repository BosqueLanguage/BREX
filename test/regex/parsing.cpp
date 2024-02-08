#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex.h"
#include "../../src/regex/brex_parser.h"

BOOST_AUTO_TEST_SUITE(Parsing)

BOOST_AUTO_TEST_CASE(aliteral) {
    auto lstr = u8"/\"abc\"/";
    auto pr = brex::RegexParser::parseUnicodeRegex(lstr);
    BOOST_CHECK(pr.first.has_value() && pr.second.empty());

    auto bre = pr.first.value()->toBSQONFormat();
    BOOST_CHECK(bre == lstr);
}

BOOST_AUTO_TEST_SUITE_END()
