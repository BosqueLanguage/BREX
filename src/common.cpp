#include "common.h"
#include <format>

#define UTF8_ENCODING_BYTE_COUNT(B) utf8_encoding_sizes[((uint8_t)(B)) >> 4]
#define UTF8_IS_CONTINUATION_BYTE(B) (((B) & 0xC0) == 0x80)

namespace brex
{
#ifdef BREX_DEBUG
    void processAssert(const char* file, int line, const char* msg)
    {
        fprintf(stderr, "Assertion failed: %s:%d -- %s\n", file, line, msg);
        abort();
    }
#else
    void processAbort(const char* file, int line, const char* msg)
    {
        fprintf(stderr, "Abort: %s:%d -- %s\n", file, line, msg);
        abort();
    }
#endif

        size_t utf8_encoding_sizes[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};

        std::vector<std::pair<uint8_t, const char*>> s_escape_names_unicode = {
        {0, "%NUL;"},
        {1, "%SOH;"},
        {2, "%STX;"},
        {3, "%ETX;"},
        {4, "%EOT;"},
        {5, "%ENQ;"},
        {6, "%ACK;"},
        {7, "%a;"},
        {8, "%b;"},
        {9, "%t;"},
        {10, "%n;"},
        {11, "%v;"},
        {12, "%f;"},
        {13, "%r;"},
        {14, "%SO;"},
        {15, "%SI;"},
        {16, "%DLE;"},
        {17, "%DC1;"},
        {18, "%DC2;"},
        {19, "%DC3;"},
        {20, "%DC4;"},
        {21, "%NAK;"},
        {22, "%SYN;"},
        {23, "%ETB;"},
        {24, "%CAN;"},
        {25, "%EM;"},
        {26, "%SUB;"},
        {27, "%e;"},
        {28, "%FS;"},
        {29, "%GS;"},
        {30, "%RS;"},
        {31, "%US;"},
        {127, "%DEL;"},

        {32, "%space;"},
        {33, "%bang;"},
        {34, "%;"},
        {34, "%quote;"},
        {35, "%hash;"},
        {36, "%dollar;"},
        {37, "%%;"},
        {37, "%percent;"},
        {38, "%amp;"},
        {39, "%tick;"},
        {40, "%lparen;"},
        {41, "%rparen;"},
        {42, "%star;"},
        {43, "%plus;"},
        {44, "%comma;"},
        {45, "%dash;"},
        {46, "%dot;"},
        {47, "%slash;"},
        {58, "%colon;"},
        {59, "%semicolon;"},
        {60, "%langle;"},
        {61, "%equal;"},
        {62, "%rangle;"},
        {63, "%question;"},
        {64, "%at;"}, 
        {91, "%lbracket;"},
        {92, "%backslash;"},
        {93, "%rbracket;"},
        {94, "%caret;"},
        {95, "%underscore;"},
        {96, "%backtick;"},
        {123, "%lbrace;"},
        {124, "%pipe;"},
        {125, "%rbrace;"},
        {126, "%tilde;"}
    };

    std::vector<std::pair<uint8_t, const char*>> s_escape_names_char = {
        {9, "%t;"},
        {10, "%n;"},

        {32, "%space;"},
        {33, "%bang;"},
        {34, "%quote;"},
        {35, "%hash;"},
        {36, "%dollar;"},
        {37, "%%;"},
        {37, "%percent;"},
        {38, "%amp;"},
        {39, "%;"},
        {39, "%tick;"},
        {40, "%lparen;"},
        {41, "%rparen;"},
        {42, "%star;"},
        {43, "%plus;"},
        {44, "%comma;"},
        {45, "%dash;"},
        {46, "%dot;"},
        {47, "%slash;"},
        {58, "%colon;"},
        {59, "%semi;"},
        {60, "%langle;"},
        {61, "%equal;"},
        {62, "%rangle;"},
        {63, "%question;"},
        {64, "%at;"}, 
        {91, "%lbracket;"},
        {92, "%backslash;"},
        {93, "%rbracket;"},
        {94, "%caret;"},
        {95, "%underscore;"},
        {96, "%backtick;"},
        {123, "%lbrace;"},
        {124, "%pipe;"},
        {125, "%rbrace;"},
        {126, "%tilde;"}
    };

    bool isLegalCChar(uint8_t c)
    {
        if(c > 126) {
            return false;
        }
        else {
            return std::isprint(c) || (c == '\t') || (c == '\n');
        }
    }

    int64_t UnicodeRegexIterator::charCodeByteCount() const
    {
        return UTF8_ENCODING_BYTE_COUNT(this->sstr->at(this->curr));
    }

