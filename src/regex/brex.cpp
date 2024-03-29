#include "brex.h"

namespace brex
{
    RegexOpt* RegexOpt::jparse(json j)
    {
        auto tag = j["tag"].get<std::string>();

        if(tag == "LiteralOpt") {
            return LiteralOpt::jparse(j);
        }
        else if(tag == "CharRangeOpt") {
            return CharRangeOpt::jparse(j);
        }
        else if(tag == "CharClassDotOpt") {
            return CharClassDotOpt::jparse(j);
        }
        else if(tag == "NamedRegexOpt") {
            return NamedRegexOpt::jparse(j);
        }
        else if(tag == "EnvRegexOpt") {
            return EnvRegexOpt::jparse(j);
        }
        else if(tag == "StarRepeatOpt") {
            return StarRepeatOpt::jparse(j);
        }
        else if(tag == "PlusRepeatOpt") {
            return PlusRepeatOpt::jparse(j);
        }
        else if(tag == "RangeRepeatOpt") {
            return RangeRepeatOpt::jparse(j);
        }
        else if(tag == "OptionalOpt") {
            return OptionalOpt::jparse(j);
        }
        else if(tag == "AnyOfOpt") {
            return AnyOfOpt::jparse(j);
        }
        else {
            return SequenceOpt::jparse(j);
        }
    }

    RegexComponent* RegexComponent::jparse(json j)
    {
        if(!j.is_array()) {
            return RegexSingleComponent::jparse(j);
        }
        else {
            return RegexAllOfComponent::jparse(j);
        }
    }
}
