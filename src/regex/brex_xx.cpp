#include "brex.h"

namespace BREX
{
    StateID BSQLiteralRe::compile(StateID follows, std::vector<NFAOpt*>& states) const
    {
        for(int64_t i = this->litstr.size() - 1; i >= 0; --i) {
            auto thisstate = (StateID)states.size();
            states.push_back(new NFAOptCharCode(thisstate, this->litstr[i], follows));

            follows = thisstate;
        }

        return follows;
    }

    StateID BSQCharRangeRe::compile(StateID follows, std::vector<NFAOpt*>& states) const
    {
        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptRange(thisstate, this->compliment, this->ranges, follows));

        return thisstate;
    }

    StateID BSQCharClassDotRe::compile(StateID follows, std::vector<NFAOpt*>& states) const
    {
        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptDot(thisstate, follows));

        return thisstate;
    }

    StateID BSQStarRepeatRe::compile(StateID follows, std::vector<NFAOpt*>& states) const
    {
        auto thisstate = (StateID)states.size();
        states.push_back(nullptr); //placeholder

        auto optfollows = this->opt->compile(thisstate, states);
        states[thisstate] = new NFAOptStar(thisstate, optfollows, follows);

        auto altstate = (StateID)states.size();
        states.push_back(new NFAOptAlternate(altstate, { optfollows, follows }));

        return altstate;
    }

    StateID BSQPlusRepeatRe::compile(StateID follows, std::vector<NFAOpt*>& states) const
    {
        auto thisstate = (StateID)states.size();
        states.push_back(nullptr); //placeholder

        auto optfollows = this->opt->compile(thisstate, states);
        states[thisstate] = new NFAOptStar(thisstate, optfollows, follows);

        return thisstate;
    }

    StateID BSQRangeRepeatRe::compile(StateID follows, std::vector<NFAOpt*>& states) const
    {
        auto followfinal = follows;
        for(int64_t i = this->high; i > this->low; --i) {
            auto followopt = this->opt->compile(follows, states);

            auto thisstate = (StateID)states.size();
            states.push_back(new NFAOptAlternate(thisstate, { followopt, followfinal }));

            follows = thisstate;
        }

        for(int64_t i = this->low; i > 0; --i) {
            follows = this->opt->compile(follows, states);
        }

        return follows;
    }

    StateID BSQOptionalRe::compile(StateID follows, std::vector<NFAOpt*>& states) const
    {
        auto followopt = this->opt->compile(follows, states);

        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptAlternate(thisstate, { followopt, follows }));

        return thisstate;
    }

    StateID BSQAlternationRe::compile(StateID follows, std::vector<NFAOpt*>& states) const
    {
        std::vector<StateID> followopts;
        for(size_t i = 0; i < this->opts.size(); ++i) {
            followopts.push_back(this->opts[i]->compile(follows, states));
        }

        auto thisstate = (StateID)states.size();
        states.push_back(new NFAOptAlternate(thisstate, followopts));

        return thisstate;
    }

    BSQSequenceRe* BSQSequenceRe::parse(json j)
    {
        std::vector<const BSQRegexOpt*> elems;
        auto jopts = j[1]["elems"];
        std::transform(jopts.cbegin(), jopts.cend(), std::back_inserter(elems), [](json arg) {
            return BSQRegexOpt::parse(arg);
        });

        return new BSQSequenceRe(elems);
    }

    StateID BSQSequenceRe::compile(StateID follows, std::vector<NFAOpt*>& states) const
    {
        for(int64_t i = this->opts.size() - 1; i >= 0; --i) {
            follows = this->opts[i]->compile(follows, states);
        }

        return follows;
    }

    BSQRegex* BSQRegex::jparse(json j)
    {
        auto bsqre = BSQRegexOpt::parse(j["re"]);

        std::vector<NFAOpt*> nfastates = { new NFAOptAccept(0) };
        auto nfastart = bsqre->compile(0, nfastates);

        auto nfare = new NFA(nfastart, 0, nfastates);
        return new BSQRegex(bsqre, nfare);
    }
}
