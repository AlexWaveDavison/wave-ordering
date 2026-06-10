#include "ordermodel.h"

#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariantMap>

OrderModel::OrderModel(QObject *parent)
    : QAbstractListModel(parent)
{
    refresh();
}

int OrderModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_orders.size();
}

QVariant OrderModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_orders.size())
        return {};

    const OrderSummary &o = m_orders.at(index.row());
    switch (role) {
    case IdRole:
        return o.id;
    case CreatedAtRole:
        return o.createdAt;
    case StatusRole:
        return o.status;
    case TotalRole:
        return o.total;
    case ItemCountRole:
        return o.itemCount;
    }
    return {};
}

QHash<int, QByteArray> OrderModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {CreatedAtRole, "createdAt"},
        {StatusRole, "status"},
        {TotalRole, "total"},
        {ItemCountRole, "itemCount"},
    };
}

void OrderModel::refresh()
{
    beginResetModel();
    m_orders.clear();

    QSqlQuery q(QSqlDatabase::database(Database::connectionName()));
    q.exec(QStringLiteral(R"(
        SELECT o.id, o.created_at, o.status, o.total,
               (SELECT COALESCE(SUM(quantity), 0) FROM order_items WHERE order_id = o.id)
        FROM orders o
        ORDER BY o.id DESC
    )"));

    while (q.next()) {
        OrderSummary o;
        o.id = q.value(0).toInt();
        o.createdAt = q.value(1).toString();
        o.status = q.value(2).toString();
        o.total = q.value(3).toDouble();
        o.itemCount = q.value(4).toInt();
        m_orders.append(o);
    }

    endResetModel();
    emit countChanged();
}

QVariantList OrderModel::itemsForOrder(int orderId) const
{
    QVariantList result;

    QSqlQuery q(QSqlDatabase::database(Database::connectionName()));
    q.prepare(QStringLiteral(
        "SELECT product_name, sku, unit, unit_price, quantity, line_total "
        "FROM order_items WHERE order_id = ? ORDER BY id"));
    q.addBindValue(orderId);
    q.exec();

    while (q.next()) {
        QVariantMap item;
        item[QStringLiteral("name")] = q.value(0).toString();
        item[QStringLiteral("sku")] = q.value(1).toString();
        item[QStringLiteral("unit")] = q.value(2).toString();
        item[QStringLiteral("unitPrice")] = q.value(3).toDouble();
        item[QStringLiteral("quantity")] = q.value(4).toInt();
        item[QStringLiteral("lineTotal")] = q.value(5).toDouble();
        result.append(item);
    }

    return result;
}
