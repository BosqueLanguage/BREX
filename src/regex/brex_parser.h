#define once

#include "common.h"
#include "brex.h"

#include <regex>

namespace BREX
{
    std::vector<uint8_t> s_whitespacechars = { ' ', '\t', '\n', '\r', '\v', '\f' };
    std::vector<uint8_t> s_doubleslash = { '%', '%' };
    std::vector<uint8_t> s_namedpfx = {'$', '{'};

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
        bool negateAllowed;
        bool envAllowed;

        size_t cline;
        std::vector<RegexParserError> errors;


        RegexParser(const uint8_t* data, size_t len, bool isUnicode, bool negateAllowed, bool envAllowed) : data(data), cpos(const_cast<uint8_t*>(data)), epos(data + len), isUnicode(isUnicode), negateAllowed(negateAllowed), envAllowed(envAllowed), cline(0), errors() {;}
        ~RegexParser() = default;

        inline bool isEOS() const
        {
            return this->cpos == this->epos;
        }

        inline bool isToken(uint8_t tk) const
        {
            return !this->isEOS() && *this->cpos == tk;
        }

        inline bool isTokenOneOf(const std::vector<uint8_t>& tks) const
        {
            return !this->isEOS() && std::find(tks.begin(), tks.end(), *this->cpos) != tks.end();
        }

        inline bool isTokenPrefix(const std::vector<uint8_t>& tks) const
        {
            for(size_t i = 0; i < tks.size(); ++i) {
                if(this->cpos + i == this->epos || *(this->cpos + i) != tks[i]) {
                    return false;
                }
            }

            return true;
        }

        inline uint8_t token() const
        {
            return *this->cpos;
        }

