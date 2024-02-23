#pragma once

#include "../common.h"

namespace bpath
{
    class PathGlob
    {
    public:
        //
        //TODO fill this in with code to handle path validation
        //

        static PathGlob* jparse(json jv)
        {
            return nullptr;
        }

        static bool test(const std::string& path)
        {
            return true;
        }
    };
}
