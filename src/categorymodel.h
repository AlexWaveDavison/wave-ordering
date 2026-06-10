#pragma once

#include <QAbstractListModel>
#include <QVector>

struct CategoryItem
{
    int id;
    QString name;
};

// Read-only list of product categories, plus a synthetic "All" entry
// (id == -1) used to clear the catalog filter.
class CategoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        IdRole = Qt::UserRole + 1,
        NameRole,
    };

    explicit CategoryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<CategoryItem> m_categories;
};
