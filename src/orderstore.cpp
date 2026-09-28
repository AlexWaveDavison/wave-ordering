#include "orderstore.h"

#include "database.h"
#include "money.h"

#include <QCoreApplication>
#include <QLocale>
#include <QSqlQuery>
#include <QStringList>

qint64 OrderRecord::totalPence() const
{
    qint64 sum = 0;
    for (const OrderLine &line : lines)
        sum += line.lineTotalPence();
    return sum;
}

bool OrderRecord::hasPoaItems() const
{
    for (const OrderLine &line : lines) {
        if (line.isPoa())
            return true;
    }
    return false;
}

int OrderRecord::itemCount() const
{
    int sum = 0;
    for (const OrderLine &line : lines)
        sum += line.quantity;
    return sum;
}

bool OrderRecord::isSent() const
{
    return status == OrderStore::StatusSent;
}

namespace OrderStore {

QString formatTimestamp(const QDateTime &dateTime)
{
    // The C locale gives stable English month names ("Sep"); en_GB's CLDR
    // data varies between Qt versions ("Sep" vs "Sept").
    return dateTime.isValid() ? QLocale::c().toString(dateTime, QStringLiteral("d MMM yyyy, HH:mm")) : QString();
}

QString statusLabel(const QString &status)
{
    if (status == StatusSent)
        return QCoreApplication::translate("OrderStore", "Sent");
    if (status == StatusPending)
        return QCoreApplication::translate("OrderStore", "Not sent");
    return status;
}

int create(const OrderRecord &order)
{
    if (order.lines.isEmpty())
        return -1;

    QSqlDatabase db = Database::connection();
    if (!db.transaction())
        return -1;

    QSqlQuery insertOrder(db);
    insertOrder.prepare(QStringLiteral(R"(
        INSERT INTO orders (created_at, status, customer_name, account_name, notes, total_pence, sent_at)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )"));
    insertOrder.addBindValue(order.createdAt.toString(Qt::ISODate));
    insertOrder.addBindValue(order.status.isEmpty() ? StatusPending : order.status);
    insertOrder.addBindValue(order.customerName);
    insertOrder.addBindValue(order.accountName);
    insertOrder.addBindValue(order.notes);
    insertOrder.addBindValue(order.totalPence());
    insertOrder.addBindValue(order.sentAt.isValid() ? order.sentAt.toString(Qt::ISODate) : QVariant());
    if (!Database::exec(insertOrder)) {
        db.rollback();
        return -1;
    }

    const int orderId = insertOrder.lastInsertId().toInt();

    QSqlQuery insertLine(db);
    insertLine.prepare(QStringLiteral(R"(
        INSERT INTO order_items (order_id, product_id, product_name, sku, unit,
                                 unit_price_pence, quantity, line_total_pence)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )"));

    for (const OrderLine &line : order.lines) {
        insertLine.addBindValue(orderId);
        insertLine.addBindValue(line.productId >= 0 ? QVariant(line.productId) : QVariant());
        insertLine.addBindValue(line.name);
        insertLine.addBindValue(line.sku);
        insertLine.addBindValue(line.unit);
        insertLine.addBindValue(line.unitPricePence);
        insertLine.addBindValue(line.quantity);
        insertLine.addBindValue(line.lineTotalPence());
        if (!Database::exec(insertLine)) {
            db.rollback();
            return -1;
        }
    }

    if (!db.commit()) {
        db.rollback();
        return -1;
    }
    return orderId;
}

std::optional<OrderRecord> load(int orderId)
{
    QSqlDatabase db = Database::connection();

    QSqlQuery header(db);
    header.prepare(QStringLiteral(R"(
        SELECT id, created_at, status, customer_name, account_name, notes, sent_at
        FROM orders WHERE id = ?
    )"));
    header.addBindValue(orderId);
    if (!Database::exec(header) || !header.next())
        return std::nullopt;

    OrderRecord order;
    order.id = header.value(0).toInt();
    order.createdAt = QDateTime::fromString(header.value(1).toString(), Qt::ISODate);
    order.status = header.value(2).toString();
    order.customerName = header.value(3).toString();
    order.accountName = header.value(4).toString();
    order.notes = header.value(5).toString();
    order.sentAt = QDateTime::fromString(header.value(6).toString(), Qt::ISODate);

    QSqlQuery lines(db);
    lines.prepare(QStringLiteral(R"(
        SELECT product_id, product_name, sku, unit, unit_price_pence, quantity
        FROM order_items WHERE order_id = ? ORDER BY id
    )"));
    lines.addBindValue(orderId);
    if (!Database::exec(lines))
        return std::nullopt;

    while (lines.next()) {
        OrderLine line;
        line.productId = lines.value(0).isNull() ? -1 : lines.value(0).toInt();
        line.name = lines.value(1).toString();
        line.sku = lines.value(2).toString();
        line.unit = lines.value(3).toString();
        line.unitPricePence = lines.value(4).toLongLong();
        line.quantity = lines.value(5).toInt();
        order.lines.append(line);
    }

    return order;
}

bool markSent(int orderId, const QDateTime &when)
{
    QSqlQuery q(Database::connection());
    q.prepare(QStringLiteral("UPDATE orders SET status = ?, sent_at = ? WHERE id = ?"));
    q.addBindValue(StatusSent);
    q.addBindValue(when.toString(Qt::ISODate));
    q.addBindValue(orderId);
    return Database::exec(q) && q.numRowsAffected() == 1;
}

} // namespace OrderStore

QString buildOrderShareText(const OrderRecord &order)
{
    auto formatRow = [](const QString &ref, const QString &name, const QString &qty, const QString &price) {
        QString shortName = name;
        if (shortName.size() > 18)
            shortName = shortName.left(16) + QStringLiteral("..");
        return QStringLiteral("%1 %2 %3 %4")
            .arg(ref.left(10), -10)
            .arg(shortName, -18)
            .arg(qty, 3)
            .arg(price, 8)
            .trimmed();
    };

    auto orDash = [](const QString &value) {
        const QString trimmed = value.trimmed();
        return trimmed.isEmpty() ? QStringLiteral("-") : trimmed;
    };

    QStringList lines;
    lines << QStringLiteral("*Wave Ordering - Order #%1*").arg(order.id);
    if (order.createdAt.isValid())
        lines << OrderStore::formatTimestamp(order.createdAt);
    lines << QString();
    lines << QStringLiteral("Customer: %1").arg(orDash(order.customerName));
    lines << QStringLiteral("Account: %1").arg(orDash(order.accountName));
    lines << QString();
    lines << QStringLiteral("```");
    lines << formatRow(QStringLiteral("Ref"), QStringLiteral("Product"), QStringLiteral("Qty"),
                       QStringLiteral("Price"));

    for (const OrderLine &line : order.lines) {
        lines << formatRow(line.sku, line.name, QString::number(line.quantity),
                           Money::formatOrPoa(line.unitPricePence));
    }

    lines << QStringLiteral("```");
    lines << QString();
    if (order.hasPoaItems())
        lines << QStringLiteral("*Total: %1* (plus POA items)").arg(Money::format(order.totalPence()));
    else
        lines << QStringLiteral("*Total: %1*").arg(Money::format(order.totalPence()));

    if (!order.notes.trimmed().isEmpty()) {
        lines << QString();
        lines << QStringLiteral("Notes: %1").arg(order.notes.trimmed());
    }

    return lines.join(QLatin1Char('\n'));
}
