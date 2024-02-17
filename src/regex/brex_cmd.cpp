#include "brex.h"
#include "brex_parser.h"
#include "brex_compiler.h"

#include <iostream>
#include <fstream>

void useage(std::optional<std::string> msg)
{
    if(msg.has_value()) {
        std::cout << msg.value() << std::endl;
    }

    std::cout << "Usage: brex [-n -c -x -s -t -h] <regex> [input]" << std::endl;
    std::cout << "  <regex> - The regex to match against" << std::endl;
    std::cout << "  [input] - The input to match" << std::endl;
    std::cout << std::endl;
    std::cout << "  -a - Test if the regex accepts the input" << std::endl;
    std::cout << "  -n - Include line numbers in the output" << std::endl;
    std::cout << "  -c - Only report the match count" << std::endl;
    std::cout << "  -x - Test whole lines instead of searching for match" << std::endl;
    std::cout << "  -s - Read input from stdin" << std::endl;
    std::cout << "  -l - Treat the input as a literal double quoted string \"...\"" << std::endl;
    std::cout << "  -h - Print this help message" << std::endl;
    std::exit(1);
}

enum class Flags
{
    Accepts,
    LineNumbers,
    Count,
    WholeLines,
    InputLiteral,
};

bool processCmdLine(int argc, char** argv, char** re, char** file, std::set<Flags>& flags, std::string& helpmsg)
{
    *re = nullptr;
    *file = nullptr;
    flags.clear();
    helpmsg = "";

    bool isstdin = false;
    for(int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if(arg == "-a") {
            flags.insert(Flags::Accepts);
        }
        else if(arg == "-n") {
            flags.insert(Flags::LineNumbers);
        }
        else if(arg == "-c") {
            flags.insert(Flags::Count);
        }
        else if(arg == "-x") {
            flags.insert(Flags::WholeLines);
        }
        else if(arg == "-h") {
            return false;
        }
        else if(arg == "-s") {
            isstdin = true;
        }
        else if(arg == "-l") {
            flags.insert(Flags::InputLiteral);
        }
        else {
            if(arg.starts_with("-")) {
                helpmsg = "Unknown argument: " + arg;
                return false;
            }
            else if(*re == nullptr) {
                *re = argv[i];
            }
            else if(*file == nullptr) {
                if(isstdin) {
                    helpmsg = "Cannot specify input file when reading from stdin";
                    return false;
                }
                *file = argv[i];
            }
            else {
                helpmsg = "Unknown argument: " + arg;
                return false;
            }
        }
    }

    if(*re == nullptr) {
        helpmsg = "No regex specified";
        return false;
    }

    if(isstdin && flags.contains(Flags::InputLiteral)) {
        helpmsg = "Cannot specify -t with -s";
        return false;
    }

    if(!isstdin && *file == nullptr) {
        helpmsg = "No input file specified";
        return false;
    }

    if(flags.contains(Flags::Accepts) && (flags.contains(Flags::LineNumbers) || flags.contains(Flags::Count) || flags.contains(Flags::WholeLines))) {
        helpmsg = "Cannot specify -a with other flags (except -s)";
        return false;
    }

    if(flags.contains(Flags::Count) && flags.contains(Flags::LineNumbers)) {
        helpmsg = "Cannot specify -c and -n together";
        return false;
    }

    return true;
}

std::u8string stdInWSTrim(const std::u8string& str)
{
    auto iter = str.begin();
    while(iter != str.end() && std::isspace(*iter)) {
        ++iter;
    }

    auto riter = str.rbegin();
    while(riter != str.rend() && std::isspace(*riter)) {
        ++riter;
    }

    return std::u8string(iter, riter.base());
}

