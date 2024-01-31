#include "brex_engine.h"

namespace BREX
{
    bool NFAMachine::inAccepted() const
    {
        return this->states.simplestates.find(this->acceptstate) != this->states.simplestates.cend();
    }

    bool NFAMachine::allRejected() const
    {
        return this->states.simplestates.empty() && this->states.singlestates.empty() && this->states.fullstates.empty();
    }

    void NFAMachine::advanceCharForSimpleStates(RegexChar c, NFAState& nstates) const
    {
        xxxx;

        for(auto iter = this->states.simplestates.cbegin(); iter != this->states.simplestates.cend(); ++iter) {
            const NFAOpt* opt = this->nfaopts[iter->cstate];
            const NFAOptTag tag = opt->tag;

            switch(tag) {
                case NFAOptTag::CharCode: {
                    const NFAOptCharCode* cc = static_cast<const NFAOptCharCode*>(opt);
                    if(cc->c == c) {
                        nstates.simplestates.insert(iter->toNextState(cc->follow));
                    }
                    break;
                }
                case NFAOptTag::Range: {
                    const NFAOptRange* range = static_cast<const NFAOptRange*>(opt);
                    auto inrng = std::find_if(range->ranges.cbegin(), range->ranges.cend(), [c](const SingleCharRange& rr) {
                        return (rr.low <= c && c <= rr.high);
                    }) != range->ranges.cend();

                    bool doinsert = !range->compliment == inrng; //either both true or both false
                    if(doinsert) {
                        nstates.simplestates.insert(iter->toNextState(range->follow));
                    }
                }
                case NFAOptTag::Dot: {
                    const NFAOptDot* dot = static_cast<const NFAOptDot*>(opt);
                    nstates.simplestates.insert(iter->toNextState(dot->follow));
                    break;
                }
                default: {
                    //No char transitions for these so just drop token
                    break;
                }
            }
        }
    }

    void NFAMachine::advanceCharForSingleStates(RegexChar c, NFAState& nstates) const
    {
        xxxx;
    }
        
    void NFAMachine::advanceCharForFullStates(RegexChar c, NFAState& nstates) const
    {
        xxxx;
    }

    bool NFAMachine::advanceEpsilonForSimpleStates(const NFAState& ostates, NFAState& nstates) const
    {
        bool advanced = false;

        for(auto iter = this->states.simplestates.cbegin(); iter != this->states.simplestates.cend(); ++iter) {
            const NFAOpt* opt = this->nfaopts[iter->cstate];
            const NFAOptTag tag = opt->tag;

            switch(tag) {
                case NFAOptTag::AnyOf: {
                    const NFAOptAnyOf* anyof = static_cast<const NFAOptAnyOf*>(opt);
                    std::transform(anyof->follows.cbegin(), anyof->follows.cend(), std::inserter(nstates.simplestates, nstates.simplestates.begin()), [iter](const StateID& follow) {
                        return iter->toNextState(follow);
                    });

                    advanced = true;
                    break;
                }
                case NFAOptTag::Star: {
                    const NFAOptStar* star = static_cast<const NFAOptStar*>(opt);
                    nstates.simplestates.insert(iter->toNextState(star->matchfollow));
                    nstates.simplestates.insert(iter->toNextState(star->skipfollow));

                    advanced = true;
                    break;
                }
                case NFAOptTag::MinK: {
                    const NFAOptMinK* mink = static_cast<const NFAOptMinK*>(opt);
                    nstates.singlestates.insert(NFASingleStateToken::toNextStateWithInitialize(mink->infollow, mink->stateid));

                    advanced = true;
                    break;
                }
                case NFAOptTag::MaxK: {
                    const NFAOptMaxK* maxk = static_cast<const NFAOptMaxK*>(opt);
                    nstates.singlestates.insert(NFASingleStateToken::toNextStateWithInitialize(maxk->infollow, maxk->stateid));

                    advanced = true;
                    break;
                }
                default: {
                    //No epsilon transitions for these so just keep token and no additonal states so no advanced = true
                    nstates.simplestates.insert(*iter);
                    break;
                }
            }
        }

        return advanced;
    }

    bool NFAMachine::advanceEpsilonForSingleStates(const NFAState& ostates, NFAState& nstates) const
    {
        xxxx;
    }

    bool NFAMachine::advanceEpsilonForFullStates(const NFAState& ostates, NFAState& nstates) const
    {
        xxxx;
    }
}
