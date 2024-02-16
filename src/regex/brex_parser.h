#define once

#include "../common.h"

#include "brex.h"

#include <regex>

namespace brex
{
    class RegexParserError
    {
    public:
        size_t line;
        std::u8string msg;

        RegexParserError(size_t line, std::u8string msg): line(line), msg(msg) { ; }
        ~RegexParserError() = default;

        RegexParserError() = default;
        RegexParserError(const RegexParserError& other) = default;
        RegexParserError(RegexParserError&& other) = default;

        RegexParserError& operator=(const RegexParserError& other) = default;
        RegexParserError& operator=(RegexParserError&& other) = default;
    };

    class RegexParser
    {
    public:
        const uint8_t* data;
        uint8_t* cpos;
        const uint8_t* epos;

        const bool isUnicode;
        const bool isStrictASCII;
        bool envAllowed;

        size_t cline;
        std::vector<RegexParserError> errors;

        RegexParser(const uint8_t* data, size_t len, bool isUnicode, bool isStrictASCII, bool envAllowed) : data(data), cpos(const_cast<uint8_t*>(data)), epos(data + len), isUnicode(isUnicode), isStrictASCII(isStrictASCII), envAllowed(envAllowed), cline(0), errors() {;}
        ~RegexParser() = default;

        inline bool isEOS() const
        {
            return this->cpos == this->epos;
        }

        inline bool isToken(uint8_t tk) const
        {
            return !this->isEOS() && *this->cpos == tk;
        }

        inline bool isTokenWS() const
        {
            return (this->cpos < this->epos && std::isspace(*this->cpos));
        }

        inline bool isTokenCommentStart() const
        {
            return (this->cpos + 1 < this->epos) && (*this->cpos == '%' && *(this->cpos + 1) == '%');
        }

        inline bool isNamedPfx()
        {
            return (this->cpos + 2 < this->epos) && (*this->cpos == '$' && *(this->cpos + 1) == '{');
        }

        inline bool isTokenEnvPfx() const
        {
            return (this->cpos + 3 < this->epos) && (*this->cpos == 'e' && *(this->cpos + 1) == 'n' && *(this->cpos + 2) == 'v' && *(this->cpos + 3) == '[');
        }

        inline uint8_t token() const
        {
            return *this->cpos;
        }

        void advanceTriviaOnly()
        {
            while(this->isTokenWS() || this->isTokenCommentStart()) {
                if(this->isToken('\n')) {
                    this->cline++;
                }

                if(this->isTokenWS()) {
                    this->cpos++;
                }
                else {
                    while(!this->isEOS() && !this->isToken('\n')) {
                        this->cpos++;
                    }
                }
            }
        }

        void advance() 
        {
            if(!this->isEOS())
            {
                if(this->isToken('\n')) {
                    this->cline++;
                }

                this->cpos++;
            }

            this->advanceTriviaOnly();
        }

        void advance(size_t dist)
        {
            while(dist > 0 && !this->isEOS()) {
                if(this->isToken('\n')) {
                    this->cline++;
                }

                this->cpos++;
                dist--;
            }

            this->advanceTriviaOnly();
        }

        void scanToSyncToken(uint8_t tk, bool andeat)
        {
            while(!this->isEOS() && !this->isToken(tk)) {
                if(this->isToken('\n')) {
                    this->cline++;
                }

                this->cpos++;
            }

            //eat the sync token
            if(!this->isEOS() && andeat) {
                this->cpos++;
            }
        }

        void scanToSyncTokenAtomic()
        {
            bool exit = false;
            bool andeat = false;
            while(!this->isEOS() && !exit) {
                switch(*this->cpos) {
                    case '(':
                    case '[':
                    case '{': 
                    case '|': 
                    case '&': 
                    case '*':
                    case '+':
                    case '?':
                    case '.': {
                        exit = true;
                        break;
                    }
                    case '\'':
                    case '"':
                    case ')':
                    case ']':
                    case '}': {
                        andeat = true;
                        exit = true;
                        break;
                    }
                    default: {
                        break;
                    }
                }

                this->cpos++;
            }

            //eat the sync token
            if(!this->isEOS() && andeat) {
                this->cpos++;
            }
        }

