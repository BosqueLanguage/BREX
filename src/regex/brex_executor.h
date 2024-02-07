#include "../common.h"
#include "nfa_executor.h"

namespace BREX
{
    template <typename TStr, typename TIter>
    class SingleCheckREInfo
    {
    public:
        NFAExecutor<TStr, TIter> executor;
        bool isNegative;
        bool isFrontCheck;
        bool isBackCheck;

        SingleCheckREInfo() = default;
        SingleCheckREInfo(const NFAExecutor<TStr, TIter>& executor, bool isNegative, bool isFrontCheck, bool isBackCheck) : executor(executor), isNegative(isNegative), isFrontCheck(isFrontCheck), isBackCheck(isBackCheck) {;}
        ~SingleCheckREInfo() = default;

        SingleCheckREInfo(const SingleCheckREInfo& other) = default;
        SingleCheckREInfo(SingleCheckREInfo&& other) = default;

        SingleCheckREInfo& operator=(const SingleCheckREInfo& other) = default;
        SingleCheckREInfo& operator=(SingleCheckREInfo&& other) = default;
    };

    enum ExecutorError
    {
        Ok,
        NotContainsable,
        NotMatchable
    };

    template <typename TStr, typename TIter>
    class REExecutor
    {
    public:
        const std::vector<SingleCheckREInfo<TStr, TIter>> checks;
        const bool isContainsable;
        const bool isMatchable;

        REExecutor(const std::vector<SingleCheckREInfo<TStr, TIter>>& checks, bool isContainsable, bool isMatchable) : checks(checks), isContainsable(isContainsable), isMatchable(isMatchable) {;}
        ~REExecutor() = default;

    private:
        void splitOpsPolarity(std::vector<SingleCheckREInfo<TStr, TIter>>& posopts, std::vector<SingleCheckREInfo<TStr, TIter>>& checkopts)
        {
            for(auto iter = this->checks.cbegin(); iter != this->checks.cend(); ++iter) {
                if(!iter->isNegative && !iter->isFrontCheck && !iter->isBackCheck) {
                    posopts.push_back(*iter);
                }
                else {
                    checkopts.push_back(*iter);
                }
            }
        }

        static std::vector<int64_t> computeSharedMatches(const std::vector<std::vector<int64_t>>& matches)
        {
            if(matches.empty()) {
                return std::vector<int64_t>();
            }

            std::vector<int64_t> sharedmatches = matches[0];
            for(auto iter = matches.cbegin() + 1; iter != matches.cend(); ++iter) {
                std::vector<int64_t> newmatches;
                std::set_intersection(sharedmatches.cbegin(), sharedmatches.cend(), iter->cbegin(), iter->cend(), std::back_inserter(newmatches));
                sharedmatches = std::move(newmatches);
            }

            return sharedmatches;
        }

        bool validateSingleOp(const SingleCheckREInfo<TStr, TIter>& check, TStr* sstr, int64_t spos, int64_t epos)
        {
            bool accepted = false;
            if(check.isFrontCheck) {
                accepted = check.executor.matchTestForward(sstr, spos, epos);
            }
            else if(check.isBackCheck) {
                accepted = check.executor.matchTestReverse(sstr, spos, epos);
            }
            else {
                accepted = check.executor.test(sstr, spos, epos);
            }

            return check.isNegative ? !accepted : accepted;
        }

        bool validateOpSet(const std::vector<SingleCheckREInfo<TStr, TIter>>& opts, TStr* sstr, int64_t spos, int64_t epos)
        {
            return std::all_of(opts.cbegin(), opts.cend(), [sstr, spos, epos](const SingleCheckREInfo<TStr, TIter>& check) {
                return validateSingleOp(check, sstr, spos, epos);
            });
        }

        std::vector<int64_t> validateMatchSetOptions(const std::vector<int64_t>& opts, const std::vector<SingleCheckREInfo<TStr, TIter>>& checks, TStr* sstr, int64_t spos)
        {
            std::vector<int64_t> matches;
            std::copy_if(opts.cbegin(), opts.cend(), std::back_inserter(matches), [sstr, spos, &checks](int64_t epos) {
                return validateOpSet(checks, sstr, spos, epos);
            });

            return matches;
        }

