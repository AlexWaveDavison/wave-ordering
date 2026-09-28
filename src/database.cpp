#include "database.h"

#include "money.h"

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringList>

Q_LOGGING_CATEGORY(lcDatabase, "wave.database")

namespace {

const QString kConnectionName = QStringLiteral("wave_ordering_db");

// Category headings for a typical foodservice wholesale catalogue.
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
    qint64 pricePence;
};

// Sample catalogue used to demonstrate the app. All products, SKUs and prices
// below are fictional placeholder data. Prices are in pence; 0 = POA.
const SeedProduct kSeedProducts[] = {
    {"Cheese", "WV-CHE-001", "Mozzarella Block", "2.5kg", 1450},
    {"Cheese", "WV-CHE-002", "Mature Cheddar Block", "2.5kg", 1520},
    {"Cheese", "WV-CHE-003", "Grated Mozzarella & Cheddar Blend", "6x2kg", 4200},
    {"Cheese", "WV-CHE-004", "Halloumi Block", "1kg", 680},

    {"Flour & Bakery Products", "WV-FLR-001", "Strong White Bread Flour", "16kg", 1240},
    {"Flour & Bakery Products", "WV-FLR-002", "00 Pizza Flour", "25kg", 1890},
    {"Flour & Bakery Products", "WV-FLR-003", "Pre-Baked Pizza Bases 12in", "20", 2400},
    {"Flour & Bakery Products", "WV-FLR-004", "Garlic Bread Slices", "40x60g", 1560},

    {"Pizza Toppings", "WV-PZA-001", "Pepperoni Slices", "2.5kg", 1320},
    {"Pizza Toppings", "WV-PZA-002", "Sliced Mushrooms in Brine", "6x2.5kg", 1980},
    {"Pizza Toppings", "WV-PZA-003", "Sweetcorn", "6x2.5kg", 1440},
    {"Pizza Toppings", "WV-PZA-004", "Jalapeno Slices", "6x2kg", 1650},

    {"Meat", "WV-MET-001", "Beef Burgers 4oz", "72x113g", 3800},
    {"Meat", "WV-MET-002", "Diced Beef Brisket", "2.5kg", 2200},
    {"Meat", "WV-MET-003", "Italian Sausages", "5kg", 1950},
    {"Meat", "WV-MET-004", "Smoked Bacon Rashers", "2.5kg", 1680},

    {"Chicken", "WV-CHK-001", "Chicken Breast Fillets", "5kg", 2450},
    {"Chicken", "WV-CHK-002", "Southern Fried Chicken Strips", "2.5kg", 1490},
    {"Chicken", "WV-CHK-003", "Chicken Wings", "10kg", 2800},
    {"Chicken", "WV-CHK-004", "Chicken Doner Kebab", "2x2.5kg", 2100},

    {"Seafood", "WV-SEA-001", "Battered Cod Fillets", "10x140g", 1900},
    {"Seafood", "WV-SEA-002", "Breaded Scampi", "1kg", 980},
    {"Seafood", "WV-SEA-003", "Calamari Rings", "1kg", 850},
    {"Seafood", "WV-SEA-004", "Prawn Crackers", "6x227g", 720},

    {"Potato Products", "WV-POT-001", "Straight Cut Chips", "6x2.5kg", 2100},
    {"Potato Products", "WV-POT-002", "Curly Fries", "6x2.27kg", 2350},
    {"Potato Products", "WV-POT-003", "Potato Wedges", "6x2.5kg", 2200},
    {"Potato Products", "WV-POT-004", "Hash Brown Triangles", "100x56g", 1840},

    {"Packaging", "WV-PKG-001", "Pizza Boxes 12in", "100", 1600},
    {"Packaging", "WV-PKG-002", "Foil Containers, Medium", "500", 2800},
    {"Packaging", "WV-PKG-003", "Chip Scoop Trays", "1000", 1250},
    {"Packaging", "WV-PKG-004", "Brown Paper Bags", "500", 900},

    {"Cleaning Materials", "WV-CLN-001", "Multi-Surface Sanitiser", "5L", 840},
    {"Cleaning Materials", "WV-CLN-002", "Heavy Duty Degreaser", "5L", 990},
    {"Cleaning Materials", "WV-CLN-003", "Catering Roll", "6", 1120},
    {"Cleaning Materials", "WV-CLN-004", "Washing Up Liquid", "5L", 650},

    {"Accessories", "WV-ACC-001", "Disposable Gloves, Medium", "100", 450},
    {"Accessories", "WV-ACC-002", "Aluminium Foil Roll", "450m", 2200},
    {"Accessories", "WV-ACC-003", "Cling Film Roll", "300m", 890},
    {"Accessories", "WV-ACC-004", "Wooden Cutlery Sets", "250", 1050},

    {"Beverages", "WV-BEV-001", "Cola, 330ml Cans", "24", 950},
    {"Beverages", "WV-BEV-002", "Sparkling Water, 330ml", "24", 880},
    {"Beverages", "WV-BEV-003", "Orange Juice Carton", "12x1L", 1400},
    {"Beverages", "WV-BEV-004", "Still Water, 500ml", "24", 600},

    {"Herbs & Spices", "WV-HRB-001", "Dried Oregano", "500g", 520},
    {"Herbs & Spices", "WV-HRB-002", "Chilli Flakes", "500g", 580},
    {"Herbs & Spices", "WV-HRB-003", "Garlic Granules", "1kg", 640},
    {"Herbs & Spices", "WV-HRB-004", "Mixed Herbs", "500g", 500},
};

