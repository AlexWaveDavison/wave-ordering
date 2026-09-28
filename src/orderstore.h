#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>
#include <optional>

struct OrderLine
{
    int productId = -1; // -1 if the product no longer exists
    QString name;
    QString sku;
    QString unit;
    qint64 unitPricePence = 0; // 0 = price on application
    int quantity = 0;

    qint64 lineTotalPence() const { return unitPricePence * quantity; }
    bool isPoa() const { return unitPricePence == 0; }
};

struct OrderRecord
{
    int id = -1;
    QDateTime createdAt;
    QString status;
    QString customerName;
    QString accountName;
    QString notes;
    QDateTime sentAt;
    QVector<OrderLine> lines;

    // Sum of priced lines. POA lines contribute nothing until priced.
    qint64 totalPence() const;
    bool hasPoaItems() const;
    int itemCount() const;
    bool isSent() const;
};

// Reads and writes orders in the local database. Orders are snapshots: each
// line keeps its own name/SKU/price so history doesn't change when the
// catalogue does.
namespace OrderStore {

// An order is "pending" once it's saved on the device, and "sent" once the
// user confirms they've sent it to their rep (e.g. over WhatsApp).
inline const QString StatusPending = QStringLiteral("pending");
inline const QString StatusSent = QStringLiteral("sent");

QString statusLabel(const QString &status);

// "28 Sep 2026, 20:01" in local time; empty for an invalid date.
QString formatTimestamp(const QDateTime &dateTime);

// Inserts the order and its lines in one transaction. Returns the new order
// id, or -1 on failure (nothing is written).
int create(const OrderRecord &order);

std::optional<OrderRecord> load(int orderId);

bool markSent(int orderId, const QDateTime &when = QDateTime::currentDateTime());

} // namespace OrderStore

// A WhatsApp-friendly plain-text summary of an order: header, customer and
// account, a monospace table of lines, total and notes.
QString buildOrderShareText(const OrderRecord &order);
