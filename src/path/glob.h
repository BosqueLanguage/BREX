#pragma once

#include "../common.h"

// TODO: Add true BSQON support

/*
 *  Types to be used by glob_parser and glob_compiler
 */

namespace brex {
    // == Glob Fragment ==
    enum class GlobFragmentTag {
        Expression,
        RecursiveWildcard,
    };

    class GlobFragment {
        public:
            const GlobFragmentTag tag;
            GlobFragment(GlobFragmentTag tag): tag(tag) {;}
            virtual ~GlobFragment() {;}

            virtual std::string toBSQStandard() const = 0;
    };

    // == Glob Expression ==
    enum class GlobExpressionTag {
        Literal,
        Union,
        Substitution,
        Wildcard,
        Sequence,
    };

    class GlobExpression { 
        public:
            const GlobExpressionTag tag;
            GlobExpression(GlobExpressionTag tag): tag(tag) {;}
            virtual ~GlobExpression() {;}

            virtual bool needsParens() const { return false; }
            virtual bool needsVarEnc() const { return false; } // Needs Variable Enclosure, ie ${<var_name>}

            virtual std::string toBSQStandard() const = 0;
    };

    // == Fragment Definitions ==
    class ExpressionFragment : public GlobFragment {
        public:
            const GlobExpression* expression;

            ExpressionFragment(const GlobExpression* expression): GlobFragment(GlobFragmentTag::Expression), expression(expression) {;}
            virtual ~ExpressionFragment() = default;
 
            virtual std::string toBSQStandard() const override final {
                return expression->toBSQStandard();
            }
    };

    class RecursiveWildcardFragment : public GlobFragment {
        public:
            RecursiveWildcardFragment(): GlobFragment(GlobFragmentTag::RecursiveWildcard) {;}
            virtual ~RecursiveWildcardFragment() = default;

            virtual std::string toBSQStandard() const override final {
                return "**_r";
            }
    };

    // == Expression Definitions ==
    class SequenceExpression : public GlobExpression {
        public:
            std::vector<const GlobExpression*> subexprs;
            SequenceExpression(std::vector<const GlobExpression*> subexprs) : GlobExpression(GlobExpressionTag::Sequence), subexprs(subexprs) {;}
            virtual ~SequenceExpression() = default;

            virtual std::string toBSQStandard() const override final {
                auto str = std::string();
                for (auto expr : subexprs) {
                    str.append(expr->toBSQStandard());
                }
                return str;
            }
    };

    class LiteralExpression : public GlobExpression { // Regex analog is 'Literal'
        // TODO: Merge these when they appear in sequence together, this implementation is a little sloppy, though it does work.
        public:
            const RegexChar code;
            const bool isunicode;

            LiteralExpression(RegexChar code, bool isunicode) : GlobExpression(GlobExpressionTag::Literal), code(code), isunicode(isunicode) {;}
            virtual ~LiteralExpression() = default;

            virtual std::string toBSQStandard() const override final {
                if (this->isunicode) {
                    return "'" + processRegexCharToBsqStandard(this->code) + "'";
                }
                else {
                    return "'" + processRegexCharToBsqStandard(this->code) + "'";
                }
            }
    };

    class UnionExpression : public GlobExpression { // Regex analog is 'AnyOf'
        public:
            const std::vector<const GlobExpression*> exprs;
            
            UnionExpression(std::vector<const GlobExpression*> exprs) : GlobExpression(GlobExpressionTag::Union), exprs(exprs) {;} 
            virtual ~UnionExpression() = default;
            
            virtual bool needsParens() const override final { return true; }

            virtual std::string toBSQStandard() const override final {
                auto str = std::string("(");
                for (auto expr : exprs) {
                    str.append(expr->toBSQStandard());
                    str.append("|");
                }
                str[str.length() - 1] = ')';
                return str;
            }
    };

    class SubstitutionExpression : public GlobExpression {
        public:
            const std::string name;

            SubstitutionExpression(std::string name) : GlobExpression(GlobExpressionTag::Substitution), name(name) {;}
            virtual ~SubstitutionExpression() = default;

            virtual std::string toBSQStandard() const override final {
                return "${" + name + "}"; 
            }
    };

    class WildcardExpression : public GlobExpression {
        public:
            WildcardExpression() : GlobExpression(GlobExpressionTag::Wildcard) {;}
            virtual ~WildcardExpression() = default;

            virtual std::string toBSQStandard() const override final {
                return "*_s";
            }
    };

    // == Glob AST Root ==
    class Glob {
        public:
            // TODO: Actually track substitutions
            const std::vector<const GlobFragment*> fragments;

            Glob(std::vector<const GlobFragment*> fragments) : fragments(fragments) {;}
            ~Glob() = default;

            std::string toBSQStandard() const {
                auto str = std::string("");
                for (auto f : fragments) {
                    str.append(f->toBSQStandard());
                    str.append("/");
                }
                str[str.length() - 1] = ' ';
                return str;
            }
    };
}