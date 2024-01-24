#define once

#include "common.h"
#include "brex.h"

namespace BREX
{
    std::vector<uint8_t> s_whitespacechars = { ' ', '\t', '\n', '\r', '\v', '\f' };
    std::vector<uint8_t> s_doubleslash = { '%', '%' };

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

        inline bool isTokenOneOf(std::vector<uint8_t> tks) const
        {
            return !this->isEOS() && std::find(tks.begin(), tks.end(), *this->cpos) != tks.end();
        }

        inline bool isTokenPrefix(std::vector<uint8_t> tks) const
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

        void advanceTrivia()
        {
            while(!this->isEOS() && (this->isTokenOneOf(s_whitespacechars) || this->isTokenPrefix(s_doubleslash))) {
                if(this->isToken('\n')) {
                    this->cline++;
                }

                if(this->isTokenOneOf(s_whitespacechars)) {
                    this->advance();
                }
                else {
                    while(!this->isEOS() && !this->isToken('\n')) {
                        this->advance();
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

            this->advanceTrivia();
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

            this->advanceTrivia();
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

            auto bytechecks = parserValidateUTF8ByteEncoding(this->cpos + 1, this->cpos + 1 + length);
            if(!bytechecks.empty()) {
                for(auto ii = bytechecks.cbegin(); ii != bytechecks.cend(); ii++) {
                    this->errors.push_back(RegexParserError(this->cline, *ii));
                }

                return new LiteralOpt({ }, true);
            }

            if(curr == this->epos) {
                this->errors.push_back(RegexParserError(this->cline, u8"Unterminated regex literal"));
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
            return new LiteralOpt(codes, true);
        }

        const RegexOpt* parseBaseComponent() 
        {
            const RegexOpt* res = nullptr;
            if(this->isToken('(')) {
                this->advance();

                res = this->parseComponent();
                if(!this->isToken(')')) {
                    return nullptr;
                }

                this->advance();
            }
            else if(this->isToken('"')) {
                return this->parseUnicodeLiteral();
            }
            else if(this->isToken('\'')) {
                return this->parseASCIILiteral();
            }
            else if(this->isToken(U'[')) {
                this->advance();

                auto compliment = this->isToken(U'^');
                if(compliment) {
                    this->advance();
                }

                std::vector<SingleCharRange> range;
                while(!this->isToken(U']')) {
                    auto lb = this->readUnescapedChar();

                    if (!this->isToken(U'-')) {
                        range.push_back({ lb, lb });
                    }
                    else {
                        this->advance();

                        auto ub = this->token();
                        range.push_back({ lb, ub });
                    }
                }

                if(!this->isToken(U']')) {
                    return nullptr;
                }
                this->advance();

                return new BSQCharRangeRe(compliment, range);
            }
            else {
                res = new BSQLiteralRe({ this->readUnescapedChar() });
            }

            return res;
        }

        const BSQRegexOpt* parseCharClassOrEscapeComponent()
        {
            if(this->isToken(U'.')) {
                this->advance();
                return new BSQCharClassDotRe();
            }
            else {
                return this->parseBaseComponent();
            }
        }

        const BSQRegexOpt* parseRepeatComponent()
        {
            auto rcc = this->parseCharClassOrEscapeComponent();
            if(rcc == nullptr) {
                return nullptr;
            }

            while(this->isToken(U'*') || this->isToken(U'+') || this->isToken(U'?') || this->isToken(U'{')) {
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
