#include <boost/test/unit_test.hpp>

#include "../../src/regex/brex_system.h"

BOOST_AUTO_TEST_SUITE(System)

BOOST_AUTO_TEST_SUITE(Single)
BOOST_AUTO_TEST_CASE(abc) {
    brex::RENSInfo ninfo = {
        {
            "Main",
            {}
        },
        {
            {
                "Foo",
                u8"/\"abc\"/"
            
            }
        }
    };

    std::vector<brex::RENSInfo> ninfos = { ninfo };
    std::vector<std::u8string> errors;
    brex::ReSystem::processSystem(ninfos, errors);

    BOOST_ASSERT(errors.empty());

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()