    int64_t UnicodeRegexIterator::charCodeByteCountReverse() const
    {
        int64_t count = 0;
        while(UTF8_IS_CONTINUATION_BYTE(this->sstr->at(this->curr - count))) {
            count++;
        }

        return count;
    }

    RegexChar UnicodeRegexIterator::toRegexCharCodeFromBytes() const
    {
        int64_t bytecount = UTF8_ENCODING_BYTE_COUNT(this->sstr->at(this->curr));
        if(this->curr + (bytecount - 1) > this->epos) {
            return 0;
        }

        if(bytecount == 1) {
            return (RegexChar)(this->sstr->at(this->curr));
        }
        else if(bytecount == 2) {
            return (RegexChar)(((this->sstr->at(this->curr) & 0x1F) << 6) | (this->sstr->at(this->curr + 1) & 0x3F));
        }
        else if(bytecount == 3) {
            return (RegexChar)(((this->sstr->at(this->curr) & 0x0F) << 12) | ((this->sstr->at(this->curr + 1) & 0x3F) << 6) | (this->sstr->at(this->curr + 2) & 0x3F));
        }
        else {
            return (RegexChar)(((this->sstr->at(this->curr) & 0x07) << 18) | ((this->sstr->at(this->curr + 1) & 0x3F) << 12) | ((this->sstr->at(this->curr + 2) & 0x3F) << 6) | (this->sstr->at(this->curr + 3) & 0x3F));
        }
    } 

    std::string processRegexCharToBsqStandard(RegexChar c)
    {
        if(c != U'%' && c != U'"' && c != U'\'' && c != U'[' && c != U']' && c != U'/' && c != U'\\' && std::isprint(c)) {
            return std::string{(char)c};
        }
        else {
            auto hh = std::format("{:x}", c);
            return "%x" + hh + ";";
        }
    }

    std::string processRegexCharsToBsqStandard(const std::vector<RegexChar>& sv)
    {
        return std::accumulate(sv.cbegin(), sv.cend(), std::string{}, [](const std::string& acc, RegexChar c) {
            return acc + processRegexCharToBsqStandard(c);
        });
    }

    std::string processRegexCharToSMT(RegexChar c)
    {
        if(c != U'%' && c != U'"' && c != U'\'' && c != U'[' && c != U']' && c != U'/' && c != U'\\' && std::isprint(c)) {
            return std::string{(char)c};
        }
        else {
            auto hh = std::format("{:x}", c);
            return "\\u{" + hh + "}";
        }
    }

    std::string processRegexCharsToSMT(const std::vector<RegexChar>& sv)
    {
        return std::accumulate(sv.cbegin(), sv.cend(), std::string{}, [](const std::string& acc, RegexChar c) {
            return acc + processRegexCharToSMT(c);
        });
    }

    size_t charCodeByteCount(const uint8_t* buff)
    {
        return UTF8_ENCODING_BYTE_COUNT(*buff);
    }

    RegexChar toRegexCharCodeFromBytes(const uint8_t* buff, size_t length)
    {
        auto bytecount = UTF8_ENCODING_BYTE_COUNT(*buff);
        if(length < bytecount) {
            return 0;
        }

        if(bytecount == 1) {
            return (RegexChar)(*buff);
        }
        else if(bytecount == 2) {
            return (RegexChar)(((*buff & 0x1F) << 6) | (*(buff + 1) & 0x3F));
        }
        else if(bytecount == 3) {
            return (RegexChar)(((*buff & 0x0F) << 12) | ((*(buff + 1) & 0x3F) << 6) | (*(buff + 2) & 0x3F));
        }
        else {
            return (RegexChar)(((*buff & 0x07) << 18) | ((*(buff + 1) & 0x3F) << 12) | ((*(buff + 2) & 0x3F) << 6) | (*(buff + 3) & 0x3F));
        }
    }

    bool isHexEscapePrefix(const uint8_t* s, const uint8_t* e)
    {
        return std::distance(s, e) > 3 && *s == '%' && *(s + 1) == 'x' && std::isxdigit(*(s + 2));
    }

