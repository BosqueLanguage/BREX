#include "glob.h"
#include "glob_compiler.h"
#include "glob_executor.h"

namespace brex
{
    CompiledSegment* compileExpression(const GlobExpr* expr, CompiledSegment* followed_by) {

    }

    CompiledSegment* compileLiteral(const LiteralExpr* expr, CompiledSegment* followed_by) {
        std::vector<uint8_t> bytes = extractRegexCharToBytes(expr->code);
        CompiledSegment* current = followed_by;
        for (auto it = bytes.crbegin(); it != bytes.crend(); it++) {
            current = new CompiledSegment({{*it, current}}, false);
        }
        return current;
    }

    CompiledSegment* compileSequence(const SequenceExpr* expr, CompiledSegment* followed_by) {
        const std::vector<const GlobExpr*> subexprs = expr->subexprs;
        CompiledSegment* current = followed_by;
        for (auto it = subexprs.crbegin(); it != subexprs.crend(); it++) {
            current = compileExpression(*it, current);
        }
        return current;
    }

    CompiledSegment* compileWildcard(const WildcardExpr* expr, CompiledSegment* followed_by) {
        followed_by->is_wildcard = true;
        return followed_by;
    }

    CompiledSegment* compileUnion(const UnionExpr* expr, CompiledSegment* followed_by) {
        const std::vector<const GlobExpr*> subexprs = expr->exprs;

        if (subexprs.size() <= 0) {
            return followed_by;
        }
       
        std::vector<CompiledSegment*> current;
        for (auto it = subexprs.cbegin(); it != subexprs.cend(); it++) {
            current.push_back(compileExpression(*it, followed_by));
        }
       
        // TODO: Merge union expressions
        // Probably something like taking the first one as a base and then tacking on the other parts of the tree at the first divergence, or not at all if the same.
        // This way, if the unions are the same, that whole tree segment gets deleted.
    }

    void compiledSegmentInsert(CompiledSegment* a, uint8_t c, CompiledSegment* b) {
        a->next_states[c] - b;
    }
} // namespace brex

