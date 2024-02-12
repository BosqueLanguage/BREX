#pragma once

#include "../common.h"

namespace brex
{
    enum class RegexOptTag
    {
        Literal,
        CharRange,
        CharClassDot,
        NamedRegex,
        EnvRegex,
        StarRepeat,
        PlusRepeat,
        RangeRepeat,
        Optional,
        AnyOf,
        Sequence
    };

    class RegexOpt
    {
    public:
        const RegexOptTag tag;

        RegexOpt(RegexOptTag tag): tag(tag) {;}
        virtual ~RegexOpt() {;}

        virtual bool needsParens() const { return false; }
        virtual bool needsSequenceParens() const { return false; }
        virtual std::u8string toBSQONFormat() const = 0;

        static RegexOpt* jparse(json j);
    };

    class LiteralOpt : public RegexOpt
    {
    public:
        const std::vector<RegexChar> codes;
        const bool isunicode;

        LiteralOpt(std::vector<RegexChar> codes, bool isunicode) : RegexOpt(RegexOptTag::Literal), codes(codes), isunicode(isunicode) {;}
        virtual ~LiteralOpt() = default;

        virtual std::u8string toBSQONFormat() const override final
        {
            if(this->isunicode) {
                auto bbytes = escapeRegexUnicodeLiteralCharBuffer(this->codes);
                return std::u8string{'"'} + std::u8string(bbytes.cbegin(), bbytes.cend()) + std::u8string{'"'};
            }
            else {
                auto bbytes = escapeRegexASCIILiteralCharBuffer(this->codes);
                return std::u8string{'\''} + std::u8string(bbytes.cbegin(), bbytes.cend()) + std::u8string{'\''};
            }
        }

        static LiteralOpt* jparse(json j)
        {
            std::vector<RegexChar> codes;
            auto jcodes = j["charcodes"];
            std::transform(jcodes.cbegin(), jcodes.cend(), std::back_inserter(codes), [](const json& rv) {
                return rv.get<RegexChar>();
            });

            const bool isunicode = j["isunicode"].get<bool>();

            return new LiteralOpt(codes, isunicode);
        }
    };

    class CharRangeOpt : public RegexOpt
    {
    public:
        const bool compliment;
        const std::vector<SingleCharRange> ranges;
        const bool isunicode;

        CharRangeOpt(bool compliment, std::vector<SingleCharRange> ranges, bool isunicode) : RegexOpt(RegexOptTag::CharRange), compliment(compliment), ranges(ranges), isunicode(isunicode) {;}
        virtual ~CharRangeOpt() = default;

        virtual std::u8string toBSQONFormat() const override final
        {
            std::u8string rngs = {'['};
            if(this->compliment) {
                rngs.push_back('^');
            }

            for(auto ii = this->ranges.cbegin(); ii != this->ranges.cend(); ++ii) {
                auto cr = *ii;

                auto lowbytes = this->isunicode ? escapeSingleUnicodeRegexChar(cr.low) : escapeSingleASCIIRegexChar(cr.low);
                rngs.append(lowbytes.cbegin(), lowbytes.cend());

                if(cr.low != cr.high) {
                    rngs.push_back('-');
                    
                    auto highbytes = this->isunicode ? escapeSingleUnicodeRegexChar(cr.high) : escapeSingleASCIIRegexChar(cr.high);
                    rngs.append(highbytes.cbegin(), highbytes.cend());
                }
            }
            rngs.push_back(']');

            return rngs;
        }

        static CharRangeOpt* jparse(json j)
        {
            const bool compliment = j["compliment"].get<bool>();

            std::vector<SingleCharRange> ranges;
            auto jranges = j["range"];
            std::transform(jranges.cbegin(), jranges.cend(), std::back_inserter(ranges), [](const json& rv) {
                auto lb = rv["lb"].get<RegexChar>();
                auto ub = rv["ub"].get<RegexChar>();

                return SingleCharRange{lb, ub};
            });

            const bool isunicode = j["isunicode"].get<bool>();

            return new CharRangeOpt(compliment, ranges, isunicode);
        }
    };

    class CharClassDotOpt : public RegexOpt
    {
    public:
        CharClassDotOpt() : RegexOpt(RegexOptTag::CharClassDot) {;}
        virtual ~CharClassDotOpt() = default;

        virtual std::u8string toBSQONFormat() const override final
        {
            return std::u8string{'.'};
        }

        static CharClassDotOpt* jparse(json j)
        {
            return new CharClassDotOpt();
        }
    };

    class NamedRegexOpt : public RegexOpt
    {
    public:
        //The namespace of the regex including scope (but not resolved)
        const std::string rname;

