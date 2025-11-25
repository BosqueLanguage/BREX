#pragma once

#include "../common.h"
#include "glob.h"

namespace brex {
    class CompiledSegment {
        public:
            std::unordered_map<uint8_t, CompiledSegment*> next_states; 
            bool is_wildcard;

            CompiledSegment(std::unordered_map<uint8_t, CompiledSegment*> next_states, bool is_wildcard): next_states(next_states), is_wildcard(is_wildcard) {;}
            virtual ~CompiledSegment() = default;

            const bool is_final() const {
                return this->next_states.empty();
            }
    };

    class FragmentExecutor {
        public:
            const CompiledSegment* segment;

            FragmentExecutor(const CompiledSegment* segment) : segment(segment) {;}
            ~FragmentExecutor() = default;

            bool test(std::string str) {
                // TODO: Maybe convert to bitmasks?
                // Use 'and' to get rid of states that are no longer filled, left shift at the end of each iteration
                // Use 'or' to add in wildcard states
                std::vector<const CompiledSegment*> current_segments;
                std::vector<const CompiledSegment*> active_wildcards;

                if (!this->segment->is_wildcard) {
                    current_segments.push_back((const CompiledSegment*) this->segment);
                } 
                else {
                    active_wildcards.push_back((const CompiledSegment*) this->segment);
                }

                for (auto it = str.cbegin(); it != str.cend(); it++) {
                    std::vector<const CompiledSegment*> next_segments;
                    for (auto s = current_segments.cbegin(); s != current_segments.cend(); s++) {
                        const CompiledSegment* c = (const CompiledSegment*) *s;
                        if (c->next_states.contains(*it)) {
                            const CompiledSegment* next = c->next_states.at((uint8_t) *it);
                            if (!this->segment->is_wildcard) {
                                next_segments.push_back((const CompiledSegment*) this->segment);
                            }
                            else {
                                active_wildcards.push_back((const CompiledSegment*) this->segment);
                            }
                        }
                    }
                    for (auto w = active_wildcards.cbegin(); w != active_wildcards.cend(); w++) {
                        const CompiledSegment* c = (const CompiledSegment*) *w;
                        if (c->next_states.contains(*it)) {
                            const CompiledSegment* next = c->next_states.at((uint8_t) *it);
                            // Compiler should get rid of * next to * that isn't a top-level segment.
                            if (!next->is_wildcard) {
                                next_segments.push_back((const CompiledSegment*) this->segment);
                            }
                        }
                    }
                }

                for (auto s = current_segments.cbegin(); s != current_segments.cend(); s++) {
                    if ((*s)->is_final()) {
                        return true;
                    }
                }
                for (auto s = active_wildcards.cbegin(); s != active_wildcards.cend(); s++) {
                    if ((*s)->is_final()) {
                        return true;
                    }
                }
                return false;
            }
    };

    class GlobExecutor {
        public:
            const Glob* glob;

        GlobExecutor(const Glob* glob) : glob(glob) {;}
        ~GlobExecutor() = default;

        // TODO: Template type for std::string
        bool test(std::string str) {
            return false;
        }
    };
}