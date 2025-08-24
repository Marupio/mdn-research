#pragma once

/*
    MdnQtInterface
    Provides conversion functionality between elemental type classes in Mdn and conceptually similar
    classes in QT
*/

#include <QString>
#include <QPoint>
#include <QRect>
#include <QJsonObject>

#include "../library/Coord.hpp"
#include "../library/Rect.hpp"


namespace mdn {

class MdnQtInterface {
public:

    // *** Strings

        // Mdn std::string to QString
        static QString toQString(const std::string& s) {
            return QString::fromStdString(s);
        }

        // QString to Mdn std::string
        static std::string fromQString(const QString& qs) {
            return qs.toStdString();
        }


    // *** Coords

        // Mdn Coord to QPoint
        static QPoint toQPoint(const Coord& c) {
            return QPoint(c.x(), c.y());
        }

        // QPoint to Mdn Coord
        static Coord fromQPoint(const QPoint& p) {
            return Coord(p.x(), p.y());
        }


    // *** Rect

        // Mdn Rect to QRect
        static QRect toQRect(const Rect& r) {
            if (!r.isValid())
                return QRect(); // returns an invalid QRect

            Coord min = r.min();
            int width = r.width();   // includes +1 already
            int height = r.height(); // includes +1 already

            return QRect(min.x(), min.y(), width, height);
        }

        // QRect to Mdn Rect
        static Rect toRect(const QRect& q) {
            if (!q.isValid())
                return Rect::GetInvalid();

            Coord min(q.left(), q.top());
            Coord max(q.right(), q.bottom()); // inclusive in MDN

            return Rect(min, max);
        }

        // Mdn Rect to Json
        static QJsonObject rectToJson(const Rect& r) {
            QJsonObject o;
            o["x0"] = r.left();
            o["y0"] = r.bottom();
            o["x1"] = r.right();
            o["y1"] = r.top();
            return o;
        }

        // Mdn Rect to Json, string form
        static QString rectToJsonStr(const Rect& r) {
            return QString
            (
                "{\"x0\": %1, \"y0\": %2, \"x1\": %3, \"y1\": %4}"
            ).arg(r.left()).arg(r.bottom()).arg(r.right()).arg(r.top());
        }


        static QString joinDigitsTSV(const std::vector<Digit>& v) {
            QStringList parts;
            parts.reserve(int(v.size()));
            for (Digit d : v) {
                parts << QString::number(int(d));
            }
            return parts.join('\t');
        }

};

} // end namespace mdn
