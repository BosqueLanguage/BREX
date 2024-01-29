#include "brex_compiler.h"

namespace BREX
{
    const RegexOpt* RegexResolver::resolveNamedRegexOpt(const NamedRegexOpt* opt)
    {
        auto realname = this->nameResolverFn(opt->rname, this->resolverState);

        if(std::find(this->pending_resolves.cbegin(), this->pending_resolves.cend(), realname) != this->pending_resolves.cend()) {
            this->errors.push_back(RegexCompileError(u8"Env regex " + std::u8string(opt->rname.cbegin(), opt->rname.cend()) + u8" is recursive resolution"));
            return opt;
        }

        auto ii = this->namedRegexes.find(realname);
        if(ii == this->namedRegexes.end()) {
            this->errors.push_back(RegexCompileError(u8"Named regex " + std::u8string(opt->rname.cbegin(), opt->rname.cend()) + u8" is not defined (resolved to " + std::u8string(realname.cbegin(), realname.cend()) + u8")"));
            return opt;
        }

        this->pending_resolves.push_back(realname);
        auto res = this->resolve(ii->second);
        this->pending_resolves.pop_back();

        return res;
    }

    const RegexOpt* RegexResolver::resolveEnvRegexOpt(const EnvRegexOpt* opt)
    {
        if(this->envRegexes == nullptr) {
            this->errors.push_back(RegexCompileError(u8"Env regexes are not supported in this context"));
            return opt;
        }

        if(std::find(this->pending_resolves.cbegin(), this->pending_resolves.cend(), "env[" + opt->ename + "]") != this->pending_resolves.cend()) {
            this->errors.push_back(RegexCompileError(u8"Env regex " + std::u8string(opt->ename.cbegin(), opt->ename.cend()) + u8" is recursive resolution"));
            return opt;
        }

        auto ii = this->envRegexes->find(opt->ename);
        if(ii == this->envRegexes->end()) {
            this->errors.push_back(RegexCompileError(u8"Env regex " + std::u8string(opt->ename.cbegin(), opt->ename.cend()) + u8" is not defined"));
            return opt;
        }

        this->pending_resolves.push_back("env[" + opt->ename + "]");
        auto res = this->resolve(ii->second);
        this->pending_resolves.pop_back();

        return res;
    }
    
    const RegexOpt* RegexResolver::resolveAnyOfOpt(const AnyOfOpt* anyofopt)
    {
        std::vector<const RegexOpt*> opts;
        for(auto ii = anyofopt->opts.cbegin(); ii != anyofopt->opts.cend(); ++ii)
        {
            auto rr = resolve(*ii);
            if(rr->tag != RegexOptTag::AnyOf) {
                opts.push_back(rr);
            }
            else {
                auto anyofopt = static_cast<const AnyOfOpt*>(rr);
                std::copy(anyofopt->opts.cbegin(), anyofopt->opts.cend(), std::back_inserter(opts));
            }
        }

        return new AnyOfOpt(opts);
    }

    const RegexOpt* RegexResolver::resolve(const RegexOpt* opt)
    {
        if(opt->tag == RegexOptTag::NamedRegex) {
            return this->resolveNamedRegexOpt(static_cast<const NamedRegexOpt*>(opt));
        }
        else if(opt->tag == RegexOptTag::EnvRegex)
        {
            return this->resolveEnvRegexOpt(static_cast<const EnvRegexOpt*>(opt));
        }
        else if(opt->tag == RegexOptTag::AnyOf) {
            return this->resolveAnyOfOpt(static_cast<const AnyOfOpt*>(opt));
        }
        else
        {
            switch(opt->tag)
            {
            case RegexOptTag::Literal:
            {
                return opt;
            }
            case RegexOptTag::CharRange:
            {
                return opt;
            }
            case RegexOptTag::CharClassDot:
            {
                return opt;
            }
            case RegexOptTag::StarRepeat:
            {
                auto staropt = static_cast<const StarRepeatOpt*>(opt);
                return new StarRepeatOpt(resolve(staropt->repeat));
            }
            case RegexOptTag::PlusRepeat:
            {
                auto plusopt = static_cast<const PlusRepeatOpt*>(opt);
                return new PlusRepeatOpt(resolve(plusopt->repeat));
            }
            case RegexOptTag::RangeRepeat:
            {
                auto rangeopt = static_cast<const RangeRepeatOpt*>(opt);
                return new RangeRepeatOpt(rangeopt->low, rangeopt->high, resolve(rangeopt->repeat));
            }
            case RegexOptTag::Optional:
            {
                auto optionalopt = static_cast<const OptionalOpt*>(opt);
                return new OptionalOpt(resolve(optionalopt->opt));
            }
            case RegexOptTag::Sequence:
            {
                auto seqopt = static_cast<const SequenceOpt*>(opt);
                std::vector<const RegexOpt*> seq;
                for(auto ii = seqopt->regexs.cbegin(); ii != seqopt->regexs.cend(); ++ii)
                {
                    seq.push_back(resolve(*ii));
                }

                return new SequenceOpt(seq);
            }
            case RegexOptTag::Negate:
            {
                auto negate = static_cast<const NegateOpt*>(opt);
                return new NegateOpt(resolve(negate->opt));
            }
            case RegexOptTag::AllOf:
            {
                auto allof = static_cast<const AllOfOpt*>(opt);
                std::vector<const RegexOpt*> opts;
                for(auto ii = allof->musts.cbegin(); ii != allof->musts.cend(); ++ii)
                {
                    opts.push_back(resolve(*ii));
                }

                return new AllOfOpt(opts);
            }
            default:
            {
                assert(false);
                return nullptr;
            }
            }
        }
    }
}