        RegexChar parseRegexChar(bool unicodeok)
        {
            auto c = this->token();
            if(c <= 127 && !std::isprint(c) && !std::isblank(c)) {
                auto esccname = parserGenerateDiagnosticUnicodeEscapeName(c);
                auto esccode = parserGenerateDiagnosticEscapeCode(c);
                this->errors.push_back(RegexParserError(this->cline, u8"Newlines and non-printable chars are not allowed in regexes -- escape them with " + esccname + u8" or " + esccode));
                this->cpos++;
                return 0;
            }

            if(unicodeok) {
                auto encerr = parserValidateUTF8ByteEncoding_SingleChar(this->cpos, this->epos);
                if(encerr.has_value()) {
                    this->errors.push_back(RegexParserError(this->cline, encerr.value()));
                    this->scanToSyncToken(']', false);
                    return 0;
                }
            }
            else {
                auto encerr = parserValidateAllASCIIEncoding_SingleChar(this->cpos, this->epos, this->isStrictASCII);
                if(encerr.has_value()) {
                    this->errors.push_back(RegexParserError(this->cline, encerr.value()));
                    this->scanToSyncToken(']', false);
                    return 0;
                }
            }

            RegexChar code = 0;
            if(this->isToken('%')) {
                auto tpos = std::find((const uint8_t*)this->cpos + 1, this->epos, ';');
                auto bytecount = std::distance((const uint8_t*)this->cpos, tpos);

                std::optional<RegexChar> ccode = std::nullopt;
                if(unicodeok) {
                    ccode = unescapeSingleUnicodeRegexChar(this->cpos, tpos + 1);
                }
                else {
                    ccode = unescapeSingleASCIIRegexChar(this->cpos, tpos + 1, this->isStrictASCII);
                }

                if(ccode.has_value()) {
                    code = ccode.value();
                }
                else {
                    auto errors = parserValidateEscapeSequences(!unicodeok, this->isStrictASCII, this->cpos, tpos + 1);
                    for(auto ii = errors.cbegin(); ii != errors.cend(); ii++) {
                        this->errors.push_back(RegexParserError(this->cline, *ii));
                    }
                }

                this->cpos += (bytecount + 1);
            }
            else {
                auto bytecount = charCodeByteCount(this->cpos);

                code = toRegexCharCodeFromBytes(this->cpos, bytecount);
                this->cpos += bytecount;
            }

            return code;
        }

        const LiteralOpt* parseUnicodeLiteral()
        {
            //read to closing " -- check for raw newlines in the expression
            size_t length = 0;
            auto curr = this->cpos + 1;
            while(curr != this->epos && *curr != '"') {
                if(*curr <= 127 && !std::isprint(*curr) && !std::isblank(*curr)) {
                    auto esccname = parserGenerateDiagnosticUnicodeEscapeName(*curr);
                    auto esccode = parserGenerateDiagnosticEscapeCode(*curr);
                    this->errors.push_back(RegexParserError(this->cline, u8"Newlines and non-printable chars are not allowed regex literals -- escape them with " + esccname + u8" or " + esccode));
                    return nullptr;
                }

                length++;
                curr++;
            }

            if(curr == this->epos) {
                this->errors.push_back(RegexParserError(this->cline, u8"Unterminated regex literal"));
                this->cpos = const_cast<uint8_t*>(this->epos);

                return new LiteralOpt({ }, true);
            }

            if(length == 0) {
                this->cpos = curr + 1;
                return new LiteralOpt({ }, true);
            }

            auto bytechecks = parserValidateUTF8ByteEncoding(this->cpos + 1, this->cpos + 1 + length);
            if(bytechecks.has_value()) {
                this->errors.push_back(RegexParserError(this->cline, bytechecks.value()));
                this->cpos = curr + 1;

                return new LiteralOpt({ }, true);
            }
            else {
                auto codes = unescapeUnicodeRegexLiteral(this->cpos + 1, length);

                if(!codes.has_value()) {
                    auto errors = parserValidateEscapeSequences(false, this->isStrictASCII, this->cpos + 1, this->cpos + 1 + length);
                    for(auto ii = errors.cbegin(); ii != errors.cend(); ii++) {
                        this->errors.push_back(RegexParserError(this->cline, *ii));
                    }
                    this->cpos = curr + 1;

                    return new LiteralOpt({ }, true);
                }
                else {
                    this->cpos = curr + 1;

                    return new LiteralOpt(codes.value(), true);
                }
            }
        }

