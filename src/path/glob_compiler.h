#pragma once

#include "../common.h"

namespace brex {
    // == Compiled Exression States ==
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

    // == Compiled Fragment States ==

    class CompiledFragment {
        public:
            GlobFragmentTag tag;

            CompiledFragment(GlobFragmentTag tag) : tag(tag) {;}
            virtual ~CompiledFragment() {;}
    };

    class CompiledExpressionFragment : public CompiledFragment{
        public:
            ExpressionMachine* exprMachine;
            CompiledExpressionFragment(ExpressionMachine* machine) : CompiledFragment(GlobFragmentTag::Expression), exprMachine(machine) {;}
    };

    class CompiledRecursiveWildcardFragment : public CompiledFragment{
        public:
            CompiledRecursiveWildcardFragment() : CompiledFragment(GlobFragmentTag::RecursiveWildcard) {;}
    };

    // == Incomplete State Machines ==

    // The "link" functions replace PlaceHolders by allowing insertion of a
    // precompiled expression. There's potential optimization for these to get
    // compiled further into bitstring operations by the state machine assembler
    // (see glob_machine.h and .cpp). The state machine assembler implementation
    // I have in mind currently doesn't remove all nondeterminism cleanly so I
    // may look at adjusting it, it also would use Boost dynamic bitsets which
    // may not be desirable if we're trying not to depend too much on external
    // libs.

    class ExpressionMachine {
        private:
            void innerLink(size_t state_id, ExpressionMachine* machine);

        public:
            std::set<size_t>* start_states;
            std::vector<const CompiledState*> states;

            ExpressionMachine(std::set<size_t>* start_states, std::vector<const CompiledState*> states) : start_states(start_states), states(states) {;}

            // TODO: See section header
            void link(std::string symbol, ExpressionMachine* machine);
    };

    // TODO: Find a better name for this, I have no idea what to call it, it's just a collection of state machines really
    class SomethingMachine {
        public:
            std::vector<const CompiledFragment*> states;
            SomethingMachine(std::vector<const CompiledFragment*> states) : states(states) {;}

            // TODO: See section header
            void link(std::string symbol, ExpressionMachine* machine);
    };

    // == Compiler Levels ==

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
            static ExpressionMachine* compile(const GlobExpression* expr);
    };

    class GlobCompiler {
        private:
            std::vector<const CompiledFragment*> states;
            void compileFragments(std::vector<const GlobFragment*> fragments);
        public:
            GlobCompiler() : states(std::vector<const CompiledFragment*>()) {;}
           
            // TODO: Replace void with an unlinked state machine type
            static SomethingMachine* compile(Glob* glob);
    };
}