#include <mdn/TextOptions.hpp>

mdn::TextWriteOptions mdn::TextWriteOptions::DefaultPretty() {
    TextWriteOptions o;
    o.axes = AxesOutput::BoxArt;
    o.alphanumeric = true;
    o.wideNegatives = true;
    o.delim = CommaTabSpace::Space;
    o.window = Rect::GetInvalid();
    return o;
}


mdn::TextWriteOptions mdn::TextWriteOptions::DefaultUtility(CommaTabSpace d) {
    TextWriteOptions o;
    o.axes = AxesOutput::None;
    o.alphanumeric = false;
    o.wideNegatives = false;
    o.delim = d;
    o.window = Rect::GetInvalid();
    return o;
}
