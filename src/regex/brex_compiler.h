#define once

#include "common.h"
#include "brex.h"

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
    public:
        RegexCompiler() { ; }
        ~RegexCompiler() = default;

        static NFAMachine* compile(const RegexOpt* opt);
        static NFAMachine* compileReverse(const RegexOpt* opt);
    };
}
