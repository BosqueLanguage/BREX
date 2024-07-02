#include "path_glob.h"

namespace bpath
{
    GlobSimpleComponent* GlobSimpleComponent::jparse(json jv)
    {
        BREX_ASSERT(jv.is_object(), "Expected object");
        BREX_ASSERT(jv.contains("tag"), "Missing tag");

        auto tag = jv["tag"].get<std::string>();

        if(tag == "LITERAL_TAG") {
            return LiteralComponent::jparse(jv);
        }
        else if(tag == "WILDCARD_TAG") {
            return WildcardComponent::jparse(jv);
        }
        else {
            BREX_ASSERT(tag == "REGEX_TAG", "Unknown tag");

            return RegexComponent::jparse(jv);
        }
    }

    LiteralComponent* LiteralComponent::jparse(json jv)
    {
        BREX_ASSERT(jv.contains("value"), "Missing value");
        BREX_ASSERT(jv["value"].is_string(), "Expected string");

        std::string value = jv["value"].get<std::string>();
        auto astr = brex::unescapeCString((uint8_t*)value.c_str(), value.size());

        BREX_ASSERT(astr.first.has_value(), "Invalid CString");
        return new LiteralComponent(astr.first.value());
    }

    WildcardComponent* WildcardComponent::jparse(json jv)
    {
        return new WildcardComponent();
    }

    RegexComponent* RegexComponent::jparse(json jv)
    {
        BREX_ASSERT(jv.contains("re"), "Missing re");
        BREX_ASSERT(jv["re"].is_object(), "Expected Regex JSON object");

        auto reo = brex::Regex::jparse(jv["re"]);
        return new RegexComponent(reo);
    }

    SegmentGlobCompnent* SegmentGlobCompnent::jparse(json jv)
    {
        BREX_ASSERT(jv.is_object(), "Expected object");
        BREX_ASSERT(jv.contains("tag"), "Missing tag");

        auto tag = jv["tag"].get<std::string>();

        if(tag == "LITERAL_TAG") {
            return SegmentLiteralComponent::jparse(jv);
        }
        else if(tag == "WILDCARD_TAG") {
            return SegmentWildcardComponent::jparse(jv);
        }
        else if(tag == "REGEX_TAG") {
            return SegmentRegexComponent::jparse(jv);
        }
        else if(tag == "EXPANSIVE_WILDCARD_TAG") {
            return SegmentExpansiveWildcardComponent::jparse(jv);
        }
        else if(tag == "EXPANSIVE_STAR_TAG") {
            return SegmentExpansiveStarComponent::jparse(jv);
        }
        else if(tag == "EXPANSIVE_PLUS_TAG") {
            return SegmentExpansivePlusComponent::jparse(jv);
        }
        else if(tag == "EXPANSIVE_RANGE_TAG") {
            return SegmentExpansiveRangeComponent::jparse(jv);
        }
        else {
            BREX_ASSERT(tag == "EXPANSIVE_QUESTION_TAG", "Unknown tag");

            return SegmentExpansiveQuestionComponent::jparse(jv);
        }
    }

    SegmentLiteralComponent* SegmentLiteralComponent::jparse(json jv)
    {
        BREX_ASSERT(jv.contains("value"), "Missing value");
        BREX_ASSERT(jv["value"].is_string(), "Expected string");

        std::string value = jv["value"].get<std::string>();
        auto astr = brex::unescapeCString((uint8_t*)value.c_str(), value.size());

        BREX_ASSERT(astr.first.has_value(), "Invalid CString");

        //TODO: need to do more validation here -- like newline is not cool
        return new SegmentLiteralComponent(astr.first.value());
    }

    SegmentWildcardComponent* SegmentWildcardComponent::jparse(json jv)
    {
        return new SegmentWildcardComponent();
    }

    SegmentRegexComponent* SegmentRegexComponent::jparse(json jv)
    {
        BREX_ASSERT(jv.contains("re"), "Missing re");
        BREX_ASSERT(jv["re"].is_object(), "Expected Regex JSON object");

        auto reo = brex::Regex::jparse(jv["re"]);
        return new SegmentRegexComponent(reo);
    }

    SegmentExpansiveWildcardComponent* SegmentExpansiveWildcardComponent::jparse(json jv)
    {
        return new SegmentExpansiveWildcardComponent();
    }

