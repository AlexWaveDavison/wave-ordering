#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QVariantMap>
#include <QVector>

struct OrderSummary
{
    int id = -1;
    QString createdAtText;
    QString status;
    QString customerName;
    qint64 totalPence = 0;
    int itemCount = 0;
};

// Past orders, newest first. Exposed to QML as the `OrderModel` singleton.
// details() returns everything the order detail sheet needs in one call.
class OrderModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY countChanged)

public:
    enum Roles
    {
        OrderIdRole = Qt::UserRole + 1,
        CreatedAtTextRole,
        StatusRole,
        StatusTextRole,
        IsSentRole,
        CustomerNameRole,
        TotalTextRole,
        ItemCountRole,
    };
    Q_ENUM(Roles)

    // No default argument on purpose: a default-constructible type would let
    // QML build its own instance instead of calling create().
    explicit OrderModel(QObject *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return static_cast<int>(m_orders.size()); }
    int pendingCount() const;

    Q_INVOKABLE void refresh();

    // Keys: orderId, createdAtText, sentAtText, status, statusText, isSent,
    // customerName, accountName, notes, totalText, hasPoaItems, itemCount,
    // canReorder, and items (list of maps: name, sku, unit, quantity,
    // unitPriceText, lineTotalText, isPoa, available). Empty map if missing.
    Q_INVOKABLE QVariantMap details(int orderId) const;

    static void setInstance(OrderModel *instance) { s_instance = instance; }
    static OrderModel *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

signals:
    void countChanged();

private:
    QVector<OrderSummary> m_orders;

    inline static OrderModel *s_instance = nullptr;
};
