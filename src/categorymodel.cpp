#include "categorymodel.h"

#include "database.h"

#include <QSqlQuery>

CategoryModel::CategoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_categories.append({-1, tr("All")});

    QSqlQuery q(Database::connection());
    if (!Database::exec(q, QStringLiteral("SELECT id, name FROM categories ORDER BY sort_order")))
        return;
    while (q.next())
        m_categories.append({q.value(0).toInt(), q.value(1).toString()});
}

CategoryModel *CategoryModel::create(QQmlEngine *, QJSEngine *jsEngine)
{
    Q_ASSERT_X(s_instance, "CategoryModel::create", "setInstance() must be called before QML loads");
    Q_ASSERT(jsEngine->thread() == s_instance->thread());
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}

int CategoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_categories.size());
}

QVariant CategoryModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
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
