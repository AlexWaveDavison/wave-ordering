#pragma once

#include <QLoggingCategory>
#include <QSqlDatabase>
#include <QString>

class QSqlQuery;

Q_DECLARE_LOGGING_CATEGORY(lcDatabase)

// Owns the local SQLite connection used by the app.
//
// Tables: categories, products, cart_items, orders, order_items and
// app_settings. The schema is versioned with SQLite's `user_version` pragma:
// fresh installs get the current schema, older databases are migrated in
// place (see migrate() in database.cpp). On first run the catalogue is seeded
// with sample data.
class Database
{
public:
    static constexpr int SchemaVersion = 2;

    // Opens (creating if needed) the database. With no path, it lives in the
    // platform's app-data directory; tests pass a temporary file instead.
    static bool initialize(const QString &path = QString());

    // Closes and removes the connection. Any QSqlQuery objects must be gone.
    static void close();

    static QSqlDatabase connection();
    static QString connectionName();
    static QString defaultPath();

    // Runs a prepared (or prepares and runs an ad-hoc) query, logging the SQL
    // error on failure. Returns whether the query succeeded.
    static bool exec(QSqlQuery &query);
    static bool exec(QSqlQuery &query, const QString &sql);

    // Small key/value store for app preferences (e.g. customer/account name).
    static QString settingValue(const QString &key, const QString &defaultValue = QString());
    static bool setSettingValue(const QString &key, const QString &value);

private:
    static bool createSchema();
    static bool migrate(int fromVersion);
    static bool seedIfEmpty();
};