        const LiteralOpt* parseASCIILiteral()
        {
            //read to closing ' -- check for raw newlines in the expression
            size_t length = 0;
            auto curr = this->cpos + 1;
            while(curr != this->epos && *curr != '\'') {
                if(*curr <= 127 && !std::isprint(*curr) && !std::isblank(*curr)) {
                    auto esccname = parserGenerateDiagnosticASCIIEscapeName(*curr);
                    auto esccode = parserGenerateDiagnosticEscapeCode(*curr);
                    this->errors.push_back(RegexParserError(this->cline, u8"Newlines and non-printable chars are not allowed regex literals -- escape them with " + esccname + u8" or " + esccode));
                    return nullptr;
                }

                length++;
                curr++;
            }

            if(curr == this->epos) {
                this->errors.push_back(RegexParserError(this->cline, u8"Unterminated regex literal"));
                this->cpos = const_cast<uint8_t*>(this->epos);

                return new LiteralOpt({ }, false);
            }

            if(length == 0) {
                this->cpos = curr + 1;
                return new LiteralOpt({ }, false);
            }

            auto bytechecks = parserValidateAllASCIIEncoding(this->cpos + 1, this->cpos + 1 + length);
            if(bytechecks.has_value()) {
                this->errors.push_back(RegexParserError(this->cline, bytechecks.value()));
                this->cpos = curr + 1;

                return new LiteralOpt({ }, false);
            }
            else {
                auto codes = unescapeASCIIRegexLiteral(this->cpos + 1, length, this->isStrictASCII);

                if(!codes.has_value()) {
                    auto errors = parserValidateEscapeSequences(true, this->isStrictASCII, this->cpos + 1, this->cpos + 1 + length);
                    for(auto ii = errors.cbegin(); ii != errors.cend(); ii++) {
                        this->errors.push_back(RegexParserError(this->cline, *ii));
                    }
                    this->cpos = curr + 1;

                    return new LiteralOpt({ }, false);
                }
                else {
                    this->cpos = curr + 1;

                    return new LiteralOpt(codes.value(), false);
                }
            }
        }

        const CharRangeOpt* parseCharRange(bool unicodeok)
        {
            //eat the "["
            this->cpos++;

            auto compliment = this->isToken('^');
            if(compliment) {
                this->cpos++;
            }

            std::vector<SingleCharRange> range;
            if(this->isToken('-')) {
                //then it is a literal hyphen in ths special start of list position
                this->cpos++;
                range.push_back({ (RegexChar)'-', (RegexChar)'-' });
            }

            while(!this->isEOS() && !this->isToken(']')) {
                auto lb = this->parseRegexChar(unicodeok);

                if (!this->isToken('-')) {
                    range.push_back({ lb, lb });
                }
                else {
                    this->cpos++;
                    if(this->isToken(']')) {
                        //then it is a literal hyphen in the special end of list position
                        range.push_back({ lb, lb });
                        range.push_back({ (RegexChar)'-', (RegexChar)'-' });
                    }
                    else {
                        if (this->isToken('-')) {
                            //then it is a literal hyphen in a special end position but as part of a range :-0
                            this->cpos++;
                            range.push_back({ std::min(lb, (RegexChar)'-'), std::max(lb, (RegexChar)'-') });
                        }
                        else {
                            auto ub = this->parseRegexChar(unicodeok);
                            range.push_back({ std::min(lb, ub), std::max(lb, ub) });
                        }
                    }
                }
            }

            if(this->isToken(']')) {
                this->cpos++;
            }
            else {
                this->errors.push_back(RegexParserError(this->cline, u8"Missing ] in char range regex"));
            }

            return new CharRangeOpt(compliment, range, unicodeok);
        }

        const RegexOpt* parseNamedRegex()
        {
            this->advance();
            this->advance();
            std::string name;
            while(!this->isEOS() && !this->isToken('}')) {
                name.push_back(this->token());
                this->cpos++;
            }

            if(this->isToken('}')) {
                this->cpos++;
            }
            else {
                this->errors.push_back(RegexParserError(this->cline, u8"Missing closing } in named regex"));
            }

            std::basic_regex scopere("^([A-Z][_a-zA-Z0-9]+::)*[A-Z][_a-zA-Z0-9]+$");
            if(!std::regex_match(name.cbegin(), name.cend(), scopere)) {
                this->errors.push_back(RegexParserError(this->cline, u8"Invalid named regex name -- must be a valid scoped identifier"));
            }

            return new NamedRegexOpt(name);
        }

