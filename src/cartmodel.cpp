#include "cartmodel.h"

#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>

CartModel::CartModel(QObject *parent)
    : QAbstractListModel(parent)
{
    load();
}

int CartModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_lines.size();
}

QVariant CartModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_lines.size())
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
    case PriceRole:
        return line.price;
    case QuantityRole:
        return line.quantity;
    case LineTotalRole:
        return line.price * line.quantity;
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
        {PriceRole, "price"},
        {QuantityRole, "quantity"},
        {LineTotalRole, "lineTotal"},
    };
}

double CartModel::total() const
{
    double sum = 0;
    for (const CartLine &line : m_lines)
        sum += line.price * line.quantity;
    return sum;
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

    QSqlQuery q(QSqlDatabase::database(Database::connectionName()));
    q.prepare(QStringLiteral("SELECT sku, name, unit, price FROM products WHERE id = ?"));
    q.addBindValue(productId);
    q.exec();
    if (!q.next())
        return;

    CartLine line;
    line.productId = productId;
    line.sku = q.value(0).toString();
    line.name = q.value(1).toString();
    line.unit = q.value(2).toString();
    line.price = q.value(3).toDouble();
    line.quantity = quantity;

    beginInsertRows(QModelIndex(), m_lines.size(), m_lines.size());
    m_lines.append(line);
    endInsertRows();

    persistLine(line);
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

    m_lines[idx].quantity = quantity;
    persistLine(m_lines[idx]);

    const QModelIndex modelIndex = index(idx);
    emit dataChanged(modelIndex, modelIndex, {QuantityRole, LineTotalRole});
    emit cartChanged();
}

void CartModel::increment(int productId)
{
    const int idx = indexOfProduct(productId);
    if (idx < 0) {
        addProduct(productId, 1);
        return;
    }
    setQuantity(productId, m_lines.at(idx).quantity + 1);
}

void CartModel::decrement(int productId)
{
    const int idx = indexOfProduct(productId);
    if (idx < 0)
        return;
    setQuantity(productId, m_lines.at(idx).quantity - 1);
}

void CartModel::removeProduct(int productId)
{
    const int idx = indexOfProduct(productId);
    if (idx < 0)
        return;

    beginRemoveRows(QModelIndex(), idx, idx);
    m_lines.removeAt(idx);
    endRemoveRows();

    removeLine(productId);
    emit cartChanged();
}

void CartModel::clear()
{
    if (m_lines.isEmpty())
        return;

    beginResetModel();
    m_lines.clear();
    endResetModel();

    QSqlQuery q(QSqlDatabase::database(Database::connectionName()));
    q.exec(QStringLiteral("DELETE FROM cart_items"));
    emit cartChanged();
}

void CartModel::load()
{
    QSqlQuery q(QSqlDatabase::database(Database::connectionName()));
    q.exec(QStringLiteral(R"(
        SELECT p.id, p.sku, p.name, p.unit, p.price, c.quantity
        FROM cart_items c
        JOIN products p ON p.id = c.product_id
        ORDER BY p.name COLLATE NOCASE
    )"));

    while (q.next()) {
        CartLine line;
        line.productId = q.value(0).toInt();
        line.sku = q.value(1).toString();
        line.name = q.value(2).toString();
        line.unit = q.value(3).toString();
        line.price = q.value(4).toDouble();
        line.quantity = q.value(5).toInt();
        m_lines.append(line);
    }
}

void CartModel::persistLine(const CartLine &line)
{
    QSqlQuery q(QSqlDatabase::database(Database::connectionName()));
    q.prepare(QStringLiteral(R"(
        INSERT INTO cart_items (product_id, quantity) VALUES (?, ?)
        ON CONFLICT(product_id) DO UPDATE SET quantity = excluded.quantity
    )"));
    q.addBindValue(line.productId);
    q.addBindValue(line.quantity);
    q.exec();
}

void CartModel::removeLine(int productId)
{
    QSqlQuery q(QSqlDatabase::database(Database::connectionName()));
    q.prepare(QStringLiteral("DELETE FROM cart_items WHERE product_id = ?"));
    q.addBindValue(productId);
    q.exec();
}

int CartModel::indexOfProduct(int productId) const
{
    for (int i = 0; i < m_lines.size(); ++i) {
        if (m_lines.at(i).productId == productId)
            return i;
    }
    return -1;
}