        void advanceTriviaOnly()
        {
            while(!this->isEOS() && (this->isTokenOneOf(s_whitespacechars) || this->isTokenPrefix(s_doubleslash))) {
                if(this->isToken('\n')) {
                    this->cline++;
                }

                if(this->isTokenOneOf(s_whitespacechars)) {
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

        const RegexChar parseRegexChar(bool unicodeok)
        {
            auto c = this->token();
            if(c == U'/' || c == U'\\' || (c <= 127 && !std::isprint(c))) {
                auto esccname = parserGenerateDiagnosticUnicodeEscapeName(c);
                auto esccode = parserGenerateDiagnosticEscapeCode(c);
                this->errors.push_back(RegexParserError(this->cline, u8"Newlines, slash chars, and non-printable chars are not allowed in regexes -- escape them with " + esccname + u8" or " + esccode));
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
                auto encerr = parserValidateAllASCIIEncoding_SingleChar(this->cpos, this->epos);
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
                    ccode = unescapeSingleRegexChar(this->cpos, tpos);
                }
                else {
                    ccode = unescapeSingleASCIIRegexChar(this->cpos, tpos);
                }

                if(ccode.has_value()) {
                    code = ccode.value();
                }
                else {
                    auto errors = parserValidateEscapeSequences(false, this->cpos, this->cpos + bytecount);
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
                return new LiteralOpt({ }, true);
            }

            auto bytechecks = parserValidateUTF8ByteEncoding(this->cpos + 1, this->cpos + 1 + length);
            if(bytechecks.has_value()) {
                this->errors.push_back(RegexParserError(this->cline, bytechecks.value()));
                return new LiteralOpt({ }, true);
            }
            else {
                this->cpos = curr + 1;

                auto codes = unescapeRegexLiteral(this->cpos + 1, length);
                if(!codes.has_value()) {
                    auto errors = parserValidateEscapeSequences(false, this->cpos + 1, this->cpos + 1 + length);
                    for(auto ii = errors.cbegin(); ii != errors.cend(); ii++) {
                        this->errors.push_back(RegexParserError(this->cline, *ii));
                    }

                    return new LiteralOpt({ }, true);
                }
                else {
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
                return new LiteralOpt({ }, false);
            }

            auto bytechecks = parserValidateAllASCIIEncoding(this->cpos + 1, this->cpos + 1 + length);
            if(bytechecks.has_value()) {
                this->errors.push_back(RegexParserError(this->cline, bytechecks.value()));
                return new LiteralOpt({ }, false);
            }
            else {
                this->cpos = curr + 1;

                auto codes = unescapeRegexLiteral(this->cpos + 1, length);
                if(!codes.has_value()) {
                    auto errors = parserValidateEscapeSequences(true, this->cpos + 1, this->cpos + 1 + length);
                    for(auto ii = errors.cbegin(); ii != errors.cend(); ii++) {
                        this->errors.push_back(RegexParserError(this->cline, *ii));
                    }

                    return new LiteralOpt({ }, false);
                }
                else {
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
            while(!this->isToken(']')) {
                auto lb = this->parseRegexChar(unicodeok);

                if (!this->isToken(U'-')) {
                    range.push_back({ lb, lb });
                }
                else {
                    this->cpos++;

                    auto ub = this->parseRegexChar(unicodeok);
                    range.push_back({ lb, ub });
                }
            }

            if(this->isToken(']')) {
                this->cpos++;
            }
            else {
                this->errors.push_back(RegexParserError(this->cline, u8"Missing ] in regex"));
            }

            return new CharRangeOpt(compliment, range);
        }

        const RegexOpt* parseNamedOrEnvRegex()
        {
            this->advance();
            std::u8string name;
            while(!this->isEOS() && !this->isToken('}')) {
                name.push_back(this->token());
                this->cpos++;
            }

            if(this->isToken('}')) {
                this->cpos++;
            }
            else {
                this->errors.push_back(RegexParserError(this->cline, u8"Missing ] in regex"));
            }

            std::regex idre("^[_a-z][_a-zA-Z0-9]*$");
            std::regex scopere("^([A-Z][_a-zA-Z0-9]*::)*[A-Z][_a-zA-Z0-9]*$");

            if(name.starts_with(u8"env[" && name.ends_with(u8"]"))) {
                if(!this->envAllowed) {
                    this->errors.push_back(RegexParserError(this->cline, u8"Env regexes are not allowed in this context"));
                }
                
                auto ssid = name.substr(4, name.size() - 5);
                if(!std::regex_match(ssid.cbegin(), ssid.cend(), idre)) {
                    this->errors.push_back(RegexParserError(this->cline, u8"Invalid env regex name -- must be a valid identifier"));
                }

                return new EnvRegexOpt(std::string(ssid.cbegin(), ssid.cend()));
            }
            else {
                //it must be a named regex

                auto splitpos = name.find_last_of(u8"::");
                if(splitpos != std::u8string::npos) {
                    xxxx;
                }
                else {
                    xxxx;
                }
            }

            return new NamedRegexOpt(name);
        }

        const RegexOpt* parseBaseComponent() 
        {
            //make sure we get any trivia out of the way
            this->advanceTriviaOnly(); 

            const RegexOpt* res = nullptr;
            if(this->isToken('(')) {
                this->advance();

                res = this->parseComponent();
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
                res = new CharClassDotOpt();
            }
            else if(this->isTokenPrefix(s_namedpfx)) {
                res = this->parseNamedOrEnvRegex();
            }
            else {
                std::u8string slice(this->cpos, this->cpos + charCodeByteCount(this->cpos));
                this->errors.push_back(RegexParserError(this->cline, u8"Invalid regex component -- expected (, [, ', \", ${, or . but found " + slice));
                res = new LiteralOpt({}, false);
            }

            //make sure we get any trivia out of the way
            this->advanceTriviaOnly();

            return res;
        }

        const RegexOpt* parseNegateOpt()
        {
            this->advanceTriviaOnly();

            auto isnegate = this->isToken('!');
            auto orignegageallowed = this->negateAllowed;

            if(isnegate) {
                this->advance();

                if(!this->negateAllowed) {
                    this->errors.push_back(RegexParserError(this->cline, u8"Negate operator ! is not allowed in this context"));
                }
                this->negateAllowed = false;
            }

            const RegexOpt* opt = this->parseBaseComponent();
            this->negateAllowed = orignegageallowed;
                
            if(!isnegate) {
                return opt;
            }
            else {
                return new NegateOpt(opt);
            }
        }

        const RegexOpt* parseRepeatComponent()
        {
            auto rcc = this->parseNegateOpt();
            if(rcc == nullptr) {
                return nullptr;
            }

            while(this->isToken('*') || this->isToken('+') || this->isToken('?') || this->isToken('{')) {
                if(this->isToken(U'*')) {
                    rcc = new BSQStarRepeatRe(rcc);
                    this->advance();
                }
                else if(this->isToken(U'+')) {
                    rcc = new BSQPlusRepeatRe(rcc);
                    this->advance();
                }
                else if(this->isToken(U'?')) {
                    rcc = new BSQOptionalRe(rcc);
                    this->advance();
                }
                else {
                    this->advance();
                    uint16_t min = 0;
                    while(!this->done() && U'0' < this->token() && this->token() < U'9') {
                        min = min * 10 + (this->token() - U'0');
                        this->advance();
                    }

                    while(!this->done() && this->isToken(U' ')) {
                        this->advance();
                    }

                    uint16_t max = min;
                    if (!this->done() && this->isToken(U',')) {
                        this->advance();

                        while(!this->done() && this->isToken(U' ')) {
                            this->advance();
                        }

                        if(!this->done() && !this->isToken(U'}')) {
                            max = 0;
                            while(!this->done() && U'0' < this->token() && this->token() < U'9') {
                                max = max * 10 + (this->token() - U'0');
                                this->advance();
                            }
                        }
                    }

                    if(this->done() || !this->isToken(U'}')) {
                        return nullptr;
                    }
                    this->advance();

                    rcc = new BSQRangeRepeatRe(min, max, rcc);
                }

                this->advanceTriviaOnly();
            }   

            return rcc;
        }

        const BSQRegexOpt* parseSequenceComponent()
        {
            std::vector<const BSQRegexOpt*> sre;

            while(!this->done() && !this->isToken(U'|') && !this->isToken(U')')) {
                auto rpe = this->parseRepeatComponent();
                if(rpe == nullptr) {
                    return nullptr;
                }

                if(sre.empty()) {
                    sre.push_back(rpe);
                }
                else {
                    auto lcc = sre[sre.size() - 1];
                    if(lcc->isLiteral() && rpe->isLiteral()) {
                        sre[sre.size() - 1] = BSQLiteralRe::mergeLiterals(static_cast<const BSQLiteralRe*>(lcc), static_cast<const BSQLiteralRe*>(rpe));
                        delete lcc;
                        delete rpe;
                    }
                    else {
                        sre.push_back(rpe);
                    }
                }

                this->advanceTriviaOnly();
            }

            if(sre.empty()) {
                return nullptr;
            }

            if (sre.size() == 1) {
                return sre[0];
            }
            else {
                return new BSQSequenceRe(sre);
            }
        }

        const BSQRegexOpt* parseAlternationComponent()
        {
            auto rpei = this->parseSequenceComponent();
            if (rpei == nullptr) {
                return nullptr;
            }

            std::vector<const BSQRegexOpt*> are = {rpei};

            while (!this->done() && this->isToken(U'|')) {
                this->advance();
                auto rpe = this->parseSequenceComponent();
                if (rpe == nullptr) {
                    return nullptr;
                }

                are.push_back(rpe);
            }

            if(are.size() == 1) {
                return are[0];
            }
            else {
                return new BSQAlternationRe(are);
            }
        }

        const BSQRegexOpt* parseComponent()
        {
            return this->parseAlternationComponent();
        }

    public:
        static BSQRegex* parseRegex(UnicodeString restr)
        {
            auto parser = RegexParser(restr);

            auto re = parser.parseComponent();
            if(re == nullptr) {
                return nullptr;
            }

            std::vector<NFAOpt*> nfastates = { new NFAOptAccept(0) };
            auto nfastart = re->compile(0, nfastates);

            auto nfare = new NFA(nfastart, 0, nfastates);
            return new BSQRegex(re, nfare);
        }
    };

}
