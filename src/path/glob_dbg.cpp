#include "glob.h"
#include "glob_parser.h"

#include <iostream>

int main(int argc, char **argv) {
    std::u8string str = u8"worms";
    auto glob = brex::GlobParser::parseGlob((uint8_t*)str.c_str(), str.length(), false);
    std::cout << "test!" << std::endl;
}