#pragma once

#include "../common.h"
#include "brex.h"
#include "brex_parser.h"
#include "brex_compiler.h"
#include "brex_executor.h"

namespace brex
{
    class ReNSRemapper
    {
    public:
        std::map<std::string, std::map<std::string, std::string>> nsmap;

        ReNSRemapper() {;}
        ~ReNSRemapper() {;}

        void addSingleNSMapping(const std::string& inns, const std::vector<std::pair<std::string, std::string>>& nsmapping)
        {
            std::for_each(nsmapping.begin(), nsmapping.end(), [this, &inns](const std::pair<std::string, std::string>& p) {
                this->nsmap[inns].insert(p);
            });
        }

        void addAllNSMappings(const std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>>& nsmappings)
        {
            std::for_each(nsmappings.begin(), nsmappings.end(), [this](const std::pair<std::string, std::vector<std::pair<std::string, std::string>>>& p) {
                this->addSingleNSMapping(p.first, p.second);
            });
        }

        std::optional<std::string> resolveNamespace(const std::string& inns, const std::string& ns) const
        {
            auto nsiter = this->nsmap.find(inns);
            if(nsiter == this->nsmap.end()) {
                return std::nullopt;
            }

            auto nameiter = nsiter->second.find(ns);
            if(nameiter == nsiter->second.end()) {
                return std::make_optional(ns);
            }

            return std::make_optional(nameiter->second);
        }
    };

    class ReSystemEntry
    {
    public:
        const std::string ns;
        const std::string name;
        const std::string fullname;

        Regex* re;
        std::vector<std::string> deps;

        ReSystemEntry(const std::string& ns, const std::string& name, const std::string& fullname): ns(ns), name(name), fullname(fullname) {;}
        virtual ~ReSystemEntry() {;}

        virtual std::optional<std::u8string> compileRegex() = 0;

        bool computeDeps(const ReNSRemapper& remapper)
        {
            std::set<std::string> constnames;
            std::set<std::string> envnames;
            bool envRequired = brex::RegexCompiler::gatherNamedRegexKeys(constnames, envnames, this->re);

            if(envRequired) {
                return false;
            }

            bool failed = false;
            std::for_each(constnames.begin(), constnames.end(), [this, &remapper, &failed](const std::string& cname) {
                auto biter = cname.find_last_of("::");

                std::string rname("[error]");
                if(biter == std::string::npos) {
                    rname = this->ns + "::" + cname;
                }
                else {
                    auto nsname = cname.substr(0, biter);
                    auto ename = cname.substr(biter + 1);

                    auto resolvedns = remapper.resolveNamespace(this->ns, nsname);
                    if(!resolvedns.has_value()) {
                        failed = true;
                    }
                    else {
                        rname = resolvedns.value() + "::" + ename;
                    }
                }

                if(std::find(this->deps.begin(), this->deps.end(), rname) == this->deps.end()) {
                    this->deps.push_back(rname);
                }   
            });

            return !failed;
        }
    };

    class ReSystemUnicodeEntry : public ReSystemEntry
    {
    public:
        const std::u8string restr;
        UnicodeRegexExecutor* executor;

        ReSystemUnicodeEntry(const std::string& ns, const std::string& name, const std::string& fullname, const std::u8string& restr): ReSystemEntry(ns, name, fullname), restr(restr) {;}
        ~ReSystemUnicodeEntry() {;}

        std::optional<std::u8string> compileRegex() override
        {
            auto pr = RegexParser::parseUnicodeRegex(this->restr, false);
            if(!pr.first.has_value() || !pr.second.empty()) {
                return !pr.second.empty() ? std::make_optional(pr.second[0].msg) : std::make_optional(u8"Invalid Regex");
            }

            this->re = pr.first.value();
            return std::nullopt;
        }
    };

    class ReSystemCEntry : public ReSystemEntry
    {
    public:
        const std::string restr;
        CRegexExecutor* executor;

        ReSystemCEntry(const std::string& ns, const std::string& name, const std::string& fullname, const std::string& restr): ReSystemEntry(ns, name, fullname), restr(restr) {;}
        ~ReSystemCEntry() {;}

        std::optional<std::u8string> compileRegex() override
        {
            auto pr = RegexParser::parseCRegex(this->restr, false);
            if(!pr.first.has_value() || !pr.second.empty()) {
                return !pr.second.empty() ? std::make_optional(pr.second[0].msg) : std::make_optional(u8"Invalid Regex");
            }

            this->re = pr.first.value();
            return std::nullopt;
        }
    };

