#pragma once

#include "../common.h"

namespace brex {
    // == Compiled States ==
    enum class CompiledStateTag {
        GroundState,  // Ground, fully compiled and linked state
        WildcardState,
        Placeholder,  // Substitution placeholder
    };

    class CompiledState {
        public:
            CompiledStateTag tag;

            std::set<size_t>* next_states;
            std::set<size_t>* default_states;

            CompiledState(CompiledStateTag tag, std::set<size_t>* next_states, std::set<size_t>* default_states) : tag(tag), next_states(next_states), default_states(default_states) {;}
            virtual ~CompiledState() {;}
    };

    class GroundState : public CompiledState {
        public:
            const RegexChar activator;
            
            GroundState(RegexChar activator, std::set<size_t>* next_states, std::set<size_t>* default_states) : CompiledState(CompiledStateTag::GroundState, next_states, default_states), activator(activator) {;}
            virtual ~GroundState() = default;
    };

    class WildcardState : public CompiledState {
        public:
            WildcardState(std::set<size_t>* next_states, std::set<size_t>* default_states) : CompiledState(CompiledStateTag::WildcardState, next_states, default_states) {;}
    };

    class PlaceholderState : public CompiledState {
        public:
            const std::string symbol;

            PlaceholderState(std::string symbol, std::set<size_t>* next_states, std::set<size_t>* default_states) : CompiledState(CompiledStateTag::Placeholder, next_states, default_states), symbol(symbol) {;}
    };

    // == Expression State Machine ==
    class ExpressionMachine {
        public:
            std::set<size_t>* start_states;
            std::vector<const CompiledState*> states;

            ExpressionMachine(std::set<size_t>* start_states, std::vector<const CompiledState*> states) : start_states(start_states), states(states) {;}
    };

    // == Expression Compiler ==
    class ExpressionCompiler {
        private:
            // const GlobExpression* data;
            size_t max_index;
            std::vector<const CompiledState*> states;

            std::set<size_t>* compileExpression(const GlobExpression* expr, std::set<size_t>* next_states);           
            std::set<size_t>* compileLiteral(LiteralExpression* expr, std::set<size_t>* next_states);
            std::set<size_t>* compileSequence(SequenceExpression* expr, std::set<size_t>* next_states);
            std::set<size_t>* compileWildcard(WildcardExpression* expr, std::set<size_t>* next_states);
            std::set<size_t>* compileUnion(UnionExpression* expr, std::set<size_t>* next_states);
            std::set<size_t>* compileSubstitution(SubstitutionExpression* expr, std::set<size_t>* next_states);
        
        public:
            ExpressionCompiler(/*GlobExpression* data*/) : /* data(data),*/ max_index(0), states(std::vector<const CompiledState*>()) {;}
            virtual ~ExpressionCompiler() = default;
            static ExpressionMachine* compile(GlobExpression* expr);
    };
}