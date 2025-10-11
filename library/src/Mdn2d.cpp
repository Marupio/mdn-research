#include <mdn/Mdn2d.hpp>

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <sstream>

#include <mdn/Constants.hpp>
#include <mdn/Logger.hpp>
#include <mdn/MdnException.hpp>
#include <mdn/Selection.hpp>
#include <mdn/Tools.hpp>


mdn::Mdn2d::Mdn2d(std::string nameIn) :
    Mdn2dRules(nameIn)
{
    Log_N_Debug2("");
}


mdn::Mdn2d::Mdn2d(Mdn2dConfig config, std::string nameIn) :
    Mdn2dRules(config, nameIn)
{
    Log_N_Debug2("Config=" << config);
}


mdn::Mdn2d::Mdn2d(const Mdn2d& other, std::string nameIn):
    Mdn2dRules(other, nameIn)
{
    Log_N_Debug2("");
}


mdn::Mdn2d& mdn::Mdn2d::operator=(const Mdn2d& other) {
    Mdn2dRules::operator=(other);
    Log_N_Debug2("Copying");
    return *this;
}


mdn::Mdn2d::Mdn2d(Mdn2d&& other) noexcept :
    Mdn2dRules(std::move(other))
{
    Log_N_Debug2("");
}


mdn::Mdn2d& mdn::Mdn2d::operator=(Mdn2d&& other) noexcept {
    Mdn2dRules::operator=(std::move(other));
    Log_N_Debug2("Move copying");
    return *this;
}


mdn::Mdn2d::~Mdn2d() = default;


const mdn::Selection& mdn::Mdn2d::selection() const {
    Log_Debug2_H("");
    auto lock = lockWriteable();
    const Selection& sel = locked_selection();
    Log_Debug2_T("");
    return sel;
}


const mdn::Selection& mdn::Mdn2d::locked_selection() const {
    Log_Debug3("");
    if (!m_selection) {
        m_selection.reset(new Selection(const_cast<Mdn2d&>(*this)));
    }
    return *(m_selection.get());
}


mdn::Selection& mdn::Mdn2d::selection() {
    Log_Debug2_H("");
    auto lock = lockWriteable();
    Selection& sel = locked_selection();
    Log_Debug2_T("");
    return sel;
}


mdn::Selection& mdn::Mdn2d::locked_selection() {
    Log_Debug3("");
    if (!m_selection) {
        m_selection.reset(new Selection(*this));
    }
    return *(m_selection.get());
}


