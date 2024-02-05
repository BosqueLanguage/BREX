#include "common.h"
#include "nfa_executor.h"

namespace BREX
{
    template <typename TStr, typename TIter>
    class SingleCheckREInfo
    {
    public:
        NFAExecutor<TStr, TIter>* executor;
        bool isNegative;
        bool isFrontCheck;
        bool isBackCheck;

        xxxx;
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
        const std::vector<SingleCheckREInfo> checks;
        const bool isContainsable;
        const bool isMatchable;

        REExecutorUnicode(const std::vector<SingleCheckREInfo>& checks, bool isContainsable, bool isMatchable) : checks(checks), isContainsable(isContainsable), isMatchable(isMatchable) {;}
        
        ~REExecutorUnicode() 
        {
            for(size_t i = 0; i < this->checks.size(); ++i) {
                delete this->checks[i].executor;
            }
        }

        bool test(UnicodeString* sstr, int64_t spos, int64_t epos);
        
        bool testContains(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error);
        bool testFront(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error);
        bool testBack(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error);

        std::optional<std::pair<int64_t, int64_t>> matchContains(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error);
        std::optional<int64_t> matchFront(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error);
        std::optional<int64_t> matchBack(UnicodeString* sstr, int64_t spos, int64_t epos, ExecutorError& error);

        bool test(UnicodeString* sstr) { return this->test(sstr, 0, (int64_t)sstr->size() - 1); }

        bool testContains(UnicodeString* sstr, ExecutorError& error) { return this->testContains(sstr, 0, (int64_t)sstr->size() - 1, error); }
        bool testFront(UnicodeString* sstr, ExecutorError& error) { return this->testFront(sstr, 0, (int64_t)sstr->size() - 1, error); }
        bool testBack(UnicodeString* sstr, ExecutorError& error) { return this->testBack(sstr, 0, (int64_t)sstr->size() - 1, error); }

        std::optional<std::pair<int64_t, int64_t>> matchContains(UnicodeString* sstr, ExecutorError& error) { return this->matchContains(sstr, 0, (int64_t)sstr->size() - 1, error); }
        std::optional<int64_t> matchFront(UnicodeString* sstr, ExecutorError& error) { return this->matchFront(sstr, 0, (int64_t)sstr->size() - 1, error); }
        std::optional<int64_t> matchBack(UnicodeString* sstr, ExecutorError& error) { return this->matchBack(sstr, 0, (int64_t)sstr->size() - 1, error); }
    };
}