std::u8string loadText(const char* file, bool isliteralin)
{
    try {
        if(file == nullptr) {
            std::cin >> std::noskipws;
            std::u8string str((std::istream_iterator<char>(std::cin)), std::istream_iterator<char>());
            return stdInWSTrim(str);
        }
        else if(isliteralin) {
            std::string ff(file);
            if(ff.starts_with('"') && ff.ends_with('"')) {
                return std::u8string(ff.cbegin() + 1, ff.cend() - 1);
            }
            else {
                std::cout << "Error: input literal must be enclosed in double quotes but got: " << ff << std::endl;
                std::exit(1);
            }
        }
        else {
            std::ifstream istr(file);
            std::u8string str((std::istreambuf_iterator<char>(istr)), std::istreambuf_iterator<char>());
            return str;
        }
    }
    catch(const std::exception& e) {
        std::cout << "Error reading file: " << e.what() << std::endl;
        std::exit(1);
    }
}

int main(int argc, char** argv)
{
    char* re;
    char* file;
    std::set<Flags> flags;
    std::string helpmsg;

    if(!processCmdLine(argc, argv, &re, &file, flags, helpmsg)) {
        useage(!helpmsg.empty() ? std::optional<std::string>(helpmsg) : std::nullopt);
    }


    ////////////
    /*
    auto apr = brex::RegexParser::parseASCIIRegex("/[%a;]/");

    std::map<std::string, const brex::RegexOpt*> aemptymap;
    std::vector<brex::RegexCompileError> acompileerror;
    auto aexecutor = brex::RegexCompiler::compileASCIIRegexToExecutor(apr.first.value(), aemptymap, nullptr, nullptr, acompileerror);

    auto aastr = brex::ASCIIString("abc");
    auto aaccepts = aexecutor->test(&aastr);
    if(aaccepts) {
        std::cout << "Accepted ASCII" << std::endl;
    }
    else {
        std::cout << "Rejected ASCII" << std::endl;
    }
    */
    auto upr = brex::RegexParser::parseUnicodeRegex(u8"/[0-9] (\"5\"|\"6\")/");
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

    auto ustr = brex::UnicodeString(u8"+i");
    auto uaccepts = uexecutor->test(&ustr);
    if(uaccepts) {
        std::cout << "Accepted Unicode" << std::endl;
    }
    else {
        std::cout << "Rejected Unicode" << std::endl;
    }
    return 1;
    ////////////


    std::u8string ure(re, re + strlen(re));
    auto pr = brex::RegexParser::parseUnicodeRegex(ure);
    if(!pr.first.has_value() || !pr.second.empty()) {
        std::cout << "Parse errors in regex:" << std::endl;
        for(auto iter = pr.second.begin(); iter != pr.second.end(); ++iter) {
            std::cout << std::string(iter->msg.cbegin(), iter->msg.cend()) << std::endl;
        }
        std::cout << std::endl;
        std::cout << "See the BREX documentation for more information -- https://github.com/BosqueLanguage/BREX" << std::endl;
        return 1;
    }

    std::map<std::string, const brex::RegexOpt*> emptymap;
    std::vector<brex::RegexCompileError> compileerror;
    auto executor = brex::RegexCompiler::compileUnicodeRegexToExecutor(pr.first.value(), emptymap, nullptr, nullptr, compileerror);
    if(!compileerror.empty()) {
        std::cout << "Errors compiling regex:" << std::endl;
        for(auto iter = compileerror.begin(); iter != compileerror.end(); ++iter) {
            std::cout << std::string(iter->msg.cbegin(), iter->msg.cend()) << std::endl;
        }
        std::cout << std::endl;
        return 1;
    }

    auto text = loadText(file, flags.contains(Flags::InputLiteral));
    auto uustr = brex::UnicodeString(text.cbegin(), text.cend());

    std::cout << std::string(uustr.cbegin(), uustr.cend()) << std::endl;

    if(flags.contains(Flags::Accepts)) {
        auto accepts = executor->test(&uustr);
        if(accepts) {
            std::cout << "Accepted" << std::endl;
        }
        else {
            std::cout << "Rejected" << std::endl;
        }
    }
    else {
        std::cout << "Other modes not supported yet!!!" << std::endl;
        return 1;
    }

    return 0;
}