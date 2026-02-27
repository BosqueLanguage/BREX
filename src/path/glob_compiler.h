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

            virtual std::string stringify() const = 0;
    };

    class GroundState : public CompiledState {
        public:
            const RegexChar activator;
            
            GroundState(RegexChar activator, std::set<size_t>* next_states, std::set<size_t>* default_states) : CompiledState(CompiledStateTag::GroundState, next_states, default_states), activator(activator) {;}
            virtual ~GroundState() = default;

            virtual std::string stringify() const override final {
                std::string next_states = "{ ";
                for (auto it = this->next_states->cbegin(); it != this->next_states->cend(); it++) {
                    next_states += std::to_string(*it) + " ";
                }
                next_states += "}";

                std::string defaults = "{ ";
                if (this->default_states != nullptr) {
                    for (auto it = this->default_states->cbegin(); it != this->default_states->cend(); it++) {
                        defaults += std::to_string(*it) + " ";
                    }
                }
                defaults += "}";

                return "{ Ground State. On '" + std::string({(char)this->activator}) + "': " + next_states + ", Default: " + defaults + " }";
            }
    };

    class WildcardState : public CompiledState {
        public:
            WildcardState(std::set<size_t>* next_states, std::set<size_t>* default_states) : CompiledState(CompiledStateTag::WildcardState, next_states, default_states) {;}

            std::string stringify() const override final {
                std::string defaults = "{ ";
                if (this->default_states != nullptr) {
                    for (auto it = this->default_states->cbegin(); it != this->default_states->cend(); it++) {
                        defaults += std::to_string(*it) + " ";
                    }
                }
                defaults += "}";

                return "{ Wildcard. Default: " + defaults + " }";
            }
    };

    class PlaceholderState : public CompiledState {
        public:
            const std::u8string symbol;

            PlaceholderState(std::u8string symbol, std::set<size_t>* next_states, std::set<size_t>* default_states) : CompiledState(CompiledStateTag::Placeholder, next_states, default_states), symbol(symbol) {;}

            std::string stringify() const override final {
                std::string next_states = "{ ";
                for (auto it = this->next_states->cbegin(); it != this->next_states->cend(); it++) {
                    next_states += std::to_string(*it) + " ";
                }
                next_states += "}";

                std::string defaults = "{ ";
                if (this->default_states != nullptr) {
                    for (auto it = this->default_states->cbegin(); it != this->default_states->cend(); it++) {
                        defaults += std::to_string(*it) + " ";
                    }
                }
                defaults += "}";

                return "{ Placeholder. On success: " + next_states + ", Default: " + defaults + " }";
            }
    };

    // == Incomplete State Machines ==

    class ExpressionMachine {
        private:
            void innerLink(size_t state_id, ExpressionMachine* machine);

        public:
            std::set<size_t>* start_states;
            std::vector<const CompiledState*> states;

            ExpressionMachine(std::set<size_t>* start_states, std::vector<const CompiledState*> states) : start_states(start_states), states(states) {;}

            // TODO: See section header
            void link(std::u8string symbol, ExpressionMachine* machine);

            std::string stringify(std::string prefix = "") const {
                std::string states = "";
                int i = 0;
                for (auto it = this->states.cbegin(); it != this->states.cend(); it++) {
                    states += prefix + "  " + std::to_string(i) + ": " + (*it)->stringify() + "\n";
                    i++;
                }

                std::string start_states = "{ ";
                for (auto it = this->start_states->cbegin(); it != this->start_states->cend(); it++) {
                    start_states += std::to_string(*it) + " ";
                }
                start_states += "}";


                return prefix + "{\n" + states + prefix + "  Start: " + start_states + "\n" + prefix + "}";
            }
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

    // == Incomplete State Machines (Cont, I had to split them for name resolution) ==

    // TODO: Find a better name for this, I have no idea what to call it, it's just a collection of state machines really
    class FragmentMachine {
        public:
            std::vector<const CompiledFragment*> states;
            FragmentMachine(std::vector<const CompiledFragment*> states) : states(states) {;}

            // TODO: See section header
            void link(std::u8string symbol, ExpressionMachine* machine);
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
            ExpressionCompiler(/*GlobExpression* data*/) : /* data(data),*/ max_index(0), states(std::vector<const CompiledState*>({new WildcardState(nullptr, nullptr)})) {;}
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
            static FragmentMachine* compile(Glob* glob);
    };
}