    SegmentExpansiveStarComponent* SegmentExpansiveStarComponent::jparse(json jv)
    {
        BREX_ASSERT(jv.contains("re"), "Missing re");
        BREX_ASSERT(jv["re"].is_object(), "Expected Regex JSON object");

        auto reo = brex::Regex::jparse(jv["re"]);
        return new SegmentExpansiveStarComponent(reo);
    }

    SegmentExpansivePlusComponent* SegmentExpansivePlusComponent::jparse(json jv)
    {
        BREX_ASSERT(jv.contains("re"), "Missing re");
        BREX_ASSERT(jv["re"].is_object(), "Expected Regex JSON object");

        auto reo = brex::Regex::jparse(jv["re"]);
        return new SegmentExpansivePlusComponent(reo);
    }

    SegmentExpansiveRangeComponent* SegmentExpansiveRangeComponent::jparse(json jv)
    {
        BREX_ASSERT(jv.contains("re"), "Missing re");
        BREX_ASSERT(jv["re"].is_object(), "Expected Regex JSON object");

        BREX_ASSERT(jv.contains("min"), "Missing min");
        BREX_ASSERT(jv["min"].is_number_integer(), "Expected integer");

        BREX_ASSERT(jv.contains("max"), "Missing max");
        BREX_ASSERT(jv["max"].is_number_integer(), "Expected integer");

        auto reo = brex::Regex::jparse(jv["re"]);
        return new SegmentExpansiveRangeComponent(reo, jv["min"].get<int>(), jv["max"].get<int>());
    }

    SegmentExpansiveQuestionComponent* SegmentExpansiveQuestionComponent::jparse(json jv)
    {
        BREX_ASSERT(jv.contains("re"), "Missing re");
        BREX_ASSERT(jv["re"].is_object(), "Expected Regex JSON object");

        auto reo = brex::Regex::jparse(jv["re"]);
        return new SegmentExpansiveQuestionComponent(reo);
    }

    GlobAuthorityInfo* GlobAuthorityInfo::jparse(json jv)
    {
        BREX_ASSERT(jv.is_object(), "Expected object");

        std::optional<GlobSimpleComponent*> userinfo = std::nullopt;
        if(jv.contains("userinfo") && !jv["userinfo"].is_null()) {
            userinfo = GlobSimpleComponent::jparse(jv["userinfo"]);
        }

        BREX_ASSERT(jv.contains("host"), "Missing host");
        auto host = GlobSimpleComponent::jparse(jv["host"]);

        return new GlobAuthorityInfo(userinfo, host);
    }

    GlobElementInfo* GlobElementInfo::jparse(json jv)
    {
        BREX_ASSERT(jv.is_object(), "Expected object");

        BREX_ASSERT(jv.contains("ename"), "Missing ename");
        auto ename = GlobSimpleComponent::jparse(jv["ename"]);

        std::optional<GlobSimpleComponent*> ext = std::nullopt;
        if(jv.contains("ext") && !jv["ext"].is_null()) {
            ext = GlobSimpleComponent::jparse(jv["ext"]);
        }

        return new GlobElementInfo(ename, ext);
    }

    PathGlob* PathGlob::jparse(json jv)
    {
        BREX_ASSERT(jv.is_object(), "Expected object");

        BREX_ASSERT(jv.contains("scheme"), "Missing scheme");
        auto scheme = GlobSimpleComponent::jparse(jv["scheme"]);

        std::optional<GlobAuthorityInfo*> authorityinfo = std::nullopt;
        if(jv.contains("authorityinfo") && !jv["authorityinfo"].is_null()) {
            authorityinfo = GlobAuthorityInfo::jparse(jv["authorityinfo"]);
        }

        BREX_ASSERT(jv.contains("segments"), "Missing segments");
        BREX_ASSERT(jv["segments"].is_array(), "Expected array");

        std::vector<SegmentGlobCompnent*> segments;
        std::transform(jv["segments"].begin(), jv["segments"].end(), std::back_inserter(segments), [](const json& js) { 
            return SegmentGlobCompnent::jparse(js);
        });

        BREX_ASSERT(jv.contains("elementinfo"), "Missing elementinfo");
        auto elementinfo = GlobElementInfo::jparse(jv["elementinfo"]);

        BREX_ASSERT(jv.contains("tailingSlash"), "Missing elementinfo");
        BREX_ASSERT(jv["tailingSlash"].is_boolean(), "Expected boolean");

        return new PathGlob(scheme, authorityinfo, segments, elementinfo, jv["tailingSlash"].get<bool>());
    }
}