// The current schema. Used as-is for fresh installs and as the target shape
// when migrating an older database.
const char *const kSchema[] = {
    R"(CREATE TABLE IF NOT EXISTS categories (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL UNIQUE,
        sort_order INTEGER NOT NULL DEFAULT 0
    ))",
    R"(CREATE TABLE IF NOT EXISTS products (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        category_id INTEGER NOT NULL REFERENCES categories(id),
        sku TEXT,
        name TEXT NOT NULL,
        unit TEXT NOT NULL DEFAULT 'Each',
        price_pence INTEGER NOT NULL DEFAULT 0,
        active INTEGER NOT NULL DEFAULT 1
    ))",
    R"(CREATE TABLE IF NOT EXISTS cart_items (
        product_id INTEGER PRIMARY KEY REFERENCES products(id) ON DELETE CASCADE,
        quantity INTEGER NOT NULL CHECK (quantity > 0)
    ))",
    R"(CREATE TABLE IF NOT EXISTS orders (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        created_at TEXT NOT NULL,
        status TEXT NOT NULL DEFAULT 'pending',
        customer_name TEXT,
        account_name TEXT,
        notes TEXT,
        total_pence INTEGER NOT NULL DEFAULT 0,
        sent_at TEXT
    ))",
    R"(CREATE TABLE IF NOT EXISTS order_items (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        order_id INTEGER NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
        product_id INTEGER REFERENCES products(id) ON DELETE SET NULL,
        product_name TEXT NOT NULL,
        sku TEXT,
        unit TEXT,
        unit_price_pence INTEGER NOT NULL DEFAULT 0,
        quantity INTEGER NOT NULL,
        line_total_pence INTEGER NOT NULL DEFAULT 0
    ))",
    R"(CREATE INDEX IF NOT EXISTS idx_order_items_order ON order_items(order_id))",
    R"(CREATE INDEX IF NOT EXISTS idx_products_category ON products(category_id))",
    R"(CREATE TABLE IF NOT EXISTS app_settings (
        key TEXT PRIMARY KEY,
        value TEXT
    ))",
};

int userVersion()
{
    QSqlQuery q(Database::connection());
    if (Database::exec(q, QStringLiteral("PRAGMA user_version")) && q.next())
        return q.value(0).toInt();
    return 0;
}

bool setUserVersion(int version)
{
    QSqlQuery q(Database::connection());
    return Database::exec(q, QStringLiteral("PRAGMA user_version = %1").arg(version));
}

bool tableExists(const QString &table)
{
    QSqlQuery q(Database::connection());
    q.prepare(QStringLiteral("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = ?"));
    q.addBindValue(table);
    return Database::exec(q) && q.next();
}

