#define once

#include "../common.h"

#include "brex.h"
#include "brex_executor.h"

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

        const std::map<std::string, const RegexOpt*> namedRegexes;
        const std::map<std::string, const RegexOpt*>* envRegexes;

        std::vector<RegexCompileError> errors;
        std::vector<std::string> pending_resolves;

        RegexResolver(NameResolverState resolverState, fnNameResolver nameResolverFn, const std::map<std::string, const RegexOpt*>& namedRegexes, const std::map<std::string, const RegexOpt*>* envRegexes) : resolverState(resolverState), nameResolverFn(nameResolverFn), namedRegexes(namedRegexes), envRegexes(envRegexes), errors(), pending_resolves() { ; }
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

        std::vector<RegexCompileError> errors;

        template <typename TStr, typename TIter>
        std::optional<SingleCheckREInfo<TStr, TIter>> compileSingleTopLevelEntry(const RegexToplevelEntry& tlre, const std::map<std::string, const RegexOpt*>& namedRegexes, const std::map<std::string, const RegexOpt*>* envRegexes, NameResolverState resolverState, fnNameResolver nameResolverFn)
        {
            RegexResolver resolver(resolverState, nameResolverFn, namedRegexes, envRegexes);
            auto fullre = resolver.resolve(tlre.opt);
            if(resolver.errors.size() > 0) {
                std::copy(resolver.errors.cbegin(), resolver.errors.cend(), std::back_inserter(this->errors));
                return std::nullopt;
            }

            std::vector<NFAOpt*> nfastates_forward = { new NFAOptAccept(0) };
            auto nfastart_forward = RegexCompiler::compileOpt(0, nfastates_forward, fullre);
            NFAMachine* nfaforward = new NFAMachine(nfastart_forward, 0, nfastates_forward);

            std::vector<NFAOpt*> nfastates_reverse = { new NFAOptAccept(0) };
            auto nfastart_reverse = RegexCompiler::reverseCompileOpt(0, nfastates_reverse, fullre);
            NFAMachine* nfareverse = new NFAMachine(nfastart_reverse, 0, nfastates_reverse);
            
            NFAExecutor<TStr, TIter> nn(nfaforward, nfareverse);

            SingleCheckREInfo<TStr, TIter> scc(nn, tlre.isNegated, tlre.isFrontCheck, tlre.isBackCheck);
            return std::make_optional(scc);
        }
        

    public:
        RegexCompiler() : errors() { ; }
        ~RegexCompiler() = default;


        template <typename TStr, typename TIter>
        static REExecutor<TStr, TIter>* compileRegexToExecutor(const Regex* re, const std::map<std::string, const RegexOpt*>& namedRegexes, const std::map<std::string, const RegexOpt*>* envRegexes, NameResolverState resolverState, fnNameResolver nameResolverFn, std::vector<RegexCompileError>& errinfo)
        {
            RegexCompiler rcc;

            std::vector<SingleCheckREInfo<TStr, TIter>> checks;
            if(re->allopt.isEmpty()) {
                auto cv = rcc.compileSingleTopLevelEntry<TStr, TIter>(re->sre, namedRegexes, envRegexes, resolverState, nameResolverFn);
                if(cv.has_value()) {
                    checks.push_back(cv.value());
                }
                else {
                    std::copy(rcc.errors.cbegin(), rcc.errors.cend(), std::back_inserter(errinfo));
                    return nullptr;
                }
            }
            else {
                for(auto ii = re->allopt.musts.cbegin(); ii != re->allopt.musts.cend(); ++ii) {
                    auto cv = rcc.compileSingleTopLevelEntry<TStr, TIter>(*ii, namedRegexes, envRegexes, resolverState, nameResolverFn);
                    if(cv.has_value()) {
                        checks.push_back(cv.value());
                    }
                    else {
                        std::copy(rcc.errors.cbegin(), rcc.errors.cend(), std::back_inserter(errinfo));
                        return nullptr;
                    }
                }
            }
            
            return new REExecutor<TStr, TIter>(checks, re->isContainsable, re->isMatchable);
        }
        
        static UnicodeRegexExecutor* compileUnicodeRegexToExecutor(const Regex* re, const std::map<std::string, const RegexOpt*>& namedRegexes, NameResolverState resolverState, fnNameResolver nameResolverFn, std::vector<RegexCompileError>& errinfo)
        {
            if(re->ctag != RegexCharInfoTag::Unicode) {
                errinfo.push_back(RegexCompileError(u8"Expected a Unicode regex"));
                return nullptr;
            }

            if(re->rtag != RegexKindTag::Std) {
                errinfo.push_back(RegexCompileError(u8"Expected a standard regex"));
                return nullptr;
            }

            return compileRegexToExecutor<UnicodeString, UnicodeRegexIterator>(re, namedRegexes, nullptr, resolverState, nameResolverFn, errinfo);
        }

        static ASCIIRegexExecutor* compileASCIIRegexToExecutor(const Regex* re, const std::map<std::string, const RegexOpt*>& namedRegexes, NameResolverState resolverState, fnNameResolver nameResolverFn, std::vector<RegexCompileError>& errinfo)
        {
            if(re->ctag != RegexCharInfoTag::ASCII) {
                errinfo.push_back(RegexCompileError(u8"Expected an ASCII regex"));
                return nullptr;
            }

            if(re->rtag != RegexKindTag::Std) {
                errinfo.push_back(RegexCompileError(u8"Expected a standard regex"));
                return nullptr;
            }

            return compileRegexToExecutor<ASCIIString, ASCIIRegexIterator>(re, namedRegexes, nullptr, resolverState, nameResolverFn, errinfo);
        }

        static ASCIIRegexExecutor* compilePathRegexToExecutor(const Regex* re, const std::map<std::string, const RegexOpt*>& namedRegexes, NameResolverState resolverState, fnNameResolver nameResolverFn, std::vector<RegexCompileError>& errinfo)
        {
            if(re->ctag != RegexCharInfoTag::ASCII) {
                errinfo.push_back(RegexCompileError(u8"Expected an ASCII regex"));
                return nullptr;
            }

            if(re->rtag != RegexKindTag::Std) {
                errinfo.push_back(RegexCompileError(u8"Expected a standard regex"));
                return nullptr;
            }

            return compileRegexToExecutor<ASCIIString, ASCIIRegexIterator>(re, namedRegexes, nullptr, resolverState, nameResolverFn, errinfo);
        }

        static ASCIIRegexExecutor* compileResourceRegexToExecutor(const Regex* re, const std::map<std::string, const RegexOpt*>& namedRegexes, const std::map<std::string, const RegexOpt*>* envRegexes, NameResolverState resolverState, fnNameResolver nameResolverFn, std::vector<RegexCompileError>& errinfo)
        {
            if(re->ctag != RegexCharInfoTag::ASCII) {
                errinfo.push_back(RegexCompileError(u8"Expected an ASCII regex"));
                return nullptr;
            }

            if(re->rtag != RegexKindTag::Std) {
                errinfo.push_back(RegexCompileError(u8"Expected a standard regex"));
                return nullptr;
            }

            return compileRegexToExecutor<ASCIIString, ASCIIRegexIterator>(re, namedRegexes, envRegexes, resolverState, nameResolverFn, errinfo);
        }
    };
}
