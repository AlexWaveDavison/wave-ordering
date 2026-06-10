#pragma once

#include <QAbstractListModel>
#include <QVector>

struct CartLine
{
    int productId;
    QString sku;
    QString name;
    QString unit;
    double price;
    int quantity;
};

// The current basket. Persisted to the cart_items table so it survives
// app restarts.
class CartModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(double total READ total NOTIFY cartChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY cartChanged)

public:
    enum Roles
    {
        ProductIdRole = Qt::UserRole + 1,
        SkuRole,
        NameRole,
        UnitRole,
        PriceRole,
        QuantityRole,
        LineTotalRole,
    };

    explicit CartModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    double total() const;
    int itemCount() const;

    int quantityForProduct(int productId) const;

    Q_INVOKABLE void addProduct(int productId, int quantity = 1);
    Q_INVOKABLE void setQuantity(int productId, int quantity);
    Q_INVOKABLE void increment(int productId);
    Q_INVOKABLE void decrement(int productId);
    Q_INVOKABLE void removeProduct(int productId);
    Q_INVOKABLE void clear();

signals:
    void cartChanged();

private:
    void load();
    void persistLine(const CartLine &line);
    void removeLine(int productId);
    int indexOfProduct(int productId) const;

    QVector<CartLine> m_lines;
};
