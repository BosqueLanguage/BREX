#include "glob.h"
#include "glob_compiler.h"
#include <algorithm>

namespace brex
{
    std::set<size_t>* ExpressionCompiler::compileExpression(const GlobExpression* expr, std::set<size_t>* next_states) {
        if (expr->tag == GlobExpressionTag::Literal) {
            return this->compileLiteral((LiteralExpression*) expr, next_states);
        }
        else if (expr->tag == GlobExpressionTag::Sequence) {
            return this->compileSequence((SequenceExpression*) expr, next_states);
        }
        else if (expr->tag == GlobExpressionTag::Union) {
            return this->compileUnion((UnionExpression*) expr, next_states);
        }
        else if (expr->tag == GlobExpressionTag::Wildcard) {
            return this->compileWildcard((WildcardExpression*) expr, next_states);
        }
        else if (expr->tag == GlobExpressionTag::Substitution) {
            return this->compileSubstitution((SubstitutionExpression*) expr, next_states);
        } 
        // Unreachable.
        return nullptr;
    }

    std::set<size_t>* ExpressionCompiler::compileLiteral(LiteralExpression* expr, std::set<size_t>* next_states) {
        this->states.push_back(new GroundState(expr->code, next_states, {}));
        this->max_index++;
        return new std::set<size_t>({this->max_index});
    }

    std::set<size_t>* ExpressionCompiler::compileSequence(SequenceExpression* expr, std::set<size_t>* next_states) {
        for (auto it = expr->subexprs.rbegin(); it != expr->subexprs.rend(); it++) {
            next_states = this->compileExpression(*it, next_states);
        }
        return next_states;
    }

    std::set<size_t>* ExpressionCompiler::compileWildcard(WildcardExpression* expr, std::set<size_t>* next_states) {
        this->max_index++;
        std::set<size_t>* wildcards = new std::set<size_t>({ this->max_index });
        for (auto it = next_states->begin(); it != next_states->end(); it++) {
            std::set<size_t>* cds = this->states[*it]->default_states;
            if (cds->size() > 0) {
                for (auto jt = cds->cbegin(); jt != cds->cend(); jt++) {
                    wildcards->insert(*jt);
                }
            }
            wildcards->insert(*it);
        }
        
        this->states.push_back(new WildcardState({}, wildcards));
        return new std::set<size_t>({this->max_index});
    }

    std::set<size_t>* ExpressionCompiler::compileUnion(UnionExpression* expr, std::set<size_t>* next_states) {
        this->max_index++;
        std::set<size_t>* next_set = new std::set<size_t>({});
        for (auto it = expr->exprs.begin(); it != expr->exprs.end(); it++) {
            std::set<size_t>* next_branch = this->compileExpression(*it, next_states);
            for (auto jt = next_branch->cbegin(); jt != next_branch->cend(); jt++) {
                next_set->insert(*jt);
            }
        }
        return next_states;
    }

    std::set<size_t>* ExpressionCompiler::compileSubstitution(SubstitutionExpression* expr, std::set<size_t>* next_states) {
        this->states.push_back(new PlaceholderState(expr->name, next_states, {}));
        this->max_index++;
        return new std::set<size_t>({this->max_index});
    }

    ExpressionMachine* ExpressionCompiler::compile(GlobExpression* expr) {
        ExpressionCompiler compiler = ExpressionCompiler();
        std::set<size_t>* start_states = compiler.compileExpression(expr, new std::set<size_t>({}));
        return new ExpressionMachine(start_states, compiler.states);
    }
} // namespace brex