void mdn::Mdn2d::plus(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_N_Debug2_H("ans = *this + rhs");
    assertNotSelf(ans, "plus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    CoordSet changed = locked_plus(rhs, ans);
    ans.locked_carryoverCleanup(changed);
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_plus(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_N_Debug3_H("ans(" << ans.m_name << ") = *this(" << m_name << ") + rhs(" << rhs.m_name << ")");
    CoordSet changed;
    ans.locked_operatorEquals(*this);
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(ans.locked_add(xy, digit));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::minus(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_N_Debug2_H("ans = *this - rhs");
    assertNotSelf(ans, "minus operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    CoordSet changed = locked_minus(rhs, ans);
    ans.locked_carryoverCleanup(changed);
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_minus(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_N_Debug3_H("ans(" << ans.m_name << ") = *this(" << m_name << ") - rhs(" << rhs.m_name << ")");
    CoordSet changed;
    ans.locked_operatorEquals(*this);
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(ans.locked_add(xy, -digit));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::multiply(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_N_Debug2_H("ans = *this * rhs");
    assertNotSelf(ans, "multiply operation");
    auto lockThis = lockReadOnly();
    ReadOnlyLock* lockRhsPtr;
    if (&rhs != this) {
        auto lockRhs = rhs.lockReadOnly();
        lockRhsPtr = &lockRhs;
    }
    auto lockAns = ans.lockWriteable();
    CoordSet changed = locked_multiply(rhs, ans);
    ans.locked_carryoverCleanup(changed);
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const {
    Log_N_Debug3_H("ans(" << ans.m_name << ") = *this(" << m_name << ") x rhs(" << rhs.m_name << ")");
    int thisPrecision = locked_getPrecision();
    int rhsPrecision = rhs.locked_getPrecision();
    int sumPrecision = thisPrecision + rhsPrecision;
    if (rhsPrecision < 0 || thisPrecision < 0) {
        sumPrecision = -1; // unlimited
    }
    ans.locked_setPrecision(sumPrecision);
    ans.locked_clear();
    for (const auto& [xy, digit] : rhs.m_raw) {
        int id = static_cast<int>(digit);
        // ans += (this x rhs_id).shift(rhs_xy)
        Log_N_Debug4("multiply loop: " << xy << ", " << static_cast<int>(digit));
        Mdn2d temp(internal_copyMultiplyAndShift(id, xy));
        ans.locked_plusEquals(std::move(temp));
        Log_N_Debug4("Done plusEquals step");
    }
    int finalPrecision =
        sumPrecision < 0 ? -1 : std::round(sumPrecision*0.5);
    ans.locked_setPrecision(finalPrecision);
    Log_N_Debug3_T("ans has " << ans.m_index.size() << " changed digits");
    return ans.m_index;
}


void mdn::Mdn2d::divideIterate(
    int nIters, const Mdn2d& rhs, Mdn2d& ans, Mdn2d& rem, long double& remMag, Fraxis fraxis
) const {
    auto lockThis = lockReadOnly();
    auto lockRhs = rhs.lockReadOnly();
    auto lockAns = ans.lockWriteable();
    auto lockRem = rem.lockWriteable();
    return locked_divideIterate(nIters, rhs, ans, rem, remMag, fraxis);
}


void mdn::Mdn2d::locked_divideIterate(
    int nIters, const Mdn2d& rhs, Mdn2d& ans, Mdn2d& rem, long double& remMag, Fraxis fraxis
) const {
    If_Log_Showing_Debug(
        Log_N_Debug_H(""
            << "nIters=" << nIters
            << ", rhs=" << rhs.locked_name()
            << ", ans=" << ans.locked_name()
            << ", rem=" << rem.locked_name()
            << ", remMag=" << remMag
            << ", fraxis=" << FraxisToName(fraxis)
        );
    );
    // Calculating: ans = *this / rhs, aka  t = p / q
    //  *this = p values (pVal, pOffset, pSign, etc)
    //  rhs   = q values (qVal, qOffset, qSign, etc)
    //  ans   = t values (tVal, tOffset, tSign, etc)
    // internal_checkFraxis(fraxis);
    if (rhs.m_index.empty()) {
        Log_N_Debug_T("Divisor is zero, answer is undefined");
        remMag = -1.0;
        return;
    }
    CoordSet ansChanged;
    CoordSet remChanged;

    // Find principal row for division - row with largest absolute magnitude
    Coord pOffset;
    long double pVal;
    if (fraxis == Fraxis::Invalid) {
        Log_Warn("Fraxis set to invalid value, changing to Fraxis::Default");
        fraxis = Fraxis::Default;
    }

    Fraxis lastFraxis = fraxis;
    if (lastFraxis == Fraxis::Default) {
        // Alternating fraxis directions, start with X
        static Fraxis altStart = Fraxis::X;
        fraxis = altStart;
        altStart = altStart == Fraxis::X ? Fraxis::Y : Fraxis::X;
    }
    Log_N_Debug3_H("rhs row/col magMax dispatch");
    if (
        (fraxis == Fraxis::X && !rhs.locked_getRowMagMax(pOffset, pVal))
        || (fraxis == Fraxis::Y && !rhs.locked_getColMagMax(pOffset, pVal))
    ) {
        Log_N_Debug3_T("rhs row/col magMax return");
        Log_Warn("Failed to find max magnitude row or col");
        remMag = -1.0;
        Log_N_Debug_T("Failed magMax")
        return;
    }
    Log_N_Debug3_T("rhs row/col magMax return, pOffset=" << pOffset << ", pVal=" << pVal);
    int iter = 0;
    for (; iter < nIters; ++iter) {
        Log_N_Debug3(
            "iter " << iter << " of " << nIters << ", fraxis=" << FraxisToName(fraxis)
        );
        Coord qOffset;
        long double qVal;
        Log_N_Debug2_H("rem row/col magMax dispatch");
        if (
            (fraxis == Fraxis::X && !rem.locked_getRowMagMax(qOffset, qVal))
            || (fraxis == Fraxis::Y && !rem.locked_getColMagMax(qOffset, qVal))
        ) {
            Log_N_Debug2_T("rem row/col magMax return - zero");
            // Maybe found exact answer, we think
            remMag = 0.0;
            Log_Debug4("");
            Log_N_Debug_T("");
            return;
        }
        Log_N_Debug3_T("rem row/col magMax return, qOffset=" << qOffset << ", qVal=" << qVal);
        long double div = qVal / pVal;
        Log_N_Debug3("div=qVal/pVal=" << qVal << "/" << pVal << "=" << div);
        // division here accounts for x position; y position must be manually calculated
        Coord emplacement;
        if (fraxis == Fraxis::X) {
            emplacement = Coord(0, qOffset.y() - pOffset.y());
        } else {
            emplacement = Coord(qOffset.x() - pOffset.x(), 0);
        }
        Log_N_Debug3("emplacement=" << emplacement);
        Mdn2d tmp(m_config, "tmp_divide");
        Log_N_Debug3("internal_emplace(emplacement, div=" << div << ", fraxis)");
        static_cast<void>(tmp.internal_emplace(emplacement, div, fraxis));
        Log_N_Debug3("ans.locked_plusEquals(tmp)");
        ansChanged.merge(ans.locked_plusEquals(tmp));

        // Next, calculate reminder: rem = *this - ans*rhs
        // Because of my awesome thread-safe design, ^^^ that calculation becomes:
        Log_N_Debug3("rem.locked_clear();");
        rem.locked_clear();
        Log_N_Debug3("rem.locked_minusEquals(ans);");
        remChanged = rem.locked_minusEquals(ans);
        Log_N_Debug3("rem.locked_timesEquals(rhs);");
        remChanged = rem.locked_timesEquals(rhs);
        Log_N_Debug3("rem.locked_plusEquals(*this);");
        remChanged = rem.locked_plusEquals(*this);
        rem.locked_carryoverCleanup(remChanged);
        remChanged.clear();
        ans.locked_carryoverCleanup(ansChanged);
        ansChanged.clear();
        if (lastFraxis != fraxis) {
            lastFraxis = fraxis;
            fraxis = fraxis == Fraxis::X ? Fraxis::Y : Fraxis::X;
        }
    }
    // Done all iterations

    Log_N_Debug3("Done all iterations, rem.getTotalMagnitude");
    remMag = std::abs(rem.locked_getTotalMagnitude());
    Log_N_Debug_T("remMag = " << remMag);
}


void mdn::Mdn2d::divide(const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis) const {
    If_Log_Showing_Debug2(
        Log_N_Debug2_H("ans = *this / rhs, fraxis: " << FraxisToName(fraxis));
    );
    assertNotSelf(ans, "divide operation");
    auto lockThis = lockReadOnly();
    auto lockAns = ans.lockWriteable();
    CoordSet changed = locked_divide(rhs, ans, fraxis);
    Log_N_Debug2_T("changed.size() = " << changed.size());
}


mdn::CoordSet mdn::Mdn2d::locked_divide(const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis) const {
    If_Log_Showing_Debug3(
        Log_N_Debug3_H("ans = *this / rhs, fraxis: " << FraxisToName(fraxis));
    );
    ans.locked_clear();
    Mdn2d rem(*this, "rem_divide");
    auto lockRem = rem.lockWriteable();
    rem.locked_operatorEquals(*this);
    long double lastRemMag = constants::ldoubleGreat;
    long double remMag;
    locked_divideIterate(100, rhs, ans, rem, remMag, fraxis);
    while (remMag < lastRemMag && remMag >= 0.0) {
        locked_divideIterate(100, rhs, ans, rem, remMag, fraxis);
    }
    if (remMag < 0) {
        // Failed
        Log_N_Debug3_T("Failed");
        return m_nullCoordSet;
    }
    Log_N_Debug3_T("Success, with remMag = " << remMag);
    // For now, divideIterate functionality performs carryoverCleanup, no changed CoordSet available
    return m_nullCoordSet;
}


void mdn::Mdn2d::add(const Coord& xy, float realNum, int nDigits, bool overwrite, Fraxis fraxis) {
    If_Log_Showing_Debug2(
        Log_N_Debug2_H("add " << realNum << ", fraxis: " << FraxisToName(fraxis));
    );
    auto lock = lockWriteable();
    internal_checkFraxis(fraxis);
    locked_carryoverCleanup(
        locked_add(xy, static_cast<double>(realNum), nDigits, overwrite, fraxis)
    );
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::add(const Coord& xy, double realNum, int nDigits, bool overwrite, Fraxis fraxis) {
    Log_N_Debug2_H("");
    auto lock = lockWriteable();
    internal_checkFraxis(fraxis);
    If_Log_Showing_Debug2(
        Log_N_Debug2("add " << realNum << ", fraxis: " << FraxisToName(fraxis));
    );
    locked_carryoverCleanup(locked_add(xy, realNum, nDigits, overwrite, fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(
    const Coord& xy, double realNum, int nDigits, bool overwrite, Fraxis fraxis
) {
    Log_N_Debug3_H("");
    If_Log_Showing_Debug3(
        Log_N_Debug3("add " << realNum << ", fraxis: " << FraxisToName(fraxis));
    );
    double fracPart, intPart;
    fracPart = modf(realNum, &intPart);
    CoordSet changed = locked_add(xy, static_cast<long>(intPart), overwrite);
    changed.merge(locked_addFraxis(xy, fracPart, nDigits, overwrite, fraxis));
    internal_operationComplete();
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::add(
    const Coord& xy,
    bool negative,
    const std::string& intPart,
    const std::string& fracPart,
    bool overwrite,
    Fraxis fraxis
) {
    Log_N_Debug2_H("");
    auto lock = lockWriteable();
    internal_checkFraxis(fraxis);
    If_Log_Showing_Debug2(
        Log_N_Debug2(""
            << "add at " << xy
            << ", neg:" << negative
            << ", intPart:" << intPart
            << ", fracPart:" << fracPart
            << ", overwrite:" << overwrite
            << ", fraxis: " << FraxisToName(fraxis)
        );
    );
    locked_carryoverCleanup(
        locked_add(
            xy,
            negative,
            intPart,
            fracPart,
            overwrite,
            fraxis
        )
    );
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(
    const Coord& xy,
    bool negative,
    const std::string& intPart,
    const std::string& fracPart,
    bool overwrite,
    Fraxis fraxis
) {
    Log_N_Debug3_H("");
    If_Log_Showing_Debug3(
        Log_N_Debug3(""
            << "add at " << xy
            << ", neg:" << negative
            << ", intPart:" << intPart
            << ", fracPart:" << fracPart
            << ", overwrite:" << overwrite
            << ", fraxis: " << FraxisToName(fraxis)
        );
    );

    CoordSet changed;
    int maxIndex = intPart.size() - 1;
    long long lSign = negative ? -1 : 1;
    long long baseFactor = 1;
    long lbase = m_config.base();
    for (int i = 0; i <= maxIndex; ++i) {
        int pos = maxIndex - i;
        char c = intPart[pos];
        long long lDigit = Tools::unsafe_alphaToDigit(c);
        changed.merge(locked_add(xy, lSign * lDigit * baseFactor, overwrite));
        baseFactor *= lbase;
    }
    if (fracPart.empty()) {
        internal_operationComplete();
        Log_N_Debug3_T("changed " << changed.size() << " digits");
        return changed;
    }

    // Prepare for fraxis expansion
    Coord xyRoot = xy;
    int dX;
    int dY;
    int c;
    internal_checkFraxis(fraxis);

    switch(fraxis) {
        case Fraxis::X:
            dX = -1;
            dY = 0;
            c = -1;
            break;
        case Fraxis::Y:
            dX = 0;
            dY = -1;
            c = 1;
            break;
        default:
            std::ostringstream oss;
            oss << "Fraxis not valid: " << FraxisToName(fraxis);
            std::invalid_argument err(oss.str());
            Log_N_Error(err.what());
            throw err;
            break;
    }
    maxIndex = fracPart.size() - 1;
    Log_Debug4("d(" << dX << "," << dY << "),c=" << c << ",maxIndex=" << maxIndex);

    int iSign = negative ? -1 : 1;
    for (int i = 0; i <= maxIndex; ++i) {
        // int pos = maxIndex - i;
        char ch = fracPart[i];
        int iDigit = Tools::unsafe_alphaToDigit(ch) * iSign;

        // Move
        xyRoot.translate(dX, dY);

        // Add fractional part root
        Log_Debug4("dispatch locked_add(" << xyRoot << "," << iDigit << ",...)");
        changed.merge(locked_add(xyRoot, iDigit, overwrite));

        Log_Debug4("dispatch cascade(" << xyRoot << "," << iDigit << ",c=" << c << ")");
        // Add fraxis cascade
        changed.merge(
            internal_fraxisCascade(xyRoot, iDigit, overwrite, c, m_config.fraxisCascadeDepth())
        );

        Log_Debug4("cascade return");

        // changed.merge(locked_add(xyWorking, d, overwrite));
        // changed.merge(internal_fraxisCascade(xyWorking, d, overwrite, c));
        // f -= d;
        // xyWorking.translate(dX, dY);

    }
    internal_operationComplete();
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::subtract(
    const Coord& xy, float realNum, int nDigits, bool overwrite, Fraxis fraxis
) {
    Log_N_Debug2_H("");
    auto lock = lockWriteable();
    internal_checkFraxis(fraxis);
    If_Log_Showing_Debug2(
        Log_N_Debug2_H("subtract " << realNum << ", fraxis: " << FraxisToName(fraxis));
    );
    locked_carryoverCleanup(
        locked_add(xy, static_cast<double>(-realNum), nDigits, overwrite, fraxis)
    );
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::subtract(
    const Coord& xy, double realNum, int nDigits, bool overwrite, Fraxis fraxis
) {
    Log_N_Debug2_H("");
    auto lock = lockWriteable();
    internal_checkFraxis(fraxis);
    If_Log_Showing_Debug2(
        Log_N_Debug2_H("subtract " << realNum << ", fraxis: " << FraxisToName(fraxis));
    );
    locked_carryoverCleanup(locked_add(xy, -realNum, nDigits, overwrite, fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::add(const Coord& xy, Digit value, bool overwrite, Fraxis unused) {
    If_Log_Showing_Debug2(
        Log_N_Debug2_H("add " << static_cast<int>(value) << ", no fraxis");
    );
    auto lock = lockWriteable();
    int ivalue = static_cast<int>(value);
    locked_carryoverCleanup(locked_add(xy, ivalue, overwrite));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, Digit value, bool overwrite) {
    Log_N_Debug3_H("");
    If_Log_Showing_Debug3(
        Log_N_Debug3("add " << static_cast<int>(value) << ", no fraxis");
    );
    CoordSet changed = locked_add(xy, static_cast<int>(value), overwrite);
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::add(const Coord& xy, int value, bool overwrite, Fraxis unused) {
    Log_N_Debug2_H("add " << value << ", no fraxis");
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_add(xy, value, overwrite));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, int value, bool overwrite) {
    int val(internal_checkOverwrite<int>(xy, overwrite));
    int sum = val + value;
    int carry = sum / m_config.base();
    int rem = sum % m_config.base();
    CoordSet changed;
    Log_N_Debug3_H(
        "at " << xy << ", add " << value << ", no fraxis, result: " << sum << ":(r"
        << rem << ",c" << carry << ")"
    );

    if (locked_setValue(xy, rem)) {
        changed.insert(xy);
    }
    if (carry != 0) {
        changed.merge(locked_add(xy.translatedX(1), carry, overwrite));
        changed.merge(locked_add(xy.translatedY(1), carry, overwrite));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
    // internal_operationComplete();
}


void mdn::Mdn2d::add(const Coord& xy, long value, bool overwrite, Fraxis unused) {
    Log_N_Debug2_H("add " << value << ", no fraxis");
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_add(xy, value, overwrite));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, long value, bool overwrite) {
    long val(internal_checkOverwrite<long>(xy, overwrite));
    long sum = val + value;
    long carry = sum / m_config.base();
    long rem = sum % m_config.base();
    Log_N_Debug3_H(
        "at " << xy << ", add " << value << ", no fraxis, result: " << sum << ":(r"
        << rem << ",c" << carry << ")"
    );
    CoordSet changed;
    if (locked_setValue(xy, rem)) {
        changed.insert(xy);
    }
    if (carry != 0) {
        changed.merge(locked_add(xy.translatedX(1), carry, overwrite));
        changed.merge(locked_add(xy.translatedY(1), carry, overwrite));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::add(const Coord& xy, long long value, bool overwrite, Fraxis unused) {
    Log_N_Debug2_H("at " << xy << ", add " << value << ", no fraxis");
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_add(xy, value, overwrite));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_add(const Coord& xy, long long value, bool overwrite) {
    long long val(internal_checkOverwrite<long long>(xy, overwrite));
    long long sum = val + value;
    long long carry = sum / m_config.base();
    long long rem = sum % m_config.base();
    Log_N_Debug3_H(
        "at " << xy << ", add " << value << ", no fraxis, result: " << sum << ":(r"
        << rem << ",c" << carry << ")"
    );
    CoordSet changed;
    if (locked_setValue(xy, rem)) {
        changed.insert(xy);
    }
    if (carry != 0) {
        changed.merge(locked_add(xy.translatedX(1), carry, overwrite));
        changed.merge(locked_add(xy.translatedY(1), carry, overwrite));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::subtract(const Coord& xy, Digit value, bool overwrite, Fraxis unused) {
    If_Log_Showing_Debug2(
        Log_N_Debug2_H("at " << xy << ", add " << static_cast<int>(value) << ", no fraxis");
    );
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_add(xy, -static_cast<int>(value), overwrite));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::subtract(const Coord& xy, int value, bool overwrite, Fraxis unused) {
    Log_N_Debug2_H("at " << xy << ", subtract " << value << ", no fraxis");
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_add(xy, -value, overwrite));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::subtract(const Coord& xy, long value, bool overwrite, Fraxis unused) {
    Log_N_Debug2_H("at " << xy << ", subtract " << value << ", no fraxis");
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_add(xy, -value, overwrite));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::subtract(const Coord& xy, long long value, bool overwrite, Fraxis unused) {
    Log_N_Debug2_H("at " << xy << ", subtract " << value << ", no fraxis");
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_add(xy, -value, overwrite));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::addFraxis(
    const Coord& xy, float fraction, int nDigits, bool overwrite, Fraxis fraxis
) {
    If_Log_Showing_Debug2(
        Log_N_Debug2_H(
            "at " << xy << ", add fraction " << fraction << ", fraxis: " << FraxisToName(fraxis)
        );
    );
    auto lock = lockWriteable();
    locked_carryoverCleanup(
        locked_addFraxis(xy, static_cast<double>(fraction), nDigits, overwrite, fraxis)
    );
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::addFraxis(
    const Coord& xy, double fraction, int nDigits, bool overwrite, Fraxis fraxis
) {
    If_Log_Showing_Debug2(
        Log_N_Debug2_H(
            "at " << xy << ", add fraction " << fraction << ", fraxis: " << FraxisToName(fraxis)
        );
    );
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_addFraxis(xy, fraction, nDigits, overwrite, fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_addFraxis(
    const Coord& xy, double fraction, int nDigits, bool overwrite, Fraxis fraxis
) {
    if (fraction < -1.0 || fraction > 1.0) {
        std::invalid_argument error(
            "Fractional part must be -1.0 < fraction < 1.0, got" + std::to_string(fraction) +
            "."
        );
        Log_N_Error(error.what());
        throw error;
    }
    If_Log_Showing_Debug3(
        Log_N_Debug3_H(
            "at " << xy << ", add fraction " << fraction << ", fraxis: " << FraxisToName(fraxis)
        );
    );

    CoordSet changed;
    switch(fraxis) {
        case Fraxis::X:
            changed = internal_fraxis(xy.translatedX(-1), fraction, nDigits, overwrite, -1, 0, -1);
            break;
        case Fraxis::Y:
            changed = internal_fraxis(xy.translatedY(-1), fraction, nDigits, overwrite, 0, -1, 1);
            break;
        default:
            std::ostringstream oss;
            oss << "Fraxis not valid: " << FraxisToName(fraxis) << ", truncating " << fraction;
            std::invalid_argument err(oss.str());
            Log_N_Error(err.what());
            throw err;
            break;
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


void mdn::Mdn2d::subtractFraxis(
    const Coord& xy, float fraction, int nDigits, bool overwrite, Fraxis fraxis
) {
    If_Log_Showing_Debug2(
        Log_N_Debug2_H(
            "at " << xy << ", subtract fraction " << fraction << ", fraxis: "
            << FraxisToName(fraxis)
        );
    );
    auto lock = lockWriteable();
    locked_carryoverCleanup(
        locked_addFraxis(xy, -static_cast<double>(fraction), nDigits, overwrite, fraxis)
    );
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::subtractFraxis(
    const Coord& xy, double fraction, int nDigits, bool overwrite, Fraxis fraxis
) {
    If_Log_Showing_Debug2(
        Log_N_Debug2_H(
            "at " << xy << ", subtract fraction " << fraction << ", fraxis: "
            << FraxisToName(fraxis)
        );
    );
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_addFraxis(xy, -fraction, nDigits, overwrite, fraxis));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::multiply(Digit value) {
    If_Log_Showing_Debug2(
        Log_N_Debug2_H("scalar multiply " << static_cast<int>(value));
    );
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_multiply(static_cast<int>(value)));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::multiply(int value) {
    auto lock = lockWriteable();
    Log_N_Debug2_H("scalar multiply " << value);
    locked_carryoverCleanup(locked_multiply(value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::multiply(long value) {
    Log_N_Debug2_H("scalar multiply " << value);
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_multiply(value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


void mdn::Mdn2d::multiply(long long value) {
    Log_N_Debug2_H("scalar multiply " << value);
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_multiply(value));
    internal_operationComplete();
    Log_N_Debug2_T("");
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(int value) {
    Log_N_Debug3_H("scalar multiply " << value);
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed;

    for (const auto& [xy, digit] : m_raw) {
        int vxd = value*static_cast<int>(digit);
        changed.merge(temp.locked_add(xy, vxd));
    }
    locked_operatorEquals(temp);
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(long value) {
    Log_N_Debug3_H("scalar multiply " << value);
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed;

    for (const auto& [xy, digit] : m_raw) {
        long vxd = value*static_cast<long>(digit);
        changed.merge(temp.locked_add(xy, vxd));
    }
    locked_operatorEquals(temp);
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::CoordSet mdn::Mdn2d::locked_multiply(long long value) {
    Log_N_Debug3_H("scalar multiply " << value);
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed;

    for (const auto& [xy, digit] : m_raw) {
        long long vxd = value*static_cast<long long>(digit);
        changed.merge(temp.locked_add(xy, vxd));
    }
    locked_operatorEquals(temp);
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


bool mdn::Mdn2d::operator==(const Mdn2d& rhs) const {
    Log_N_Debug2_H("");
    auto lock = lockReadOnly();
    auto lockRhs = rhs.lockReadOnly();
    if (rhs.m_config != m_config) {
        Log_N_Debug2_T("equality test, result: config differs");
        return false;
    }
    if (m_raw == rhs.m_raw) {
        Log_N_Debug2_T("equality test, result: data are equal");
        return true;
    }
    if (
        m_config.signConvention() == SignConvention::Positive ||
        m_config.signConvention() == SignConvention::Negative
    ) {
        Log_N_Debug2_T(
            "equality test, result: data !=, with sign convention - no need for polymorphism tests"
        );
        return false;
    }
    Log_N_Debug2("equality test, digits differ, checking polymorphism");
    Mdn2d dlhs = Duplicate(*this);
    Mdn2d drhs = Duplicate(rhs);
    auto dlhsLock = dlhs.lockWriteable();
    auto drhsLock = drhs.lockWriteable();
    dlhs.m_config.setSignConvention(SignConvention::Positive);
    drhs.m_config.setSignConvention(SignConvention::Positive);
    dlhs.locked_carryoverCleanupAll();
    drhs.locked_carryoverCleanupAll();
    bool result = dlhs.m_raw == drhs.m_raw;
    Log_N_Debug2_T(
        "equality test, final polymorphism result = " << result
    );
    return result;
}


bool mdn::Mdn2d::operator!=(const Mdn2d& rhs) const {
    Log_N_Debug3_H("inequality test");
    bool result = !(*this == rhs);
    Log_N_Debug3_T("result=" << result);
    return result;
}


mdn::Mdn2d& mdn::Mdn2d::operator+=(const Mdn2d& rhs) {
    Log_N_Debug2_H("plus equals");
    auto lock = lockWriteable();
    ReadOnlyLock* lockRhsPtr;
    if (&rhs != this) {
        auto lockRhs = rhs.lockReadOnly();
        lockRhsPtr = &lockRhs;
    }
    CoordSet changed = locked_plusEquals(rhs);
    locked_carryoverCleanup(changed);
    internal_operationComplete();
    Log_N_Debug2_T("");
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_plusEquals(const Mdn2d& rhs) {
    Log_N_Debug3_H("plus equals");
    CoordSet changed;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(locked_add(xy, digit));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator-=(const Mdn2d& rhs) {
    Log_N_Debug2_H("minus equals");
    auto lock = lockWriteable();
    ReadOnlyLock* lockRhsPtr;
    if (&rhs != this) {
        auto lockRhs = rhs.lockReadOnly();
        lockRhsPtr = &lockRhs;
    }
    CoordSet changed = locked_minusEquals(rhs);
    locked_carryoverCleanup(changed);
    internal_operationComplete();
    Log_N_Debug2_T("");
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_minusEquals(const Mdn2d& rhs) {
    Log_N_Debug3_H("minus equals");
    CoordSet changed;
    for (const auto& [xy, digit] : rhs.m_raw) {
        changed.merge(locked_add(xy, -digit));
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(const Mdn2d& rhs) {
    Log_N_Debug2_H("times equals (Mdn2d)");
    auto lock = lockWriteable();
    ReadOnlyLock* lockRhsPtr;
    if (&rhs != this) {
        auto lockRhs = rhs.lockReadOnly();
        lockRhsPtr = &lockRhs;
    }
    locked_carryoverCleanup(locked_timesEquals(rhs));
    internal_operationComplete();
    Log_N_Debug2_T("");
    return *this;
}


mdn::CoordSet mdn::Mdn2d::locked_timesEquals(const Mdn2d& rhs) {
    Log_N_Debug3_H("times equals (Mdn2d)");
    Mdn2d temp = NewInstance(m_config);
    auto tempLock = temp.lockWriteable();
    CoordSet changed = locked_multiply(rhs, temp);
    locked_operatorEquals(temp);
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::Mdn2d& mdn::Mdn2d::operator/=(const Mdn2d& rhs) {
    Log_N_Debug3_H("divide equals (Mdn2d)");
    Mdn2d temp = Duplicate(*this);
    auto lock = lockWriteable();
    auto tempLock = temp.lockWriteable();
    CoordSet changed = locked_divide(rhs, temp);
    locked_operatorEquals(temp);
    locked_carryoverCleanup(changed);
    internal_operationComplete();
    Log_N_Debug3_T("");
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(int scalar) {
    auto lock = lockWriteable();
    Log_N_Debug3_H("times equals (int scalar)");
    locked_carryoverCleanup(locked_multiply(scalar));
    internal_operationComplete();
    Log_N_Debug3_T("");
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(long scalar) {
    Log_N_Debug3_H("times equals (long scalar)");
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_multiply(scalar));
    internal_operationComplete();
    Log_N_Debug3_T("");
    return *this;
}


mdn::Mdn2d& mdn::Mdn2d::operator*=(long long scalar) {
    Log_N_Debug3_H("times equals (long long scalar)");
    auto lock = lockWriteable();
    locked_carryoverCleanup(locked_multiply(scalar));
    internal_operationComplete();
    Log_N_Debug3_T("");
    return *this;
}


void mdn::Mdn2d::internal_checkFraxis(Fraxis& fraxis) const {
    if (fraxis == Fraxis::Default) {
        If_Log_Showing_Debug4(
            Log_N_Debug4("applying default fraxis: " << FraxisToName(fraxis));
        );
        fraxis = m_config.fraxis();
    } else {
        If_Log_Showing_Debug4(
            Log_N_Debug4("supplied fraxis: " << FraxisToName(fraxis));
        );
    }
}


mdn::CoordSet mdn::Mdn2d::internal_fraxis(
    const Coord& xy, double f, int nDigits, bool overwrite, int dX, int dY, int c
) {
    #ifdef MDN_DEBUG
        if (f < -1.0 || f > 1.0)
        {
            throw std::invalid_argument(
                "Fractional part must be -1.0 < f < 1.0, got" + std::to_string(f) +
                "."
            );
        }
    #endif
    Log_N_Debug3_H(
        "fraxis at " << xy << ", fraction: " << f << ", nDigits: " << nDigits << ", delta: ("
        << dX << "," << dY << ",c" << c << ")"
    );

    CoordSet changed;

    if (nDigits < 0) {
        nDigits = m_config.precision();
        if (nDigits < 0) {
            // Arbitrarily
            nDigits = 100;
        }
    }
    // --nDigits;

    // Calculate next digit
    Coord xyWorking(xy);
    bool precisionOkay = locked_checkPrecisionWindow(xyWorking) != PrecisionStatus::Below;
    bool numericsOkay = fabs(f) > m_config.epsilon();
    If_Log_Showing_Debug4(
        Log_N_Debug4(
            "internal_fraxis loop, starting at " << xyWorking << ", "
            << "current vals: f=" << f << ", " << "xy=" << xyWorking << ", "
            << "loop: prec=" << precisionOkay << ", num=" << numericsOkay
        );
    );
    while (precisionOkay && numericsOkay && nDigits--) {
        f *= m_config.baseDouble();
        Digit d = nDigits ? static_cast<Digit>(f) : static_cast<Digit>(std::round(f));
        changed.merge(locked_add(xyWorking, d, overwrite));
        changed.merge(
            internal_fraxisCascade(xyWorking, d, overwrite, c, m_config.fraxisCascadeDepth())
        );
        f -= d;
        xyWorking.translate(dX, dY);
        precisionOkay = locked_checkPrecisionWindow(xyWorking) != PrecisionStatus::Below;
        numericsOkay = fabs(f) > m_config.epsilon();
        If_Log_Showing_Debug4(
            Log_N_Debug4(
                "internal_fraxis loop, added " << static_cast<int>(d) << " to "
                << xyWorking.translated(-dX, -dY) << ", "
                << "current vals: f=" << f << ", " << "xy=" << xyWorking << ", "
                << "loop: prec=" << precisionOkay << ", num=" << numericsOkay
            );
        );
    }
    // if (precisionOkay && numericsOkay) {
    //     Log_N_Warn(
    //         "Internal error: possible infinite loop detected in internal_fraxis algorithm.\n"
    //         << "parameters: fraxis at " << xy << ", fraction: " << f << ", delta: ("
    //         << dX << "," << dY << ",c" << c << ")"
    //     );
    // }

    if (changed.size()) {
        internal_modified();
    }
    Log_N_Debug3_T("changed " << changed.size() << " digits");
    return changed;
}


mdn::CoordSet mdn::Mdn2d::internal_fraxisCascade(
    const Coord& xy, Digit d, bool overwrite, int c, int cascade
) {
    If_Log_Showing_Debug4(
        Log_N_Debug4_H("cascade: " << static_cast<int>(d) << " at " << xy);
    );
    CoordSet changed;
    Coord xyNext = xy.translated(c, -c);
    d *= -1;
    if (locked_checkPrecisionWindow(xyNext) == PrecisionStatus::Below) {
        // Cascade done
        Log_N_Debug4_T(
            "cascaded to edge of precision window, changed " << changed.size() << " digits"
        );
        return changed;
    }
    changed.merge(locked_add(xyNext, d, overwrite));
    if (--cascade) {
        Log_N_Debug4("cascade=" << cascade);
        changed.merge(internal_fraxisCascade(xyNext, d, overwrite, c, cascade));
    }
    Log_N_Debug4_T("finalising cascade, changed " << changed.size() << " digits");
    return changed;
}


mdn::Mdn2d mdn::Mdn2d::internal_copyMultiplyAndShift(int value, const Coord& shiftXY) const {
    Log_N_Debug4_H("value: " << value << ", shift: " << shiftXY);
    Mdn2d temp = Duplicate(*this);
    {
        auto tempLock = temp.lockWriteable();
        temp.locked_multiply(value);
        temp.locked_shift(shiftXY);
    }
    Log_N_Debug4_T("");
    return temp;
}
