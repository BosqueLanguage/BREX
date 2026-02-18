#pragma once

// I'd like to use bitsets, but bitsets have to have a length known at compile time.
// #include <bitset>
#include "../common.h"

namespace brex {
    // These may or may not have an activator character.
    // Should they be tagged or use a reference?
    class ExprBitState {
    };
    // Need a class for the assembler
    // Need a class for the assembled states
}