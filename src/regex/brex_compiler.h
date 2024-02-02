#define once

#include "common.h"
#include "brex.h"
#include "brex_engine.h"

namespace BREX
{
    typedef void* NameResolverState;
    typedef std::string (*fnNameResolver)(const std::string& name, void* resolverS);

    class RegexCompileError
    {
    public:
        std::u8string msg;

        RegexCompileError(std::u8string msg): msg(msg) { ; }
        ~RegexCompileError() = default;

        RegexCompileError() = default;
        RegexCompileError(const RegexCompileError& other) = default;
        RegexCompileError(RegexCompileError&& other) = default;

        RegexCompileError& operator=(const RegexCompileError& other) = default;
        RegexCompileError& operator=(RegexCompileError&& other) = default;
    };

    class RegexResolver
    {
    private:
        const RegexOpt* resolveNamedRegexOpt(const NamedRegexOpt* opt);
        const RegexOpt* resolveEnvRegexOpt(const EnvRegexOpt* opt);

        const RegexOpt* resolveAnyOfOpt(const AnyOfOpt* opt);

    public:
        NameResolverState resolverState;
        fnNameResolver nameResolverFn;

        std::map<std::string, const RegexOpt*> namedRegexes;
        std::map<std::string, const RegexOpt*>* envRegexes;

        std::vector<RegexCompileError> errors;
        std::vector<std::string> pending_resolves;

        RegexResolver(NameResolverState resolverState, fnNameResolver nameResolverFn, std::map<std::string, const RegexOpt*> namedRegexes, std::map<std::string, const RegexOpt*>* envRegexes) : resolverState(resolverState), nameResolverFn(nameResolverFn), errors(), namedRegexes(namedRegexes), envRegexes(envRegexes) 
        {
            for(auto ii = this->namedRegexes.begin(); ii != this->namedRegexes.end(); ++ii)
            {
                if(ii->second->tag == RegexOptTag::Negate || ii->second->tag == RegexOptTag::AllOf)
                {
                    this->errors.push_back(RegexCompileError(u8"Negation and AllOf regexes cannot be used in substitution patterns"));
                }
            }

            if(this->envRegexes)
            {
                for(auto ii = this->envRegexes->begin(); ii != this->envRegexes->end(); ++ii)
                {
                    if(ii->second->tag == RegexOptTag::Negate || ii->second->tag == RegexOptTag::AllOf)
                    {
                        this->errors.push_back(RegexCompileError(u8"Negation and AllOf regexes cannot be used in substitution patterns"));
                    }
                }
            }
        }
        
        ~RegexResolver() = default;

        const RegexOpt* resolve(const RegexOpt* opt);
    };

    class RegexCompiler
    {
    private:
        static StateID compileLiteralOpt(StateID follows, std::vector<NFAOpt*>& states, const LiteralOpt* opt);
        static StateID compileCharRangeOpt(StateID follows, std::vector<NFAOpt*>& states, const CharRangeOpt* opt);
        static StateID compileCharClassDotOpt(StateID follows, std::vector<NFAOpt*>& states, const CharClassDotOpt* opt);
        static StateID compileStarRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const StarRepeatOpt* opt);
        static StateID compilePlusRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const PlusRepeatOpt* opt);
        static StateID compileRangeRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const RangeRepeatOpt* opt);
        static StateID compileOptionalOpt(StateID follows, std::vector<NFAOpt*>& states, const OptionalOpt* opt);
        static StateID compileAnyOfOpt(StateID follows, std::vector<NFAOpt*>& states, const AnyOfOpt* opt);
        static StateID compileSequenceOpt(StateID follows, std::vector<NFAOpt*>& states, const SequenceOpt* opt);

        static StateID compileOpt(StateID follows, std::vector<NFAOpt*>& states, const RegexOpt* opt);

        static StateID reverseCompileLiteralOpt(StateID follows, std::vector<NFAOpt*>& states, const LiteralOpt* opt);
        static StateID reverseCompileCharRangeOpt(StateID follows, std::vector<NFAOpt*>& states, const CharRangeOpt* opt);
        static StateID reverseCompileCharClassDotOpt(StateID follows, std::vector<NFAOpt*>& states, const CharClassDotOpt* opt);
        static StateID reverseCompileStarRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const StarRepeatOpt* opt);
        static StateID reverseCompilePlusRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const PlusRepeatOpt* opt);
        static StateID reverseCompileRangeRepeatOpt(StateID follows, std::vector<NFAOpt*>& states, const RangeRepeatOpt* opt);
        static StateID reverseCompileOptionalOpt(StateID follows, std::vector<NFAOpt*>& states, const OptionalOpt* opt);
        static StateID reverseCompileAnyOfOpt(StateID follows, std::vector<NFAOpt*>& states, const AnyOfOpt* opt);
        static StateID reverseCompileSequenceOpt(StateID follows, std::vector<NFAOpt*>& states, const SequenceOpt* opt);

        static StateID reverseCompileOpt(StateID follows, std::vector<NFAOpt*>& states, const RegexOpt* opt);

    public:
        RegexCompiler() { ; }
        ~RegexCompiler() = default;

        xxxx;
        static NFAMachine* compile(const RegexOpt* opt);
        static NFAMachine* compileReverse(const RegexOpt* opt);

        xxxx;
        static REExecutorUnicode* compileUnicodeRegexToExecutor(const RegexOpt* opt, const std::map<std::string, const RegexOpt*>& namedRegexes, NameResolverState resolverState, fnNameResolver nameResolverFn);
        static REExecutorUnicode* compileACIIRegexToExecutor(const RegexOpt* opt, const std::map<std::string, const RegexOpt*>& namedRegexes, NameResolverState resolverState, fnNameResolver nameResolverFn);

        static REExecutorUnicode* compilePathRegexToExecutor(const RegexOpt* opt, const std::map<std::string, const RegexOpt*>& namedRegexes, NameResolverState resolverState, fnNameResolver nameResolverFn);
        static REExecutorUnicode* compileResourceRegexToExecutor(const RegexOpt* opt, const std::map<std::string, const RegexOpt*>& namedRegexes, const std::map<std::string, const RegexOpt*>* envRegexes, NameResolverState resolverState, fnNameResolver nameResolverFn);
    };
}