    public:
        bool test(TStr* sstr, int64_t spos, int64_t epos)
        {
           if(this->checks.size() == 1) {
                auto accept = this->checks[0].executor->test(sstr, spos, epos);
                return this->checks[0].isNegative ? !accept : accept;
            }
            else {
                return validateOpSet(this->checks, sstr, spos, epos);
            }
        }
        
        bool testContains(TStr* sstr, int64_t spos, int64_t epos, ExecutorError& error)
        {
            error = ExecutorError::Ok;
            if(!this->isContainsable) {
                error = ExecutorError::NotContainsable;
                return false;
            }

            //by def a single option that is not negative or front/back marked
            auto scheck = this->checks[0];
            for(int64_t ii = spos; ii <= epos; ++ii) {
                if(scheck.executor.matchTestForward(sstr, ii, epos)) {
                    return true;
                }
            }
        }

        bool testFront(TStr* sstr, int64_t spos, int64_t epos, ExecutorError& error)
        {
            error = ExecutorError::Ok;
            if(!this->isMatchable) {
                error = ExecutorError::NotMatchable;
                return false;
            }

            if(this->checks.size() == 1) {
                //by def a single option that is not negative or front/back marked
                auto accept = this->checks[0].executor.matchTestForward(sstr, spos, epos);
                return this->checks[0].isNegative ? !accept : accept;
            }
            else {
                std::vector<SingleCheckREInfo<TStr, TIter>> matchopts;
                std::vector<SingleCheckREInfo<TStr, TIter>> checkops;
                splitOpsPolarity(this->checks, matchopts, checkops);

                if(matchopts.size() == 1) {
                    auto matches = matchopts[0].executor.matchForward(sstr, spos, epos);
                    return !matches.empty() && validateOpSet(checkops, sstr, spos, matches.back());
                }
                else {
                    std::vector<std::vector<int64_t>> matches;
                    std::transform(matchopts.cbegin(), matchopts.cend(), std::back_inserter(matches), [sstr, spos, epos](const SingleCheckREInfo<TStr, TIter>& check) {
                        return check.executor.matchForward(sstr, spos, epos);
                    });

                    auto sharedmatches = computeSharedMatches(matches);
                    auto validmatches = validateMatchSetOptions(sharedmatches, checkops, sstr, spos);
                
                    return !validmatches.empty();
                }
            }
        }

        bool testBack(TStr* sstr, int64_t spos, int64_t epos, ExecutorError& error)
        {
            error = ExecutorError::Ok;
            if(!this->isMatchable) {
                error = ExecutorError::NotMatchable;
                return false;
            }

            if(this->checks.size() == 1) {
                //by def a single option that is not negative or front/back marked
                auto accept = this->checks[0].executor.matchTestReverse(sstr, spos, epos);
                return this->checks[0].isNegative ? !accept : accept;
            }
            else {
                std::vector<SingleCheckREInfo<TStr, TIter>> matchopts;
                std::vector<SingleCheckREInfo<TStr, TIter>> checkops;
                splitOpsPolarity(this->checks, matchopts, checkops);

                if(matchopts.size() == 1) {
                    auto matches = matchopts[0].executor.matchReverse(sstr, spos, epos);
                    return !matches.empty() && validateOpSet(checkops, sstr, spos, matches.back());
                }
                else {
                    std::vector<std::vector<int64_t>> matches;
                    std::transform(matchopts.cbegin(), matchopts.cend(), std::back_inserter(matches), [sstr, spos, epos](const SingleCheckREInfo<TStr, TIter>& check) {
                        return check.executor.matchReverse(sstr, spos, epos);
                    });

                    auto sharedmatches = computeSharedMatches(matches);
                    auto validmatches = validateMatchSetOptions(sharedmatches, checkops, sstr, spos);
                
                    return !validmatches.empty();
                }
            }
        }

        std::optional<std::pair<int64_t, int64_t>> matchContains(TStr* sstr, int64_t spos, int64_t epos, ExecutorError& error)
        {
            error = ExecutorError::Ok;
            if(!this->isContainsable) {
                error = ExecutorError::NotContainsable;
                return std::nullopt;
            }

            //by def a single option that is not negative or front/back marked
            auto scheck = this->checks[0];

            for(int64_t ii = spos; ii <= epos; ++ii) {
                auto mm = scheck.executor.matchForward(sstr, ii, epos);

                if(!mm.empty()) {
                    return std::make_optional(std::make_pair(ii, mm.back()));
                }
            }
        }