        const RegexOpt* parseEnvRegex()
        {
            if(!this->envAllowed) {
                this->errors.push_back(RegexParserError(this->cline, u8"Env regexes are not allowed in this context"));
            }

            this->cpos += 4;
            std::string name;
            while(!this->isEOS() && !this->isToken(']')) {
                name.push_back(this->token());
                this->cpos++;
            }

            if(this->isToken(']')) {
                this->cpos++;
            }
            else {
                this->errors.push_back(RegexParserError(this->cline, u8"Missing closing ] in env regex"));
            }

            std::basic_regex idre("^[_a-z][_a-zA-Z0-9]*$");
            if(!std::regex_match(name.cbegin(), name.cend(), idre)) {
                this->errors.push_back(RegexParserError(this->cline, u8"Invalid env regex name -- must be a valid identifier"));
            }

            return new EnvRegexOpt(name);
        }

        const RegexOpt* parseBaseComponent() 
        {
            //make sure we get any trivia out of the way
            this->advanceTriviaOnly(); 

            const RegexOpt* res = nullptr;
            if(this->isToken('(')) {
                this->advance();

                res = this->parsePositiveComponent();
                if(this->isToken(')')) {
                    this->advance();
                }
                else {
                    this->errors.push_back(RegexParserError(this->cline, u8"Missing ) in regex"));
                }
            }
            else if(this->isToken('"')) {
                if(this->isUnicode) {
                    res = this->parseUnicodeLiteral();
                }
                else {
                    this->errors.push_back(RegexParserError(this->cline, u8"Unicode literals are not allowed in ASCII regexes"));
                    this->scanToSyncToken('"', true);
                    res = new LiteralOpt({}, false);
                }
            }
            else if(this->isToken('\'')) {
                if(!this->isUnicode) {
                    res = this->parseASCIILiteral();
                }
                else {
                    this->errors.push_back(RegexParserError(this->cline, u8"ASCII literals are not allowed in Unicode regexes"));
                    this->scanToSyncToken('\'', true);
                    res = new LiteralOpt({}, true);  
                }
            }
            else if(this->isToken('[')) {
                res = this->parseCharRange(this->isUnicode);
            }
            else if(this->isToken('.')) {
                this->cpos++;
                res = new CharClassDotOpt();
            }
            else if(this->isNamedPfx()) {
                res = this->parseNamedRegex();
            }
            else if(this->isTokenEnvPfx()) {
                res = this->parseEnvRegex();
            }
            else {
                std::u8string slice(this->cpos, this->cpos + charCodeByteCount(this->cpos));
                this->scanToSyncTokenAtomic();

                this->errors.push_back(RegexParserError(this->cline, u8"Invalid regex component -- expected (, [, ', \", {, or . but found \"" + slice + u8"\""));
                res = new LiteralOpt({}, false);
            }

            //make sure we get any trivia out of the way
            this->advanceTriviaOnly();

            return res;
        }

        bool parseRangeRepeatBound(uint16_t& vv)
        {
            if(this->isToken('-')) {
                this->errors.push_back(RegexParserError(this->cline, u8"Invalid range repeat bound -- cannot have negative bound"));
                this->cpos++;
            }

            if(this->isEOS() || !std::isdigit(this->token())) {
                return false;
            }

            auto drange = std::find_if((const uint8_t*)this->cpos, this->epos, [](uint8_t c) { return !std::isdigit(c); });
            std::string istr = std::string((const char*)this->cpos, (const char*)drange);
            this->cpos += istr.size();

            if(istr == "0") {
                vv = 0;
                return true;
            }
            else {
                if(istr.starts_with("0")) {
                    this->errors.push_back(RegexParserError(this->cline, u8"Invalid range repeat bound -- invalid leading 0 on bound"));
                    return false;
                }

                char* eptr = nullptr;
                auto sval = std::strtol(istr.c_str(), &eptr, 10);
                if(sval == 0) {
                    this->errors.push_back(RegexParserError(this->cline, u8"Invalid range repeat bound -- invalid number"));
                    return false;
                }
                else if(sval > UINT16_MAX) {
                    this->errors.push_back(RegexParserError(this->cline, u8"Invalid range repeat bound -- number too large (max is 65535)"));
                    return false;
                }
                else {
                    vv = (uint16_t)sval;
                    return true;
                }
            }
        }

