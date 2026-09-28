#include "cartmodel.h"

#include "database.h"
#include "money.h"

#include <QSqlQuery>

CartModel::CartModel(QObject *parent)
    : QAbstractListModel(parent)
{
    load();
}

CartModel *CartModel::create(QQmlEngine *, QJSEngine *jsEngine)
{
    Q_ASSERT_X(s_instance, "CartModel::create", "setInstance() must be called before QML loads");
    Q_ASSERT(jsEngine->thread() == s_instance->thread());
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}

int CartModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_lines.size());
}

QVariant CartModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return {};

    const CartLine &line = m_lines.at(index.row());
    switch (role) {
    case ProductIdRole:
        return line.productId;
    case SkuRole:
        return line.sku;
    case NameRole:
        return line.name;
    case UnitRole:
        return line.unit;
    case PricePenceRole:
        return line.pricePence;
    case PriceTextRole:
        return Money::formatOrPoa(line.pricePence);
    case IsPoaRole:
        return line.pricePence == 0;
    case QuantityRole:
        return line.quantity;
    case LineTotalTextRole:
        return Money::formatOrPoa(line.lineTotalPence());
    }
    return {};
}

QHash<int, QByteArray> CartModel::roleNames() const
{
    return {
        {ProductIdRole, "productId"},
        {SkuRole, "sku"},
        {NameRole, "name"},
        {UnitRole, "unit"},
        {PricePenceRole, "pricePence"},
        {PriceTextRole, "priceText"},
        {IsPoaRole, "isPoa"},
        {QuantityRole, "quantity"},
        {LineTotalTextRole, "lineTotalText"},
    };
}

qint64 CartModel::totalPence() const
{
    qint64 sum = 0;
    for (const CartLine &line : m_lines)
        sum += line.lineTotalPence();
    return sum;
}

QString CartModel::totalText() const
{
    if (!hasPoaItems())
        return Money::format(totalPence());
    if (totalPence() == 0)
        return QStringLiteral("POA");
    return tr("%1 + POA").arg(Money::format(totalPence()));
}

bool CartModel::hasPoaItems() const
{
    for (const CartLine &line : m_lines) {
        if (line.pricePence == 0)
            return true;
    }
    return false;
}

int CartModel::itemCount() const
{
    int sum = 0;
    for (const CartLine &line : m_lines)
        sum += line.quantity;
    return sum;
}

int CartModel::quantityForProduct(int productId) const
{
    const int idx = indexOfProduct(productId);
    return idx >= 0 ? m_lines.at(idx).quantity : 0;
}

void CartModel::addProduct(int productId, int quantity)
{
    if (quantity <= 0)
        return;

    const int idx = indexOfProduct(productId);
    if (idx >= 0) {
        setQuantity(productId, m_lines.at(idx).quantity + quantity);
        return;
    }

    QSqlQuery q(Database::connection());
    q.prepare(QStringLiteral("SELECT sku, name, unit, price_pence FROM products WHERE id = ? AND active = 1"));
    q.addBindValue(productId);
    if (!Database::exec(q) || !q.next())
        return;

    CartLine line;
    line.productId = productId;
    line.sku = q.value(0).toString();
    line.name = q.value(1).toString();
    line.unit = q.value(2).toString();
    line.pricePence = q.value(3).toLongLong();
    line.quantity = quantity;

    if (!persistLine(line))
        return;

    const int row = static_cast<int>(m_lines.size());
    beginInsertRows(QModelIndex(), row, row);
    m_lines.append(line);
    endInsertRows();

    emit quantityChanged(productId, quantity);
    emit cartChanged();
}

void CartModel::setQuantity(int productId, int quantity)
{
    const int idx = indexOfProduct(productId);

    if (quantity <= 0) {
        if (idx >= 0)
            removeProduct(productId);
        return;
    }

    if (idx < 0) {
        addProduct(productId, quantity);
        return;
    }

    if (m_lines.at(idx).quantity == quantity)
        return;

    CartLine updated = m_lines.at(idx);
    updated.quantity = quantity;
    if (!persistLine(updated))
        return;
    m_lines[idx] = updated;

    const QModelIndex modelIndex = index(idx);
    emit dataChanged(modelIndex, modelIndex, {QuantityRole, LineTotalTextRole});
    emit quantityChanged(productId, quantity);
    emit cartChanged();
}

void CartModel::increment(int productId)
{
    setQuantity(productId, quantityForProduct(productId) + 1);
}

void CartModel::decrement(int productId)
{
    const int current = quantityForProduct(productId);
    if (current > 0)
        setQuantity(productId, current - 1);
}

void CartModel::removeProduct(int productId)
{
    const int idx = indexOfProduct(productId);
    if (idx < 0 || !removeLine(productId))
        return;

    beginRemoveRows(QModelIndex(), idx, idx);
    m_lines.removeAt(idx);
    endRemoveRows();

    emit quantityChanged(productId, 0);
    emit cartChanged();
}

void CartModel::clear()
{
    if (m_lines.isEmpty())
        return;

    QSqlQuery q(Database::connection());
    if (!Database::exec(q, QStringLiteral("DELETE FROM cart_items")))
        return;

    const QVector<CartLine> removed = m_lines;
    beginResetModel();
    m_lines.clear();
    endResetModel();

    for (const CartLine &line : removed)
        emit quantityChanged(line.productId, 0);
    emit cartChanged();
}

void CartModel::load()
{
    QSqlQuery q(Database::connection());
    if (!Database::exec(q, QStringLiteral(R"(
        SELECT p.id, p.sku, p.name, p.unit, p.price_pence, c.quantity
        FROM cart_items c
        JOIN products p ON p.id = c.product_id
        WHERE p.active = 1
        ORDER BY p.name COLLATE NOCASE
    )")))
        return;

    while (q.next()) {
        CartLine line;
        line.productId = q.value(0).toInt();
        line.sku = q.value(1).toString();
        line.name = q.value(2).toString();
        line.unit = q.value(3).toString();
        line.pricePence = q.value(4).toLongLong();
        line.quantity = q.value(5).toInt();
        m_lines.append(line);
    }
}

bool CartModel::persistLine(const CartLine &line)
{
    QSqlQuery q(Database::connection());
    q.prepare(QStringLiteral(R"(
        INSERT INTO cart_items (product_id, quantity) VALUES (?, ?)
        ON CONFLICT(product_id) DO UPDATE SET quantity = excluded.quantity
    )"));
    q.addBindValue(line.productId);
    q.addBindValue(line.quantity);
    return Database::exec(q);
}

bool CartModel::removeLine(int productId)
{
    QSqlQuery q(Database::connection());
    q.prepare(QStringLiteral("DELETE FROM cart_items WHERE product_id = ?"));
    q.addBindValue(productId);
    return Database::exec(q);
}

int CartModel::indexOfProduct(int productId) const
{
    for (int i = 0; i < m_lines.size(); ++i) {
        if (m_lines.at(i).productId == productId)
            return i;
    }
    return -1;
}
