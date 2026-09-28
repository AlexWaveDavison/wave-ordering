#include "ordermodel.h"

#include "database.h"
#include "money.h"
#include "orderstore.h"

#include <QSet>
#include <QSqlQuery>
#include <QVariantList>

OrderModel::OrderModel(QObject *parent)
    : QAbstractListModel(parent)
{
    refresh();
}

OrderModel *OrderModel::create(QQmlEngine *, QJSEngine *jsEngine)
{
    Q_ASSERT_X(s_instance, "OrderModel::create", "setInstance() must be called before QML loads");
    Q_ASSERT(jsEngine->thread() == s_instance->thread());
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}

int OrderModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_orders.size());
}

QVariant OrderModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return {};

    const OrderSummary &o = m_orders.at(index.row());
    switch (role) {
    case OrderIdRole:
        return o.id;
    case CreatedAtTextRole:
        return o.createdAtText;
    case StatusRole:
        return o.status;
    case StatusTextRole:
        return OrderStore::statusLabel(o.status);
    case IsSentRole:
        return o.status == OrderStore::StatusSent;
    case CustomerNameRole:
        return o.customerName;
    case TotalTextRole:
        return Money::format(o.totalPence);
    case ItemCountRole:
        return o.itemCount;
    }
    return {};
}

QHash<int, QByteArray> OrderModel::roleNames() const
{
    return {
        {OrderIdRole, "orderId"},
        {CreatedAtTextRole, "createdAtText"},
        {StatusRole, "status"},
        {StatusTextRole, "statusText"},
        {IsSentRole, "isSent"},
        {CustomerNameRole, "customerName"},
        {TotalTextRole, "totalText"},
        {ItemCountRole, "itemCount"},
    };
}

int OrderModel::pendingCount() const
{
    int pending = 0;
    for (const OrderSummary &o : m_orders) {
        if (o.status != OrderStore::StatusSent)
            ++pending;
    }
    return pending;
}

void OrderModel::refresh()
{
    QSqlQuery q(Database::connection());
    const bool ok = Database::exec(q, QStringLiteral(R"(
        SELECT o.id, o.created_at, o.status, o.customer_name, o.total_pence,
               (SELECT COALESCE(SUM(quantity), 0) FROM order_items WHERE order_id = o.id)
        FROM orders o
        ORDER BY o.id DESC
    )"));

    beginResetModel();
    m_orders.clear();
    while (ok && q.next()) {
        OrderSummary o;
        o.id = q.value(0).toInt();
        o.createdAtText = OrderStore::formatTimestamp(QDateTime::fromString(q.value(1).toString(), Qt::ISODate));
        o.status = q.value(2).toString();
        o.customerName = q.value(3).toString();
        o.totalPence = q.value(4).toLongLong();
        o.itemCount = q.value(5).toInt();
        m_orders.append(o);
    }
    endResetModel();
    emit countChanged();
}

QVariantMap OrderModel::details(int orderId) const
{
    const std::optional<OrderRecord> order = OrderStore::load(orderId);
    if (!order)
        return {};

    // Which of the ordered products can still be added back to the basket.
    QSet<int> activeProductIds;
    {
        QSqlQuery q(Database::connection());
        if (Database::exec(q, QStringLiteral("SELECT id FROM products WHERE active = 1"))) {
            while (q.next())
                activeProductIds.insert(q.value(0).toInt());
        }
    }

    bool canReorder = false;
    QVariantList items;
    for (const OrderLine &line : order->lines) {
        const bool available = line.productId >= 0 && activeProductIds.contains(line.productId);
        canReorder = canReorder || available;
        items.append(QVariantMap{
            {QStringLiteral("name"), line.name},
            {QStringLiteral("sku"), line.sku},
            {QStringLiteral("unit"), line.unit},
            {QStringLiteral("quantity"), line.quantity},
            {QStringLiteral("unitPriceText"), Money::formatOrPoa(line.unitPricePence)},
            {QStringLiteral("lineTotalText"), Money::formatOrPoa(line.lineTotalPence())},
            {QStringLiteral("isPoa"), line.isPoa()},
            {QStringLiteral("available"), available},
        });
    }

    return {
        {QStringLiteral("orderId"), order->id},
        {QStringLiteral("createdAtText"), OrderStore::formatTimestamp(order->createdAt)},
        {QStringLiteral("sentAtText"), OrderStore::formatTimestamp(order->sentAt)},
        {QStringLiteral("status"), order->status},
        {QStringLiteral("statusText"), OrderStore::statusLabel(order->status)},
        {QStringLiteral("isSent"), order->isSent()},
        {QStringLiteral("customerName"), order->customerName},
        {QStringLiteral("accountName"), order->accountName},
        {QStringLiteral("notes"), order->notes},
        {QStringLiteral("totalText"), Money::format(order->totalPence())},
        {QStringLiteral("hasPoaItems"), order->hasPoaItems()},
        {QStringLiteral("itemCount"), order->itemCount()},
        {QStringLiteral("canReorder"), canReorder},
        {QStringLiteral("items"), items},
    };
}
