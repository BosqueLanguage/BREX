#include "brex.h"
#include "brex_parser.h"
#include "brex_compiler.h"

#include <iostream>
#include <fstream>


bool dbg_tryParseIntoNameMap(const std::string& name, const std::u8string& str, std::map<std::string, const brex::RegexOpt*>& nmap) {
    auto pr = brex::RegexParser::parseUnicodeRegex(str);
    if(!pr.first.has_value() || !pr.second.empty()) {
        return false;
    }

    if(pr.first.value()->preanchor != nullptr || pr.first.value()->postanchor != nullptr) {
        return false;
    }

    if(pr.first.value()->re->tag != brex::RegexComponentTag::Single) {
        return false;
    }

    auto sre = static_cast<const brex::RegexSingleComponent*>(pr.first.value()->re);
    if(sre->entry.isFrontCheck || sre->entry.isBackCheck || sre->entry.isNegated) {
        return false;
    }

    return nmap.insert({ name, sre->entry.opt }).second;
}

std::string dbg_fnresolve(const std::string& name, brex::NameResolverState s) {
    return name;
}

int main(int argc, char** argv)
{
    brex::ExecutorError dummyerr;
    std::map<std::string, const brex::RegexOpt*> nmap;

    /*
    auto apr = brex::RegexParser::parseASCIIRegex("/[%a;]/");

    std::map<std::string, const brex::RegexOpt*> aemptymap;
    std::vector<brex::RegexCompileError> acompileerror;
    auto aexecutor = brex::RegexCompiler::compileASCIIRegexToExecutor(apr.first.value(), aemptymap, nullptr, nullptr, acompileerror);

    auto aastr = brex::ASCIIString("abc");
    auto aaccepts = aexecutor->test(&aastr, dummyerr);
    if(aaccepts) {
        std::cout << "Accepted ASCII" << std::endl;
    }
    else {
        std::cout << "Rejected ASCII" << std::endl;
    }
    */

    bool ok = dbg_tryParseIntoNameMap("Digit", u8"/[0-9]/", nmap);
    if(!ok) {
        std::cout << "Failed to parse into name map" << std::endl;
        return 1;
    }

    auto upr = brex::RegexParser::parseUnicodeRegex(u8"/[+-]${Digit}+/");
    if(!upr.first.has_value() || !upr.second.empty()) {
        for(auto iter = upr.second.begin(); iter != upr.second.end(); ++iter) {
            std::cout << std::string(iter->msg.cbegin(), iter->msg.cend()) << " ";
        }
        std::cout << std::endl;
        return 1;
    }

    std::vector<brex::RegexCompileError> ucompileerror;
    auto uexecutor = brex::RegexCompiler::compileUnicodeRegexToExecutor(upr.first.value(), nmap, nullptr, dbg_fnresolve, ucompileerror);

    auto ustr = brex::UnicodeString(u8"abc");
    auto uaccepts = uexecutor->test(&ustr, dummyerr);
    if(uaccepts) {
        std::cout << "Accepted Unicode" << std::endl;
    }
    else {
        std::cout << "Rejected Unicode" << std::endl;
    }


    return 0;
}