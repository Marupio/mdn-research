#pragma once

#include <cstdint>

namespace mdn {

using Digit = int8_t;

// enum class PrecisionStatus {
//     Below,
//     Inside,
//     Above
// };
// enum class DigitClass {
//     Unknown,
//     NonZero,
//     Zero,  // but within precision window
//     BelowPrecisionWindow
// };
//
// enum class SetValueResult {
//     Unknown,
//     AlreadyZero,
//     BelowPrecision,
//     AddressAdded,
//     ExistingDigitChanged
// };
//
// enum class AddressStatus {
//     Unknown,
//     AddedEntries,
//     RemovedEntries
// };
//
// enum class DigitStatus {
//     Unknown,
//     NonZero,
//     Zero
// };
//
// enum class PrecisionStatus {
//     Unknown,
//     Inside,
//     Below,
//     Above
// };
//
// enum class CheckBoundsResult {
//     Unknown,
//     BelowWindow,
//     InsideWindow,
//     WindowShiftedUp
// };
//
// // DigitStatus is pair<SomethingChanged?, DigitClass(above)>
// using DigitStatus = std::pair<bool, DigitClass>;


} // end namespace mdn
