#define once

#include "common.h"

namespace BREX
{
    typedef size_t StateID;
    
    class NFASimpleStateToken
    {
    public:
        StateID cstate;

        NFASimpleStateToken() : cstate(0) {;}
        NFASimpleStateToken(StateID cstate) : cstate(cstate) {;}
        ~NFASimpleStateToken() {;}

        NFASimpleStateToken(const NFASimpleStateToken& other) = default;
        NFASimpleStateToken(NFASimpleStateToken&& other) = default;

        NFASimpleStateToken& operator=(const NFASimpleStateToken& other) = default;
        NFASimpleStateToken& operator=(NFASimpleStateToken&& other) = default;

        bool operator==(const NFASimpleStateToken& other) const
        {
            return this->cstate == other.cstate;
        }

        bool operator!=(const NFASimpleStateToken& other) const
        {
            return this->cstate != other.cstate;
        }

        NFASimpleStateToken toNextState(StateID next) const
        {
            return NFASimpleStateToken(next);
        }
    };

    class NFAFullStateToken
    {
    public:
        StateID cstate;
        std::vector<std::pair<StateID, uint16_t>> rangecounts;
        
        NFAFullStateToken() : cstate(0), rangecounts() {;}
        NFAFullStateToken(StateID cstate, std::vector<std::pair<StateID, uint16_t>> rangecounts) : cstate(cstate), rangecounts(rangecounts) {;}
        ~NFAFullStateToken() {;}

        NFAFullStateToken(const NFAFullStateToken& other) = default;
        NFAFullStateToken(NFAFullStateToken&& other) = default;

        NFAFullStateToken& operator=(const NFAFullStateToken& other) = default;
        NFAFullStateToken& operator=(NFAFullStateToken&& other) = default;

        bool operator==(const NFAFullStateToken& other) const
        {
            return this->cstate == other.cstate && std::equal(this->rangecounts.cbegin(), this->rangecounts.cend(), other.rangecounts.cbegin(), other.rangecounts.cend());
        }

        bool operator!=(const NFAFullStateToken& other) const
        {
            return this->cstate != other.cstate || !std::equal(this->rangecounts.cbegin(), this->rangecounts.cend(), other.rangecounts.cbegin(), other.rangecounts.cend());
        }

        NFAFullStateToken toNextState(StateID next) const
        {
            return NFAFullStateToken(next, this->rangecounts);
        }

        NFAFullStateToken toNextStateWithIncrement(StateID next, StateID incState) const
        {
            return xxxx;
        }
    };

    class NFAOpt
    {
    public:
        const StateID stateid;

        NFAOpt(StateID stateid) : stateid(stateid) {;}
        virtual ~NFAOpt() {;}

        virtual void advanceChar(RegexChar c, const std::vector<NFAOpt*>& nfaopts, std::vector<StateID>& nstates) const
        {
            return;
        }
        
        virtual void advanceEpsilon(const std::vector<NFAOpt*>& nfaopts, std::vector<StateID>& nstates) const
        {
            nstates.push_back(this->stateid);
        }
    };

    class NFAOptAccept : public NFAOpt
    {
    public:
        NFAOptAccept(StateID stateid) : NFAOpt(stateid) {;}
        virtual ~NFAOptAccept() {;}
    };

    class NFAOptCharCode : public NFAOpt
    {
    public:
        const RegexChar c;
        const StateID follow;

        NFAOptCharCode(StateID stateid, RegexChar c, StateID follow) : NFAOpt(stateid), c(c), follow(follow) {;}
        virtual ~NFAOptCharCode() {;}

        virtual void advanceChar(RegexChar c, const std::vector<NFAOpt*>& nfaopts, std::vector<StateID>& nstates) const override final
        {
            if(this->c == c) {
                nstates.push_back(this->follow);
            }
        }
    };

    class NFAOptRange : public NFAOpt
    {
    public:
        const bool compliment;
        const std::vector<SingleCharRange> ranges;
        const StateID follow;

        NFAOptRange(StateID stateid, bool compliment, std::vector<SingleCharRange> ranges, StateID follow) : NFAOpt(stateid), compliment(compliment), ranges(ranges), follow(follow) {;}
        virtual ~NFAOptRange() {;}

        virtual void advanceChar(RegexChar c, const std::vector<NFAOpt*>& nfaopts, std::vector<StateID>& nstates) const override final
        {
            auto chkrng = std::find_if(this->ranges.cbegin(), this->ranges.cend(), [c](const SingleCharRange& rr) {
                return (rr.low <= c && c <= rr.high);
            });

            if(!compliment) {
                if(chkrng != this->ranges.cend()) {
                    nstates.push_back(this->follow);
                }
            }
            else {
                if(chkrng == this->ranges.cend()) {
                    nstates.push_back(this->follow);
                }
            }
        }
    };

    class NFAOptDot : public NFAOpt
    {
    public:
        const StateID follow;

        NFAOptDot(StateID stateid, StateID follow) : NFAOpt(stateid), follow(follow) {;}
        virtual ~NFAOptDot() {;}

