#pragma once

#include <QAbstractListModel>
#include <QVariantList>
#include <QVector>

struct OrderSummary
{
    int id;
    QString createdAt;
    QString status;
    double total;
    int itemCount;
};

// Past orders, newest first. itemsForOrder() returns the line items for a
// given order so the UI can show a detail view on demand.
class OrderModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles
    {
        IdRole = Qt::UserRole + 1,
        CreatedAtRole,
        StatusRole,
        TotalRole,
        ItemCountRole,
    };

    explicit OrderModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_orders.size(); }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QVariantList itemsForOrder(int orderId) const;

signals:
    void countChanged();

private:
    QVector<OrderSummary> m_orders;
};