    std::optional<RegexChar> decodeHexEscapeAsRegex(const uint8_t* s, const uint8_t* e, bool ischar)
    {
        size_t ccount = std::distance(s, e);

        if(ischar) {
            //1-2 digits and a %x...;
            if(ccount < 4 || 5 < ccount) {
                return std::nullopt;
            }
        }
        else {
            //1-6 digits and a %x...;
            if(ccount < 4 || 9 < ccount) {
                return std::nullopt;
            }
        }

        //leading %x
        if(*(s + 1) != 'x') {
            return std::nullopt;
        }

        if(!std::all_of(s + 2, e - 1, [](uint8_t c) { return std::isxdigit(c); })) {
            return std::nullopt;
        }

        uint32_t cval = (uint32_t)std::strtoull((const char*)(s + 2), nullptr, 16);
        if(errno == ERANGE) {
            return std::nullopt;
        }

        if(!ischar) {
            if(cval > 0x10FFFF) {
                return std::nullopt;
            }
        }
        else {
            if(!isLegalCChar(cval)) {
                return std::nullopt;
            }
        }

        return std::make_optional(cval);
    }

    std::optional<UnicodeString> decodeHexEscapeAsUnicode(const uint8_t* s, const uint8_t* e)
    {
        size_t ccount = std::distance(s, e);

        //1-6 digits and a %;
        if(ccount < 4 || 9 < ccount) {
            return std::nullopt;
        }

        uint32_t cval = (uint32_t)std::strtoull((const char*)(s + 2), nullptr, 16);
        if(errno == ERANGE) {
            return std::nullopt;
        }

        if(cval > 0x10FFFF) {
            return std::nullopt;
        }
        else {
            if(cval <= 0x7F) {
                return std::make_optional<UnicodeString>({ (UnicodeStringChar)cval });
            }
            else if(cval < 0x7FF) {
                return std::make_optional<UnicodeString>({ (UnicodeStringChar)(0xC0 | (cval >> 6)), (UnicodeStringChar)(0x80 | (cval & 0x3F)) });
            }
            else if(cval < 0xFFFF) {
                return std::make_optional<UnicodeString>({ (UnicodeStringChar)(0xE0 | (cval >> 12)), (UnicodeStringChar)(0x80 | ((cval >> 6) & 0x3F)), (UnicodeStringChar)(0x80 | (cval & 0x3F)) });
            }
            else {
                return std::make_optional<UnicodeString>({ (UnicodeStringChar)(0xF0 | (cval >> 18)), (UnicodeStringChar)(0x80 | ((cval >> 12) & 0x3F)), (UnicodeStringChar)(0x80 | ((cval >> 6) & 0x3F)), (UnicodeStringChar)(0x80 | (cval & 0x3F)) });
            }
        }
    }

    std::optional<CString> decodeHexEscapeAsC(const uint8_t* s, const uint8_t* e)
    {
        size_t ccount = std::distance(s, e);

        //1-2 digits and a %;
        if(ccount < 4 || 5 < ccount) {
            return std::nullopt;
        }

        uint32_t cval = (uint32_t)std::strtoull((const char*)(s + 2), nullptr, 16);
        if(errno == ERANGE) {
            return std::nullopt;
        }

        if(!isLegalCChar(cval)) {
            return std::nullopt;
        }
        else {
            return std::make_optional<CString>({ (CStringChar)cval });
        }
    }

    std::vector<uint8_t> extractRegexCharToBytes(RegexChar rc)
    {
        if(rc <= 0x7F) {
            return std::vector<uint8_t>{ (uint8_t)rc };
        }
        else if(rc < 0x7FF) {
            return std::vector<uint8_t>({ (uint8_t)(0xC0 | (rc >> 6)), (uint8_t)(0x80 | (rc & 0x3F)) });
        }
        else if(rc < 0xFFFF) {
            return std::vector<uint8_t>({ (uint8_t)(0xE0 | (rc >> 12)), (uint8_t)(0x80 | ((rc >> 6) & 0x3F)), (uint8_t)(0x80 | (rc & 0x3F)) });
        }
        else {
            return std::vector<uint8_t>({ (uint8_t)(0xF0 | (rc >> 18)), (uint8_t)(0x80 | ((rc >> 12) & 0x3F)), (uint8_t)(0x80 | ((rc >> 6) & 0x3F)), (uint8_t)(0x80 | (rc & 0x3F)) });
       }
    }

    const char* resolveEscapeUnicodeFromCode(UnicodeStringChar c)
    {
        auto ii = std::find_if(s_escape_names_unicode.cbegin(), s_escape_names_unicode.cend(), [c](const std::pair<uint8_t, const char*>& p) { 
            return p.first == c; 
        });
        return ii->second;
    }

