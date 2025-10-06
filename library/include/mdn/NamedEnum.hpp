#pragma once

#include <string>
#include <string_view>
#include <array>
#include <optional>
#include <ostream>
#include <istream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

#ifdef MDN_LOGS
// All use of Logger functionality is enclosed in MDN_LOGS conditionals
#include "Logger.hpp"
#endif

namespace mdn {

// *** Utilities ***

enum class CaseMode { Exact, Insensitive };

inline std::string_view ne_trim(std::string_view s) {
    auto is_space = [](unsigned char c){ return std::isspace(c) != 0; };
    while (!s.empty() && is_space(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && is_space(static_cast<unsigned char>(s.back())))  s.remove_suffix(1);
    return s;
}

inline bool ne_iequals(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        unsigned char ac = static_cast<unsigned char>(a[i]);
        unsigned char bc = static_cast<unsigned char>(b[i]);
        if (std::tolower(ac) != std::tolower(bc)) return false;
    }
    return true;
}

// *** Policy hooks ***
// Forward-declared generic trait so policies can introspect any generated enum.
template <class E> struct NamedEnumTraits;

// Throw on invalid, set failbit on stream >>.
struct ThrowOnError {
    template <class E>
    static E on_parse_fail(std::string_view s) {
        throw std::invalid_argument(std::string("Invalid token: '") + std::string(s) + "'");
    }
    static void on_stream_fail(std::istream& is) { is.setstate(std::ios::failbit); }
    static void on_warn(std::string_view) {}
};

// Return invalid value silently, leave stream state unchanged.
struct FailSilently {
    template <class E>
    static E on_parse_fail(std::string_view) { return NamedEnumTraits<E>::invalid_value; }
    static void on_stream_fail(std::istream&) {}
    static void on_warn(std::string_view) {}
};

// Log a warning (via your Logger macros) and return invalid.
// Template parameter controls whether operator>> sets failbit.
template <bool SetFailBit>
struct WarnWithLogger {
    template <class E>
    static E on_parse_fail(std::string_view bad) {
#ifdef MDN_LOGS
        // Build "Valid: a, b, c" (skip the designated invalid)
        std::string valids;
        const auto& arr = NamedEnumTraits<E>::names;
        for (size_t i = 0, out = 0; i < NamedEnumTraits<E>::count; ++i) {
            if (static_cast<E>(i) == NamedEnumTraits<E>::invalid_value) continue;
            if (out++) valids += ", ";
            valids += arr[i];
        }
        Log_Warn("Invalid token '" << bad << "'. Valid: " << valids);
#endif
        return NamedEnumTraits<E>::invalid_value;
    }
    static void on_stream_fail(std::istream& is) {
        if constexpr (SetFailBit) is.setstate(std::ios::failbit);
    }
    static void on_warn(std::string_view msg) {
#ifdef MDN_LOGS
        Log_Warn(msg);
#endif
    }
};

// *** Core generator ***
// Dev supplies a LIST macro like:
//   #define FRUIT_LIST(X) X(unknown) X(apple) X(orange) X(banana)
// Then invoke:
//   DECLARE_NAMED_ENUM(Fruit, FRUIT_LIST, unknown, CaseMode::Insensitive, WarnWithLogger<false>)

#define _NE_ENUM_VALUE(name) name,
#define _NE_ENUM_CSTR(name)  #name,

#define DECLARE_NAMED_ENUM(EnumName, LIST_MACRO, INVALID_TOKEN, CASEMODE, ErrorPolicyPolicy) \
    enum class EnumName { LIST_MACRO(_NE_ENUM_VALUE) }; \
 \
    struct EnumName##Traits { \
        static constexpr CaseMode case_mode = CASEMODE; \
        static constexpr EnumName invalid_value = EnumName::INVALID_TOKEN; \
        /* Build a C-array so we can get the count portably in C++17 */ \
        static constexpr const char* cstr_names[] = { LIST_MACRO(_NE_ENUM_CSTR) }; \
        static constexpr size_t count = sizeof(cstr_names) / sizeof(cstr_names[0]); \
        /* Materialize as std::array<string_view> for ergonomics */ \
        static const std::array<std::string_view, count> names; \
    }; \
    const std::array<std::string_view, EnumName##Traits::count> EnumName##Traits::names = []{ \
        std::array<std::string_view, EnumName##Traits::count> a{}; \
        for (size_t i = 0; i < EnumName##Traits::count; ++i) a[i] = EnumName##Traits::cstr_names[i]; \
        return a; \
    }(); \
 \
    /* Specialize the generic trait so policies can introspect this enum */ \
    template <> struct NamedEnumTraits<EnumName> : EnumName##Traits {}; \
 \
    /* to_string */ \
    [[nodiscard]] inline std::string_view to_string(EnumName e) { \
        const auto idx = static_cast<size_t>(e); \
        return (idx < EnumName##Traits::count) ? EnumName##Traits::names[idx] \
                                               : std::string_view{"<out_of_range>"}; \
    } \
 \
    [[nodiscard]] inline std::optional<EnumName> try_parse_##EnumName(std::string_view s) { \
        s = ne_trim(s); \
        for (size_t i = 0; i < EnumName##Traits::count; ++i) { \
            const auto name = EnumName##Traits::names[i]; \
            const bool match = (EnumName##Traits::case_mode == CaseMode::Exact) ? (s == name) \
                                                                 : ne_iequals(s, name); \
            if (match) return static_cast<EnumName>(i); \
        } \
        return std::nullopt; \
    } \
 \
    /* parse with a policy (defaults to declaration-time policy) */ \
    template <class Policy = ErrorPolicyPolicy> \
    [[nodiscard]] inline EnumName parse_##EnumName(std::string_view s) { \
        if (auto v = try_parse_##EnumName(s)) return *v; \
        return Policy::template on_parse_fail<EnumName>(s); \
    } \
 \
    /* toc(): user-facing list (skip invalid unless asked) */ \
    inline std::string toc_##EnumName(bool include_invalid = false) { \
        std::string out; \
        for (size_t i = 0; i < EnumName##Traits::count; ++i) { \
            if (!include_invalid && static_cast<EnumName>(i) == EnumName##Traits::invalid_value) \
                continue; \
            if (!out.empty()) out += ", "; \
            out += EnumName##Traits::names[i]; \
        } \
        return out; \
    } \
 \
    /* stream operators use the declaration-time policy */ \
    inline std::ostream& operator<<(std::ostream& os, EnumName e) { \
        return os << to_string(e); \
    } \
    inline std::istream& operator>>(std::istream& is, EnumName& e) { \
        std::string token; \
        if (!(is >> token)) return is; \
        if (auto v = try_parse_##EnumName(token)) { \
            e = *v; \
        } else { \
            ErrorPolicyPolicy::on_stream_fail(is); \
        } \
        return is; \
    }

// *** (Optional) convenience: list of valid strings via ADL ***
template <class E>
inline std::string named_enum_toc(bool include_invalid = false) {
    // Expect toc_E to be visible via ADL; if you prefer, create explicit overloads.
    return {}; // Intentionally empty unless you add specific overloads per enum.
}

} // namespace mdn
