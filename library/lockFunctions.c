void locked_clear();
void locked_clearMetadata() const;
void locked_rebuildMetadata() const;
void locked_insertAddress(const Coord& xy) const;

Digit locked_getValue(const Coord& xy) const;
std::vector<Digit> locked_getRow(int y) const;
void locked_getRow(int y, std::vector<Digit>& digits) const;
std::vector<Digit> locked_getCol(int x) const;
void locked_getCol(int x, std::vector<Digit>& digits) const;


void locked_setValue(const Coord& xy, Digit value);

void locked_plus(const Mdn2d& rhs, Mdn2d& ans) const;
void locked_minus(const Mdn2d& rhs, Mdn2d& ans) const;
void locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const;
void locked_divide(const Mdn2d& rhs, Mdn2d& ans) const;

void locked_add(const Coord& xy, double realNum, Fraxis fraxis);
void locked_subtract(const Coord& xy, double realNum, Fraxis fraxis);
void locked_add(const Coord& xy, Digit value);
void locked_add(const Coord& xy, int value);
void locked_add(const Coord& xy, long value);
void locked_add(const Coord& xy, long long value);
void locked_subtract(const Coord& xy, Digit value);
void locked_subtract(const Coord& xy, int value);
void locked_subtract(const Coord& xy, long value);
void locked_subtract(const Coord& xy, long long value);
void locked_add(const Coord& xy, double fraction, Fraxis fraxis));
void locked_subtract(const Coord& xy, double fraction, Fraxis fraxis));
void locked_toString() const;
void locked_toStringRows() const;
void locked_toStringCols() const;
void locked_carryOver(const Coord& xy);
void locked_shiftRight(nDigits);
void locked_shiftLeft(nDigits);
void locked_shiftUp(nDigits);
void locked_shiftDown(nDigits);
void locked_transpose();
