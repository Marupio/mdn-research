struct DecodedPaste {
    // "rect" | "mdn" | ""(unknown)
    std::string scope;

    mdn::Rect srcRect = mdn::Rect::Invalid();

    // rows[r][c]
    std::vector<std::vector<mdn::Digit>> rows;

    bool valid() const { return !rows.empty() && !rows.front().empty(); }
    int width()  const { return valid()? int(rows.front().size()) : 0; }
    int height() const { return valid()? int(rows.size())         : 0; }
};

// decodeClipboard(): extend your current helper
// - read JSON -> scope, origin.rect, rect, grid_tsv
// - if JSON missing, TSV fallback -> scope="rect" ; srcRect remains Invalid

bool Project::pasteOnSelection() {
    const auto& sel = selection();
    mdn::Mdn2d* dst = sel.get();
    if (!dst) {
        return false;
    }

    DecodedPaste p = decodeClipboard();
    if (!p.valid()) {
        return false;
    }

    const int W = p.width(), H = p.height();
    const bool haveSel = sel.rect.isValid();

    int ax=0, ay=0;

    if (haveSel) {
        const int SW = sel.rect.width();
        const int SH = sel.rect.height();
        if (SW==1 && SH==1) {
            ax=sel.rect.left();
            ay=sel.rect.bottom();
        }
        else if (SW==W && SH==H) {
            ax=sel.rect.left();
            ay=sel.rect.bottom();
        }
        else { return showSizeMismatch(SW,SH,W,H), false; }
    } else {
        if (p.scope=="rect") {
            if (p.srcRect.isValid()) {
                ax=p.srcRect.left();
                ay=p.srcRect.bottom();
            }
            else {
                ax=0;
                ay=0;
            }
        } else if (p.scope=="mdn") {
            clearAll(*dst); // clear target MDN (bounds or whole domain)
            ax = p.srcRect.isValid()? p.srcRect.left() : 0;
            ay = p.srcRect.isValid()? p.srcRect.bottom(): 0;
        } else {
            // unknown scope -> conservative default
            ax=0;
            ay=0;
        }
    }

    for (int r=0; r<H; ++r) {
        dst->setRowRange(ay + r, ax, p.rows[size_t(r)]);
    }

    return true;
}
