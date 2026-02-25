#pragma once

#include "../common.h"
#include "glob.h"
#include "glob_compiler.h"

namespace brex {
    bool match(FragmentMachine* machine, std::string str) {
    }

    std::pair<bool, size_t> matchExpr(ExpressionMachine* machine, std::string str, size_t start_pos) {
        assert(str.size() - start_pos > 0);

        // Copy constructor
        std::set<size_t> current_state = std::set<size_t>(*(machine->start_states));

        for (auto it = str.cbegin() + start_pos; it != str.cend(); it++) {
            std::set<size_t> next_state = std::set<size_t>();

            for (auto state_id = current_state.begin(); state_id != current_state.cend(); state_id++) {
                const CompiledState* state = machine->states[*state_id];
                if (state->tag == CompiledStateTag::GroundState && ((const GroundState*)state)->activator == *it) {
                    for (auto ns = state->next_states->cbegin(); ns != state->next_states->cend(); ns++) {
                        next_state.insert(*ns);
                    }
                }
                for (auto ds = state->default_states->cbegin(); ds != state->default_states->cend(); ds++) {
                    next_state.insert(*ds);
                }
            }

            current_state = next_state;
            start_pos++;

            if (current_state.size() == 0) {
                return std::pair<bool, size_t>(false, start_pos);
            }

            // TODO: Path separator variable for when these functions are inside a class
            if ((*it) == '/') {
                break;
            }
        }

        return std::pair<bool, size_t>(current_state.contains(0), start_pos);
    }
}