#pragma once

#include "common.h"

namespace BREX
{
    struct SingleCharRange
    {
        RegexChar low;
        RegexChar high;
    };

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
        Sequence,
        Negate,
        AllOf
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
            auto bbytes = escapeRegexLiteralCharBuffer(this->codes);
            if(this->isunicode) {
                return std::u8string{'"'} + std::u8string(bbytes.cbegin(), bbytes.cend()) + std::u8string{'"'};
            }
            else {
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

        CharRangeOpt(bool compliment, std::vector<SingleCharRange> ranges) : RegexOpt(RegexOptTag::CharRange), compliment(compliment), ranges(ranges) {;}
        virtual ~CharRangeOpt() = default;

        virtual std::u8string toBSQONFormat() const override final
        {
            std::u8string rngs = {'['};
            if(this->compliment) {
                rngs.push_back('^');
            }

            for(auto ii = this->ranges.cbegin(); ii != this->ranges.cend(); ++ii) {
                auto cr = *ii;

                auto lowbytes = escapeSingleRegexChar(cr.low);
                rngs.append(lowbytes.cbegin(), lowbytes.cend());

                if(cr.low != cr.high) {
                    rngs.push_back('-');
                    
                    auto highbytes = escapeSingleRegexChar(cr.high);
                    rngs.append(highbytes.cbegin(), highbytes.cend());
                }
            }
            rngs.push_back(']');

            return std::move(rngs);
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

            return new CharRangeOpt(compliment, ranges);
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
        virtual ~StarRepeatOpt() { delete this->repeat; }

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
        virtual ~PlusRepeatOpt() { delete this->repeat; }

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
        virtual ~RangeRepeatOpt() { delete this->repeat; }

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
            auto high = j["high"].is_null() ? UINT16_MAX : j["high"].get<uint16_t>();

            return new RangeRepeatOpt(low, high, repeat);
        }
    };

    class OptionalOpt : public RegexOpt
    {
    public:
        const RegexOpt* opt;

        OptionalOpt(const RegexOpt* opt) : RegexOpt(RegexOptTag::Optional), opt(opt) {;}
        virtual ~OptionalOpt() { delete this->opt; }

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

        virtual ~AnyOfOpt()
        {
            for(size_t i = 0; i < this->opts.size(); ++i) {
                delete this->opts[i];
            }
        }

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

        virtual ~SequenceOpt()
        {
            for(size_t i = 0; i < this->regexs.size(); ++i) {
                delete this->regexs[i];
            }
        }

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

    class NegateOpt : public RegexOpt
    {
    public:
        const RegexOpt* opt;

        NegateOpt(const RegexOpt* opt) : RegexOpt(RegexOptTag::Negate), opt(opt) {;}
        virtual ~NegateOpt() { delete this->opt; }

        virtual bool needsParens() const override final { return true; }
        virtual std::u8string toBSQONFormat() const override final
        {
            if(!this->opt->needsParens()) {
                return std::u8string{'!'} + this->opt->toBSQONFormat();
            }
            else {
                return std::u8string{'!'} + std::u8string{'('} + this->opt->toBSQONFormat() + std::u8string{')'};
            }
        }

        static NegateOpt* jparse(json j)
        {
            auto opt = RegexOpt::jparse(j["opt"]);
            return new NegateOpt(opt);
        }
    };

    class AllOfOpt : public RegexOpt
    {
    public:
        const std::vector<const RegexOpt*> musts;

        AllOfOpt(std::vector<const RegexOpt*> AllOfOpt) : RegexOpt(RegexOptTag::Negate), musts(musts) {;}

        virtual ~AllOfOpt()
        {
            for(size_t i = 0; i < this->musts.size(); ++i) {
                delete this->musts[i];
            }
        }

        virtual bool needsParens() const override final { return true; }
        virtual bool needsSequenceParens() const override final { return true; }
        virtual std::u8string toBSQONFormat() const override final
        {
            std::u8string muststr;
            for(auto ii = this->musts.cbegin(); ii != this->musts.cend(); ++ii) {
                if(ii != this->musts.cbegin()) {
                    muststr += std::u8string{'&'};
                }

                if(!(*ii)->needsParens()) {
                    muststr += (*ii)->toBSQONFormat();
                }
                else {
                    muststr += std::u8string{'('} + (*ii)->toBSQONFormat() + std::u8string{')'};
                }
            }
            
            return muststr;
        }

        static AllOfOpt* jparse(json j)
        {
            std::vector<const RegexOpt*> musts;
            auto jmusts = j["musts"];
            std::transform(jmusts.cbegin(), jmusts.cend(), std::back_inserter(musts), [](json arg) {
                return RegexOpt::jparse(arg);
            });

            return new AllOfOpt(musts);
        }
    };

    enum class RegexMatchTag
    {
        ValidateOnly,
        MatchAndValidate
    };

    enum class RegexCharInfoTag
    {
        Unicode,
        ASCII
    };

    class Regex
    {
    public:
        const RegexMatchTag mtag;
        const RegexCharInfoTag ctag;

        Regex(RegexMatchTag mtag, RegexCharInfoTag ctag): mtag(mtag), ctag(ctag) {;}
        virtual ~Regex() = default;

        static Regex* jparse(json j);

        virtual std::u8string toBSQONFormat() const = 0;
    };

