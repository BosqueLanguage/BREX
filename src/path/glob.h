#pragma once

#include "../common.h"

// I want to ask for clarity on the desire to preserve path segments, I'm not
// sure what to take away from that

// TODO: Add BSQON support

/*
 *  Types to be used by glob_parser and glob_compiler
 */

namespace brex {
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
            const std::vector<const GlobExpr*> expressions;

            ExpressionFragment(std::vector<const GlobExpr*> expressions): GlobFragment(GlobFragmentTag::Expression), expressions(expressions) {;}
            virtual ~ExpressionFragment() = default;
    };

    class RecursiveWildcardFragment : public GlobFragment {
        public:
            RecursiveWildcardFragment(): GlobFragment(GlobFragmentTag::RecursiveWildcard) {;}
            virtual ~RecursiveWildcardFragment() = default;
    };
    
    enum class GlobExprTag {
        Literal,
        Union,
        Substitution,
        Wildcard,
    };
    class GlobExpr { 
        public:
            const GlobExprTag tag;
            GlobExpr(GlobExprTag tag): tag(tag) {;}
            virtual ~GlobExpr() {;}

            virtual bool needsParens() const { return false; }
            virtual bool needsVarEnc() const { return false; } // Needs Variable Enclosure, ie ${<var_name>}
    };

    class LiteralExpr : public GlobExpr { // Regex analog is 'Literal'
        public:
            const std::vector<RegexChar> codes;
            const bool isunicode;

            LiteralExpr(std::vector<RegexChar> codes, bool isunicode) : GlobExpr(GlobExprTag::Literal), codes(codes), isunicode(isunicode) {;}
            virtual ~LiteralExpr() = default;
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
            const std::vector<GlobFragment> fragments;

            Glob(std::vector<GlobFragment> fragments) : fragments(fragments) {;}
            ~Glob() = default;
    };
}