        std::optional<int64_t> matchFront(TStr* sstr, int64_t spos, int64_t epos, ExecutorError& error)
        {
            error = ExecutorError::Ok;
            if(!this->isMatchable) {
                error = ExecutorError::NotMatchable;
                return false;
            }

            if(this->checks.size() == 1) {
                //by def a single option that is not negative or front/back marked
                auto opts = this->checks[0].executor.matchForward(sstr, spos, epos);
                return !opts.empty() ? std::make_optional(opts.back()) : std::nullopt;
            }
            else {
                std::vector<SingleCheckREInfo<TStr, TIter>> matchopts;
                std::vector<SingleCheckREInfo<TStr, TIter>> checkops;
                splitOpsPolarity(this->checks, matchopts, checkops);

                std::vector<int64_t> realmatches;
                if(matchopts.size() == 1) {
                    realmatches = matchopts[0].executor.matchForward(sstr, spos, epos);
                }
                else {
                    std::vector<std::vector<int64_t>> matches;
                    std::transform(matchopts.cbegin(), matchopts.cend(), std::back_inserter(matches), [sstr, spos, epos](const SingleCheckREInfo<TStr, TIter>& check) {
                        return check.executor.matchForward(sstr, spos, epos);
                    });

                    realmatches = computeSharedMatches(matches);
                }

                auto validmatches = validateMatchSetOptions(realmatches, checkops, sstr, spos);
                return !validmatches.empty() ? std::make_optional(validmatches.back()) : std::nullopt;
            }
        }

        std::optional<int64_t> matchBack(TStr* sstr, int64_t spos, int64_t epos, ExecutorError& error)
        {
            error = ExecutorError::Ok;
            if(!this->isMatchable) {
                error = ExecutorError::NotMatchable;
                return false;
            }

            if(this->checks.size() == 1) {
                //by def a single option that is not negative or front/back marked
                auto opts = this->checks[0].executor.matchReverse(sstr, spos, epos);
                return !opts.empty() ? std::make_optional(opts.back()) : std::nullopt;
            }
            else {
                std::vector<SingleCheckREInfo<TStr, TIter>> matchopts;
                std::vector<SingleCheckREInfo<TStr, TIter>> checkops;
                splitOpsPolarity(this->checks, matchopts, checkops);

                std::vector<int64_t> realmatches;
                if(matchopts.size() == 1) {
                    realmatches = matchopts[0].executor.matchReverse(sstr, spos, epos);
                }
                else {
                    std::vector<std::vector<int64_t>> matches;
                    std::transform(matchopts.cbegin(), matchopts.cend(), std::back_inserter(matches), [sstr, spos, epos](const SingleCheckREInfo<TStr, TIter>& check) {
                        return check.executor->matchReverse(sstr, spos, epos);
                    });

                    realmatches = computeSharedMatches(matches);
                }

                auto validmatches = validateMatchSetOptions(realmatches, checkops, sstr, spos);
                return !validmatches.empty() ? std::make_optional(validmatches.back()) : std::nullopt;
            }
        }

        bool test(TStr* sstr) { return this->test(sstr, 0, (int64_t)sstr->size() - 1); }

        bool testContains(TStr* sstr, ExecutorError& error) { return this->testContains(sstr, 0, (int64_t)sstr->size() - 1, error); }
        bool testFront(TStr* sstr, ExecutorError& error) { return this->testFront(sstr, 0, (int64_t)sstr->size() - 1, error); }
        bool testBack(TStr* sstr, ExecutorError& error) { return this->testBack(sstr, 0, (int64_t)sstr->size() - 1, error); }

        std::optional<std::pair<int64_t, int64_t>> matchContains(TStr* sstr, ExecutorError& error) { return this->matchContains(sstr, 0, (int64_t)sstr->size() - 1, error); }
        std::optional<int64_t> matchFront(TStr* sstr, ExecutorError& error) { return this->matchFront(sstr, 0, (int64_t)sstr->size() - 1, error); }
        std::optional<int64_t> matchBack(TStr* sstr, ExecutorError& error) { return this->matchBack(sstr, 0, (int64_t)sstr->size() - 1, error); }
    };

    typedef REExecutor<UnicodeString, UnicodeRegexIterator> UnicodeRegexExecutor;
    typedef REExecutor<ASCIIString, ASCIIRegexIterator> ASCIIRegexExecutor;
}
