#include "productmodel.h"

#include "cartmodel.h"
#include "database.h"
#include "money.h"

#include <QSqlQuery>
#include <QStringList>

namespace {

// Escapes LIKE wildcards so a search for "50%" or "a_b" matches literally.
QString likePattern(const QString &text)
{
    QString escaped = text;
    escaped.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
    escaped.replace(QLatin1Char('%'), QStringLiteral("\\%"));
    escaped.replace(QLatin1Char('_'), QStringLiteral("\\_"));
    return QLatin1Char('%') + escaped + QLatin1Char('%');
}

} // namespace

ProductModel::ProductModel(QObject *parent)
    : QAbstractListModel(parent)
{
    reload();
}

ProductModel *ProductModel::create(QQmlEngine *, QJSEngine *jsEngine)
{
    Q_ASSERT_X(s_instance, "ProductModel::create", "setInstance() must be called before QML loads");
    Q_ASSERT(jsEngine->thread() == s_instance->thread());
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}

int ProductModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_products.size());
}

QVariant ProductModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return {};

    const ProductItem &p = m_products.at(index.row());
    switch (role) {
    case IdRole:
        return p.id;
    case CategoryIdRole:
        return p.categoryId;
    case CategoryNameRole:
        return p.categoryName;
    case SkuRole:
        return p.sku;
    case NameRole:
        return p.name;
    case UnitRole:
        return p.unit;
    case PricePenceRole:
        return p.pricePence;
    case PriceTextRole:
        return Money::formatOrPoa(p.pricePence);
    case IsPoaRole:
        return p.pricePence == 0;
    case QuantityInCartRole:
        return p.quantityInCart;
    }
    return {};
}

QHash<int, QByteArray> ProductModel::roleNames() const
{
    return {
        {IdRole, "productId"},
        {CategoryIdRole, "categoryId"},
        {CategoryNameRole, "categoryName"},
        {SkuRole, "sku"},
        {NameRole, "name"},
        {UnitRole, "unit"},
        {PricePenceRole, "pricePence"},
        {PriceTextRole, "priceText"},
        {IsPoaRole, "isPoa"},
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
    const QString trimmed = text.trimmed();
    if (m_searchText == trimmed)
        return;
    m_searchText = trimmed;
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
        connect(m_cart, &CartModel::quantityChanged, this, &ProductModel::onCartQuantityChanged);

    reload();
}

void ProductModel::refresh()
{
    reload();
}

void ProductModel::onCartQuantityChanged(int productId, int quantity)
{
    for (int row = 0; row < m_products.size(); ++row) {
        if (m_products.at(row).id != productId)
            continue;
        if (m_products.at(row).quantityInCart != quantity) {
            m_products[row].quantityInCart = quantity;
            const QModelIndex idx = index(row);
            emit dataChanged(idx, idx, {QuantityInCartRole});
        }
        return;
    }
}

void ProductModel::reload()
{
    QString sql = QStringLiteral(R"(
        SELECT p.id, p.category_id, c.name, p.sku, p.name, p.unit, p.price_pence
        FROM products p
        JOIN categories c ON c.id = p.category_id
        WHERE p.active = 1
    )");
    if (m_categoryFilter >= 0)
        sql += QStringLiteral(" AND p.category_id = ?");
    if (!m_searchText.isEmpty())
        sql += QStringLiteral(" AND (p.name LIKE ? ESCAPE '\\' OR p.sku LIKE ? ESCAPE '\\')");
    sql += QStringLiteral(" ORDER BY p.name COLLATE NOCASE");

    QSqlQuery q(Database::connection());
    q.prepare(sql);
    if (m_categoryFilter >= 0)
        q.addBindValue(m_categoryFilter);
    if (!m_searchText.isEmpty()) {
        const QString pattern = likePattern(m_searchText);
        q.addBindValue(pattern);
        q.addBindValue(pattern);
    }

    const int previousCount = count();
    beginResetModel();
    m_products.clear();

    if (Database::exec(q)) {
        while (q.next()) {
            ProductItem item;
            item.id = q.value(0).toInt();
            item.categoryId = q.value(1).toInt();
            item.categoryName = q.value(2).toString();
            item.sku = q.value(3).toString();
            item.name = q.value(4).toString();
            item.unit = q.value(5).toString();
            item.pricePence = q.value(6).toLongLong();
            item.quantityInCart = m_cart ? m_cart->quantityForProduct(item.id) : 0;
            m_products.append(item);
        }
    }

    endResetModel();
    if (count() != previousCount)
        emit countChanged();
}
