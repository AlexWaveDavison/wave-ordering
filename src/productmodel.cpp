#include "productmodel.h"

#include "cartmodel.h"
#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>

ProductModel::ProductModel(QObject *parent)
    : QAbstractListModel(parent)
{
    reload();
}

int ProductModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_products.size();
}

QVariant ProductModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_products.size())
        return {};

    const ProductItem &p = m_products.at(index.row());
    switch (role) {
    case IdRole:
        return p.id;
    case SkuRole:
        return p.sku;
    case NameRole:
        return p.name;
    case UnitRole:
        return p.unit;
    case PriceRole:
        return p.price;
    case CategoryIdRole:
        return p.categoryId;
    case QuantityInCartRole:
        return p.quantityInCart;
    }
    return {};
}

QHash<int, QByteArray> ProductModel::roleNames() const
{
    return {
        {IdRole, "productId"},
        {SkuRole, "sku"},
        {NameRole, "name"},
        {UnitRole, "unit"},
        {PriceRole, "price"},
        {CategoryIdRole, "categoryId"},
        {QuantityInCartRole, "quantityInCart"},
    };
}

void ProductModel::setCategoryFilter(int categoryId)
{
    if (m_categoryFilter == categoryId)
        return;
    m_categoryFilter = categoryId;
    emit categoryFilterChanged();
    reload();
}

void ProductModel::setSearchText(const QString &text)
{
    if (m_searchText == text)
        return;
    m_searchText = text;
    emit searchTextChanged();
    reload();
}

void ProductModel::setCartModel(CartModel *cart)
{
    if (m_cart == cart)
        return;

    if (m_cart)
        disconnect(m_cart, nullptr, this, nullptr);

    m_cart = cart;

    if (m_cart)
        connect(m_cart, &CartModel::cartChanged, this, &ProductModel::refresh);

    reload();
}

void ProductModel::refresh()
{
    reload();
}

void ProductModel::reload()
{
    beginResetModel();
    m_products.clear();

    QSqlQuery q(QSqlDatabase::database(Database::connectionName()));

    QString sql = QStringLiteral("SELECT id, category_id, sku, name, unit, price "
                                  "FROM products WHERE active = 1");
    QStringList conditions;
    if (m_categoryFilter >= 0)
        conditions << QStringLiteral("category_id = %1").arg(m_categoryFilter);
    if (!m_searchText.isEmpty()) {
        QString escaped = m_searchText;
        escaped.replace(QLatin1Char('\''), QStringLiteral("''"));
        conditions << QStringLiteral("(name LIKE '%%1%' OR sku LIKE '%%1%')").arg(escaped);
    }
    if (!conditions.isEmpty())
        sql += QStringLiteral(" AND ") + conditions.join(QStringLiteral(" AND "));
    sql += QStringLiteral(" ORDER BY name COLLATE NOCASE");

    q.exec(sql);
    while (q.next()) {
        ProductItem item;
        item.id = q.value(0).toInt();
        item.categoryId = q.value(1).toInt();
        item.sku = q.value(2).toString();
        item.name = q.value(3).toString();
        item.unit = q.value(4).toString();
        item.price = q.value(5).toDouble();
        item.quantityInCart = m_cart ? m_cart->quantityForProduct(item.id) : 0;
        m_products.append(item);
    }

    endResetModel();
}
