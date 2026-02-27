#include "glob.h"
#include "glob_parser.h"
#include "glob_compiler.h"
#include "glob_executor.h"

#include <iostream>

int main(int argc, char **argv) {
    std::u8string str = u8"w(o|y)rm";
    auto glob = brex::GlobParser::parseGlob((uint8_t*)str.c_str(), str.length(), false);
    auto compiled_glob = brex::GlobCompiler::compile(glob);
    auto machine = compiled_glob;
    std::cout << (int) brex::match(machine, "wyrm") << std::endl;
}