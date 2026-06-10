#pragma once

#include <QString>

// Owns the local SQLite connection used by the app. Tables: categories,
// products, cart_items, orders, order_items. On first run the schema is
// created and the catalog is seeded with the starting product list.
class Database
{
public:
    static bool initialize();
    static QString connectionName();

    // Small key/value store for app preferences (e.g. customer/account name).
    static QString settingValue(const QString &key, const QString &defaultValue = QString());
    static void setSettingValue(const QString &key, const QString &value);

private:
    static void createSchema();
    static void seedIfEmpty();
};
