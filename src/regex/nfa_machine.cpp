#include "nfa_machine.h"

namespace brex
{
    uint16_t saturateNFATokenIncrement(uint16_t count)
    {
        return count == UINT16_MAX ? count : count + 1;
    }

    bool NFAMachine::inAccepted(const NFAState& ostates) const
    {
        return ostates.simplestates.find(this->acceptstate) != ostates.simplestates.cend();
    }

    bool NFAMachine::allRejected(const NFAState& ostates) const
    {
        return ostates.simplestates.empty() && ostates.singlestates.empty() && ostates.fullstates.empty();
    }

    void NFAMachine::advanceCharForSimpleStates(RegexChar c, const NFAState& ostates, NFAState& nstates) const
    {
        for(auto iter = ostates.simplestates.cbegin(); iter != ostates.simplestates.cend(); ++iter) {
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
                    break;
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

    void NFAMachine::advanceCharForSingleStates(RegexChar c, const NFAState& ostates, NFAState& nstates) const
    {
        for(auto iter = ostates.singlestates.cbegin(); iter != ostates.singlestates.cend(); ++iter) {
            const NFAOpt* opt = this->nfaopts[iter->cstate];
            const NFAOptTag tag = opt->tag;

            switch(tag) {
                case NFAOptTag::CharCode: {
                    const NFAOptCharCode* cc = static_cast<const NFAOptCharCode*>(opt);
                    if(cc->c == c) {
                        nstates.singlestates.insert(iter->toNextState(cc->follow));
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
                        nstates.singlestates.insert(iter->toNextState(range->follow));
                    }
                    break;
                }
                case NFAOptTag::Dot: {
                    const NFAOptDot* dot = static_cast<const NFAOptDot*>(opt);
                    nstates.singlestates.insert(iter->toNextState(dot->follow));
                    break;
                }
                default: {
                    //No char transitions for these so just drop token
                    break;
                }
            }
        }
    }
        
    void NFAMachine::advanceCharForFullStates(RegexChar c, const NFAState& ostates, NFAState& nstates) const
    {
        for(auto iter = ostates.fullstates.cbegin(); iter != ostates.fullstates.cend(); ++iter) {
            const NFAOpt* opt = this->nfaopts[iter->cstate];
            const NFAOptTag tag = opt->tag;

            switch(tag) {
                case NFAOptTag::CharCode: {
                    const NFAOptCharCode* cc = static_cast<const NFAOptCharCode*>(opt);
                    if(cc->c == c) {
                        nstates.fullstates.insert(iter->toNextState(cc->follow));
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
                        nstates.fullstates.insert(iter->toNextState(range->follow));
                    }
                    break;
                }
                case NFAOptTag::Dot: {
                    const NFAOptDot* dot = static_cast<const NFAOptDot*>(opt);
                    nstates.fullstates.insert(iter->toNextState(dot->follow));
                    break;
                }
                default: {
                    //No char transitions for these so just drop token
                    break;
                }
            }
        }
    }

    bool NFAMachine::advanceEpsilonForSimpleStates(const NFAState& ostates, NFAState& nstates) const
    {
        bool advanced = false;

        for(auto iter = ostates.simplestates.cbegin(); iter != ostates.simplestates.cend(); ++iter) {
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
                case NFAOptTag::RangeK: {
                    const NFAOptRangeK* rngk = static_cast<const NFAOptRangeK*>(opt);
                    nstates.singlestates.insert(NFASingleStateToken::toNextStateWithInitialize(rngk->infollow, rngk->stateid));

                    if(rngk->mink == 0) {
                        nstates.simplestates.insert(iter->toNextState(rngk->outfollow));
                    }

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
        bool advanced = false;

        for(auto iter = ostates.singlestates.cbegin(); iter != ostates.singlestates.cend(); ++iter) {
            const NFAOpt* opt = this->nfaopts[iter->cstate];
            const NFAOptTag tag = opt->tag;

            switch(tag) {
                case NFAOptTag::AnyOf: {
                    const NFAOptAnyOf* anyof = static_cast<const NFAOptAnyOf*>(opt);
                    std::transform(anyof->follows.cbegin(), anyof->follows.cend(), std::inserter(nstates.singlestates, nstates.singlestates.begin()), [iter](const StateID& follow) {
                        return iter->toNextState(follow);
                    });

                    advanced = true;
                    break;
                }
                case NFAOptTag::Star: {
                    const NFAOptStar* star = static_cast<const NFAOptStar*>(opt);
                    nstates.singlestates.insert(iter->toNextState(star->matchfollow));
                    nstates.singlestates.insert(iter->toNextState(star->skipfollow));

                    advanced = true;
                    break;
                }
                case NFAOptTag::RangeK: {
                    const NFAOptRangeK* rngk = static_cast<const NFAOptRangeK*>(opt);
                    if(rngk->stateid != iter->rangecount.first) {
                        nstates.fullstates.insert(NFAFullStateToken::toNextStateWithInitialize(rngk->infollow, iter->rangecount, rngk->stateid));

                        if(rngk->mink == 0) {
                            nstates.singlestates.insert(iter->toNextState(rngk->outfollow));
                        }
                    }
                    else {
                        if(iter->rangecount.second < rngk->mink) {
                            nstates.singlestates.insert(iter->toNextStateWithIncrement(rngk->infollow));
                        }
                        else if(rngk->maxk != INT16_MAX && iter->rangecount.second == rngk->maxk) {
                            nstates.simplestates.insert(iter->toNextStateWithDoneRange(rngk->outfollow));
                        }
                        else {
                            nstates.singlestates.insert(iter->toNextStateWithIncrement(rngk->infollow));
                            nstates.simplestates.insert(iter->toNextStateWithDoneRange(rngk->outfollow));
                        }
                    }

                    advanced = true;
                    break;
                }
                default: {
                    //No epsilon transitions for these so just keep token and no additonal states so no advanced = true
                    nstates.singlestates.insert(*iter);
                    break;
                }
            }
        }

        return advanced;
    }

    bool NFAMachine::advanceEpsilonForFullStates(const NFAState& ostates, NFAState& nstates) const
    {
        bool advanced = false;

        for(auto iter = ostates.fullstates.cbegin(); iter != ostates.fullstates.cend(); ++iter) {
            BREX_ASSERT(false, "Not implemented");
        }

        return advanced;
    }

    void NFAMachine::mergeStatesFrom(const NFAState& ostates, NFAState& nstates)
    {
        std::copy(ostates.simplestates.cbegin(), ostates.simplestates.cend(), std::inserter(nstates.simplestates, nstates.simplestates.begin()));
        std::copy(ostates.singlestates.cbegin(), ostates.singlestates.cend(), std::inserter(nstates.singlestates, nstates.singlestates.begin()));
        std::copy(ostates.fullstates.cbegin(), ostates.fullstates.cend(), std::inserter(nstates.fullstates, nstates.fullstates.begin()));
    }

    bool NFAMachine::isNewState(const NFAState& ostates, const NFAState& nstates)
    {
        if(ostates.simplestates.size() != nstates.simplestates.size() || ostates.singlestates.size() != nstates.singlestates.size() || ostates.fullstates.size() != nstates.fullstates.size()) {
            return true;
        }

        return !std::equal(ostates.simplestates.cbegin(), ostates.simplestates.cend(), nstates.simplestates.cbegin()) || !std::equal(ostates.singlestates.cbegin(), ostates.singlestates.cend(), nstates.singlestates.cbegin()) || !std::equal(ostates.fullstates.cbegin(), ostates.fullstates.cend(), nstates.fullstates.cbegin());
    }
}
