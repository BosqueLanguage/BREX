#include "brex_engine.h"

namespace BREX
{
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
        BREX_ASSERT(false, "Not implemented");
        return true;
    }

    void NFAUnicodeExecutor::runIntialStep()
    {
        this->cstates.intitialize();
        this->cstates.simplestates.insert(NFASimpleStateToken{this->m->startstate});

        NFAState nstates;
        while(this->m->advanceEpsilon(this->cstates, nstates)) {
            this->cstates = std::move(nstates);
            nstates.reset();
        }
    }

    void NFAUnicodeExecutor::runStep(RegexChar c)
    {
        NFAState ncstates;
        this->m->advanceChar(c, this->cstates, ncstates);

        NFAState nstates;
        while(this->m->advanceEpsilon(ncstates, nstates)) {
            ncstates = std::move(nstates);
            nstates.reset();
        }

        this->cstates = std::move(ncstates);
    }

    bool NFAUnicodeExecutor::accepted() const
    {
        return this->m->inAccepted(this->cstates);
    }

    bool NFAUnicodeExecutor::rejected() const
    {
        return this->m->allRejected(this->cstates);
    }

    bool NFAUnicodeExecutor::test(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        this->m = this->forward;
        this->iter = UnicodeRegexIterator{sstr, spos, epos, spos};

        this->runIntialStep();
        while(this->iter.valid()) {
            this->runStep(this->iter.get());
            this->iter.inc();

            if(this->rejected()) {
                return false;
            }
        }

        return this->accepted();
    }

    bool NFAUnicodeExecutor::matchTestForward(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        this->m = this->forward;
        this->iter = UnicodeRegexIterator{sstr, spos, epos, spos};

        this->runIntialStep();
        while(this->iter.valid() && !this->accepted()) {
            this->runStep(this->iter.get());
            this->iter.inc();
        }

        return this->accepted();
    }

    bool NFAUnicodeExecutor::matchTestReverse(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        this->m = this->reverse;
        this->iter = UnicodeRegexIterator{sstr, spos, epos, epos};

        this->runIntialStep();
        while(this->iter.valid() && !this->accepted()) {
            this->runStep(this->iter.get());
            this->iter.dec();
        }

        return this->accepted();
    }

    std::vector<int64_t> NFAUnicodeExecutor::matchForward(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        this->m = this->forward;
        this->iter = UnicodeRegexIterator{sstr, spos, epos, spos};

        std::vector<int64_t> matches;
        this->runIntialStep();
        while(this->iter.valid()) {
            this->runStep(this->iter.get());

            if(this->accepted()) {
                matches.push_back(this->iter.curr);
            }

            this->iter.inc();
        }

        return matches;
    }

    std::vector<int64_t> NFAUnicodeExecutor::matchReverse(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        this->m = this->reverse;
        this->iter = UnicodeRegexIterator{sstr, spos, epos, epos};

        std::vector<int64_t> matches;
        this->runIntialStep();
        while(this->iter.valid()) {
            this->runStep(this->iter.get());

            if(this->accepted()) {
                matches.push_back(this->iter.curr);
            }

            this->iter.dec();
        }

        return matches;
    }

    void NFAASCIIExecutor::runIntialStep()
    {
        this->cstates.intitialize();
        this->cstates.simplestates.insert(NFASimpleStateToken{this->m->startstate});

        NFAState nstates;
        while(this->m->advanceEpsilon(this->cstates, nstates)) {
            this->cstates = std::move(nstates);
            nstates.reset();
        }
    }

    void NFAASCIIExecutor::runStep(RegexChar c)
    {
        NFAState ncstates;
        this->m->advanceChar(c, this->cstates, ncstates);

        NFAState nstates;
        while(this->m->advanceEpsilon(ncstates, nstates)) {
            ncstates = std::move(nstates);
            nstates.reset();
        }

        this->cstates = std::move(ncstates);
    }

    bool NFAASCIIExecutor::accepted() const
    {
        return this->m->inAccepted(this->cstates);
    }

    bool NFAASCIIExecutor::rejected() const
    {
        return this->m->allRejected(this->cstates);
    }

    bool NFAASCIIExecutor::test(ASCIIString* sstr, int64_t spos, int64_t epos)
    {
        this->m = this->forward;
        this->iter = ASCIIRegexIterator{sstr, spos, epos, spos};

        this->runIntialStep();
        while(this->iter.valid()) {
            this->runStep(this->iter.get());
            this->iter.inc();

            if(this->rejected()) {
                return false;
            }
        }

        return this->accepted();
    }

    bool NFAASCIIExecutor::matchTestForward(ASCIIString* sstr, int64_t spos, int64_t epos)
    {
        this->m = this->forward;
        this->iter = ASCIIRegexIterator{sstr, spos, epos, spos};

        this->runIntialStep();
        while(this->iter.valid() && !this->accepted()) {
            this->runStep(this->iter.get());
            this->iter.inc();
        }

        return this->accepted();
    }

    bool NFAASCIIExecutor::matchTestReverse(ASCIIString* sstr, int64_t spos, int64_t epos)
    {
        this->m = this->reverse;
        this->iter = ASCIIRegexIterator{sstr, spos, epos, epos};

        this->runIntialStep();
        while(this->iter.valid() && !this->accepted()) {
            this->runStep(this->iter.get());
            this->iter.dec();
        }

        return this->accepted();
    }

    std::vector<int64_t> NFAASCIIExecutor::matchForward(ASCIIString* sstr, int64_t spos, int64_t epos)
    {
        this->m = this->forward;
        this->iter = ASCIIRegexIterator{sstr, spos, epos, spos};

        std::vector<int64_t> matches;
        this->runIntialStep();
        while(this->iter.valid()) {
            this->runStep(this->iter.get());

            if(this->accepted()) {
                matches.push_back(this->iter.curr);
            }

            this->iter.inc();
        }

        return matches;
    }

    std::vector<int64_t> NFAASCIIExecutor::matchReverse(ASCIIString* sstr, int64_t spos, int64_t epos)
    {
        this->m = this->reverse;
        this->iter = ASCIIRegexIterator{sstr, spos, epos, epos};

        std::vector<int64_t> matches;
        this->runIntialStep();
        while(this->iter.valid()) {
            this->runStep(this->iter.get());

            if(this->accepted()) {
                matches.push_back(this->iter.curr);
            }

            this->iter.dec();
        }

        return matches;
    }

    bool REExecutorUnicode::test(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        if(this->sanchor == nullptr && this->eanchor == nullptr) {
            return this->re->test(sstr, spos, epos);
        }
        else {
            if(this->sanchor == nullptr) {
                auto opts = this->re->matchForward(sstr, spos, epos);
                auto mm = std::find_if(opts.cbegin(), opts.cend(), [sstr, this, epos](int64_t pos) {
                    return this->eanchor->test(sstr, pos + 1, epos);
                });

                return mm != opts.cend();
            }
            else if(this->eanchor == nullptr) {
                auto opts = this->re->matchReverse(sstr, spos, epos);
                auto mm = std::find_if(opts.cbegin(), opts.cend(), [sstr, this, spos](int64_t pos) {
                    return this->sanchor->test(sstr, spos, pos - 1);
                });

                return mm != opts.cend();
            }
            else {
                xxxx;
            }
        }
    }

    bool REExecutorUnicode::matchTestFront(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        if(this->sanchor == nullptr && this->eanchor == nullptr) {
            return this->re->matchTestForward(sstr, spos, epos);
        }
        else {
            xxxx;
        }
    }

    bool REExecutorUnicode::matchTestBack(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        if(this->sanchor == nullptr && this->eanchor == nullptr) {
            return this->re->matchTestReverse(sstr, spos, epos);
        }
        else {
            xxxx;
        }
    }

    std::optional<int64_t> REExecutorUnicode::matchFront(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        if(this->sanchor == nullptr && this->eanchor == nullptr) {
            auto opts = this->re->matchForward(sstr, spos, epos);
            if(opts.empty()) {
                return std::nullopt;
            }
            else {
                return std::make_optional(opts[0]);
            }
        }
        else {
            xxxx;
        }
    }

    std::optional<int64_t> REExecutorUnicode::matchBack(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        if(this->sanchor == nullptr && this->eanchor == nullptr) {
            auto opts = this->re->matchReverse(sstr, spos, epos);
            if(opts.empty()) {
                return std::nullopt;
            }
            else {
                return std::make_optional(opts[0]);
            }
        }
        else {
            xxxx;
        }
    }
}
