#include "brex_compiler.h"

namespace brex
{
    const RegexOpt* RegexResolver::resolveNamedRegexOpt(const NamedRegexOpt* opt)
    {
        if(this->nameResolverFn == nullptr) {
            this->errors.push_back(RegexCompileError(u8"Named regexes are not supported in this context"));
            return opt;
        }

        auto realname = this->nameResolverFn(opt->rname, this->resolverState);

        if(std::find(this->pending_resolves.cbegin(), this->pending_resolves.cend(), realname) != this->pending_resolves.cend()) {
            this->errors.push_back(RegexCompileError(u8"Named regex " + std::u8string(opt->rname.cbegin(), opt->rname.cend()) + u8" is recursive resolution"));
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
            default:
            {
                assert(false);
                return nullptr;
            }
            }
        }
    }

    StateID RegexCompiler::compileLiteralOpt(StateID follows, std::vector<NFAOpt*>& states, const LiteralOpt* opt)
    {
        for(int64_t i = opt->codes.size() - 1; i >= 0; --i) {
            auto thisstate = (StateID)states.size();
            states.push_back(new NFAOptCharCode(thisstate, opt->codes[i], follows));

            follows = thisstate;
        }

        return follows;
    }

    StateID RegexCompiler::compileCharRangeOpt(StateID follows, std::vector<NFAOpt*>& states, const CharRangeOpt* opt)
    {
        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptRange(thisstate, opt->compliment, opt->ranges, follows));

        return thisstate;
    }

    StateID RegexCompiler::compileCharClassDotOpt(StateID follows, std::vector<NFAOpt*>& states, const CharClassDotOpt* opt)
    {
        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptDot(thisstate, follows));

        return thisstate;
    }

    StateID RegexCompiler::compileStarRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const StarRepeatOpt* opt)
    {
        auto thisstate = (StateID)states.size();
        states.push_back(nullptr); //placeholder

        auto optfollows = RegexCompiler::compileOpt(thisstate, states, opt->repeat);
        states[thisstate] = new NFAOptStar(thisstate, optfollows, follows);

        return thisstate;
    }

    StateID RegexCompiler::compilePlusRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const PlusRepeatOpt* opt)
    {
        auto thisstate = (StateID)states.size();
        states.push_back(nullptr); //placeholder

        auto optfollows = RegexCompiler::compileOpt(thisstate, states, opt->repeat);
        states[thisstate] = new NFAOptStar(thisstate, optfollows, follows);

        return optfollows;
    }

    StateID RegexCompiler::compileRangeRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const RangeRepeatOpt* opt)
    {
        auto thisstate = (StateID)states.size();
        states.push_back(nullptr); //placeholder

        auto optfollows = RegexCompiler::compileOpt(thisstate, states, opt->repeat);
        states[thisstate] = new NFAOptRangeK(thisstate, opt->low, opt->high, optfollows, follows);

        return thisstate;
    }

    StateID RegexCompiler::compileOptionalOpt(StateID follows, std::vector<NFAOpt*>& states, const OptionalOpt* opt)
    {
        auto followopt = RegexCompiler::compileOpt(follows, states, opt->opt);

        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptAnyOf(thisstate, { followopt, follows }));

        return thisstate;
    }

    StateID RegexCompiler::compileAnyOfOpt(StateID follows, std::vector<NFAOpt*>& states, const AnyOfOpt* opt)
    {
        std::vector<StateID> followopts;
        for(size_t i = 0; i < opt->opts.size(); ++i) {
            followopts.push_back(RegexCompiler::compileOpt(follows, states, opt->opts[i]));
        }

        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptAnyOf(thisstate, followopts));

        return thisstate;
    }

    StateID RegexCompiler::compileSequenceOpt(StateID follows, std::vector<NFAOpt*>& states, const SequenceOpt* opt)
    {
        for(int64_t i = opt->regexs.size() - 1; i >= 0; --i) {
            follows = RegexCompiler::compileOpt(follows, states, opt->regexs[i]);
        }

        return follows;
    }

    StateID RegexCompiler::compileOpt(StateID follows, std::vector<NFAOpt*>& states, const RegexOpt* opt)
    {
        switch(opt->tag)
        {
        case RegexOptTag::Literal: {
            return RegexCompiler::compileLiteralOpt(follows, states, static_cast<const LiteralOpt*>(opt));
        }
        case RegexOptTag::CharRange: {
            return RegexCompiler::compileCharRangeOpt(follows, states, static_cast<const CharRangeOpt*>(opt));
        }
        case RegexOptTag::CharClassDot: {
            return RegexCompiler::compileCharClassDotOpt(follows, states, static_cast<const CharClassDotOpt*>(opt));
        }
        case RegexOptTag::StarRepeat: {
            return RegexCompiler::compileStarRepeatOpt(follows, states, static_cast<const StarRepeatOpt*>(opt));
        }
        case RegexOptTag::PlusRepeat: {
            return RegexCompiler::compilePlusRepeatOpt(follows, states, static_cast<const PlusRepeatOpt*>(opt));
        }
        case RegexOptTag::RangeRepeat: {
            return RegexCompiler::compileRangeRepeatOpt(follows, states, static_cast<const RangeRepeatOpt*>(opt));
        }
        case RegexOptTag::Optional: {
            return RegexCompiler::compileOptionalOpt(follows, states, static_cast<const OptionalOpt*>(opt));
        }
        case RegexOptTag::AnyOf: {
            return RegexCompiler::compileAnyOfOpt(follows, states, static_cast<const AnyOfOpt*>(opt));
        }
        case RegexOptTag::Sequence: {
            return RegexCompiler::compileSequenceOpt(follows, states, static_cast<const SequenceOpt*>(opt));
        }
        default: {
            BREX_ABORT("Invalid regex opt tag");
            return 0;
        }
        }
    }

    StateID RegexCompiler::reverseCompileLiteralOpt(StateID follows, std::vector<NFAOpt*>& states, const LiteralOpt* opt)
    {
        for(size_t i = 0; i < opt->codes.size(); ++i) {
            auto thisstate = (StateID)states.size();
            states.push_back(new NFAOptCharCode(thisstate, opt->codes[i], follows));

            follows = thisstate;
        }

        return follows;
    }

    StateID RegexCompiler::reverseCompileCharRangeOpt(StateID follows, std::vector<NFAOpt*>& states, const CharRangeOpt* opt)
    {
        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptRange(thisstate, opt->compliment, opt->ranges, follows));

        return thisstate;
    }

    StateID RegexCompiler::reverseCompileCharClassDotOpt(StateID follows, std::vector<NFAOpt*>& states, const CharClassDotOpt* opt)
    {
        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptDot(thisstate, follows));

        return thisstate;
    }

    StateID RegexCompiler::reverseCompileStarRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const StarRepeatOpt* opt)
    {
        auto thisstate = (StateID)states.size();
        states.push_back(nullptr); //placeholder

        auto optfollows = RegexCompiler::reverseCompileOpt(thisstate, states, opt->repeat);
        states[thisstate] = new NFAOptStar(thisstate, optfollows, follows);

        return thisstate;
    }

    StateID RegexCompiler::reverseCompilePlusRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const PlusRepeatOpt* opt)
    {
        auto thisstate = (StateID)states.size();
        states.push_back(nullptr); //placeholder

        auto optfollows = RegexCompiler::reverseCompileOpt(thisstate, states, opt->repeat);
        states[thisstate] = new NFAOptStar(thisstate, optfollows, follows);

        return optfollows;
    }

    StateID RegexCompiler::reverseCompileRangeRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const RangeRepeatOpt* opt)
    {
        auto thisstate = (StateID)states.size();
        states.push_back(nullptr); //placeholder

        auto optfollows = RegexCompiler::reverseCompileOpt(thisstate, states, opt->repeat);
        states[thisstate] = new NFAOptRangeK(thisstate, opt->low, opt->high, optfollows, follows);

        return thisstate;
    }

    StateID RegexCompiler::reverseCompileOptionalOpt(StateID follows, std::vector<NFAOpt*>& states, const OptionalOpt* opt)
    {
        auto followopt = RegexCompiler::reverseCompileOpt(follows, states, opt->opt);

        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptAnyOf(thisstate, { followopt, follows }));

        return thisstate;
    }

    StateID RegexCompiler::reverseCompileAnyOfOpt(StateID follows, std::vector<NFAOpt*>& states, const AnyOfOpt* opt)
    {
        std::vector<StateID> followopts;
        for(size_t i = 0; i < opt->opts.size(); ++i) {
            followopts.push_back(RegexCompiler::reverseCompileOpt(follows, states, opt->opts[i]));
        }

        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptAnyOf(thisstate, followopts));

        return thisstate;
    }

    StateID RegexCompiler::reverseCompileSequenceOpt(StateID follows, std::vector<NFAOpt*>& states, const SequenceOpt* opt)
    {
        for(size_t i = 0; i < opt->regexs.size(); ++i) {
            follows = RegexCompiler::reverseCompileOpt(follows, states, opt->regexs[i]);
        }

        return follows;
    }

    StateID RegexCompiler::reverseCompileOpt(StateID follows, std::vector<NFAOpt*>& states, const RegexOpt* opt)
    {
        switch(opt->tag)
        {
        case RegexOptTag::Literal: {
            return RegexCompiler::reverseCompileLiteralOpt(follows, states, static_cast<const LiteralOpt*>(opt));
        }
        case RegexOptTag::CharRange: {
            return RegexCompiler::reverseCompileCharRangeOpt(follows, states, static_cast<const CharRangeOpt*>(opt));
        }
        case RegexOptTag::CharClassDot: {
            return RegexCompiler::reverseCompileCharClassDotOpt(follows, states, static_cast<const CharClassDotOpt*>(opt));
        }
        case RegexOptTag::StarRepeat: {
            return RegexCompiler::reverseCompileStarRepeatOpt(follows, states, static_cast<const StarRepeatOpt*>(opt));
        }
        case RegexOptTag::PlusRepeat: {
            return RegexCompiler::reverseCompilePlusRepeatOpt(follows, states, static_cast<const PlusRepeatOpt*>(opt));
        }
        case RegexOptTag::RangeRepeat: {
            return RegexCompiler::reverseCompileRangeRepeatOpt(follows, states, static_cast<const RangeRepeatOpt*>(opt));
        }
        case RegexOptTag::Optional: {
            return RegexCompiler::reverseCompileOptionalOpt(follows, states, static_cast<const OptionalOpt*>(opt));
        }
        case RegexOptTag::AnyOf: {
            return RegexCompiler::reverseCompileAnyOfOpt(follows, states, static_cast<const AnyOfOpt*>(opt));
        }
        case RegexOptTag::Sequence: {
            return RegexCompiler::reverseCompileSequenceOpt(follows, states, static_cast<const SequenceOpt*>(opt));
        }
        default: {
            BREX_ABORT("Invalid regex opt tag");
            return 0;
        }
        }
    }
}
