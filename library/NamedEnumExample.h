///////////////////////////////////////////////////////////////////////////////

// Example 1 — basic usage + warn logger policy

// example_basic.cpp
#define MDN_LOGS 1                // so WarnWithLogger actually logs
#include "Logger.h"               // your logger (as provided)
#include "named_enum.hpp"         // the header we finalized

using namespace mdn;

// 1) Define an enum (case-insensitive parsing; "unknown" is the invalid value)
#define FRUIT_LIST(X) X(unknown) X(apple) X(orange) X(banana)
DECLARE_NAMED_ENUM(Fruit, FRUIT_LIST, unknown, CaseMode::Insensitive, WarnWithLogger<false>)
// WarnWithLogger<false> = warn, but do NOT set failbit on operator>>

int main() {
    // Parse (default policy = WarnWithLogger<false>)
    Fruit a = parse_Fruit("Orange");
    std::cout << "a = " << a << "\n";      // prints: orange

    // to_string:
    std::cout << "to_string(apple) = " << to_string(Fruit::apple) << "\n";

    // try_parse (optional)
    if (auto v = try_parse_Fruit("BANANA")) {
        std::cout << "try_parse ok: " << to_string(*v) << "\n";
    }

    // bad token: logs a warning, returns invalid value (Fruit::unknown)
    Fruit bad = parse_Fruit("kumquat");
    std::cout << "bad => " << to_string(bad) << "\n"; // "unknown"

    // Show valid choices (skip invalid by default)
    std::cout << "Valid fruits: " << toc_Fruit() << "\n";           // "apple, orange, banana"
    std::cout << "All (incl invalid): " << toc_Fruit(true) << "\n"; // "unknown, apple, orange, banana"

    // Stream extraction (no failbit in this variant)
    std::istringstream iss("BANANA kumquat apple");
    Fruit f1{}, f2{}, f3{};
    iss >> f1 >> f2 >> f3;  // f2 becomes unknown; stream stays usable
    std::cout << f1 << " | " << f2 << " | " << f3 << "\n"; // banana | unknown | apple

    return 0;
}


///////////////////////////////////////////////////////////////////////////////

// Example 2 — set failbit on bad input via policy

// example_failbit.cpp
#include "named_enum.hpp"
using namespace mdn;

#define SHAPE_LIST(X) X(unknown) X(circle) X(square) X(triangle)
DECLARE_NAMED_ENUM(Shape, SHAPE_LIST, unknown, CaseMode::Exact, WarnWithLogger<true>)
// Exact case; operator>> sets failbit on errors because <true>

int main() {
    std::istringstream iss("square hexagon circle");
    Shape s1{}, s2{}, s3{};

    iss >> s1;                 // ok
    std::cout << s1 << "\n";   // square

    iss >> s2;                 // invalid => logs + sets failbit
    if (iss.fail()) {
        std::cout << "failbit set after invalid token\n";
        iss.clear();           // recover so we can keep reading
    }

    iss >> s3;                 // ok after clear
    std::cout << s3 << "\n";   // circle
}

///////////////////////////////////////////////////////////////////////////////

// Example 3 — throw on bad input in debug, warn in release

// example_throw_or_warn.cpp
#include "named_enum.hpp"
using namespace mdn;

// Choose a project-wide default parse policy by config:
#ifdef MDN_DEBUG
    using DefaultParse = ThrowOnError;
#else
    using DefaultParse = WarnWithLogger<false>;
#endif

#define LEVEL_LIST(X) X(unknown) X(low) X(medium) X(high)
DECLARE_NAMED_ENUM(Level, LEVEL_LIST, unknown, CaseMode::Exact, DefaultParse)

