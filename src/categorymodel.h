#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QVector>

struct CategoryItem
{
    int id;
    QString name;
};

// Read-only list of product categories, plus a synthetic "All" entry
// (id == -1) used to clear the catalogue filter. Exposed to QML as the
// `CategoryModel` singleton.
class CategoryModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    enum Roles
    {
        IdRole = Qt::UserRole + 1,
        NameRole,
    };

    // No default argument on purpose: a default-constructible type would let
    // QML build its own instance instead of calling create().
    explicit CategoryModel(QObject *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    static void setInstance(CategoryModel *instance) { s_instance = instance; }
    static CategoryModel *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

private:
    QVector<CategoryItem> m_categories;

    inline static CategoryModel *s_instance = nullptr;
};