// The CREATE TABLE statement for `table` from kSchema, renamed to `asName`.
QString tableDdl(const char *table, const QString &asName)
{
    const QString prefix = QStringLiteral("CREATE TABLE IF NOT EXISTS %1 (").arg(QLatin1String(table));
    for (const char *sql : kSchema) {
        QString ddl = QString::fromUtf8(sql);
        if (ddl.startsWith(prefix))
            return ddl.replace(0, prefix.size(), QStringLiteral("CREATE TABLE %1 (").arg(asName));
    }
    Q_UNREACHABLE();
    return {};
}

bool runAll(const QStringList &statements)
{
    for (const QString &sql : statements) {
        QSqlQuery q(Database::connection());
        if (!Database::exec(q, sql))
            return false;
    }
    return true;
}

} // namespace

bool Database::initialize(const QString &path)
{
    const QString dbPath = path.isEmpty() ? defaultPath() : path;
    QDir().mkpath(QFileInfo(dbPath).absolutePath());

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), kConnectionName);
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qCCritical(lcDatabase) << "Failed to open database at" << dbPath << ":" << db.lastError().text();
        return false;
    }

    const int version = userVersion();
    if (version > SchemaVersion) {
        qCCritical(lcDatabase) << "Database schema version" << version
                               << "is newer than this app supports (" << SchemaVersion << ")";
        return false;
    }

    // Databases created before schema versioning existed report version 0 but
    // already contain tables; those need migrating rather than creating.
    if (version < SchemaVersion && tableExists(QStringLiteral("products"))) {
        if (!migrate(version))
            return false;
    }

    QSqlQuery pragma(db);
    exec(pragma, QStringLiteral("PRAGMA foreign_keys = ON"));

    return createSchema() && setUserVersion(SchemaVersion) && seedIfEmpty();
}

void Database::close()
{
    {
        QSqlDatabase db = QSqlDatabase::database(kConnectionName, false);
        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(kConnectionName);
}

QSqlDatabase Database::connection()
{
    return QSqlDatabase::database(kConnectionName);
}

QString Database::connectionName()
{
    return kConnectionName;
}

QString Database::defaultPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + QStringLiteral("/waveordering.sqlite");
}

bool Database::exec(QSqlQuery &query)
{
    if (query.exec())
        return true;
    qCWarning(lcDatabase).noquote() << "Query failed:" << query.lastError().text()
                                    << "\n  SQL:" << query.lastQuery();
    return false;
}

bool Database::exec(QSqlQuery &query, const QString &sql)
{
    if (query.exec(sql))
        return true;
    qCWarning(lcDatabase).noquote() << "Query failed:" << query.lastError().text() << "\n  SQL:" << sql;
    return false;
}

QString Database::settingValue(const QString &key, const QString &defaultValue)
{
    QSqlQuery q(connection());
    q.prepare(QStringLiteral("SELECT value FROM app_settings WHERE key = ?"));
    q.addBindValue(key);
    if (exec(q) && q.next())
        return q.value(0).toString();
    return defaultValue;
}

