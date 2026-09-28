#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QVector>

struct CartLine
{
    int productId = -1;
    QString sku;
    QString name;
    QString unit;
    qint64 pricePence = 0; // 0 = price on application
    int quantity = 0;

    qint64 lineTotalPence() const { return pricePence * quantity; }
};

// The current basket. Persisted to the cart_items table so it survives app
// restarts. Exposed to QML as the `CartModel` singleton.
class CartModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(qint64 totalPence READ totalPence NOTIFY cartChanged)
    Q_PROPERTY(QString totalText READ totalText NOTIFY cartChanged)
    Q_PROPERTY(bool hasPoaItems READ hasPoaItems NOTIFY cartChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY cartChanged)
    Q_PROPERTY(int lineCount READ lineCount NOTIFY cartChanged)

public:
    enum Roles
    {
        ProductIdRole = Qt::UserRole + 1,
        SkuRole,
        NameRole,
        UnitRole,
        PricePenceRole,
        PriceTextRole,
        IsPoaRole,
        QuantityRole,
        LineTotalTextRole,
    };
    Q_ENUM(Roles)

    // No default argument on purpose: a default-constructible type would let
    // QML build its own instance instead of calling create().
    explicit CartModel(QObject *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    const QVector<CartLine> &lines() const { return m_lines; }

    qint64 totalPence() const;
    QString totalText() const;
    bool hasPoaItems() const;
    int itemCount() const;
    int lineCount() const { return static_cast<int>(m_lines.size()); }

    Q_INVOKABLE int quantityForProduct(int productId) const;

    Q_INVOKABLE void addProduct(int productId, int quantity = 1);
    Q_INVOKABLE void setQuantity(int productId, int quantity);
    Q_INVOKABLE void increment(int productId);
    Q_INVOKABLE void decrement(int productId);
    Q_INVOKABLE void removeProduct(int productId);
    Q_INVOKABLE void clear();

    // QML singleton plumbing: main() creates the instance, QML reuses it.
    static void setInstance(CartModel *instance) { s_instance = instance; }
    static CartModel *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

signals:
    void cartChanged();
    // Emitted for a single product whose quantity changed (including removal),
    // so the catalogue can update one row instead of reloading everything.
    void quantityChanged(int productId, int quantity);

private:
    void load();
    bool persistLine(const CartLine &line);
    bool removeLine(int productId);
    int indexOfProduct(int productId) const;

    QVector<CartLine> m_lines;

    inline static CartModel *s_instance = nullptr;
};
