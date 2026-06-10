#include "database.h"

#include <QDir>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDebug>

namespace {

const QString kConnectionName = QStringLiteral("wave_ordering_db");

// Category headings for a typical foodservice wholesale catalog.
const char *const kCategories[] = {
    "Cheese",
    "Flour & Bakery Products",
    "Pizza Toppings",
    "Meat",
    "Chicken",
    "Seafood",
    "Potato Products",
    "Packaging",
    "Cleaning Materials",
    "Accessories",
    "Beverages",
    "Herbs & Spices",
};

struct SeedProduct
{
    const char *category;
    const char *sku;
    const char *name;
    const char *unit;
    double price;
};

// Sample catalog used to demonstrate the app. All products, SKUs and prices
// below are fictional placeholder data.
const SeedProduct kSeedProducts[] = {
    {"Cheese", "WV-CHE-001", "Mozzarella Block", "2.5kg", 14.50},
    {"Cheese", "WV-CHE-002", "Mature Cheddar Block", "2.5kg", 15.20},
    {"Cheese", "WV-CHE-003", "Grated Mozzarella & Cheddar Blend", "6x2kg", 42.00},
    {"Cheese", "WV-CHE-004", "Halloumi Block", "1kg", 6.80},

    {"Flour & Bakery Products", "WV-FLR-001", "Strong White Bread Flour", "16kg", 12.40},
    {"Flour & Bakery Products", "WV-FLR-002", "00 Pizza Flour", "25kg", 18.90},
    {"Flour & Bakery Products", "WV-FLR-003", "Pre-Baked Pizza Bases 12in", "20", 24.00},
    {"Flour & Bakery Products", "WV-FLR-004", "Garlic Bread Slices", "40x60g", 15.60},

    {"Pizza Toppings", "WV-PZA-001", "Pepperoni Slices", "2.5kg", 13.20},
    {"Pizza Toppings", "WV-PZA-002", "Sliced Mushrooms in Brine", "6x2.5kg", 19.80},
    {"Pizza Toppings", "WV-PZA-003", "Sweetcorn", "6x2.5kg", 14.40},
    {"Pizza Toppings", "WV-PZA-004", "Jalapeno Slices", "6x2kg", 16.50},

    {"Meat", "WV-MET-001", "Beef Burgers 4oz", "72x113g", 38.00},
    {"Meat", "WV-MET-002", "Diced Beef Brisket", "2.5kg", 22.00},
    {"Meat", "WV-MET-003", "Italian Sausages", "5kg", 19.50},
    {"Meat", "WV-MET-004", "Smoked Bacon Rashers", "2.5kg", 16.80},

    {"Chicken", "WV-CHK-001", "Chicken Breast Fillets", "5kg", 24.50},
    {"Chicken", "WV-CHK-002", "Southern Fried Chicken Strips", "2.5kg", 14.90},
    {"Chicken", "WV-CHK-003", "Chicken Wings", "10kg", 28.00},
    {"Chicken", "WV-CHK-004", "Chicken Doner Kebab", "2x2.5kg", 21.00},

    {"Seafood", "WV-SEA-001", "Battered Cod Fillets", "10x140g", 19.00},
    {"Seafood", "WV-SEA-002", "Breaded Scampi", "1kg", 9.80},
    {"Seafood", "WV-SEA-003", "Calamari Rings", "1kg", 8.50},
    {"Seafood", "WV-SEA-004", "Prawn Crackers", "6x227g", 7.20},

    {"Potato Products", "WV-POT-001", "Straight Cut Chips", "6x2.5kg", 21.00},
    {"Potato Products", "WV-POT-002", "Curly Fries", "6x2.27kg", 23.50},
    {"Potato Products", "WV-POT-003", "Potato Wedges", "6x2.5kg", 22.00},
    {"Potato Products", "WV-POT-004", "Hash Brown Triangles", "100x56g", 18.40},

    {"Packaging", "WV-PKG-001", "Pizza Boxes 12in", "100", 16.00},
    {"Packaging", "WV-PKG-002", "Foil Containers, Medium", "500", 28.00},
    {"Packaging", "WV-PKG-003", "Chip Scoop Trays", "1000", 12.50},
    {"Packaging", "WV-PKG-004", "Brown Paper Bags", "500", 9.00},

    {"Cleaning Materials", "WV-CLN-001", "Multi-Surface Sanitiser", "5L", 8.40},
    {"Cleaning Materials", "WV-CLN-002", "Heavy Duty Degreaser", "5L", 9.90},
    {"Cleaning Materials", "WV-CLN-003", "Catering Roll", "6", 11.20},
    {"Cleaning Materials", "WV-CLN-004", "Washing Up Liquid", "5L", 6.50},

    {"Accessories", "WV-ACC-001", "Disposable Gloves, Medium", "100", 4.50},
    {"Accessories", "WV-ACC-002", "Aluminium Foil Roll", "450m", 22.00},
    {"Accessories", "WV-ACC-003", "Cling Film Roll", "300m", 8.90},
    {"Accessories", "WV-ACC-004", "Wooden Cutlery Sets", "250", 10.50},

    {"Beverages", "WV-BEV-001", "Cola, 330ml Cans", "24", 9.50},
    {"Beverages", "WV-BEV-002", "Sparkling Water, 330ml", "24", 8.80},
    {"Beverages", "WV-BEV-003", "Orange Juice Carton", "12x1L", 14.00},
    {"Beverages", "WV-BEV-004", "Still Water, 500ml", "24", 6.00},

    {"Herbs & Spices", "WV-HRB-001", "Dried Oregano", "500g", 5.20},
    {"Herbs & Spices", "WV-HRB-002", "Chilli Flakes", "500g", 5.80},
    {"Herbs & Spices", "WV-HRB-003", "Garlic Granules", "1kg", 6.40},
    {"Herbs & Spices", "WV-HRB-004", "Mixed Herbs", "500g", 5.00},
};

} // namespace