bool Database::setSettingValue(const QString &key, const QString &value)
{
    QSqlQuery q(connection());
    q.prepare(QStringLiteral(R"(
        INSERT INTO app_settings (key, value) VALUES (?, ?)
        ON CONFLICT(key) DO UPDATE SET value = excluded.value
    )"));
    q.addBindValue(key);
    q.addBindValue(value);
    return exec(q);
}

bool Database::createSchema()
{
    QStringList statements;
    for (const char *sql : kSchema)
        statements << QString::fromUtf8(sql);
    return runAll(statements);
}

// Version 0/1 (before this versioning existed) stored money as REAL pounds
// and used 'Submitted' as the only order status. Version 2 stores pence,
// tracks pending/sent orders and records who each order was for.
//
// Tables are rebuilt (create new, copy, drop old, rename) rather than ALTERed,
// following SQLite's recommended procedure, so the result is identical to a
// fresh install. Foreign keys are off during the rebuild so dropping the old
// tables doesn't cascade into the basket.
bool Database::migrate(int fromVersion)
{
    qCInfo(lcDatabase) << "Migrating database schema from version" << fromVersion << "to" << SchemaVersion;

    QSqlDatabase db = connection();
    QSqlQuery pragma(db);
    exec(pragma, QStringLiteral("PRAGMA foreign_keys = OFF"));

    if (!db.transaction())
        return false;

    const bool ok = runAll({
                        tableDdl("products", QStringLiteral("products_new")),
                        tableDdl("orders", QStringLiteral("orders_new")),
                        tableDdl("order_items", QStringLiteral("order_items_new")),
                        QStringLiteral(R"(
                INSERT INTO products_new (id, category_id, sku, name, unit, price_pence, active)
                SELECT id, category_id, sku, name, unit, CAST(ROUND(price * 100) AS INTEGER), active
                FROM products
            )"),
                        QStringLiteral(R"(
                INSERT INTO orders_new (id, created_at, status, notes, total_pence, sent_at)
                SELECT id, created_at,
                       CASE WHEN status = 'Submitted' THEN 'sent' ELSE 'pending' END,
                       notes, CAST(ROUND(total * 100) AS INTEGER),
                       CASE WHEN status = 'Submitted' THEN created_at END
                FROM orders
            )"),
                        QStringLiteral(R"(
                INSERT INTO order_items_new (id, order_id, product_name, sku, unit,
                                             unit_price_pence, quantity, line_total_pence)
                SELECT id, order_id, product_name, sku, unit,
                       CAST(ROUND(unit_price * 100) AS INTEGER), quantity,
                       CAST(ROUND(line_total * 100) AS INTEGER)
                FROM order_items
            )"),
                        // Link historic order lines back to products by SKU so "Reorder" works.
                        QStringLiteral(R"(
                UPDATE order_items_new SET product_id =
                    (SELECT p.id FROM products_new p WHERE p.sku IS NOT NULL AND p.sku = order_items_new.sku)
            )"),
                        QStringLiteral("DROP TABLE order_items"),
                        QStringLiteral("DROP TABLE orders"),
                        QStringLiteral("DROP TABLE products"),
                        QStringLiteral("ALTER TABLE products_new RENAME TO products"),
                        QStringLiteral("ALTER TABLE orders_new RENAME TO orders"),
                        QStringLiteral("ALTER TABLE order_items_new RENAME TO order_items"),
                    })
        && setUserVersion(SchemaVersion);

    if (!ok || !db.commit()) {
        db.rollback();
        exec(pragma, QStringLiteral("PRAGMA foreign_keys = ON"));
        qCCritical(lcDatabase) << "Schema migration failed; database left unchanged";
        return false;
    }

    exec(pragma, QStringLiteral("PRAGMA foreign_keys = ON"));
    return true;
}

bool Database::seedIfEmpty()
{
    QSqlDatabase db = connection();

    QSqlQuery check(db);
    if (!exec(check, QStringLiteral("SELECT COUNT(*) FROM categories")) || !check.next())
        return false;
    if (check.value(0).toInt() > 0)
        return true;

    if (!db.transaction())
        return false;

    QHash<QString, int> categoryIds;
    QSqlQuery insertCategory(db);
    insertCategory.prepare(QStringLiteral("INSERT INTO categories (name, sort_order) VALUES (?, ?)"));

    int order = 0;
    for (const char *name : kCategories) {
        const QString qname = QString::fromUtf8(name);
        insertCategory.addBindValue(qname);
        insertCategory.addBindValue(order++);
        if (!exec(insertCategory)) {
            db.rollback();
            return false;
        }
        categoryIds.insert(qname, insertCategory.lastInsertId().toInt());
    }

    QSqlQuery insertProduct(db);
    insertProduct.prepare(QStringLiteral(
        "INSERT INTO products (category_id, sku, name, unit, price_pence) VALUES (?, ?, ?, ?, ?)"));

    for (const SeedProduct &p : kSeedProducts) {
        insertProduct.addBindValue(categoryIds.value(QString::fromUtf8(p.category)));
        insertProduct.addBindValue(QString::fromUtf8(p.sku));
        insertProduct.addBindValue(QString::fromUtf8(p.name));
        insertProduct.addBindValue(QString::fromUtf8(p.unit));
        insertProduct.addBindValue(p.pricePence);
        if (!exec(insertProduct)) {
            db.rollback();
            return false;
        }
    }

    return db.commit();
}
