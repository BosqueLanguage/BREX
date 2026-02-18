#pragma once
#include "../common.h"
#include "glob.h"

// Default path separator
#define BREX_GLOB_PATHSEP '/'

// Substitution syntax
#define BREX_GLOB_SUB_PREFIX_0 '$'
#define BREX_GLOB_SUB_PREFIX_1 '{'
#define BREX_GLOB_SUB_SUFFIX '}'

// Union syntax
#define BREX_GLOB_OPEN_UNION '('
#define BREX_GLOB_SEP_UNION '|'
#define BREX_GLOB_CLOSE_UNION ')'

// Wildcard syntax
#define BREX_GLOB_WILDCARD '*'
#define BREX_GLOB_WILDCARD_MAKE_RECURSIVE '*'

namespace brex {
    class GlobParser {
        public:
            const uint8_t* data; // Bytes of Glob plaintext
            uint8_t* cpos; // Pointer to current token
            const uint8_t* epos; // Pointer to last token

            const bool isUnicode;

            GlobParser(const uint8_t* data, size_t len, bool isUnicode) : 
                data(data), 
                cpos(const_cast<uint8_t*>(data)), epos(data + len),
                isUnicode(isUnicode) {;}
            ~GlobParser() = default;

            // Checks if parser is at end of sequence
            inline bool isEOS() const {
                return this->cpos >= this->epos;
            }

            // Checks if the current token being processed is equal to the given
            // token.
            inline bool isToken(uint8_t tk) const {
                return !this->isEOS() && *this->cpos == tk;
            }

            inline bool isSubstitutionPrefix() {
                return (this->cpos + 2 <= this->epos) && (*this->cpos == BREX_GLOB_SUB_PREFIX_0 && *(this->cpos + 1) == BREX_GLOB_SUB_PREFIX_1); 
            }

            inline bool isRecursiveWildcard() {
                return (this->cpos + 2 <= this->epos) && (*this->cpos == BREX_GLOB_WILDCARD && *(this->cpos + 1) == BREX_GLOB_WILDCARD_MAKE_RECURSIVE);
            }

            inline uint8_t token() const {
                return *this->cpos;
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

            char parseSubstitionNameChar() {
                char c = (char) *(this->cpos);
                this->advance();
                return c;
            }

            /**
             * Top level parser for a glob path fragment.
             */
            GlobFragment* parseFragment() {
                if (this->isRecursiveWildcard()) {
                    this->advance();
                    this->advance();
                    return new RecursiveWildcardFragment();
                }
                return new ExpressionFragment(this->parseExprSequence());
            }

            /**
             * Parser for an Expression Sequence - Multiple expressions chained
             * one after another.
             */
            const GlobExpression* parseExprSequence() {
                std::vector<const GlobExpression*> expr;

                // Loop until we hit something that closes the current scope
                while (!this->isEOS() &&
                       !this->isToken(BREX_GLOB_PATHSEP) &&
                       !this->isToken(BREX_GLOB_CLOSE_UNION) && 
                       !this->isToken(BREX_GLOB_SEP_UNION)) {
                    expr.push_back(this->parseExprBase());
                }
                return new SequenceExpression(expr);
            }

            /**
             * Parser for a base non-sequence expression. May delegate to Union,
             * delegate to Substition, or process a literal character.
             */
            const GlobExpression* parseExprBase() {
                const GlobExpression* ret = nullptr;
               
                if (this->isToken(BREX_GLOB_WILDCARD)) {
                    this->advance();
                    ret = new WildcardExpression();
                }
                else if (this->isToken(BREX_GLOB_OPEN_UNION)) {
                    this->advance();

                    ret = this->parseUnionComponent();
                    
                    if (this->isToken(BREX_GLOB_CLOSE_UNION)) {
                        this->advance();
                    }
                    else {
                        // TODO: Errors
                    }
                }
                else if (this->isSubstitutionPrefix()) {
                    this->advance();
                    this->advance();
                    ret = this->parseSubstitutionExpr();

                    if (this->isToken(BREX_GLOB_SUB_SUFFIX)) {
                        this->advance();
                    }
                    else {
                        // TODO: Errors
                    }
                }
                else {
                    RegexChar c = parseRegexChar(this->isUnicode);
                    ret = new LiteralExpression(c, this->isUnicode);
                }

                return ret;
            }

            /**
             * Parser for a Union. Collects expression sequences until closed
             * by a CLOSE_UNION.
             */
            const GlobExpression* parseUnionComponent() {
                std::vector<const GlobExpression*> union_exprs;

                while (!this->isToken(BREX_GLOB_CLOSE_UNION)) {
                    union_exprs.push_back(this->parseExprSequence());

                    if (this->isToken(BREX_GLOB_SEP_UNION)) {
                        this->advance();
                    }
                    else if (!this->isToken(BREX_GLOB_CLOSE_UNION)) {
                        // TODO: Errors
                        RegexChar code = 0;
                        this->advance();
                        return new LiteralExpression(code, this->isUnicode);
                    }
                }

                return new UnionExpression(union_exprs);
            }

            const GlobExpression* parseSubstitutionExpr() {
                std::string name;
                while (!this->isEOS() && !this->isToken(BREX_GLOB_SUB_SUFFIX)) {
                    name.push_back(this->parseSubstitionNameChar());
                }

                SubstitutionExpression* expr = new SubstitutionExpression(name);
                return expr;
            }

            const std::vector<const GlobFragment*> parseGlobFragments() {
                std::vector<const GlobFragment*> fragments;

                while (!this->isEOS()) {
                    if (this->isToken(BREX_GLOB_PATHSEP)){
                        this->advance();
                    }
                    fragments.push_back(this->parseFragment());
                }

                return fragments;
            }

            // TODO: Expose another static function returning a GlobExpression
            // for handling compilation of substitution segments.
            static Glob* parseGlob(uint8_t* data, size_t datalen, bool is_unicode) {
                auto parser = GlobParser(data, datalen, is_unicode);
                return new Glob(parser.parseGlobFragments());
            }
    };
}

#undef BREX_GLOB_PATHSEP
#undef BREX_GLOB_SUB_PREFIX_0
#undef BREX_GLOB_SUB_PREFIX_1
#undef BREX_GLOB_SUB_SUFFIX
#undef BREX_GLOB_OPEN_UNION
#undef BREX_GLOB_SEP_UNION
#undef BREX_GLOB_CLOSE_UNION
#undef BREX_GLOB_WILDCARD
#undef BREX_GLOB_WILDCARD_MAKE_RECURSIVE