    std::optional<uint8_t> resolveEscapeUnicodeFromName(const uint8_t* s, const uint8_t* e)
    {
        auto ii = std::find_if(s_escape_names_unicode.cbegin(), s_escape_names_unicode.cend(), [s, e](const std::pair<uint8_t, const char*>& p) { 
            return std::equal(p.second, p.second + strlen(p.second), s, e);
        });
        if(ii == s_escape_names_unicode.cend()) {
            return std::nullopt;
        }
        else {
            return std::make_optional(ii->first);
        }
    }

    const char* resolveEscapeCFromCode(CStringChar c)
    {
        auto ii = std::find_if(s_escape_names_char.cbegin(), s_escape_names_char.cend(), [c](const std::pair<uint8_t, const char*>& p) { 
            return p.first == c; 
        });
        return ii->second;
    }

    std::optional<uint8_t> resolveEscapeCFromName(const uint8_t* s, const uint8_t* e)
    {
        auto ii = std::find_if(s_escape_names_char.cbegin(), s_escape_names_char.cend(), [s, e](const std::pair<uint8_t, const char*>& p) { 
            return std::equal(p.second, p.second + strlen(p.second), s, e); 
        });
        if(ii == s_escape_names_char.cend()) {
            return std::nullopt;
        }
        else {
            return std::make_optional(ii->first);
        }
    }

    size_t msScanCount(const uint8_t* bytes, size_t cpos, size_t length)
    {
        size_t count = 0;
        for(size_t i = cpos + 1; i < length && bytes[i] != '\\'; ++i) {
            char cc = bytes[i];
            if(!std::iswspace(cc) || (cc == '\n')) {
                return 0; //no trailing slash so not an alignment
            }

            count++;
        }

        if(count >= 1) {
            return count + 1; //number of spaces + the trailing slash
        }
        else {
            //it is \n\ which we don't consider an alignment so just eat the newline
            return 0;
        }
    }

    std::pair<std::optional<UnicodeString>, std::optional<std::u8string>> unescapeUnicodeStringLiteralGeneral(const uint8_t* bytes, size_t length, bool multilinechk)
    {
        auto byteschk = parserValidateUTF8ByteEncoding(bytes, bytes + length);
        if(byteschk.has_value()) {
            return std::make_pair(std::nullopt, byteschk);
        }

        auto eschk = parserValidateEscapeSequences(false, bytes, bytes + length);
        if(!eschk.empty()) {
            return std::make_pair(std::nullopt, eschk.front());
        }
        
        std::vector<UnicodeStringChar> acc;
        for(size_t i = 0; i < length; ++i) {
            uint8_t c = bytes[i];

            if(c <= 127 && !std::isprint(c) && !std::isspace(c)) {
                return std::make_pair(std::nullopt, std::make_optional(u8"Invalid character in string"));
            }

            if(c == '%') {
                auto sc = std::find(bytes + i, bytes + length, ';');
                if(sc == bytes + length) {
                    return std::make_pair(std::nullopt, std::make_optional(u8"Unterminated escape sequence -- missing ;"));
                }

                if(isHexEscapePrefix(bytes + i, sc + 1)) {
                    //it should be a hex number
                    auto esc = decodeHexEscapeAsUnicode(bytes + i, sc + 1);
                    if(!esc.has_value()) {
                        return std::make_pair(std::nullopt, std::make_optional(u8"Invalid hex escape sequence -- " + std::u8string(bytes + i, sc + 1)));
                    }

                    std::copy(esc.value().cbegin(), esc.value().cend(), std::back_inserter(acc));
                }
                else {
                    auto esc = resolveEscapeUnicodeFromName(bytes + i, sc + 1);
                    if(!esc.has_value()) {
                        return std::make_pair(std::nullopt, std::make_optional(u8"Invalid escape name -- " + std::u8string(bytes + i, sc + 1)));
                    }

                    acc.push_back(esc.value());
                }

                i += std::distance(bytes + i, sc);
            }
            else {
                if(multilinechk && c == '\n') {
                    //check if the text is of the form \n\s+\ and if so then skip then this is a multiline-aligned string so skip the whitespace
                    auto dist = msScanCount(bytes, i, length);
                    if(dist != 0) {
                        i += dist;
                    }
                }
                    
                acc.push_back(c);
            }
        }

        return std::make_pair(std::make_optional<UnicodeString>(acc.cbegin(), acc.cend()), std::nullopt);
    }

    std::pair<std::optional<UnicodeString>, std::optional<std::u8string>> unescapeUnicodeString(const uint8_t* bytes, size_t length)
    {
        return unescapeUnicodeStringLiteralGeneral(bytes, length, false);
    }

