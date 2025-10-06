#pragma once

#include "NamedEnum.hpp"

namespace mdn {
#define CARRY_OVER_ENUM(C) \
    C(Invalid) \
    C(OptionalPositive) \
    C(OptionalNegative) \
    C(Required)
DECLARE_NAMED_ENUM(Carryover, CARRY_OVER_ENUM, Invalid, CaseMode::Exact, WarnWithLogger<false>)

#define FRAXIS_ENUM(F)
    F(Invalid) \
    F(Default) \
    F(X) \
    F(Y)
DECLARE_NAMED_ENUM(Fraxis, FRAXIS_ENUM, Invalid, CaseMode::Exact, WarnWithLogger<false>)



} // end namespace mdn
