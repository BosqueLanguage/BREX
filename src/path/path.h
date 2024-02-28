#pragma once

#include "../common.h"

namespace bpath
{
    class AuthorityInfo
    {
    public:
        const std::optional<brex::ASCIIString> userinfo;
        const brex::ASCIIString host;
    };

    class ElementInfo
    {
    public:
        const brex::ASCIIString ename;
        const std::optional<brex::ASCIIString> ext;
    };

    class PathElement
    {
    public:
        const brex::ASCIIString scheme;
        const std::optional<AuthorityInfo> authorityinfo;
        const std::vector<brex::ASCIIString> segments;
        const std::optional<ElementInfo> elementinfo;

        static PathElement* jparse(json jv)
        {
            return nullptr;
        }

        static bool test(const std::string& path)
        {
            return true;
        }
    };

    class PathGroup
    {
    public:
        const brex::ASCIIString scheme;
        const std::optional<AuthorityInfo> authorityinfo;
        const std::vector<brex::ASCIIString> segments;

        static PathGroup* jparse(json jv)
        {
            return nullptr;
        }

        static bool test(const std::string& path)
        {
            return true;
        }
    };
}
