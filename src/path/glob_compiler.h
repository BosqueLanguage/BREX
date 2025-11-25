#pragma once

#include "../common.h"
#include "glob_executor.h"

namespace brex {
    class GlobCompiler {
        static CompiledSegment* compileBaseExpression(GlobExpr* expression);
    };
}