    class ValidatorRegex : public Regex
    {
    public:
        const RegexOpt* re; //of the form R+, !R+, or /\(R+|!R+)

        ValidatorRegex(const RegexOpt* re, RegexCharInfoTag rtag) : Regex(RegexMatchTag::ValidateOnly, rtag), re(re) {;}
        virtual ~ValidatorRegex() { delete this->re; };
    };

    class UnicodeValidatorRegex : public ValidatorRegex
    {
    public:
        UnicodeValidatorRegex(const RegexOpt* re) : ValidatorRegex(re, RegexCharInfoTag::Unicode) {;}
        virtual ~UnicodeValidatorRegex() = default;

        virtual std::u8string toBSQONFormat() const override final
        {
            return std::u8string{'/'} + this->re->toBSQONFormat() + std::u8string{'/'};
        }

        static UnicodeValidatorRegex* jparse(json j)
        {
            auto re = RegexOpt::jparse(j["re"]);
            return new UnicodeValidatorRegex(re);
        }
    };

    class ASCIIValidatorRegex : public ValidatorRegex
    {
    public:
        const bool isPathRegex;
        const bool isResourceRegex;

        ASCIIValidatorRegex(const RegexOpt* re, bool isPathRegex, bool isResourceRegex) : ValidatorRegex(re, RegexCharInfoTag::ASCII), isPathRegex(isPathRegex), isResourceRegex(isResourceRegex) {;}
        virtual ~ASCIIValidatorRegex() = default;

        virtual std::u8string toBSQONFormat() const override final
        {
            char8_t fchar;
            if(this->isPathRegex) {
                fchar = 'p';
            }
            else if(this->isResourceRegex) {
                fchar = 'r';
            }
            else {
                fchar = 'a';
            }

            return std::u8string{'/'} + this->re->toBSQONFormat() + std::u8string{'/', fchar};
        }

        static ASCIIValidatorRegex* jparse(json j)
        {
            auto re = RegexOpt::jparse(j["re"]);
            auto isPathRegex = j["isPathRegex"].is_null() ? false : j["isPathRegex"].get<bool>();
            auto isResourceRegex = j["isResourceRegex"].is_null() ? false : j["isResourceRegex"].get<bool>();

            return new ASCIIValidatorRegex(re, isPathRegex, isResourceRegex);
        }
    };

    class MatcherRegex : public Regex
    {
    public:
        const RegexOpt* sanchor; //may be nullptr or R
        const RegexOpt* re; //of the form R+
        const RegexOpt* eanchor; //may be nullptr or R

        MatcherRegex(const RegexOpt* sanchor, const RegexOpt* re, const RegexOpt* eanchor, RegexCharInfoTag rtag) : Regex(RegexMatchTag::MatchAndValidate, rtag), sanchor(sanchor), re(re), eanchor(eanchor) {;}
        
        virtual ~MatcherRegex() 
        {
            if(this->sanchor != nullptr) {
                delete this->sanchor;
            } 

            delete this->re; 

            if(this->eanchor != nullptr) {
                delete this->eanchor;
            }
        };
    };

    class UnicodeMatcherRegex : public MatcherRegex
    {
    public:
        UnicodeMatcherRegex(const RegexOpt* sanchor, const RegexOpt* re, const RegexOpt* eanchor) : MatcherRegex(sanchor, re, eanchor, RegexCharInfoTag::Unicode) {;}
        virtual ~UnicodeMatcherRegex() = default;

        virtual std::u8string toBSQONFormat() const override final
        {
            std::u8string fstr;
            if(this->sanchor != nullptr) {
                fstr = this->sanchor->toBSQONFormat() + std::u8string{'$'};
            }

            std::u8string estr;
            if(this->eanchor != nullptr) {
                estr = std::u8string{'$'} + this->eanchor->toBSQONFormat();
            }

            return std::u8string{'/'} + fstr + this->re->toBSQONFormat() + estr + std::u8string{'/', 'm'};
        }

        static UnicodeMatcherRegex* jparse(json j)
        {
            auto sanchor = j["sanchor"].is_null() ? nullptr : RegexOpt::jparse(j["sanchor"]);
            auto re = RegexOpt::jparse(j["re"]);
            auto eanchor = j["eanchor"].is_null() ? nullptr : RegexOpt::jparse(j["eanchor"]);

            return new UnicodeMatcherRegex(sanchor, re, eanchor);
        }
    };

    class ASCIIMatcherRegex : public MatcherRegex
    {
    public:
        ASCIIMatcherRegex(const RegexOpt* sanchor, const RegexOpt* re, const RegexOpt* eanchor) : MatcherRegex(sanchor, re, eanchor, RegexCharInfoTag::ASCII) {;}
        virtual ~ASCIIMatcherRegex() = default;

        virtual std::u8string toBSQONFormat() const override final
        {
            return std::u8string{'/'} + this->re->toBSQONFormat() + std::u8string{'/', 'a', 'm'};
        }

        static ASCIIMatcherRegex* jparse(json j)
        {
            auto sanchor = j["sanchor"].is_null() ? nullptr : RegexOpt::jparse(j["sanchor"]);
            auto re = RegexOpt::jparse(j["re"]);
            auto eanchor = j["eanchor"].is_null() ? nullptr : RegexOpt::jparse(j["eanchor"]);

            return new ASCIIMatcherRegex(sanchor, re, eanchor);
        }
    };
}