        const RegexOpt* parseRepeatComponent()
        {
            auto rcc = this->parseBaseComponent();

            while(this->isToken('*') || this->isToken('+') || this->isToken('?') || this->isToken('{')) {
                if(this->isToken('*')) {
                    rcc = new StarRepeatOpt(rcc);
                    this->advance();
                }
                else if(this->isToken('+')) {
                    rcc = new PlusRepeatOpt(rcc);
                    this->advance();
                }
                else if(this->isToken('?')) {
                    rcc = new OptionalOpt(rcc);
                    this->advance();
                }
                else {
                    this->advance();
                    uint16_t min = 0;
                    auto hasmin = this->parseRangeRepeatBound(min);

                    this->advanceTriviaOnly();
                    if(hasmin && !this->isToken(',') && !this->isToken('}')) {
                        this->errors.push_back(RegexParserError(this->cline, u8"Missing comma (possibly) in range repeat"));
                    }

                    uint16_t max = min;
                    if (this->isToken(',')) {
                        this->advance();
                        max = UINT16_MAX;
                        this->parseRangeRepeatBound(max);

                        this->advanceTriviaOnly();
                    }

                    if(this->isToken('}')) {
                        this->advance();
                    }
                    else {
                        this->errors.push_back(RegexParserError(this->cline, u8"Missing } in range repeat"));
                    }

                    if(min == 0 && max == 0) {
                        this->errors.push_back(RegexParserError(this->cline, u8"Invalid range repeat bounds -- both min and max are 0 so the repeat is empty"));
                    }

                    if(min == 1 && max == 1) {
                        this->errors.push_back(RegexParserError(this->cline, u8"Invalid range repeat bounds -- min and max are both 1 so the repeat is redundant"));
                    }

                    if(max < min) {
                        this->errors.push_back(RegexParserError(this->cline, u8"Invalid range repeat bounds -- max is less than min"));
                    }

                    if(min == 0 && max == UINT16_MAX) {
                        return new StarRepeatOpt(rcc);
                    }
                    else if(min == 1 && max == UINT16_MAX) {
                        return new PlusRepeatOpt(rcc);
                    }
                    else if(min == 0 && max == 1) {
                        return new OptionalOpt(rcc);
                    }
                    else {
                        rcc = new RangeRepeatOpt(min, max, rcc);
                    }
                }
            }   

            return rcc;
        }

        const RegexOpt* parseSequenceComponent()
        {
            std::vector<const RegexOpt*> sre;

            while(!this->isEOS() && !this->isToken('&') && !this->isToken('|') && !this->isToken(')')) {
                sre.push_back(this->parseRepeatComponent());
            }

            if(sre.empty()) {
                this->errors.push_back(RegexParserError(this->cline, u8"Empty regex sequence"));
            }

            if (sre.size() == 1) {
                return sre[0];
            }
            else {
                return new SequenceOpt(sre);
            }
        }

        const RegexOpt* parseAnyOfComponent()
        {
            std::vector<const RegexOpt*> are = {this->parseSequenceComponent()};

            while (this->isToken('|')) {
                this->advance();
                are.push_back(this->parseSequenceComponent());
            }

            if(are.size() == 1) {
                return are[0];
            }
            else {
                return new AnyOfOpt(are);
            }
        }

        const RegexOpt* parsePositiveComponent()
        {
            return this->parseAnyOfComponent();
        }

        RegexToplevelEntry parseSingleToplevelRegexComponent()
        {
            std::vector<const RegexOpt*> are;

            this->advanceTriviaOnly();
            bool isNegate = false;
            bool isFrontCheck = false;
            bool isBackCheck = false;

            while(this->isToken('!') || this->isToken('$') || this->isToken('^')) {
                if(this->isToken('!')) {
                    if(isNegate) {
                        this->errors.push_back(RegexParserError(this->cline, u8"Invalid regex -- multiple negations"));
                    }
                    else {
                        isNegate = true;
                    }
                }
                else if(this->isToken('$')) {
                    if(isBackCheck) {
                        this->errors.push_back(RegexParserError(this->cline, u8"Invalid regex -- multiple back checks"));
                    }
                    else {
                        isBackCheck = true;
                    }
                }
                else if(this->isToken('^')) {
                    if(isFrontCheck) {
                        this->errors.push_back(RegexParserError(this->cline, u8"Invalid regex -- multiple front checks"));
                    }
                    else {
                        isFrontCheck = true;
                    }
                }

                this->advance();
            }

            if(isFrontCheck && isBackCheck) {
                this->errors.push_back(RegexParserError(this->cline, u8"Invalid regex -- front and back checks cannot be used together"));
            }

            const RegexOpt* popt = this->parsePositiveComponent();
            return RegexToplevelEntry(isNegate, isFrontCheck, isBackCheck, popt);
        }

