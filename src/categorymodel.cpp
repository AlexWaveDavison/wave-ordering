#include "categorymodel.h"

#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>

CategoryModel::CategoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_categories.append({-1, QStringLiteral("All")});

    QSqlQuery q(QSqlDatabase::database(Database::connectionName()));
    q.exec(QStringLiteral("SELECT id, name FROM categories ORDER BY sort_order"));
    while (q.next())
        m_categories.append({q.value(0).toInt(), q.value(1).toString()});
}

int CategoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_categories.size();
}

QVariant CategoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_categories.size())
        return {};

    const CategoryItem &item = m_categories.at(index.row());
    switch (role) {
    case IdRole:
        return item.id;
    case NameRole:
        return item.name;
    }
    return {};
}

QHash<int, QByteArray> CategoryModel::roleNames() const
{
    return {
        {IdRole, "categoryId"},
        {NameRole, "name"},
    };
}