    std::pair<std::optional<UnicodeString>, std::optional<std::u8string>> unescapeUnicodeStringLiteralInclMultiline(const uint8_t* bytes, size_t length)
    {
        return unescapeUnicodeStringLiteralGeneral(bytes, length, true);
    }

    std::vector<uint8_t> escapeUnicodeString(const UnicodeString& sv)
    {
        std::vector<uint8_t> acc = {};
        for(auto ii = sv.cbegin(); ii != sv.cend(); ++ii) {
            char8_t c = *ii;

            if(c == '%' || c == '"' || (c <= 127 && !std::isprint(c))) {
                auto escc = resolveEscapeUnicodeFromCode(c);
                while(*escc != '\0') {
                    acc.push_back(*escc++);
                }
            }
            else {
                acc.push_back(c);
            }
        }

        return acc;
    }

    std::pair<std::optional<CString>, std::optional<std::u8string>> unescapeCStringGeneral(const uint8_t* bytes, size_t length, bool multilinechk)
    {
        auto byteschk = parserValidateAllCEncoding(bytes, bytes + length);
        if(byteschk.has_value()) {
            return std::make_pair(std::nullopt, byteschk);
        }

        auto eschk = parserValidateEscapeSequences(true, bytes, bytes + length);
        if(!eschk.empty()) {
            return std::make_pair(std::nullopt, eschk.front());
        }

        std::vector<CStringChar> acc;
        for(size_t i = 0; i < length; ++i) {
            uint8_t c = bytes[i];

            if(!isLegalCChar(c)) {
                return std::make_pair(std::nullopt, std::make_optional(u8"Invalid character in string"));
            }

            if(c == '%') {
                auto sc = std::find(bytes + i, bytes + length, ';');
                if(sc == bytes + length) {
                    return std::make_pair(std::nullopt, std::make_optional(u8"Unterminated escape sequence -- missing ;"));
                }

                if(isHexEscapePrefix(bytes + i, sc + 1)) {
                    auto esc = decodeHexEscapeAsC(bytes + i, sc + 1);
                    if(!esc.has_value()) {
                        return std::make_pair(std::nullopt, std::make_optional(u8"Invalid hex escape sequence -- " + std::u8string(bytes + i, sc + 1)));
                    }

                    std::copy(esc.value().cbegin(), esc.value().cend(), std::back_inserter(acc));
                }
                else {
                    auto esc = resolveEscapeCFromName(bytes + i, sc + 1);
                    if(!esc.has_value()) {
                        return std::make_pair(std::nullopt, std::make_optional(u8"Invalid escape name -- " + std::u8string(bytes + i, sc + 1)));
                    }

                    acc.push_back(esc.value());
                }

                i += std::distance(bytes + i, sc);
            }
            else {
                if(multilinechk && c == '\n') {
                    //check if the text is of the form \n\s+\ and if so then skip then this is a multiline-aligned string so skip the whitespace
                    auto dist = msScanCount(bytes, i, length);
                    if(dist != 0) {
                        i += dist;
                    }
                }
                
                acc.push_back(c);
            }
        }

        return std::make_pair(std::make_optional<CString>(acc.cbegin(), acc.cend()), std::nullopt);
    }

    std::pair<std::optional<CString>, std::optional<std::u8string>> unescapeCString(const uint8_t* bytes, size_t length)
    {
        return unescapeCStringGeneral(bytes, length, false);
    }

    std::pair<std::optional<CString>, std::optional<std::u8string>> unescapeCStringLiteralInclMultiline(const uint8_t* bytes, size_t length)
    {
        return unescapeCStringGeneral(bytes, length, true);
    }

    std::vector<uint8_t> escapeCString(const CString& sv)
    {
        std::vector<uint8_t> acc = {};
        for(auto ii = sv.cbegin(); ii != sv.cend(); ++ii) {
            char c = *ii;

            if(c == '%' || c == '\'' || !std::isprint(c)) {
                auto escc = resolveEscapeCFromCode(c);
                while(*escc != '\0') {
                    acc.push_back(*escc++);
                }
            }
            else {
                acc.push_back(c);
            }
        }

        return acc;
    }