        std::pair<RegexToplevelEntry, RegexTopLevelAllOf> parseToplevelRegexComponent()
        {
            std::vector<RegexToplevelEntry> are;

            are.push_back(this->parseSingleToplevelRegexComponent());
            while (this->isToken('&')) {
                this->advance();
                are.push_back(this->parseSingleToplevelRegexComponent());
            }

            if(std::all_of(are.cbegin(), are.cend(), [](const RegexToplevelEntry& e) { return e.isFrontCheck || e.isBackCheck; })) {
                this->errors.push_back(RegexParserError(this->cline, u8"Invalid regex -- all top-level components are front or back checks"));
            }

            if(are.size() == 1) {
                return std::make_pair(are[0], RegexTopLevelAllOf{});
            }
            else {
                return std::make_pair(RegexToplevelEntry{}, RegexTopLevelAllOf(are));
            }
        }

    public:
        static std::pair<std::optional<Regex*>, std::vector<RegexParserError>> parseRegex(uint8_t* data, size_t len, bool isUnicode, bool isStrictASCII, bool isPath, bool isResource)
        {
            if(len < 1) {
                return std::make_pair(std::nullopt, std::vector<RegexParserError>{RegexParserError(0, u8"Empty string is not a valid regex -- must be of form /.../")});
            }

            if(*data != '/') {
                return std::make_pair(std::nullopt, std::vector<RegexParserError>{RegexParserError(0, u8"Invalid regex -- must start with /")});
            }
            if(*(data + len - 1) != '/') {
                return std::make_pair(std::nullopt, std::vector<RegexParserError>{RegexParserError(0, u8"Invalid regex -- must end with /")});
            }

            auto parser = RegexParser(data + 1, len - 2, isUnicode, isStrictASCII, isResource);

            auto rr = parser.parseToplevelRegexComponent();

            if(parser.cpos != parser.epos) {
                parser.errors.push_back(RegexParserError(parser.cline, u8"Invalid regex -- trailing characters after end of regex"));
            }

            if(!parser.errors.empty()) {
                return std::make_pair(std::nullopt, parser.errors);
            }

            auto chartype = isUnicode ? RegexCharInfoTag::Unicode : RegexCharInfoTag::ASCII;
            auto kindtag = isPath ? RegexKindTag::Path : (isResource ? RegexKindTag::Resource : RegexKindTag::Std);

            //TODO: maybe semantic check on containsable that:
            // (1) does not contain epsilon
            // (2) has an anchor set for the front *OR* back of the match -- e.g. epsilon is not a prefix or suffix
            // (3) neither front/back match all chars

            bool isContainsable = false;
            bool isMatchable = false;
            if(rr.second.isEmpty()) {
                isContainsable = !rr.first.isFrontCheck && !rr.first.isBackCheck && !rr.first.isNegated;
                isMatchable = !rr.first.isFrontCheck && !rr.first.isBackCheck && !rr.first.isNegated;
            }
            else {
                //isContainsable stays false
                isMatchable = std::any_of(rr.second.musts.cbegin(), rr.second.musts.cend(), [](const RegexToplevelEntry& opt) { 
                    return !opt.isNegated && !opt.isFrontCheck && !opt.isBackCheck;
                });
            }

            return std::make_pair(std::make_optional(new Regex(kindtag, chartype, isContainsable, isMatchable, rr.first, rr.second)), std::vector<RegexParserError>());
        }

        static std::pair<std::optional<Regex*>, std::vector<RegexParserError>> parseUnicodeRegex(const std::u8string& re)
        {
            return parseRegex((uint8_t*)re.c_str(), re.size(), true, false, false, false);
        }

        static std::pair<std::optional<Regex*>, std::vector<RegexParserError>> parseASCIIRegex(const std::string& re)
        {
            return parseRegex((uint8_t*)re.c_str(), re.size(), false, true, false, false);
        }
    };
}