int main() {
    // At declare time, Level uses DefaultParse.
    try {
        Level l1 = parse_Level("medium");         // ok
        Level l2 = parse_Level("invalid-token");  // throws in debug, warns in release
        (void)l1; (void)l2;
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
}

///////////////////////////////////////////////////////////////////////////////

// Example 4 — replace bool parameters with named policies (your idea)

// example_policy_enums.cpp
#include "named_enum.hpp"
using namespace mdn;

// Replace “bool flags” with small named enums
#define INPUT_LIST(X) X(TrustInputs) X(CheckInputs)
DECLARE_NAMED_ENUM(InputHandling, INPUT_LIST, TrustInputs, CaseMode::Exact, WarnWithLogger<false>)

#define ERRORPOLICY_LIST(X) X(FailSilently) X(PostWarnings) X(ThrowErrors)
DECLARE_NAMED_ENUM(ErrorPolicy, ERRORPOLICY_LIST, FailSilently, CaseMode::Exact, WarnWithLogger<false>)

// Bridge a runtime ErrorPolicy choice to a concrete parse policy for any enum:
template <class Enum>
Enum parse_with(std::string_view s, ErrorPolicy p) {
    switch (p) {
        case ErrorPolicy::FailSilently:  return parse_##Enum<FailSilently>(s);
        case ErrorPolicy::PostWarnings:  return parse_##Enum<WarnWithLogger<false>>(s);
        case ErrorPolicy::ThrowErrors:   return parse_##Enum<ThrowOnError>(s);
        default:                         return parse_##Enum<FailSilently>(s);
    }
}

#define MODE_LIST(X) X(unknown) X(light) X(dark) X(system)
DECLARE_NAMED_ENUM(ThemeMode, MODE_LIST, unknown, CaseMode::Insensitive, WarnWithLogger<false>)

int main() {
    ErrorPolicy uiPolicy = ErrorPolicy::PostWarnings;   // set from config/CLI/etc.
    ThemeMode m = parse_with<ThemeMode>("System", uiPolicy);
    std::cout << "theme = " << m << "\n";               // prints "system"
}

///////////////////////////////////////////////////////////////////////////////

// Example 5 — case sensitivity

// example_case.cpp
#include "named_enum.hpp"
using namespace mdn;

// Case-insensitive (user input friendly)
#define COLOUR_LIST(X) X(unknown) X(red) X(green) X(blue)
DECLARE_NAMED_ENUM(Colour, COLOUR_LIST, unknown, CaseMode::Insensitive, WarnWithLogger<false>)

// Case-exact (for strict config files)
#define ROLE_LIST(X) X(unknown) X(Admin) X(Guest) X(User)
DECLARE_NAMED_ENUM(Role, ROLE_LIST, unknown, CaseMode::Exact, WarnWithLogger<false>)

int main() {
    std::cout << to_string(parse_Colour("BLUE")) << "\n"; // blue
    std::cout << to_string(parse_Role("User"))  << "\n";  // User

    // This one will warn (case mismatch) and return unknown
    std::cout << to_string(parse_Role("user"))  << "\n";  // unknown
}

///////////////////////////////////////////////////////////////////////////////

// Optional: generic “list valid values” helper

// You can always call toc_Fruit() etc. directly. If you want a generic wrapper:
.
// put in some common header
template <class Enum>
std::string enum_valid_values(bool include_invalid = false);

// Provide tiny per-enum overloads (one-liners), or wrap in a macro:
#define DECLARE_ENUM_TOC_OVERLOAD(EnumName) \
    template <> inline std::string enum_valid_values<EnumName>(bool inc){ return toc_##EnumName(inc); }

DECLARE_ENUM_TOC_OVERLOAD(Fruit)
DECLARE_ENUM_TOC_OVERLOAD(Shape)
DECLARE_ENUM_TOC_OVERLOAD(Level)
// ...one line per enum

///////////////////////////////////////////////////////////////////////////////

g++ -std=c++17 -O2 example_basic.cpp -o example_basic
# or if you’re OK with GNU extensions:
g++ -std=gnu++17 -O2 example_basic.cpp -o example_basic

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