    std::optional<std::vector<RegexChar>> unescapeUnicodeRegexLiteral(const uint8_t* bytes, size_t length)
    {
        std::vector<RegexChar> acc;
        for(size_t i = 0; i < length; ++i) {
            uint8_t c = bytes[i];

            if(c <= 127 && !std::isprint(c) && !std::isblank(c)) {
                return std::nullopt;
            }

            if(c == '%') {
                auto sc = std::find(bytes + i, bytes + length, ';');
                if(sc == bytes + length) {
                    return std::nullopt;
                }

                if(isHexEscapePrefix(bytes + i, sc + 1)) {
                    //it should be a hex number
                    auto esc = decodeHexEscapeAsRegex(bytes + i, sc + 1, false);
                    if(!esc.has_value()) {
                        return std::nullopt;
                    }

                    acc.push_back(esc.value());
                }
                else {
                    auto esc = resolveEscapeUnicodeFromName(bytes + i, sc + 1);
                    if(!esc.has_value()) {
                        return std::nullopt;
                    }

                    acc.push_back(esc.value());
                }

                i += std::distance(bytes + i, sc);
            }
            else {
                acc.push_back(toRegexCharCodeFromBytes(bytes + i, length - i));

                i += charCodeByteCount(bytes + i) - 1;
            }
        }

        return std::make_optional<std::vector<RegexChar>>(acc.cbegin(), acc.cend());
    }

    std::optional<std::vector<RegexChar>> unescapeCRegexLiteral(const uint8_t* bytes, size_t length)
    {
        std::vector<RegexChar> acc;
        for(size_t i = 0; i < length; ++i) {
            uint8_t c = bytes[i];

            if(!std::isprint(c) && !std::isblank(c)) {
                return std::nullopt;
            }

            if(c == '%') {
                auto sc = std::find(bytes + i, bytes + length, ';');
                if(sc == bytes + length) {
                    return std::nullopt;
                }

                if(isHexEscapePrefix(bytes + i, sc + 1)) {
                    auto esc = decodeHexEscapeAsRegex(bytes + i, sc + 1, true);
                    if(!esc.has_value()) {
                        return std::nullopt;
                    }

                    acc.push_back(esc.value());
                }
                else {
                    auto esc = resolveEscapeCFromName(bytes + i, sc + 1);
                    if(!esc.has_value()) {
                        return std::nullopt;
                    }

                    acc.push_back(esc.value());
                }

                i += std::distance(bytes + i, sc);
            }
            else {
                acc.push_back(c);
            }
        }

        return std::make_optional<std::vector<RegexChar>>(acc.cbegin(), acc.cend());
    }

    std::optional<RegexChar> unescapeSingleUnicodeRegexChar(const uint8_t* s, const uint8_t* e)
    {
        if(isHexEscapePrefix(s, e)) {
            return decodeHexEscapeAsRegex(s, e, false);
        }
        else {
            return resolveEscapeUnicodeFromName(s, e);
        }
    }

    std::optional<RegexChar> unescapeSingleCRegexChar(const uint8_t* s, const uint8_t* e)
    {
        if(isHexEscapePrefix(s, e)) {
            return decodeHexEscapeAsRegex(s, e, true);
        }
        else {
            return resolveEscapeCFromName(s, e);
        }
    }

    std::vector<uint8_t> escapeSingleUnicodeRegexChar(RegexChar c)
    {
        std::vector<uint8_t> acc = {};

        if(c == U'%' || c == U'"' || c == '[' || c == ']' || c == U'/' || c == U'\\' || (c <= 127 && !std::isprint(c))) {
            auto escc = resolveEscapeUnicodeFromCode(c);
            while(*escc != '\0') {
                acc.push_back(*escc++);
            }
        }
        else {
            auto escc = extractRegexCharToBytes(c);
            std::copy(escc.cbegin(), escc.cend(), std::back_inserter(acc));
        }

        return acc;
    }

    std::vector<uint8_t> escapeRegexUnicodeLiteralCharBuffer(const std::vector<RegexChar>& sv)
    {
        std::vector<uint8_t> acc = {};
        for(auto ii = sv.cbegin(); ii != sv.cend(); ++ii) {
            RegexChar c = *ii;

            if(c == U'%' || c == U'"' || c == U'/' || c == U'\\' || (c <= 127 && !std::isprint(c))) {
                auto escc = resolveEscapeUnicodeFromCode(c);
                while(*escc != '\0') {
                    acc.push_back(*escc++);
                }
            }
            else {
                auto escc = extractRegexCharToBytes(c);
                std::copy(escc.cbegin(), escc.cend(), std::back_inserter(acc));
            }
        }

        return acc;
    }


    std::vector<uint8_t> escapeSingleCRegexChar(RegexChar c)
    {
        std::vector<uint8_t> acc = {};

        if(c == U'%' || c == U'\'' || c == '[' || c == ']' || c == U'/' || c == U'\\' || (c <= 127 && !std::isprint(c))) {
            auto escc = resolveEscapeCFromCode(c);
            while(*escc != '\0') {
                acc.push_back(*escc++);
            }
        }
        else {
            auto escc = extractRegexCharToBytes(c);
            std::copy(escc.cbegin(), escc.cend(), std::back_inserter(acc));
        }

        return acc;
    }