    class ReSystem
    {
    public:
        ReNSRemapper remapper;
        std::vector<ReSystemEntry*> entries;
        std::map<std::string, std::vector<ReSystemEntry*>> depmap;

        ReSystem() {;}
        ~ReSystem() {;}

        void loadUnicodeEntry(const std::string& ns, const std::string& name, const std::string& fullname, const std::u8string& restr)
        {
            auto entry = new ReSystemUnicodeEntry(ns, name, fullname, restr);
            this->entries.push_back(entry);
        }

        void loadCStringEntry(const std::string& ns, const std::string& name, const std::string& fullname, const std::string& restr)
        {
            auto entry = new ReSystemCEntry(ns, name, fullname, restr);
            this->entries.push_back(entry);
        }

        void computeDependencies(std::vector<std::u8string>& errors)
        {
            std::for_each(this->entries.begin(), this->entries.end(), [this, &errors](ReSystemEntry* entry) {
                auto err = entry->compileRegex();
                if(err.has_value()) {
                    errors.push_back(err.value());
                }
                else {
                    auto depsok = entry->computeDeps(remapper);
                    
                    if(!depsok) {
                        errors.push_back(u8"Failed to compute dependencies for " + std::u8string(entry->fullname.cbegin(), entry->fullname.cend()));
                    }
                }
            });

            std::for_each(this->entries.begin(), this->entries.end(), [this, &errors](ReSystemEntry* entry) {
                if(this->depmap.find(entry->fullname) == this->depmap.end()) {
                    this->depmap.insert({ entry->fullname, {} });
                }

                std::for_each(entry->deps.begin(), entry->deps.end(), [this, entry, &errors](const std::string& dep) {
                    auto ii = std::find_if(this->entries.begin(), this->entries.end(), [dep](ReSystemEntry* e) {
                        return e->fullname == dep;
                    });

                    if(ii != this->entries.end()) {
                        this->depmap[entry->fullname].push_back(*ii);
                    }
                    else {
                        errors.push_back(u8"Failed to find dependency " + std::u8string(dep.cbegin(), dep.cend()) + u8" for " + std::u8string(entry->fullname.cbegin(), entry->fullname.cend()));
                    }
                });
            });
        }

        bool generateSingleExecutable(ReSystemEntry* entry, std::vector<std::u8string>& errors, std::vector<std::string>& pending)
        {
            if(std::find(pending.begin(), pending.end(), entry->fullname) != pending.end()) {
                return false;
            }

            pending.push_back(entry->fullname);
 
            std::map<std::string, const RegexOpt*> namedRegexes;
            std::vector<ReSystemEntry*>& deps = this->depmap.find(entry->fullname)->second;
            auto recok = std::reduce(deps.begin(), deps.end(), true, [this, &errors, &pending, &namedRegexes](bool acc, ReSystemEntry* dep) {
                auto ok = this->generateSingleExecutable(dep, errors, pending);
                if(!ok) {
                    return false;
                }
                else {
                    namedRegexes.insert({ dep->name, dep->re->re });
                    return acc;
                }
            });

            pending.pop_back();
            if(!recok) {
                return false;
            }

xxxx;
            if(entry->re->ctag == RegexCharInfoTag::Unicode) {
                auto uentry = dynamic_cast<ReSystemUnicodeEntry*>(entry);
                auto executor = RegexCompiler::compileUnicodeRegexToExecutor(uentry->re, {}, {}, false, nullptr, nullptr, errors);
                if(executor == nullptr) {
                    errors.push_back(u8"Failed to compile executor for " + std::u8string(entry->fullname.cbegin(), entry->fullname.cend()));
                    return;
                }

                uentry->executor = executor;
            }
            else {
                auto centry = dynamic_cast<ReSystemCEntry*>(entry);
                auto executor = RegexCompiler::compileCRegexToExecutor(centry->re, {}, {}, false, nullptr, nullptr, errors);
                if(executor == nullptr) {
                    errors.push_back(u8"Failed to compile executor for " + std::u8string(entry->fullname.cbegin(), entry->fullname.cend()));
                    return;
                }

                centry->executor = executor;
            }

           
            return true;
        }

        void generateExecutables(std::vector<std::u8string>& errors) {
            xxxx;            
        }
    };

}