        virtual void advanceChar(RegexChar c, const std::vector<NFAOpt*>& nfaopts, std::vector<StateID>& nstates) const override final
        {
            nstates.push_back(this->follow);
        }
    };

    class NFAOptAllOf : public NFAOpt
    {
    public:
        const std::vector<StateID> follows;

        NFAOptAlternate(StateID stateid, std::vector<StateID> follows) : NFAOpt(stateid), follows(follows) {;}
        virtual ~NFAOptAlternate() {;}

        virtual void advanceEpsilon(const std::vector<NFAOpt*>& nfaopts, std::vector<StateID>& nstates) const override final
        {
            for(size_t i = 0; i < this->follows.size(); ++i) {
                nfaopts[this->follows[i]]->advanceEpsilon(nfaopts, nstates);
            }
        }
    };

    class NFAOptAnyOf : public NFAOpt
    {
    public:
        const std::vector<StateID> follows;

        NFAOptAlternate(StateID stateid, std::vector<StateID> follows) : NFAOpt(stateid), follows(follows) {;}
        virtual ~NFAOptAlternate() {;}

        virtual void advanceEpsilon(const std::vector<NFAOpt*>& nfaopts, std::vector<StateID>& nstates) const override final
        {
            for(size_t i = 0; i < this->follows.size(); ++i) {
                nfaopts[this->follows[i]]->advanceEpsilon(nfaopts, nstates);
            }
        }
    };

    class NFAOptStar : public NFAOpt
    {
    public:
        const StateID matchfollow;
        const StateID skipfollow;

        NFAOptStar(StateID stateid, StateID matchfollow, StateID skipfollow) : NFAOpt(stateid), matchfollow(matchfollow), skipfollow(skipfollow) {;}
        virtual ~NFAOptStar() {;}

        virtual void advanceEpsilon(const std::vector<NFAOpt*>& nfaopts, std::vector<StateID>& nstates) const override final
        {
            nfaopts[this->matchfollow]->advanceEpsilon(nfaopts, nstates);
            nfaopts[this->skipfollow]->advanceEpsilon(nfaopts, nstates);
        }
    };

    class NFAOptMinK : public NFAOpt
    {
    public:
        const StateID matchfollow;
        const StateID skipfollow;

        NFAOptStar(StateID stateid, StateID matchfollow, StateID skipfollow) : NFAOpt(stateid), matchfollow(matchfollow), skipfollow(skipfollow) {;}
        virtual ~NFAOptStar() {;}

        virtual void advanceEpsilon(const std::vector<NFAOpt*>& nfaopts, std::vector<StateID>& nstates) const override final
        {
            nfaopts[this->matchfollow]->advanceEpsilon(nfaopts, nstates);
            nfaopts[this->skipfollow]->advanceEpsilon(nfaopts, nstates);
        }
    };

    class NFAOptMaxK : public NFAOpt
    {
    public:
        const StateID matchfollow;
        const StateID skipfollow;

        NFAOptStar(StateID stateid, StateID matchfollow, StateID skipfollow) : NFAOpt(stateid), matchfollow(matchfollow), skipfollow(skipfollow) {;}
        virtual ~NFAOptStar() {;}

        virtual void advanceEpsilon(const std::vector<NFAOpt*>& nfaopts, std::vector<StateID>& nstates) const override final
        {
            nfaopts[this->matchfollow]->advanceEpsilon(nfaopts, nstates);
            nfaopts[this->skipfollow]->advanceEpsilon(nfaopts, nstates);
        }
    };

    class NFA
    {
    public:
        const StateID startstate;
        const StateID acceptstate;

        const std::vector<NFAOpt*> nfaopts;

        NFA(StateID startstate, StateID acceptstate, std::vector<NFAOpt*> nfaopts) : startstate(startstate), acceptstate(acceptstate), nfaopts(nfaopts) 
        {
            ;
        }
        
        ~NFA() 
        {
            for(size_t i = 0; i < this->nfaopts.size(); ++i) {
                delete this->nfaopts[i];
            }
        }

        bool test(CharCodeIterator& cci) const
        {
            std::vector<StateID> cstates;
            this->nfaopts[this->startstate]->advanceEpsilon(this->nfaopts, cstates);
            
            while(cci.valid()) {
                auto cc = cci.get();
                cci.advance();

                std::vector<StateID> nstates;
                for(size_t i = 0; i < cstates.size(); ++i) {
                    this->nfaopts[cstates[i]]->advanceChar(cc, this->nfaopts, nstates);
                }

                std::sort(nstates.begin(), nstates.end());
                auto nend = std::unique(nstates.begin(), nstates.end());
                nstates.erase(nend, nstates.end());

                std::vector<StateID> estates;
                for(size_t i = 0; i < nstates.size(); ++i) {
                    this->nfaopts[nstates[i]]->advanceEpsilon(this->nfaopts, estates);
                }

                std::sort(estates.begin(), estates.end());
                auto eend = std::unique(estates.begin(), estates.end());
                estates.erase(eend, estates.end());

                cstates = std::move(estates);
                if(cstates.empty()) {
                    return false;
                }
            }

            return std::find(cstates.cbegin(), cstates.cend(), this->acceptstate) != cstates.cend();
        }
    };
}