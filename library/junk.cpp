void mdn::Mdn2d::locked_addReal(const Coord& xy, double realNum, Fraxis fraxis) {
    double fracPart, intPart;
    fracPart = modf(realNum, &intPart);
    locked_addInteger(xy, intPart);
    locked_addFraxis(xy, fracPart, fraxis);
}


void mdn::Mdn2d::locked_addInteger(const Coord& xy, int value) {
    int div;
    if (value == 0) {
        return;
    }

    // Grad current value, cast to int
    auto it = m_raw.find(xy);
    if (it == m_raw.end()) {
        // xy has zero value
        int sum = value;
        div = sum / m_base;
        int rem = sum % m_base;
        if (rem != 0) {
            // Check bounds, only insert value if valid
            if (locked_checkBounds(xy)) {
                locked_insertAddress(xy);
                m_raw[xy] = rem;
            }
        }
    } else {
        // xy is non-zero
        int curVal = it->second;
        int sum = value + curVal;
        div = sum / m_base;
        int rem = sum % m_base;
        if (rem != 0) {
            m_raw[xy] = rem;
        }
    }
    if (div != 0) {
        locked_addInteger(Coord({xy.first+1, xy.second}), div);
        locked_addInteger(Coord({xy.first, xy.second+1}), div);
    }
}


void mdn::Mdn2d::locked_addFraxis(const Coord& xy, double fraction, Fraxis fraxis) {
    #ifdef MDN_DEBUG
        if (fraction < -1.0 || fraction > 1.0)
        {
            std::ostringstream oss;
            oss << "fraction out of range (-1.0 .. 1.0), got " << fraction;
            Logger::instance().error(oss.str());
        }
    #endif

    switch(fraxis)
    {
        case Fraxis::X:
            locked_addFraxisX(xy, fraction);
            break;
        case Fraxis::Y:
            locked_addFraxisY(xy, fraction);
            break;
        default:
            std::ostringstream oss;
            oss << "Fraxis not valid: " << FraxisToName(fraxis) << ", truncating " << fraction;
            Logger::instance().error(oss.str());
            break;
    }
}


void mdn::Mdn2d::locked_setValue(const Coord& xy, int value) {
    #ifdef MDN_DEBUG
        if (value >= m_base || value <= -m_base) {
            mdn::OutOfRange ex(xy, value, m_base);
            Logger::instance().error(ex.what());
            throw ex;
        }
    #endif

    // Sparse storage does not store zeroes
    if (value == 0)
    {
        locked_setToZero(xy);
        return;
    }

    // Non-zero entry - set / overwrite
    m_raw[xy] = value;
}


