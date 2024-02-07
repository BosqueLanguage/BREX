#include "brex.h"
#include "brex_parser.h"

#include <iostream>
#include <fstream>

void useage(std::optional<std::string> msg)
{
    if(msg.has_value()) {
        std::cout << msg.value() << std::endl;
    }

    std::cout << "Usage: brex [-n -c -x -s -h] <regex> [input]" << std::endl;
    std::cout << "  <regex> - The regex to match against" << std::endl;
    std::cout << "  [input] - The input to match" << std::endl;
    std::cout << std::endl;
    std::cout << "  -n - Include line numbers in the output" << std::endl;
    std::cout << "  -c - Only report the match count" << std::endl;
    std::cout << "  -x - Test whole lines instead of searching for match" << std::endl;
    std::cout << "  -s - Read input from stdin" << std::endl;
    std::cout << "  -h - Print this help message" << std::endl;
    std::exit(1);
}

enum class Flags
{
    LineNumbers,
    Count,
    WholeLines
};


bool processCmdLine(int argc, char** argv, char** re, char** file, std::set<Flags>& flags, std::string& helpmsg)
{
    *re = nullptr;
    *file = nullptr;
    flags.clear();
    helpmsg = "";

    bool isstdin = false;
    for(int i = 0; i < argc; ++i) {
        std::string arg = argv[i];

        if(arg == "-n") {
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
        else {
            if(*re == nullptr) {
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

    if(!isstdin && *file == nullptr) {
        helpmsg = "No input file specified";
        return false;
    }

    if(flags.contains(Flags::Count) && flags.contains(Flags::LineNumbers)) {
        helpmsg = "Cannot specify -c and -n together";
        return false;
    }

    return true;
}

std::string loadText(const char* file)
{
    try {
        if(file == nullptr) {
            std::cin >> std::noskipws;
            std::string str((std::istream_iterator<char>(std::cin)), std::istream_iterator<char>());
            return str;
        }
        else {
            std::ifstream istr(file);
            std::string str((std::istreambuf_iterator<char>(istr)), std::istreambuf_iterator<char>());
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

    std::u8string ure(re, re + strlen(re));
    auto pr = brex::RegexParser::parseUnicodeRegex(ure);
    if(!pr.first.has_value() || !pr.second.empty()) {
        std::cout << "Invalid regex ";
        for(auto iter = pr.second.begin(); iter != pr.second.end(); ++iter) {
            std::cout << std::string(iter->msg.cbegin(), iter->msg.cend()) << " ";
        }
        std::cout << std::endl;

        std::cout << "See the BREX documentation for more information -- https://github.com/BosqueLanguage/BREX" << std::endl;
        return 1;
    }



    return 0;
}