    std::vector<uint8_t> escapeRegexCLiteralCharBuffer(const std::vector<RegexChar>& sv)
    {
        std::vector<uint8_t> acc = {};
        for(auto ii = sv.cbegin(); ii != sv.cend(); ++ii) {
            RegexChar c = *ii;

            if(c == U'%' || c == U'\'' || c == U'/' || c == U'\\' || (c <= 127 && !std::isprint(c))) {
                auto escc = resolveEscapeCFromCode(c);
                while(*escc != '\0') {
                    acc.push_back(*escc++);
                }
            }
            else {
                auto escc = extractRegexCharToBytes(c);
                std::copy(escc.cbegin(), escc.cend(), std::back_inserter(acc));
            }
        }

        return acc;
    }

    std::u8string parserGenerateDiagnosticUnicodeEscapeName(uint8_t c)
    {
        auto ii = std::find_if(s_escape_names_unicode.cbegin(), s_escape_names_unicode.cend(), [c](const std::pair<uint8_t, const char*>& p) { 
            return p.first == c; 
        });
        return std::u8string(ii->second, ii->second + strlen(ii->second));
    }

    std::u8string parserGenerateDiagnosticCEscapeName(uint8_t c)
    {
        auto ii = std::find_if(s_escape_names_char.cbegin(), s_escape_names_char.cend(), [c](const std::pair<uint8_t, const char*>& p) { 
            return p.first == c; 
        });
        return std::u8string(ii->second, ii->second + strlen(ii->second));
    }

    std::u8string parserGenerateDiagnosticEscapeCode(uint8_t c)
    {
        char s_escape_code_buffer[16];
        sprintf(s_escape_code_buffer, "%%x%x;", c);

        return std::u8string(s_escape_code_buffer, s_escape_code_buffer + strlen(s_escape_code_buffer));
    }

    //If we have decode failures then go through and generate nice messages for them
    std::vector<std::u8string> parserValidateEscapeSequences(bool ischar, const uint8_t* s, const uint8_t* e)
    {
        std::vector<std::u8string> errors;
        for(auto curr = s; curr != e; curr++) {
            uint8_t c = *curr;
            
            if(c == '%') {
                auto sc = std::find(curr, e, ';');
                if(sc == e) {
                    errors.push_back(std::u8string(u8"Escape sequence is missing terminal ';'"));
                    return errors;
                }

                if(isHexEscapePrefix(curr, sc + 1)) {
                    if(!std::all_of(curr + 2, sc, [](uint8_t c) { return std::isxdigit(c); })) {
                        errors.push_back(std::u8string(u8"Hex escape sequence contains non-hex characters"));
                    }

                    auto esc = ischar ? decodeHexEscapeAsC(curr, sc + 1).has_value() : decodeHexEscapeAsUnicode(curr, sc + 1).has_value();
                    if(!esc) {
                        errors.push_back(std::u8string(u8"Invalid hex escape sequence"));
                    }
                }
                else {
                    auto esc = ischar ? resolveEscapeCFromName(curr, sc + 1).has_value() : resolveEscapeUnicodeFromName(curr, sc + 1).has_value();
                    if(!esc) {
                        errors.push_back(std::u8string(u8"Invalid escape sequence -- unknown escape name '" + std::u8string(curr + 1, sc) + u8"'"));
                    }
                }

                curr = sc;
            }
        }

        return errors;
    }

    //Scan the string and ensure that there are no multibyte chars that have messed up encodings
    std::optional<std::u8string> parserValidateUTF8ByteEncoding(const uint8_t* s, const uint8_t* e)
    {
        if((*s & 0x80) != 0 && (*s & 0x40) == 0) {
            return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- string contains a truncated character at the beginning"));
        }

        while(s != e) {
            if((*s & 0x80) == 0) {
                s++;
            }
            else {
                auto bytecount = UTF8_ENCODING_BYTE_COUNT(*s);
                if(s + bytecount > e) {
                    return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- string contains a truncated character at the end"));
                }
                else {
                    if(bytecount == 2) {
                        if((*s & 0xC0) != 0xC0 || (*(s + 1) & 0xC0) != 0x80) {
                            return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- string contains a mis-encoded 2 byte character"));
                        }
                    }
                    else if(bytecount == 3) {
                        if((*s & 0xE0) != 0xE0 || (*(s + 1) & 0xC0) != 0x80 || (*(s + 2) & 0xC0) != 0x80) {
                            return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- string contains a mis-encoded 3 byte character"));
                        }
                    }
                    else if(bytecount == 4) {
                        if((*s & 0xF0) != 0xF0 || (*(s + 1) & 0xC0) != 0x80 || (*(s + 2) & 0xC0) != 0x80 || (*(s + 3) & 0xC0) != 0x80) {
                            return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- string contains a mis-encoded 4 byte character"));
                        }
                    }
                    else {
                        return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- utf8 is at most 4 bytes per character"));
                    }
                }

                s += bytecount;
            }
        }

        return std::nullopt;
    }

