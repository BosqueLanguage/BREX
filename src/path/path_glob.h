#pragma once

#include "../common.h"
#include "path.h"

#include "../regex/brex.h"

namespace bpath
{
    enum class GlobComponentType
    {
        LITERAL,
        WILDCARD,
        REGEX,
        SEGMENT_LITERAL,
        SEGMENT_WILDCARD,
        SEGMENT_REGEX,
        SEGMENT_EXPANSIVE_WILDCARD,
        SEGMENT_EXPANSIVE_STAR,
        SEGMENT_EXPANSIVE_PLUS,
        SEGMENT_EXPANSIVE_RANGE,
        SEGMENT_EXPANSIVE_QUESTION
    };

    class GlobComponent
    {
    public:
        const GlobComponentType type;

        GlobComponent(GlobComponentType type) : type(type) {;}
        virtual ~GlobComponent() {;}

        virtual std::u8string toBSQONFormat() const = 0;

        static GlobComponent* jparse(json jv);
    };

    class LiteralComponent : public GlobComponent
    {
    public:
        const brex::ASCIIString value;

        LiteralComponent(brex::ASCIIString value) : GlobComponent(GlobComponentType::LITERAL), value(value) {;}
        virtual ~LiteralComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            return std::u8string(this->value.cbegin(), this->value.cend());
        }
    };

    class WildcardComponent : public GlobComponent
    {
    public:
        WildcardComponent() : GlobComponent(GlobComponentType::WILDCARD) {;}
        virtual ~WildcardComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            return u8"*";
        }
    };

    class RegexComponent : public GlobComponent
    {
    public:
        const brex::Regex* re;

        RegexComponent(const brex::Regex* re) : GlobComponent(GlobComponentType::REGEX), re(re) {;}
        virtual ~RegexComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            return re->toBSQONGlobFormat();
        }
    };

    class SegmentGlobCompnent : public GlobComponent
    {
    public:
        SegmentGlobCompnent(GlobComponentType type) : GlobComponent(type) {;}
        virtual ~SegmentGlobCompnent() {;}
    };

    class SegmentLiteralComponent : public SegmentGlobCompnent
    {
    public:
        const brex::ASCIIString value;

        SegmentLiteralComponent(brex::ASCIIString value) : SegmentGlobCompnent(GlobComponentType::SEGMENT_LITERAL), value(value) {;}
        virtual ~SegmentLiteralComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            return std::u8string(this->value.cbegin(), this->value.cend());
        }
    };
    
    class SegmentWildcardComponent : public SegmentGlobCompnent
    {
    public:
        SegmentWildcardComponent() : SegmentGlobCompnent(GlobComponentType::SEGMENT_WILDCARD) {;}
        virtual ~SegmentWildcardComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            return u8"*";
        }
    };

    class SegmentRegexComponent : public SegmentGlobCompnent
    {
    public:
        const brex::Regex* re;

        SegmentRegexComponent(const brex::Regex* re) : SegmentGlobCompnent(GlobComponentType::SEGMENT_REGEX), re(re) {;}
        virtual ~SegmentRegexComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            return re->toBSQONGlobFormat();
        }
    };

    class SegmentExpansiveComponent : public SegmentGlobCompnent
    {
    public:
        SegmentExpansiveComponent(GlobComponentType type) : SegmentGlobCompnent(type) {;}
        virtual ~SegmentExpansiveComponent() {;}
    };

    class SegmentExpansiveWildcardComponent : public SegmentExpansiveComponent
    {
    public:
        SegmentExpansiveWildcardComponent() : SegmentExpansiveComponent(GlobComponentType::SEGMENT_EXPANSIVE_WILDCARD) {;}
        virtual ~SegmentExpansiveWildcardComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            return u8"**";
        }
    };

    class SegmentExpansiveRegexComponent : public SegmentExpansiveComponent
    {
    public:
        const brex::Regex* re;

        SegmentExpansiveRegexComponent(GlobComponentType type, const brex::Regex* re) : SegmentExpansiveComponent(type), re(re) {;}
        virtual ~SegmentExpansiveRegexComponent() {;} 
    };

    class SegmentExpansiveStarComponent : public SegmentExpansiveRegexComponent
    {
    public:
        SegmentExpansiveStarComponent(const brex::Regex* re) : SegmentExpansiveRegexComponent(GlobComponentType::SEGMENT_EXPANSIVE_STAR, re) {;}
        virtual ~SegmentExpansiveStarComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            return re->toBSQONGlobFormat() + u8'*';
        }
    };

    class SegmentExpansivePlusComponent : public SegmentExpansiveRegexComponent
    {
    public:
        SegmentExpansivePlusComponent(const brex::Regex* re) : SegmentExpansiveRegexComponent(GlobComponentType::SEGMENT_EXPANSIVE_PLUS, re) {;}
        virtual ~SegmentExpansivePlusComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            return re->toBSQONGlobFormat() + u8'+';
        }
    };

    class SegmentExpansiveRangeComponent : public SegmentExpansiveRegexComponent
    {
    public:
        const int64_t low;
        const int64_t high;

        SegmentExpansiveRangeComponent(const brex::Regex* re, int64_t low, int64_t high) : SegmentExpansiveRegexComponent(GlobComponentType::SEGMENT_EXPANSIVE_RANGE, re), low(low), high(high) {;}
        virtual ~SegmentExpansiveRangeComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            std::string iterstr{'{'};
            if(this->low == this->high) {
                iterstr += std::to_string(this->low) + std::string{'}'};
            }
            else {
                if(this->low == 0) {
                    iterstr += std::string{','} + std::to_string(this->high) + std::string{'}'};
                }
                else if(this->high == UINT16_MAX) {
                    iterstr += std::to_string(this->low) + std::string{','} + std::string{'}'};
                }
                else {
                    iterstr += std::to_string(this->low) + std::string{','} + std::to_string(this->high) + std::string{'}'};
                }   
            }
            
            return re->toBSQONGlobFormat() + std::u8string(iterstr.cbegin(), iterstr.cend());
        }
    };

    class SegmentExpansivQuestionComponent : public SegmentExpansiveRegexComponent
    {
    public:
        SegmentExpansivQuestionComponent(const brex::Regex* re) : SegmentExpansiveRegexComponent(GlobComponentType::SEGMENT_EXPANSIVE_QUESTION, re) {;}
        virtual ~SegmentExpansivQuestionComponent() {;}

        std::u8string toBSQONFormat() const override final
        {
            return re->toBSQONGlobFormat() + u8'?';
        }
    };

    class AuthorityInfo
    {
    public:
        const std::optional<GlobSimple*> userinfo;
        const GlobSimple* host;
    };

    class ElementInfo
    {
    public:
        const GlobSimple* ename;
        const std::optional<GlobSimple*> ext;
    };

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

    typedef PathGlob ResourceDescriptor;
}
