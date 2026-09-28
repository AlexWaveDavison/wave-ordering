#pragma once

#include <QString>
#include <QtGlobal>

// Money is stored and calculated as whole pence (qint64) so totals never pick
// up floating-point rounding errors. A price of 0 means "price on
// application" (POA): the item can be ordered but has no list price.
namespace Money {

// Converts a decimal pounds value (e.g. 14.5) to pence, rounding half away
// from zero. Only used at the edges (seed data, legacy migrations).
qint64 fromPounds(double pounds);

// "£1,234.50", "-£3.00", "£0.00"
QString format(qint64 pence);

// Like format(), but 0 becomes "POA".
QString formatOrPoa(qint64 pence);

} // namespace Money
