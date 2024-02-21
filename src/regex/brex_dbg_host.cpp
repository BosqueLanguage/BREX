#include "brex.h"
#include "brex_parser.h"
#include "brex_compiler.h"

#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
    brex::ExecutorError dummyerr;
    
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

    auto upr = brex::RegexParser::parseUnicodeRegex(u8"/\"abc\"/");
    if(!upr.first.has_value() || !upr.second.empty()) {
        for(auto iter = upr.second.begin(); iter != upr.second.end(); ++iter) {
            std::cout << std::string(iter->msg.cbegin(), iter->msg.cend()) << " ";
        }
        std::cout << std::endl;
        return 1;
    }

    std::map<std::string, const brex::RegexOpt*> uemptymap;
    std::vector<brex::RegexCompileError> ucompileerror;
    auto uexecutor = brex::RegexCompiler::compileUnicodeRegexToExecutor(upr.first.value(), uemptymap, nullptr, nullptr, ucompileerror);

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