        NamedRegexOpt(const std::string& rname) : RegexOpt(RegexOptTag::NamedRegex), rname(rname) {;}
        virtual ~NamedRegexOpt() = default;

        virtual std::u8string toBSQONFormat() const override final
        {
            return std::u8string{'{'} + std::u8string(this->rname.cbegin(), this->rname.cend()) + std::u8string{'}'};
        }

        static NamedRegexOpt* jparse(json j)
        {
            const std::string rname = j["rname"].get<std::string>();

            return new NamedRegexOpt(rname);
        }
    };

    class EnvRegexOpt : public RegexOpt
    {
    public:
        const std::string ename;

        EnvRegexOpt(const std::string& ename) : RegexOpt(RegexOptTag::EnvRegex), ename(ename) {;}
        virtual ~EnvRegexOpt() = default;

        virtual std::u8string toBSQONFormat() const override final
        {
            return std::u8string{'{'} + std::u8string(this->ename.cbegin(), this->ename.cend()) + std::u8string{'}'};
        }

        static EnvRegexOpt* jparse(json j)
        {
            const std::string ename = j["ename"].get<std::string>();

            return new EnvRegexOpt(ename);
        }
    };

    class StarRepeatOpt : public RegexOpt
    {
    public:
        const RegexOpt* repeat;

        StarRepeatOpt(const RegexOpt* repeat) : RegexOpt(RegexOptTag::StarRepeat), repeat(repeat) {;}
        virtual ~StarRepeatOpt() = default;

        virtual bool needsParens() const override final { return true; }
        virtual std::u8string toBSQONFormat() const override final
        {
            if(!this->repeat->needsParens()) {
                return this->repeat->toBSQONFormat() + std::u8string{'*'};
            }
            else {
                return std::u8string{'('} + this->repeat->toBSQONFormat() + std::u8string{')'} + std::u8string{'*'};
            }
        }

        static StarRepeatOpt* jparse(json j)
        {
            auto repeat = RegexOpt::jparse(j["repeat"]);
            return new StarRepeatOpt(repeat);
        }
    };

    class PlusRepeatOpt : public RegexOpt
    {
    public:
        const RegexOpt* repeat;

        PlusRepeatOpt(const RegexOpt* repeat) : RegexOpt(RegexOptTag::PlusRepeat), repeat(repeat) {;}
        virtual ~PlusRepeatOpt() = default;

        virtual bool needsParens() const override final { return true; }
        virtual std::u8string toBSQONFormat() const override final
        {
            if (!this->repeat->needsParens()) {
                return this->repeat->toBSQONFormat() + std::u8string{'+'};
            }
            else {
                return std::u8string{'('} + this->repeat->toBSQONFormat() + std::u8string{')'} + std::u8string{'+'};
            }
        }

        static PlusRepeatOpt* jparse(json j)
        {
            auto repeat = RegexOpt::jparse(j["repeat"]);
            return new PlusRepeatOpt(repeat);
        }
    };

    class RangeRepeatOpt : public RegexOpt
    {
    public:
        const RegexOpt* repeat;
        const uint16_t low;
        const uint16_t high; //if high == INT16_MAX then this is an unbounded repeat

        RangeRepeatOpt(uint16_t low, uint16_t high, const RegexOpt* repeat) : RegexOpt(RegexOptTag::RangeRepeat), repeat(repeat), low(low), high(high) {;}
        virtual ~RangeRepeatOpt() = default;

        virtual bool needsParens() const override final { return true; }
        virtual std::u8string toBSQONFormat() const override final
        {
            std::u8string repeatstr;
            if(!this->repeat->needsParens()) {
                repeatstr = this->repeat->toBSQONFormat();
            }
            else {
                repeatstr = std::u8string{'('} + this->repeat->toBSQONFormat() + std::u8string{')'};
            }

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

            return repeatstr + std::u8string(iterstr.cbegin(), iterstr.cend());
        }

        static RangeRepeatOpt* jparse(json j)
        {
            auto repeat = RegexOpt::jparse(j["repeat"]);
            auto low = j["low"].get<uint16_t>();
            auto high = (!j.contains("high") || j["high"].is_null()) ? UINT16_MAX : j["high"].get<uint16_t>();

            return new RangeRepeatOpt(low, high, repeat);
        }
    };

    class OptionalOpt : public RegexOpt
    {
    public:
        const RegexOpt* opt;

        OptionalOpt(const RegexOpt* opt) : RegexOpt(RegexOptTag::Optional), opt(opt) {;}
        virtual ~OptionalOpt() = default;