bool Database::initialize()
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    const QString dbPath = dataDir + QStringLiteral("/waveordering.sqlite");

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), kConnectionName);
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qWarning() << "Failed to open database at" << dbPath << ":" << db.lastError().text();
        return false;
    }

    QSqlQuery(db).exec(QStringLiteral("PRAGMA foreign_keys = ON"));

    createSchema();
    seedIfEmpty();
    return true;
}

QString Database::connectionName()
{
    return kConnectionName;
}

QString Database::settingValue(const QString &key, const QString &defaultValue)
{
    QSqlQuery q(QSqlDatabase::database(kConnectionName));
    q.prepare(QStringLiteral("SELECT value FROM app_settings WHERE key = ?"));
    q.addBindValue(key);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return defaultValue;
}

void Database::setSettingValue(const QString &key, const QString &value)
{
    QSqlQuery q(QSqlDatabase::database(kConnectionName));
    q.prepare(QStringLiteral(R"(
        INSERT INTO app_settings (key, value) VALUES (?, ?)
        ON CONFLICT(key) DO UPDATE SET value = excluded.value
    )"));
    q.addBindValue(key);
    q.addBindValue(value);
    q.exec();
}

void Database::createSchema()
{
    QSqlDatabase db = QSqlDatabase::database(kConnectionName);
    QSqlQuery q(db);

    q.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            sort_order INTEGER NOT NULL DEFAULT 0
        )
    )"));

    q.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS products (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            category_id INTEGER NOT NULL REFERENCES categories(id),
            sku TEXT,
            name TEXT NOT NULL,
            unit TEXT NOT NULL DEFAULT 'Each',
            price REAL NOT NULL DEFAULT 0,
            active INTEGER NOT NULL DEFAULT 1
        )
    )"));

    q.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS cart_items (
            product_id INTEGER PRIMARY KEY REFERENCES products(id) ON DELETE CASCADE,
            quantity INTEGER NOT NULL
        )
    )"));

    q.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS orders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_at TEXT NOT NULL,
            status TEXT NOT NULL DEFAULT 'Submitted',
            notes TEXT,
            total REAL NOT NULL DEFAULT 0
        )
    )"));

    q.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS order_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            order_id INTEGER NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
            product_name TEXT NOT NULL,
            sku TEXT,
            unit TEXT,
            unit_price REAL NOT NULL,
            quantity INTEGER NOT NULL,
            line_total REAL NOT NULL
        )
    )"));

    q.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS app_settings (
            key TEXT PRIMARY KEY,
            value TEXT
        )
    )"));
}

void Database::seedIfEmpty()
{
    QSqlDatabase db = QSqlDatabase::database(kConnectionName);

    QSqlQuery check(db);
    check.exec(QStringLiteral("SELECT COUNT(*) FROM categories"));
    if (check.next() && check.value(0).toInt() > 0)
        return;

    db.transaction();

    QHash<QString, int> categoryIds;
    QSqlQuery insertCategory(db);
    insertCategory.prepare(QStringLiteral("INSERT INTO categories (name, sort_order) VALUES (?, ?)"));

    int order = 0;
    for (const char *name : kCategories) {
        const QString qname = QString::fromUtf8(name);
        insertCategory.addBindValue(qname);
        insertCategory.addBindValue(order);
        insertCategory.exec();
        categoryIds.insert(qname, insertCategory.lastInsertId().toInt());
        ++order;
    }

    QSqlQuery insertProduct(db);
    insertProduct.prepare(QStringLiteral(
        "INSERT INTO products (category_id, sku, name, unit, price) VALUES (?, ?, ?, ?, ?)"));

    for (const SeedProduct &p : kSeedProducts) {
        insertProduct.addBindValue(categoryIds.value(QString::fromUtf8(p.category)));
        insertProduct.addBindValue(QString::fromUtf8(p.sku));
        insertProduct.addBindValue(QString::fromUtf8(p.name));
        insertProduct.addBindValue(QString::fromUtf8(p.unit));
        insertProduct.addBindValue(p.price);
        insertProduct.exec();
    }

    db.commit();
}
