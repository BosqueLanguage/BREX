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

    void splitOpsPolarity(const std::vector<SingleCheckREInfo>& allopts, std::vector<SingleCheckREInfo>& posopts, std::vector<SingleCheckREInfo>& negopts)
    {
        for(auto iter = allopts.cbegin(); iter != allopts.cend(); ++iter) {
            if(iter->isNegative) {
                negopts.push_back(*iter);
            }
            else {
                posopts.push_back(*iter);
            }
        }
    }

    bool REExecutorUnicode::test(UnicodeString* sstr, int64_t spos, int64_t epos)
    {
        if(this->checks.size() == 1) {
            auto accept = this->checks[0].executor->test(sstr, spos, epos);
            return this->checks[0].isNegative ? !accept : accept;
        }
        else {
            auto accept = std::all_of(this->checks.cbegin(), this->checks.cend(), [sstr, spos, epos](const SingleCheckREInfo& check) {
                bool accepted = false;
                if(check.isFrontCheck) {
                    accepted = check.executor->matchTestForward(sstr, spos, epos);
                }
                else if(check.isBackCheck) {
                    accepted = check.executor->matchTestReverse(sstr, spos, epos);
                }
                else {
                    accepted = check.executor->test(sstr, spos, epos);
                }

                return check.isNegative ? !accepted : accepted;
            });

            return accept;
        }
    }

    bool REExecutorUnicode::testContains(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error)
    {
        error = ExecutorError::Ok;
        if(!this->isContainsable) {
            error = ExecutorError::NotContainsable;
            return false;
        }

        for(int64_t ii = spos; ii <= epos; ++ii) {
            if(this->checks[0].executor->test(sstr, ii, epos)) {
                return true;
            }
        }
    }

    bool REExecutorUnicode::testFront(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error)
    {
        error = ExecutorError::Ok;
        if(!this->isMatchable) {
            error = ExecutorError::NotMatchable;
            return false;
        }

        if(this->checks.size() == 1) {
            auto accept = this->checks[0].executor->matchTestForward(sstr, spos, epos);
            return this->checks[0].isNegative ? !accept : accept;
        }
        else {
            std::vector<SingleCheckREInfo> posopts;
            std::vector<SingleCheckREInfo> negopts;
            splitOpsPolarity(this->checks, posopts, negopts);

            if(posopts.size() == 1 && negopts.empty()) {
                std::vector<int64_t> pmatches = posopts[0].executor->matchForward(sstr, spos, epos);

                xxxx;//extract this logic
                return std::any_of(pmatches.cbegin(), pmatches.cend(), [&sstr, spos, &negopts](int64_t mend) {
                    return std::all_of(negopts.cbegin(), negopts.cend(), [&sstr, spos, mend](const SingleCheckREInfo& check) {
                        xxxx; //or do we need to handle the front back anchors?
                        return !check.executor->test(sstr, spos, mend);
                    });
                });
            }
            else {
                xxxx;
                std::vector<std::vector<int64_t>> posmatches(posopts.size());
            }
        }
    }

    bool REExecutorUnicode::testBack(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error)
    {
        error = ExecutorError::Ok;
        if(!this->isMatchable) {
            error = ExecutorError::NotMatchable;
            return false;
        }

        if(this->checks.size() == 1) {
            auto accept = this->checks[0].executor->matchTestReverse(sstr, spos, epos);
            return this->checks[0].isNegative ? !accept : accept;
        }
        else {
            xxxx;
        }
    }

    std::optional<std::pair<int64_t, int64_t>> REExecutorUnicode::matchContains(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error)
    {
        error = ExecutorError::Ok;
        if(!this->isContainsable) {
            error = ExecutorError::NotContainsable;
            return false;
        }

        xxxx;
    }
        

    std::optional<int64_t> REExecutorUnicode::matchFront(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error)
    {
        error = ExecutorError::Ok;
        if(!this->isMatchable) {
            error = ExecutorError::NotMatchable;
            return false;
        }

        if(this->checks.size() == 1) {
            auto opts = this->checks[0].executor->matchForward(sstr, spos, epos);
            return !opts.empty() ? std::make_optional(opts.back()) : std::nullopt;
        }
        else {
            xxxx;
        }
    }

    std::optional<int64_t> REExecutorUnicode::matchBack(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error)
    {
        error = ExecutorError::Ok;
        if(!this->isMatchable) {
            error = ExecutorError::NotMatchable;
            return false;
        }

        if(this->checks.size() == 1) {
            auto opts = this->checks[0].executor->matchReverse(sstr, spos, epos);
            return !opts.empty() ? std::make_optional(opts.back()) : std::nullopt;
        }
        else {
            xxxx;
        }
    }
}
