#include "common.h"

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

    std::vector<std::pair<uint8_t, const char*>> s_escape_names_ascii = {
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
        if(this->curr + bytecount > this->epos) {
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

    std::optional<RegexChar> decodeHexEscapeAsRegex(const uint8_t* s, const uint8_t* e)
    {
        size_t ccount = std::distance(s, e);

        //1-6 digits and a %;
        if(ccount <= 2 || 8 < ccount) {
            return std::nullopt;
        }

        //either 0x or a leading 0
        if(*(s + 1) == '0') {
            return std::nullopt;
        }

        if(!std::all_of(s + 1, e, [](uint8_t c) { return std::isxdigit(c); })) {
            return std::nullopt;
        }

        uint32_t cval;
        auto sct = sscanf((char*)(s + 1), "%%%x;", &cval);
        if(sct != 1 || sct > 0x10FFFF) {
            return std::nullopt;
        }
        else {
            return std::make_optional(cval);
        }
    }

    std::optional<UnicodeString> decodeHexEscapeAsUnicode(const uint8_t* s, const uint8_t* e)
    {
        size_t ccount = std::distance(s, e);

        //1-6 digits and a %;
        if(ccount <= 2 || 8 < ccount) {
            return std::nullopt;
        }

        uint32_t cval = 0;
        auto sct = sscanf((char*)(s + 1), "%%%x;", &cval);
        if(sct != 1 || cval > 0x10FFFF) {
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

    std::optional<ASCIIString> decodeHexEscapeAsASCII(const uint8_t* s, const uint8_t* e)
    {
        size_t ccount = std::distance(s, e);

        //1-6 digits and a %;
        if(ccount <= 2 || 8 < ccount) {
            return std::nullopt;
        }

        uint32_t cval = 0;
        auto sct = sscanf((char*)(s + 1), "%%%x;", &cval);
        if(sct != 1 || cval > 127) {
            return std::nullopt;
        }
        else {
            return std::make_optional<ASCIIString>({ (ASCIIStringChar)cval });
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

    const char* resolveEscapeASCIIFromCode(ASCIIStringChar c)
    {
        auto ii = std::find_if(s_escape_names_ascii.cbegin(), s_escape_names_ascii.cend(), [c](const std::pair<uint8_t, const char*>& p) { 
            return p.first == c; 
        });
        return ii->second;
    }

    std::optional<uint8_t> resolveEscapeASCIIFromName(const uint8_t* s, const uint8_t* e)
    {
        auto ii = std::find_if(s_escape_names_ascii.cbegin(), s_escape_names_ascii.cend(), [s, e](const std::pair<uint8_t, const char*>& p) { 
            return std::equal(p.second, p.second + strlen(p.second), s, e); 
        });
        if(ii == s_escape_names_ascii.cend()) {
            return std::nullopt;
        }
        else {
            return std::make_optional(ii->first);
        }
    }

    std::optional<UnicodeString> unescapeString(const uint8_t* bytes, size_t length)
    {
        std::vector<UnicodeStringChar> acc;
        for(size_t i = 0; i < length; ++i) {
            uint8_t c = bytes[i];

            if(c <= 127 && !std::isprint(c) && !std::iswspace(c)) {
                return std::nullopt;
            }

            if(c == '%') {
                auto sc = std::find(bytes + i, bytes + length, ';');
                if(sc == bytes + length) {
                    return std::nullopt;
                }

                if(std::isxdigit(bytes[i + 1])) {
                    //it should be a hex number
                    auto esc = decodeHexEscapeAsUnicode(bytes + i, sc + 1);
                    if(!esc.has_value()) {
                        return std::nullopt;
                    }

                    std::copy(esc.value().cbegin(), esc.value().cend(), std::back_inserter(acc));
                }
                else {
                    auto esc = resolveEscapeUnicodeFromName(bytes + i, sc + 1);
                    if(!esc.has_value()) {
                        return std::nullopt;
                    }

                    acc.push_back(esc.value());
                }

                i += std::distance(bytes + i, sc) - 1;
            }
            else {
                acc.push_back(c);
            }
        }

        return std::make_optional<UnicodeString>(acc.cbegin(), acc.cend());
    }

    std::vector<uint8_t> escapeString(const UnicodeString& sv)
    {
        std::vector<uint8_t> acc = {};
        for(auto ii = sv.cbegin(); ii != sv.cend(); ++ii) {
            char8_t c = *ii;

            if(c == U'%' || c == U'"' || (c <= 127 && !std::isprint(c))) {
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

    std::optional<ASCIIString> unescapeASCIIString(const uint8_t* bytes, size_t length)
    {
        std::vector<ASCIIStringChar> acc;
        for(size_t i = 0; i < length; ++i) {
            uint8_t c = bytes[i];

            if(!std::isprint(c) && !std::iswspace(c)) {
                return std::nullopt;
            }

            if(c == '%') {
                auto sc = std::find(bytes + i, bytes + length, ';');
                if(sc == bytes + length) {
                    return std::nullopt;
                }

                if(std::isxdigit(bytes[i + 1])) {
                    auto esc = decodeHexEscapeAsASCII(bytes + i, sc);
                    if(!esc.has_value()) {
                        return std::nullopt;
                    }

                    std::copy(esc.value().cbegin(), esc.value().cend(), std::back_inserter(acc));
                }
                else {
                    auto esc = resolveEscapeASCIIFromName(bytes + i, sc);
                    if(!esc.has_value()) {
                        return std::nullopt;
                    }

                    acc.push_back(esc.value());
                }

                i += std::distance(bytes + i, sc) - 1;
            }
            else {
                acc.push_back(c);
            }
        }

        return std::make_optional<ASCIIString>(acc.cbegin(), acc.cend());
    }

    std::vector<uint8_t> escapeASCIIString(const ASCIIString& sv)
    {
        std::vector<uint8_t> acc = {};
        for(auto ii = sv.cbegin(); ii != sv.cend(); ++ii) {
            char c = *ii;

            if(c == '%' || c == '\'' || !std::isprint(c)) {
                auto escc = resolveEscapeASCIIFromCode(c);
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

    std::optional<std::vector<RegexChar>> unescapeRegexLiteral(const uint8_t* bytes, size_t length)
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

                if(std::isxdigit(bytes[i + 1])) {
                    //it should be a hex number
                    auto esc = decodeHexEscapeAsRegex(bytes + i, sc + 1);
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

                i += std::distance(bytes + i, sc) - 1;
            }
            else {
                acc.push_back(toRegexCharCodeFromBytes(bytes + i, length - i));

                i += charCodeByteCount(bytes + i) - 1;
            }
        }

        return std::make_optional<std::vector<RegexChar>>(acc.cbegin(), acc.cend());
    }

    std::optional<std::vector<RegexChar>> unescapeASCIIRegexLiteral(const uint8_t* bytes, size_t length)
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

                if(std::isxdigit(bytes[i + 1])) {
                    auto esc = decodeHexEscapeAsRegex(bytes + i, sc);
                    if(!esc.has_value()) {
                        return std::nullopt;
                    }

                    acc.push_back(esc.value());
                }
                else {
                    auto esc = resolveEscapeASCIIFromName(bytes + i, sc);
                    if(!esc.has_value()) {
                        return std::nullopt;
                    }

                    acc.push_back(esc.value());
                }

                i += std::distance(bytes + i, sc) - 1;
            }
            else {
                acc.push_back(c);
            }
        }

        return std::make_optional<std::vector<RegexChar>>(acc.cbegin(), acc.cend());
    }

    std::optional<RegexChar> unescapeSingleRegexChar(const uint8_t* s, const uint8_t* e)
    {
        if(std::isxdigit(*s)) {
            return decodeHexEscapeAsRegex(s, e);
        }
        else {
            return resolveEscapeUnicodeFromName(s, e);
        }
    }

    std::optional<RegexChar> unescapeSingleASCIIRegexChar(const uint8_t* s, const uint8_t* e)
    {
        if(std::isxdigit(*s)) {
            return decodeHexEscapeAsRegex(s, e);
        }
        else {
            return resolveEscapeASCIIFromName(s, e);
        }
    }

    std::vector<uint8_t> escapeSingleRegexChar(RegexChar c)
    {
        std::vector<uint8_t> acc = {};

        if(c == U'%' || c == U'"' || c == U'\'' || c == U'/' || c == U'\\' || (c <= 127 && !std::isprint(c))) {
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

    std::vector<uint8_t> escapeRegexLiteralCharBuffer(const std::vector<RegexChar>& sv)
    {
        std::vector<uint8_t> acc = {};
        for(auto ii = sv.cbegin(); ii != sv.cend(); ++ii) {
            RegexChar c = *ii;

            if(c == U'%' || c == U'"' || c == U'\'' || c == U'/' || c == U'\\' || (c <= 127 && !std::isprint(c))) {
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

    std::u8string parserGenerateDiagnosticUnicodeEscapeName(uint8_t c)
    {
        auto ii = std::find_if(s_escape_names_unicode.cbegin(), s_escape_names_unicode.cend(), [c](const std::pair<uint8_t, const char*>& p) { 
            return p.first == c; 
        });
        return std::u8string(ii->second, ii->second + strlen(ii->second));
    }

    std::u8string parserGenerateDiagnosticASCIIEscapeName(uint8_t c)
    {
        auto ii = std::find_if(s_escape_names_ascii.cbegin(), s_escape_names_ascii.cend(), [c](const std::pair<uint8_t, const char*>& p) { 
            return p.first == c; 
        });
        return std::u8string(ii->second, ii->second + strlen(ii->second));
    }

    std::u8string parserGenerateDiagnosticEscapeCode(uint8_t c)
    {
        char s_escape_code_buffer[16];
        sprintf(s_escape_code_buffer, "%%%x;", c);

        return std::u8string(s_escape_code_buffer, s_escape_code_buffer + strlen(s_escape_code_buffer));
    }

    //If we have decode failures then go through and generate nice messages for them
    std::vector<std::u8string> parserValidateEscapeSequences(bool isascii, const uint8_t* s, const uint8_t* e)
    {
        std::vector<std::u8string> errors;
        for(auto curr = s; s != e; curr++) {
            uint8_t c = *curr;

            if(c == '%') {
                auto sc = std::find(curr, e, ';');
                if(sc == e) {
                    errors.push_back(std::u8string(u8"Escape sequence is missing terminal ';'"));
                    return errors;
                }

                if(std::isxdigit(*(curr + 1))) {
                    //it should be a hex number
                    if(*(curr + 1) == '0' && *(curr + 2) == 'x') {
                        errors.push_back(std::u8string(u8"Invalid hex escape sequence -- do not need '0x' prefix"));
                    }
                    else {
                        auto esc = decodeHexEscapeAsUnicode(curr, sc + 1);
                        if(!esc.has_value()) {
                            errors.push_back(std::u8string(u8"Invalid hex escape sequence"));
                        }
                    }
                }
                else {
                    auto esc = isascii ? resolveEscapeASCIIFromName(curr, sc + 1).has_value() : resolveEscapeUnicodeFromName(curr, sc + 1).has_value();
                    if(!esc) {
                        errors.push_back(std::u8string(u8"Invalid escape sequence -- unknown escape name '" + std::u8string(curr + 1, sc - 1) + u8"'"));
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
        if((*s & 0x40) == 0) {
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

    std::optional<std::u8string> parserValidateAllASCIIEncoding(const uint8_t* s, const uint8_t* e)
    {
        while(s != e) {
            if((*s & 0x80) != 0) {
                return std::make_optional(std::u8string(u8"Invalid ASCII encoding -- string contains a non-ASCII character"));
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

                if(std::isxdigit(*(sc + 1))) {
                    if(*(s + 1) == '0' && *(s + 2) == 'x') {
                        return make_optional(std::u8string(u8"Invalid hex escape sequence -- do not need '0x' prefix"));
                    }
                    else {
                        //it should be a hex number
                        if(!std::all_of(s + 1, sc, [](uint8_t c) { return std::isxdigit(c); })) {
                            return std::make_optional(std::u8string(u8"Hex escape sequence contains non-hex characters -- ") + std::u8string(s + 1, sc));
                        }

                        auto esc = decodeHexEscapeAsRegex(s, sc + 1);
                        if(esc.has_value() && esc.value() > 0x10FFFF) {
                            return std::make_optional(std::u8string(u8"Hex escape sequence is not a valid Unicode character -- ") + std::u8string(s + 1, sc));
                        }
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

    std::optional<std::u8string> parserValidateAllASCIIEncoding_SingleChar(const uint8_t* s, const uint8_t* epos)
    {
        if((*s & 0x80) != 0) {
            return std::make_optional(std::u8string(u8"Invalid ASCII encoding -- string contains a non-ASCII character"));
        }
        else {
            if(*s == '%') {
                auto sc = std::find(s, epos, ';');
                if(sc == epos) {
                    return std::make_optional(std::u8string(u8"Escape sequence is missing terminal ';'"));
                }

                if(std::isxdigit(*(s + 1))) {
                    //it should be a hex number
                    if(*(s + 1) == '0' && *(s + 2) == 'x') {
                        return make_optional(std::u8string(u8"Invalid hex escape sequence -- do not need '0x' prefix"));
                    }
                    else {
                        if(!std::all_of(s + 1, sc, [](uint8_t c) { return std::isxdigit(c); })) {
                            return std::make_optional(std::u8string(u8"Hex escape sequence contains non-hex characters -- ") + std::u8string(s + 1, sc));
                        }

                        auto esc = decodeHexEscapeAsRegex(s, sc + 1);
                        if(!esc.has_value() || esc.value() > 127) {
                            return std::make_optional(std::u8string(u8"Hex escape sequence is not a valid ASCII character -- ") + std::u8string(s + 1, sc));
                        }
                    }
                }
            }
        }

        return std::nullopt;
    }
}
