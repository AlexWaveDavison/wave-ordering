#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct ProductItem
{
    int id;
    int categoryId;
    QString sku;
    QString name;
    QString unit;
    double price;
    int quantityInCart;
};

class CartModel;

// Catalog of products, filterable by category and free-text search.
// Re-queries the database whenever the filters change or the cart changes
// (so each row's quantityInCart stays in sync).
class ProductModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int categoryFilter READ categoryFilter WRITE setCategoryFilter NOTIFY categoryFilterChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    enum Roles
    {
        IdRole = Qt::UserRole + 1,
        SkuRole,
        NameRole,
        UnitRole,
        PriceRole,
        CategoryIdRole,
        QuantityInCartRole,
    };

    explicit ProductModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int categoryFilter() const { return m_categoryFilter; }
    void setCategoryFilter(int categoryId);

    QString searchText() const { return m_searchText; }
    void setSearchText(const QString &text);

    void setCartModel(CartModel *cart);

    Q_INVOKABLE void refresh();

signals:
    void categoryFilterChanged();
    void searchTextChanged();

private:
    void reload();

    QVector<ProductItem> m_products;
    int m_categoryFilter = -1;
    QString m_searchText;
    CartModel *m_cart = nullptr;
};
