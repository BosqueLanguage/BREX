#include "glob.h"
#include "glob_parser.h"

#include <iostream>

int main(int argc, char **argv) {
    std::u8string str = u8"/a/lot/${of}/(chars|sy(mb|mb)ols)/to/pa*r_?/**/with_more!";
    auto glob = brex::GlobParser::parseGlob((uint8_t*)str.c_str(), str.length(), false);
    std::cout << glob->toBSQStandard() << std::endl;
}