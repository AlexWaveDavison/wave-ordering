#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QString>
#include <QVector>

struct ProductItem
{
    int id = -1;
    int categoryId = -1;
    QString categoryName;
    QString sku;
    QString name;
    QString unit;
    qint64 pricePence = 0; // 0 = price on application
    int quantityInCart = 0;
};

class CartModel;

// Catalogue of products, filterable by category and free-text search.
// Exposed to QML as the `ProductModel` singleton. Basket quantities are kept
// in sync row-by-row, so tapping +/- never resets the list or its scroll
// position.
class ProductModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(int categoryFilter READ categoryFilter WRITE setCategoryFilter NOTIFY categoryFilterChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles
    {
        IdRole = Qt::UserRole + 1,
        CategoryIdRole,
        CategoryNameRole,
        SkuRole,
        NameRole,
        UnitRole,
        PricePenceRole,
        PriceTextRole,
        IsPoaRole,
        QuantityInCartRole,
    };
    Q_ENUM(Roles)

    // No default argument on purpose: a default-constructible type would let
    // QML build its own instance instead of calling create().
    explicit ProductModel(QObject *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int categoryFilter() const { return m_categoryFilter; }
    void setCategoryFilter(int categoryId);

    QString searchText() const { return m_searchText; }
    void setSearchText(const QString &text);

    int count() const { return static_cast<int>(m_products.size()); }

    void setCartModel(CartModel *cart);

    Q_INVOKABLE void refresh();

    static void setInstance(ProductModel *instance) { s_instance = instance; }
    static ProductModel *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

signals:
    void categoryFilterChanged();
    void searchTextChanged();
    void countChanged();

private:
    void reload();
    void onCartQuantityChanged(int productId, int quantity);

    QVector<ProductItem> m_products;
    int m_categoryFilter = -1;
    QString m_searchText;
    CartModel *m_cart = nullptr;

    inline static ProductModel *s_instance = nullptr;
};