        virtual bool needsParens() const override final { return true; }
        virtual std::u8string toBSQONFormat() const override final
        {
            if (!this->opt->needsParens()) {
                return this->opt->toBSQONFormat() + std::u8string{'?'};
            }
            else {
                return std::u8string{'('} + this->opt->toBSQONFormat() + std::u8string{')'} + std::u8string{'?'};
            }
        }

        static OptionalOpt* jparse(json j)
        {
            auto opt = RegexOpt::jparse(j["opt"]);
            return new OptionalOpt(opt);
        }
    };

    class AnyOfOpt : public RegexOpt
    {
    public:
        const std::vector<const RegexOpt*> opts;

        AnyOfOpt(std::vector<const RegexOpt*> opts) : RegexOpt(RegexOptTag::AnyOf), opts(opts) {;}
        virtual ~AnyOfOpt() = default;

        virtual bool needsParens() const override final { return true; }
        virtual bool needsSequenceParens() const override final { return true; }
        virtual std::u8string toBSQONFormat() const override final
        {
            std::u8string optstr;
            for(auto ii = this->opts.cbegin(); ii != this->opts.cend(); ++ii) {
                if(ii != this->opts.cbegin()) {
                    optstr += std::u8string{' ', '|', ' '};
                }

                if(!(*ii)->needsParens()) {
                    optstr += (*ii)->toBSQONFormat();
                }
                else {
                    optstr += std::u8string{'('} + (*ii)->toBSQONFormat() + std::u8string{')'};
                }
            }
            
            return optstr;
        }

        static AnyOfOpt* jparse(json j)
        {
            std::vector<const RegexOpt*> opts;
            auto jopts = j["opts"];
            std::transform(jopts.cbegin(), jopts.cend(), std::back_inserter(opts), [](json arg) {
                return RegexOpt::jparse(arg);
            });

            return new AnyOfOpt(opts);
        }
    };

    class SequenceOpt : public RegexOpt
    {
    public:
        const std::vector<const RegexOpt*> regexs;

        SequenceOpt(std::vector<const RegexOpt*> regexs) : RegexOpt(RegexOptTag::Sequence), regexs(regexs) {;}
        virtual ~SequenceOpt() = default;

        virtual bool needsParens() const override final { return true; }
        virtual std::u8string toBSQONFormat() const override final
        {
            std::u8string regexstr;
            for(auto ii = this->regexs.cbegin(); ii != this->regexs.cend(); ++ii) {
                if(!(*ii)->needsSequenceParens()) {
                    regexstr += (*ii)->toBSQONFormat();
                }
                else {
                    regexstr += std::u8string{'('} + (*ii)->toBSQONFormat() + std::u8string{')'};
                }
            }
            
            return regexstr;
        }

        static SequenceOpt* jparse(json j)
        {
            std::vector<const RegexOpt*> regexs;
            auto jregexs = j["regexs"];
            std::transform(jregexs.cbegin(), jregexs.cend(), std::back_inserter(regexs), [](json arg) {
                return RegexOpt::jparse(arg);
            });

            return new SequenceOpt(regexs);
        }
    };

    class RegexToplevelEntry
    {
    public:
        bool isNegated;
        bool isFrontCheck;
        bool isBackCheck;

        const RegexOpt* opt;

        RegexToplevelEntry() : isNegated(false), isFrontCheck(false), isBackCheck(false), opt(nullptr) {;}
        RegexToplevelEntry(bool isNegated, bool isFrontCheck, bool isBackCheck, const RegexOpt* opt) : isNegated(isNegated), isFrontCheck(isFrontCheck), isBackCheck(isBackCheck), opt(opt) {;}
        ~RegexToplevelEntry() = default;

        RegexToplevelEntry(const RegexToplevelEntry& other) = default;
        RegexToplevelEntry(RegexToplevelEntry&& other) = default;

        RegexToplevelEntry& operator=(const RegexToplevelEntry& other) = default;
        RegexToplevelEntry& operator=(RegexToplevelEntry&& other) = default;

        std::u8string toBSQONFormat() const
        {
            std::u8string optstr;
            if(this->isNegated) {
                optstr += std::u8string{'!'};
            }

            if(this->isFrontCheck) {
                optstr += std::u8string{'^'};
            }

            if(this->isBackCheck) {
                optstr += std::u8string{'$'};
            }

            if(!this->opt->needsParens()) {
                optstr += this->opt->toBSQONFormat();
            }
            else {
                optstr += std::u8string{'('} + this->opt->toBSQONFormat() + std::u8string{')'};
            }

            return optstr;
        }

