#pragma once

#include "../common.h"

// TODO: Add true BSQON support

/*
 *  Types to be used by glob_parser and glob_compiler
 */

namespace brex { 
    enum class GlobExprTag {
        Literal,
        Union,
        Substitution,
        Wildcard,
        Sequence,
    };
    class GlobExpr { 
        public:
            const GlobExprTag tag;
            GlobExpr(GlobExprTag tag): tag(tag) {;}
            virtual ~GlobExpr() {;}

            virtual bool needsParens() const { return false; }
            virtual bool needsVarEnc() const { return false; } // Needs Variable Enclosure, ie ${<var_name>}

            virtual std::string toBSQStandard() const = 0;
    };

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

    class ExpressionFragment : public GlobFragment {
        public:
            const GlobExpr* expression;

            ExpressionFragment(const GlobExpr* expression): GlobFragment(GlobFragmentTag::Expression), expression(expression) {;}
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

    class SequenceExpr : public GlobExpr {
        public:
            std::vector<const GlobExpr*> subexprs;
            SequenceExpr(std::vector<const GlobExpr*> subexprs) : GlobExpr(GlobExprTag::Sequence), subexprs(subexprs) {;}
            virtual ~SequenceExpr() = default;

            virtual std::string toBSQStandard() const override final {
                auto str = std::string();
                for (auto expr : subexprs) {
                    str.append(expr->toBSQStandard());
                }
                return str;
            }
    };

    class LiteralExpr : public GlobExpr { // Regex analog is 'Literal'
        // TODO: Merge these when they appear in sequence together, this implementation is a little sloppy, though it does work.
        public:
            const RegexChar code;
            const bool isunicode;

            LiteralExpr(RegexChar code, bool isunicode) : GlobExpr(GlobExprTag::Literal), code(code), isunicode(isunicode) {;}
            virtual ~LiteralExpr() = default;

            virtual std::string toBSQStandard() const override final {
                if (this->isunicode) {
                    return "'" + processRegexCharToBsqStandard(this->code) + "'";
                }
                else {
                    return "'" + processRegexCharToBsqStandard(this->code) + "'";
                }
            }
    };

    class UnionExpr : public GlobExpr { // Regex analog is 'AnyOf'
        public:
            const std::vector<const GlobExpr*> exprs;
            
            UnionExpr(std::vector<const GlobExpr*> exprs) : GlobExpr(GlobExprTag::Union), exprs(exprs) {;} 
            virtual ~UnionExpr() = default;
            
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

    class SubstitutionExpr : public GlobExpr {
        public:
            const std::string name;

            SubstitutionExpr(std::string name) : GlobExpr(GlobExprTag::Substitution), name(name) {;}
            virtual ~SubstitutionExpr() = default;

            virtual std::string toBSQStandard() const override final {
                return "${" + name + "}"; 
            }
    };

    class WildcardExpr : public GlobExpr {
        public:
            WildcardExpr() : GlobExpr(GlobExprTag::Wildcard) {;}
            virtual ~WildcardExpr() = default;

            virtual std::string toBSQStandard() const override final {
                return "*_s";
            }
    };

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