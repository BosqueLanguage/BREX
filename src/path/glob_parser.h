#pragma once

#include "../common.h"

#include "glob.h"

namespace brex {
    class GlobSegmentParser {
        public:
            const uint8_t* data; // Bytes of Glob
            uint8_t* cpos; // Pointer to current token
            const uint8_t* epos; // Pointer to last token

            const bool isUnicode;
            bool envAllowed;

            GlobSegmentParser(const uint8_t* data, size_t len, bool isUnicode, bool envAllowed) : 
                data(data), 
                cpos(const_cast<uint8_t*>(data)), epos(data + len), 
                isUnicode(isUnicode), 
                envAllowed(envAllowed) {;}
            ~GlobSegmentParser() = default;

            // Checks if parser is at end of sequence
            inline bool isEOS() const {
                return this->cpos == this->epos;
            }

            // Checks if the current token being processed is equal to the given
            // token.
            inline bool isToken(uint8_t tk) const {
                return !this->isEOS() && *this->cpos == tk;
            }

            inline bool isSubstitutionPrefix() {
                return (this->cpos + 2 < this->epos) && (*this->cpos == '$' && *(this->cpos + 1) == '{'); 
            }

            inline uint8_t token() const {
                return *this->cpos;
            }

            // Check if a character is special
            inline bool isSpecial(uint8_t tk) const {
                return false;
            }

            void advance() {
                if (!this->isEOS()) {
                    this->cpos++;
                }
            }

            RegexChar parseRegexChar(bool unicodeok) {
                auto c = this->token();
                if (c <= 127 && !std::isprint(c) && !std::isblank(c)) {
                    auto esccname = parserGenerateDiagnosticUnicodeEscapeName(c);
                    auto esccode = parserGenerateDiagnosticEscapeCode(c);
                    // TODO: Errors
                    this->cpos++;
                    return 0;
                }

                if (unicodeok) {
                    auto encerr = parserValidateUTF8ByteEncoding_SingleChar(this->cpos, this->epos);
                    if (encerr.has_value()) {
                        // TODO: Errors
                        return 0;
                    }
                }
                else {
                    auto encerr = parserValidateAllCEncoding_SingleChar(this->cpos, this->epos);
                    if (encerr.has_value()) {
                        // TODO: Errors
                        return 0;
                    }
                }

                RegexChar code = 0;
                auto bytecount = charCodeByteCount(this->cpos);
                code = toRegexCharCodeFromBytes(this->cpos, bytecount);
                this->cpos += bytecount;
                return code;
            }

            // Don't need to do this at all! Can handle text in the base component, only need to add special stuff for special characters. 
            // TODO: Start writing the base component parser, handle special chars one at a time, write functions as needed.
            const LiteralExpr* parseUnicodeLiteral() {
                size_t length = 0;
                auto curr = this->cpos + 1;
                while (curr != this->epos && this->isSpecial(*curr)) { 
                    if (*curr <= 127 && !std::isprint(*curr) && !std::isblank(*curr)) {
                        auto esccname = parserGenerateDiagnosticUnicodeEscapeName(*curr);
                        auto esccode = parserGenerateDiagnosticEscapeCode(*curr);
                        // TODO: Errors
                        this->cpos++;
                        return 0;
                    }

                    length++;
                    curr++;
                }

                // Need to handle the fact that we're not dealing with a string
                // that has a terminating character.

            }

    };
}