        static RegexToplevelEntry jparse(json j)
        {
            bool isNegated = j["isNegated"].get<bool>();
            bool isFrontCheck = j["isFrontCheck"].get<bool>();
            bool isBackCheck = j["isBackCheck"].get<bool>();

            auto opt = RegexOpt::jparse(j["opt"]);
            return RegexToplevelEntry(isNegated, isFrontCheck, isBackCheck, opt);
        }
    };

    class RegexTopLevelAllOf
    {
    public:
        std::vector<RegexToplevelEntry> musts;

        RegexTopLevelAllOf() : musts() {;}
        RegexTopLevelAllOf(const std::vector<RegexToplevelEntry>& musts) : musts(musts) {;}
        ~RegexTopLevelAllOf() = default;

        RegexTopLevelAllOf(const RegexTopLevelAllOf& other) = default;
        RegexTopLevelAllOf(RegexTopLevelAllOf&& other) = default;

        RegexTopLevelAllOf& operator=(const RegexTopLevelAllOf& other) = default;
        RegexTopLevelAllOf& operator=(RegexTopLevelAllOf&& other) = default;

        bool isEmpty() const
        {
            return this->musts.empty();
        }

        std::u8string toBSQONFormat() const
        {
            std::u8string muststr;
            for(auto ii = this->musts.cbegin(); ii != this->musts.cend(); ++ii) {
                if(ii != this->musts.cbegin()) {
                    muststr += std::u8string{'&'};
                }

                muststr += ii->toBSQONFormat();
            }
            
            return muststr;
        }

        static RegexTopLevelAllOf jparse(json j)
        {
            auto jmusts = j["musts"];
            std::vector<RegexToplevelEntry> musts;

            std::transform(jmusts.cbegin(), jmusts.cend(), std::back_inserter(musts), [](json arg) {
                return RegexToplevelEntry::jparse(arg);
            });

            return RegexTopLevelAllOf(musts);
        }
    };

    enum class RegexKindTag
    {
        Std,
        Path,
        Resource
    };

    enum class RegexCharInfoTag
    {
        Unicode,
        ASCII
    };

    class Regex
    {
    public:
        const RegexKindTag rtag;
        const RegexCharInfoTag ctag;
        const bool isContainsable;
        const bool isMatchable;

        const RegexToplevelEntry sre; //null if allopt is used
        const RegexTopLevelAllOf allopt; //if empty then no allopt is not used

        Regex(RegexKindTag rtag, RegexCharInfoTag ctag, bool isContainsable, bool isMatchable, const RegexToplevelEntry& sre, const RegexTopLevelAllOf& allopt): rtag(rtag), ctag(ctag), isContainsable(isContainsable), isMatchable(isMatchable), sre(sre), allopt(allopt) {;}
        ~Regex() = default;

        static Regex* jparse(json j)
        {
            auto rtag = (!j.contains("kind") || j["kind"].is_null()) ? RegexKindTag::Std : (j["kind"].get<std::string>() == "path" ? RegexKindTag::Path : (j["kind"].get<std::string>() == "resource" ? RegexKindTag::Resource : RegexKindTag::Std));
            auto ctag = (!j.contains("isASCII") || !j["isASCII"].get<bool>()) ? RegexCharInfoTag::ASCII : RegexCharInfoTag::Unicode;
            auto isContainsable = (!j.contains("isContainsable") || !j["isContainsable"].get<bool>()) ? false : j["isContainsable"].get<bool>();
            auto isMatchable = (!j.contains("isMatchable") || !j["isMatchable"].get<bool>()) ? false : j["isMatchable"].get<bool>();

            RegexToplevelEntry sre;
            if(j.contains("sre") && !j["sre"].is_null()) {
                sre = RegexToplevelEntry::jparse(j["sre"]);
            }

            RegexTopLevelAllOf allopt;
            if((j.contains("allopt") || !j["allopt"].is_null())) {
                allopt = RegexTopLevelAllOf::jparse(j["allopt"]);
            }

            return new Regex(rtag, ctag, isContainsable, isMatchable, sre, allopt);
        }

        std::u8string toBSQONFormat() const
        {
            std::u8string fstr;
            if(this->allopt.isEmpty()) {
                fstr = this->sre.toBSQONFormat();
            }
            else {
                fstr = this->allopt.toBSQONFormat();
            }

            std::u8string fchar = u8"";
            if(this->rtag == RegexKindTag::Path) {
                fchar = u8"p";
            }
            else if(this->rtag == RegexKindTag::Resource) {
                fchar = u8"r";
            }
            else {
                if(this->ctag == RegexCharInfoTag::ASCII) {
                    fchar = u8"a";
                }
            }

            return std::u8string{'/'} + fstr + std::u8string{'/'} + fchar;
        }
    };
}