    std::optional<std::u8string> parserValidateAllCEncoding(const uint8_t* s, const uint8_t* e)
    {
        while(s != e) {
            if((*s & 0x80) != 0) {
                return std::make_optional(std::u8string(u8"Invalid Char encoding -- string contains a non-char character"));
            }
            else {
                s++;
            }
        }

        return std::nullopt;
    }

    std::optional<std::u8string> parserValidateUTF8ByteEncoding_SingleChar(const uint8_t* s, const uint8_t* epos)
    {
        if((*s & 0x80) == 0) {
            if(*s == '%') {
                auto sc = std::find(s, epos, ';');
                if(sc == epos) {
                    return std::make_optional(std::u8string(u8"Escape sequence is missing terminal ';'"));
                }

                if(isHexEscapePrefix(s, sc + 1)) {
                    //it should be a hex number
                    if(!std::all_of(s + 2, sc, [](uint8_t c) { return std::isxdigit(c); })) {
                        return std::make_optional(std::u8string(u8"Hex escape sequence contains non-hex characters -- ") + std::u8string(s + 1, sc));
                    }

                    auto esc = decodeHexEscapeAsRegex(s, sc + 1, false);
                    if(esc.has_value() && esc.value() > 0x10FFFF) {
                        return std::make_optional(std::u8string(u8"Hex escape sequence is not a valid Unicode character -- ") + std::u8string(s + 1, sc));
                    }
                }
            }
        }
        else {
            auto bytecount = UTF8_ENCODING_BYTE_COUNT(*s);
            if(s + bytecount > epos) {
                return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- string contains a truncated character at the end"));
            }
            else {
                if(bytecount == 2) {
                    if((*s & 0xC0) != 0xC0 || (*(s + 1) & 0xC0) != 0x80) {
                        return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- string contains a mis-encoded 2 byte character"));
                    }
                }
                else if(bytecount == 3) {
                    if((*s & 0xE0) != 0xE0 || (*(s + 1) & 0xC0) != 0x80 || (*(s + 2) & 0xC0) != 0x80) {
                        return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- string contains a mis-encoded 3 byte character"));
                    }
                }
                else if(bytecount == 4) {
                    if((*s & 0xF0) != 0xF0 || (*(s + 1) & 0xC0) != 0x80 || (*(s + 2) & 0xC0) != 0x80 || (*(s + 3) & 0xC0) != 0x80) {
                        return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- string contains a mis-encoded 4 byte character"));
                    }
                }
                else {
                    return std::make_optional(std::u8string(u8"Invalid UTF8 encoding -- utf8 is at most 4 bytes per character"));
                }
            }
        }

        return std::nullopt;
    }

    std::optional<std::u8string> parserValidateAllCEncoding_SingleChar(const uint8_t* s, const uint8_t* epos)
    {
        if((*s & 0x80) != 0) {
            return std::make_optional(std::u8string(u8"Invalid char encoding -- string contains a non-char character"));
        }
        else {
            if(*s == '%') {
                auto sc = std::find(s, epos, ';');
                if(sc == epos) {
                    return std::make_optional(std::u8string(u8"Escape sequence is missing terminal ';'"));
                }

                if(isHexEscapePrefix(s, sc + 1)) {
                    //it should be a hex number
                    if(!std::all_of(s + 2, sc, [](uint8_t c) { return std::isxdigit(c); })) {
                        return std::make_optional(std::u8string(u8"Hex escape sequence contains non-hex characters -- ") + std::u8string(s + 1, sc));
                    }

                    auto esc = decodeHexEscapeAsRegex(s, sc + 1, true);
                    if(!esc.has_value() || esc.value() > 127) {
                        return std::make_optional(std::u8string(u8"Hex escape sequence is not a valid char character -- ") + std::u8string(s + 1, sc));
                    }
                }
            }
        }

        return std::nullopt;
    }
}
