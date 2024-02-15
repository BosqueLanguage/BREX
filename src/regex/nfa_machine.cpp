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

    void NFAMachine::advanceCharForSimpleStates(RegexChar c, const NFAState& ostates, NFAEpsilonWorkSet& workset, NFAState& nstates) const
    {
        for(auto iter = ostates.simplestates.cbegin(); iter != ostates.simplestates.cend(); ++iter) {
            const NFAOpt* opt = this->nfaopts[iter->cstate];
            const NFAOptTag tag = opt->tag;

            switch(tag) {
                case NFAOptTag::CharCode: {
                    const NFAOptCharCode* cc = static_cast<const NFAOptCharCode*>(opt);
                    if(cc->c == c) {
                        this->addNextSimpleState(nstates, workset, iter->toNextState(cc->follow));
                    }
                    break;
                }
                case NFAOptTag::CharRange: {
                    const NFAOptRange* range = static_cast<const NFAOptRange*>(opt);
                    auto inrng = std::find_if(range->ranges.cbegin(), range->ranges.cend(), [c](const SingleCharRange& rr) {
                        return (rr.low <= c && c <= rr.high);
                    }) != range->ranges.cend();

                    bool doinsert = !range->compliment == inrng; //either both true or both false
                    if(doinsert) {
                        this->addNextSimpleState(nstates, workset, iter->toNextState(range->follow));
                    }
                    break;
                }
                case NFAOptTag::Dot: {
                    const NFAOptDot* dot = static_cast<const NFAOptDot*>(opt);
                    this->addNextSimpleState(nstates, workset, iter->toNextState(dot->follow));
                    break;
                }
                default: {
                    //No char transitions for these so just drop token
                    break;
                }
            }
        }
    }

    void NFAMachine::advanceCharForSingleStates(RegexChar c, const NFAState& ostates, NFAEpsilonWorkSet& workset, NFAState& nstates) const
    {
        for(auto iter = ostates.singlestates.cbegin(); iter != ostates.singlestates.cend(); ++iter) {
            const NFAOpt* opt = this->nfaopts[iter->cstate];
            const NFAOptTag tag = opt->tag;

            switch(tag) {
                case NFAOptTag::CharCode: {
                    const NFAOptCharCode* cc = static_cast<const NFAOptCharCode*>(opt);
                    if(cc->c == c) {
                        this->addNextSingleState(nstates, workset, iter->toNextState(cc->follow));
                    }
                    break;
                }
                case NFAOptTag::CharRange: {
                    const NFAOptRange* range = static_cast<const NFAOptRange*>(opt);
                    auto inrng = std::find_if(range->ranges.cbegin(), range->ranges.cend(), [c](const SingleCharRange& rr) {
                        return (rr.low <= c && c <= rr.high);
                    }) != range->ranges.cend();

                    bool doinsert = !range->compliment == inrng; //either both true or both false
                    if(doinsert) {
                        this->addNextSingleState(nstates, workset, iter->toNextState(range->follow));
                    }
                    break;
                }
                case NFAOptTag::Dot: {
                    const NFAOptDot* dot = static_cast<const NFAOptDot*>(opt);
                    this->addNextSingleState(nstates, workset, iter->toNextState(dot->follow));
                    break;
                }
                default: {
                    //No char transitions for these so just drop token
                    break;
                }
            }
        }
    }
        
    void NFAMachine::advanceCharForFullStates(RegexChar c, const NFAState& ostates, NFAEpsilonWorkSet& workset, NFAState& nstates) const
    {
        for(auto iter = ostates.fullstates.cbegin(); iter != ostates.fullstates.cend(); ++iter) {
            const NFAOpt* opt = this->nfaopts[iter->cstate];
            const NFAOptTag tag = opt->tag;

            switch(tag) {
                case NFAOptTag::CharCode: {
                    const NFAOptCharCode* cc = static_cast<const NFAOptCharCode*>(opt);
                    if(cc->c == c) {
                        this->addNextFullState(nstates, workset, iter->toNextState(cc->follow));
                    }
                    break;
                }
                case NFAOptTag::CharRange: {
                    const NFAOptRange* range = static_cast<const NFAOptRange*>(opt);
                    auto inrng = std::find_if(range->ranges.cbegin(), range->ranges.cend(), [c](const SingleCharRange& rr) {
                        return (rr.low <= c && c <= rr.high);
                    }) != range->ranges.cend();

                    bool doinsert = !range->compliment == inrng; //either both true or both false
                    if(doinsert) {
                        this->addNextFullState(nstates, workset, iter->toNextState(range->follow));
                    }
                    break;
                }
                case NFAOptTag::Dot: {
                    const NFAOptDot* dot = static_cast<const NFAOptDot*>(opt);
                    this->addNextFullState(nstates, workset, iter->toNextState(dot->follow));
                    break;
                }
                default: {
                    //No char transitions for these so just drop token
                    break;
                }
            }
        }
    }

    void NFAMachine::advanceEpsilonForSimpleStates(NFAEpsilonFixpointSet& fixpoint, NFAEpsilonWorkSet& workset, NFAState& nstates) const
    {
        while(workset.hasSimpleStates()) {
            const NFASimpleStateToken stok = workset.getNextSimpleState();

            const NFAOpt* opt = this->nfaopts[stok.cstate];
            const NFAOptTag tag = opt->tag;

            switch(tag) {
                case NFAOptTag::AnyOf: {
                    const NFAOptAnyOf* anyof = static_cast<const NFAOptAnyOf*>(opt);
                    for(auto iter = anyof->follows.cbegin(); iter != anyof->follows.cend(); ++iter) {
                        this->processSimpleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextState(*iter));
                    }

                    break;
                }
                case NFAOptTag::Star: {
                    const NFAOptStar* star = static_cast<const NFAOptStar*>(opt);
                    this->processSimpleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextState(star->matchfollow));
                    this->processSimpleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextState(star->skipfollow));

                    break;
                }
                case NFAOptTag::RangeK: {
                    const NFAOptRangeK* rngk = static_cast<const NFAOptRangeK*>(opt);
                    nstates.singlestates.insert(NFASingleStateToken::toNextStateWithInitialize(rngk->infollow, rngk->stateid));

                    if(rngk->mink == 0) {
                        this->processSimpleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextState(rngk->outfollow));
                    }

                    break;
                }
                default: {
                    //No epsilon transitions for these so just keep token
                    break;
                }
            }
        }
    }

    void NFAMachine::advanceEpsilonForSingleStates(NFAEpsilonFixpointSet& fixpoint, NFAEpsilonWorkSet& workset, NFAState& nstates) const
    {
        while(workset.hasSingleStates()) {
            const NFASingleStateToken stok = workset.getNextSingleState();

            const NFAOpt* opt = this->nfaopts[stok.cstate];
            const NFAOptTag tag = opt->tag;

            switch(tag) {
                case NFAOptTag::AnyOf: {
                    const NFAOptAnyOf* anyof = static_cast<const NFAOptAnyOf*>(opt);
                    for(auto iter = anyof->follows.cbegin(); iter != anyof->follows.cend(); ++iter) {
                        this->processSingleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextState(*iter));
                    }

                    break;
                }
                case NFAOptTag::Star: {
                    const NFAOptStar* star = static_cast<const NFAOptStar*>(opt);
                    this->processSingleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextState(star->matchfollow));
                    this->processSingleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextState(star->skipfollow));

                    break;
                }
                case NFAOptTag::RangeK: {
                    const NFAOptRangeK* rngk = static_cast<const NFAOptRangeK*>(opt);
                    if(rngk->stateid != stok.rangecount.first) {
                        this->processFullStateEpsilonTransition(nstates, fixpoint, workset, NFAFullStateToken::toNextStateWithInitialize(rngk->infollow, stok.rangecount, rngk->stateid));

                        if(rngk->mink == 0) {
                            this->processSingleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextState(rngk->outfollow));
                        }
                    }
                    else {
                        if(stok.rangecount.second < rngk->mink) {
                            this->processSingleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextStateWithIncrement(rngk->infollow));
                        }
                        else if(rngk->maxk != INT16_MAX && stok.rangecount.second == rngk->maxk) {
                            this->processSimpleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextStateWithDoneRange(rngk->outfollow));
                        }
                        else {
                            this->processSingleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextStateWithIncrement(rngk->infollow));
                            this->processSimpleStateEpsilonTransition(nstates, fixpoint, workset, stok.toNextStateWithDoneRange(rngk->outfollow));
                        }
                    }

                    break;
                }
                default: {
                    //No epsilon transitions for these so just keep token
                    break;
                }
            }
        }
    }

    void NFAMachine::advanceEpsilonForFullStates(NFAEpsilonFixpointSet& fixpoint, NFAEpsilonWorkSet& workset, NFAState& nstates) const
    {
        while(workset.hasFullStates()) {
            BREX_ASSERT(false, "Not implemented");
        }
    }
}
