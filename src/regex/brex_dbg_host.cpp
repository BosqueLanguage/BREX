#include "brex.h"
#include "brex_parser.h"
#include "brex_compiler.h"
#include "brex_system.h"

#include <iostream>
#include <fstream>

bool dbg_tryParseIntoNameMap(const std::string &name, const std::u8string &str, std::map<std::string, const brex::RegexOpt *> &nmap)
{
    auto pr = brex::RegexParser::parseUnicodeRegex(str, true);
    if (!pr.first.has_value() || !pr.second.empty())
    {
        return false;
    }

    if (pr.first.value()->preanchor != nullptr || pr.first.value()->postanchor != nullptr)
    {
        return false;
    }

    if (pr.first.value()->re->tag != brex::RegexComponentTag::Single)
    {
        return false;
    }

    auto sre = static_cast<const brex::RegexSingleComponent *>(pr.first.value()->re);
    if (sre->entry.isFrontCheck || sre->entry.isBackCheck || sre->entry.isNegated)
    {
        return false;
    }

    return nmap.insert({name, sre->entry.opt}).second;
}

std::string dbg_fnresolve(const std::string &name, brex::NameResolverState s)
{
    return name;
}

int main(int argc, char **argv)
{
    brex::ExecutorError dummyerr;
    std::vector<brex::RegexCompileError> compileerror;
    std::map<std::string, const brex::RegexOpt*> nmap;
    std::map<std::string, const brex::LiteralOpt*> emap;

    auto apr = brex::RegexParser::parseCRegex("/[0-9]/c", false);

    auto aexecutor = brex::RegexCompiler::compileCRegexToExecutor(apr.first.value(), nmap, emap, false, nullptr, dbg_fnresolve, compileerror);

    auto aastr = brex::CString("1");
    auto aaccepts = aexecutor->test(&aastr, dummyerr);
    if(aaccepts) {
        std::cout << "Accepted Chars" << std::endl;
    }
    else {
        std::cout << "Rejected Chars" << std::endl;
    }

    /*
    bool ok = true;
    ok &= dbg_tryParseIntoNameMap("FilenameFragment", u8"/[a-zA-Z0-9_]+/", nmap);
    if(!ok) {
        std::cout << "Failed to parse into name map" << std::endl;
        return 1;
    }

    auto upr = brex::RegexParser::parseUnicodeRegex(u8"/\"mark_\"^<${FilenameFragment}>$!(\".tmp\" | \".scratch\")/", true);
    if(!upr.first.has_value() || !upr.second.empty()) {
        for(auto iter = upr.second.begin(); iter != upr.second.end(); ++iter) {
            std::cout << std::string(iter->msg.cbegin(), iter->msg.cend()) << " ";
        }
        std::cout << std::endl;
        return 1;
    }

    auto uexecutor = brex::RegexCompiler::compileUnicodeRegexToExecutor(upr.first.value(), nmap, emap, false, nullptr, dbg_fnresolve, compileerror);

    auto ustr = brex::UnicodeString(u8"mark_a.tmp");
    auto uaccepts = uexecutor->test(&ustr, 5, 5, dummyerr);
    if(uaccepts) {
        std::cout << "Accepted Unicode" << std::endl;
    }
    else {
        std::cout << "Rejected Unicode" << std::endl;
    }
    */

    /*
    auto str = std::u8string(u8"%");
    auto res = brex::unescapeUnicodeStringLiteralInclMultiline((uint8_t*)str.c_str(), str.size());

    auto xstr = res.second.value();
    std::cout << std::string(xstr.cbegin(), xstr.cend()) << std::endl;
    */

/*
    brex::RENSInfo ninfo = {
        {"Main",
         {}},
        {{"Foo",
          u8"/${Baz}/"

         },
         {"Baz",
          u8"/${Foo}/"

         }}};

    std::vector<brex::RENSInfo> ninfos = {ninfo};
    std::vector<std::u8string> errors;
    auto sys = brex::ReSystem::processSystem(ninfos, errors);

    for (size_t i = 0; i < errors.size(); ++i)
    {
        std::cout << "Error: " << std::string(errors[i].cbegin(), errors[i].cend()) << std::endl;
    }

    auto executor = sys.getUnicodeRE("Other::Baz");
    brex::UnicodeString ustr = u8"abc-xyz";
    brex::UnicodeString estr = u8"abc-123";
    brex::ExecutorError err = brex::ExecutorError::Ok;

    auto okr = executor->test(&ustr, err);
    auto failr = !executor->test(&estr, err);

    std::cout << "Ok: " << okr << " Fail: " << failr << std::endl;
*/
    return 0;
}