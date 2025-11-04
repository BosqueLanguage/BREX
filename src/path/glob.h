#pragma once

#include "../common.h"

// I want to ask for clarity on the desire to preserve path segments, I'm not
// sure what to take away from that

// TODO: Add BSQON support

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

            // TODO: IMPLEMENT ME FOR DEBUGGING!
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
    };

    class ExpressionFragment : public GlobFragment {
        public:
            const GlobExpr* expression;

            ExpressionFragment(const GlobExpr* expression): GlobFragment(GlobFragmentTag::Expression), expression(expression) {;}
            virtual ~ExpressionFragment() = default;
    };

    class RecursiveWildcardFragment : public GlobFragment {
        public:
            RecursiveWildcardFragment(): GlobFragment(GlobFragmentTag::RecursiveWildcard) {;}
            virtual ~RecursiveWildcardFragment() = default;
    };

    class SequenceExpr : public GlobExpr {
        public:
            std::vector<const GlobExpr*> subexprs;
            SequenceExpr(std::vector<const GlobExpr*> subexprs) : GlobExpr(GlobExprTag::Sequence), subexprs(subexprs) {;}
            virtual ~SequenceExpr() = default;
    };

    class LiteralExpr : public GlobExpr { // Regex analog is 'Literal'
        public:
            const RegexChar code;
            const bool isunicode;

            LiteralExpr(RegexChar code, bool isunicode) : GlobExpr(GlobExprTag::Literal), code(code), isunicode(isunicode) {;}
            virtual ~LiteralExpr() = default;

            virtual std::string toBSQStandard() const override final {
                if (this->isunicode) {
                    return "\"" + processRegexCharToBsqStandard(this->code) + "\"";
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
    };

    // TODO: Talk with Dr. Marron about implementation for this, I'm not 100% sure I recognize how to do this
    // class SubstitutionExpr : public GlobExpr {
    //     public:
            
    // };

    class WildcardExpr : public GlobExpr {
        public:
            WildcardExpr() : GlobExpr(GlobExprTag::Wildcard) {;}
            virtual ~WildcardExpr() = default;
    };

    class Glob {
        public:
            const std::vector<const GlobFragment*> fragments;

            Glob(std::vector<const GlobFragment*> fragments) : fragments(fragments) {;}
            ~Glob() = default;
    };
}