#include "money.h"

#include <QLocale>
#include <cmath>

namespace Money {

qint64 fromPounds(double pounds)
{
    return static_cast<qint64>(std::llround(pounds * 100.0));
}

QString format(qint64 pence)
{
    static const QLocale ukLocale(QLocale::English, QLocale::UnitedKingdom);

    const bool negative = pence < 0;
    const qint64 absolute = negative ? -pence : pence;
    const QString pounds = ukLocale.toString(static_cast<qlonglong>(absolute / 100));
    const QString penceText = QStringLiteral("%1").arg(absolute % 100, 2, 10, QLatin1Char('0'));

    return QStringLiteral("%1£%2.%3")
        .arg(negative ? QStringLiteral("-") : QString(), pounds, penceText);
}

QString formatOrPoa(qint64 pence)
{
    return pence == 0 ? QStringLiteral("POA") : format(pence);
}

} // namespace Money
