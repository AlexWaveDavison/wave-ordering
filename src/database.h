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

private:
    static void createSchema();
    static void seedIfEmpty();
};
