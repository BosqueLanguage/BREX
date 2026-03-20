#pragma once

#include "../common.h"

// TODO: Add true BSQON support
// TODO: Actual Unicode support without just relying on blind luck
//   - I need a better understanding of this!

namespace brex {

    /// @brief Glob Fragment Types
    enum class GlobFragmentTag {
        Expression,
        RecursiveWildcard,
    };

    /// @brief Glob Fragment Base Type
    class GlobFragment {
        public:
            const GlobFragmentTag tag;
            GlobFragment(GlobFragmentTag tag): tag(tag) {;}
            virtual ~GlobFragment() {;}

            virtual std::u8string toBSQONFormat() const = 0;
    };

    /// @brief Glob Expression Types
    enum class GlobExpressionTag {
        Literal,
        Union,
        Substitution,
        Wildcard,
        Sequence,
    };

    /// @brief Glob Expression Base Type
    class GlobExpression { 
        public:
            const GlobExpressionTag tag;
            GlobExpression(GlobExpressionTag tag): tag(tag) {;}
            virtual ~GlobExpression() {;}

            virtual bool needsParens() const { return false; }
            virtual bool needsVarEnc() const { return false; } // Needs Variable Enclosure, ie ${<var_name>}

            virtual std::u8string toBSQONFormat() const = 0;
    };

    /// @brief Glob Expression-Containing Fragment
    class ExpressionFragment : public GlobFragment {
        public:
            const GlobExpression* expression;

            ExpressionFragment(const GlobExpression* expression): GlobFragment(GlobFragmentTag::Expression), expression(expression) {;}
            virtual ~ExpressionFragment() = default;
 
            virtual std::u8string toBSQONFormat() const override final {
                return expression->toBSQONFormat();
            }
    };

    /// @brief Glob Recursive Wildcard Fragment
    class RecursiveWildcardFragment : public GlobFragment {
        public:
            RecursiveWildcardFragment(): GlobFragment(GlobFragmentTag::RecursiveWildcard) {;}
            virtual ~RecursiveWildcardFragment() = default;

            virtual std::u8string toBSQONFormat() const override final {
                return u8"**";
            }
    };

    /// @brief Glob Expression containing an arbitrary number of sub-expressions
    class SequenceExpression : public GlobExpression {
        public:
            std::vector<const GlobExpression*> subexprs;
            SequenceExpression(std::vector<const GlobExpression*> subexprs) : GlobExpression(GlobExpressionTag::Sequence), subexprs(subexprs) {;}
            virtual ~SequenceExpression() = default;

            virtual std::u8string toBSQONFormat() const override final {
                auto str = std::u8string();
                for (auto expr : subexprs) {
                    str.append(expr->toBSQONFormat());
                }
                return str;
            }
    };

    /// @brief Glob Expression containing a character (ascii or unicode)
    class LiteralExpression : public GlobExpression { // Regex analog is 'LiteralOpt'
        public:
            const RegexChar code;
            const bool isunicode;

            LiteralExpression(RegexChar code, bool isunicode) : GlobExpression(GlobExpressionTag::Literal), code(code), isunicode(isunicode) {;}
            virtual ~LiteralExpression() = default;

            virtual std::u8string toBSQONFormat() const override final {
                if (this->isunicode) {
                    std::vector<uint8_t> bytes = escapeRegexUnicodeLiteralCharBuffer({this->code});
                    return u8"'" + std::u8string(bytes.begin(), bytes.end()) + u8"'";

                }
                else {
                    std::vector<uint8_t> bytes = escapeRegexCLiteralCharBuffer({this->code});
                    return u8"'" + std::u8string(bytes.begin(), bytes.end()) + u8"'";
                }
            }
    };

    /// @brief Glob Expression representing one of several choices of sub-expression
    class UnionExpression : public GlobExpression { // Regex analog is 'AnyOfOpt'
        public:
            const std::vector<const GlobExpression*> exprs;
            
            UnionExpression(std::vector<const GlobExpression*> exprs) : GlobExpression(GlobExpressionTag::Union), exprs(exprs) {;} 
            virtual ~UnionExpression() = default;
            
            virtual bool needsParens() const override final { return true; }

            virtual std::u8string toBSQONFormat() const override final {
                auto str = std::u8string(u8"(");
                for (auto expr : exprs) {
                    str.append(expr->toBSQONFormat());
                    str.push_back('|');
                }
                str[str.length() - 1] = ')';
                return str;
            }
    };

    /// @brief Glob Expression that marks some identifier which can be replaced later.
    class SubstitutionExpression : public GlobExpression {
        public:
            // I don't actually care that the unicode character is or looks like, I just want to
            // make sure the bytes are the same.

            const std::u8string name;

            SubstitutionExpression(std::u8string name) : GlobExpression(GlobExpressionTag::Substitution), name(name) {;}
            virtual ~SubstitutionExpression() = default;

            virtual std::u8string toBSQONFormat() const override final {
                return u8"${" + this->name + u8"}"; 
            }
    };

    /// @brief Glob Expression for a wildcard, consuming 0 or more characters.
    class WildcardExpression : public GlobExpression {
        public:
            WildcardExpression() : GlobExpression(GlobExpressionTag::Wildcard) {;}
            virtual ~WildcardExpression() = default;

            virtual std::u8string toBSQONFormat() const override final {
                return u8"*";
            }
    };

    /// @brief Root of a glob AST, made up of a sequence of fragments
    class Glob {
        public:
            const std::vector<const GlobFragment*> fragments;

            Glob(std::vector<const GlobFragment*> fragments) : fragments(fragments) {;}
            ~Glob() = default;

            std::u8string toBSQStandard() const {
                auto str = std::u8string();
                for (auto f : fragments) {
                    str.append(f->toBSQONFormat());
                    str.push_back('/');
                }
                str[str.length() - 1] = ' ';
                return str;
            }
    };
}