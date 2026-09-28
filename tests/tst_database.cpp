#include "database.h"
#include "orderstore.h"
#include "productmodel.h"
#include "testdatabase.h"

#include <QSqlQuery>
#include <QtTest>

class TestDatabaseSchema : public QObject
{
    Q_OBJECT

private slots:
    void cleanup() { m_db.close(); }

    void freshInstallIsSeeded()
    {
        QVERIFY(m_db.open());

        QSqlQuery q(Database::connection());
        QVERIFY(q.exec(QStringLiteral("PRAGMA user_version")) && q.next());
        QCOMPARE(q.value(0).toInt(), Database::SchemaVersion);

        QVERIFY(q.exec(QStringLiteral("SELECT COUNT(*) FROM categories")) && q.next());
        QCOMPARE(q.value(0).toInt(), 12);

        QVERIFY(q.exec(QStringLiteral("SELECT COUNT(*), SUM(price_pence) FROM products")) && q.next());
        QCOMPARE(q.value(0).toInt(), 48);
        QVERIFY(q.value(1).toLongLong() > 0);
    }

    void reopeningDoesNotReseed()
    {
        QVERIFY(m_db.open());
        const QString path = m_db.path();
        Database::close();
        QVERIFY(Database::initialize(path));

        QSqlQuery q(Database::connection());
        QVERIFY(q.exec(QStringLiteral("SELECT COUNT(*) FROM products")) && q.next());
        QCOMPARE(q.value(0).toInt(), 48);
    }

    void settingsRoundTrip()
    {
        QVERIFY(m_db.open());
        QCOMPARE(Database::settingValue(QStringLiteral("missing"), QStringLiteral("fallback")),
                 QStringLiteral("fallback"));
        QVERIFY(Database::setSettingValue(QStringLiteral("k"), QStringLiteral("one")));
        QVERIFY(Database::setSettingValue(QStringLiteral("k"), QStringLiteral("two")));
        QCOMPARE(Database::settingValue(QStringLiteral("k")), QStringLiteral("two"));
    }

    // A database written by v0.1 (REAL prices, 'Submitted' status, no
    // user_version) must upgrade in place without losing the basket or orders.
    void migratesLegacyDatabase()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath(QStringLiteral("legacy.sqlite"));
        {
            QSqlDatabase legacy = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("legacy"));
            legacy.setDatabaseName(path);
            QVERIFY(legacy.open());
            QSqlQuery q(legacy);
            const QStringList statements = {
                "CREATE TABLE categories (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL UNIQUE, "
                "sort_order INTEGER NOT NULL DEFAULT 0)",
                "CREATE TABLE products (id INTEGER PRIMARY KEY AUTOINCREMENT, category_id INTEGER NOT NULL "
                "REFERENCES categories(id), sku TEXT, name TEXT NOT NULL, unit TEXT NOT NULL DEFAULT 'Each', "
                "price REAL NOT NULL DEFAULT 0, active INTEGER NOT NULL DEFAULT 1)",
                "CREATE TABLE cart_items (product_id INTEGER PRIMARY KEY REFERENCES products(id) ON DELETE "
                "CASCADE, quantity INTEGER NOT NULL)",
                "CREATE TABLE orders (id INTEGER PRIMARY KEY AUTOINCREMENT, created_at TEXT NOT NULL, status "
                "TEXT NOT NULL DEFAULT 'Submitted', notes TEXT, total REAL NOT NULL DEFAULT 0)",
                "CREATE TABLE order_items (id INTEGER PRIMARY KEY AUTOINCREMENT, order_id INTEGER NOT NULL "
                "REFERENCES orders(id) ON DELETE CASCADE, product_name TEXT NOT NULL, sku TEXT, unit TEXT, "
                "unit_price REAL NOT NULL, quantity INTEGER NOT NULL, line_total REAL NOT NULL)",
                "CREATE TABLE app_settings (key TEXT PRIMARY KEY, value TEXT)",
                "INSERT INTO categories (id, name) VALUES (1, 'Cheese')",
                "INSERT INTO products (id, category_id, sku, name, unit, price) "
                "VALUES (1, 1, 'WV-CHE-001', 'Mozzarella Block', '2.5kg', 14.5)",
                "INSERT INTO products (id, category_id, sku, name, unit, price) "
                "VALUES (2, 1, 'WV-CHE-002', 'Cheddar', '2.5kg', 15.2)",
                "INSERT INTO cart_items (product_id, quantity) VALUES (2, 3)",
                "INSERT INTO orders (id, created_at, status, notes, total) "
                "VALUES (1, '2026-06-01T10:00:00', 'Submitted', 'Back door', 29.0)",
                "INSERT INTO order_items (order_id, product_name, sku, unit, unit_price, quantity, line_total) "
                "VALUES (1, 'Mozzarella Block', 'WV-CHE-001', '2.5kg', 14.5, 2, 29.0)",
            };
            for (const QString &sql : statements)
                QVERIFY2(q.exec(sql), qPrintable(sql));
            legacy.close();
        }
        QSqlDatabase::removeDatabase(QStringLiteral("legacy"));

        QVERIFY(Database::initialize(path));

        QSqlQuery q(Database::connection());
        QVERIFY(q.exec(QStringLiteral("SELECT price_pence FROM products WHERE id = 2")) && q.next());
        QCOMPARE(q.value(0).toLongLong(), qint64(1520));

        QVERIFY(q.exec(QStringLiteral("SELECT quantity FROM cart_items WHERE product_id = 2")) && q.next());
        QCOMPARE(q.value(0).toInt(), 3); // basket survived the rebuild

        QVERIFY(q.exec(QStringLiteral("SELECT COUNT(*) FROM categories")) && q.next());
        QCOMPARE(q.value(0).toInt(), 1); // existing catalogue is not reseeded

        const std::optional<OrderRecord> order = OrderStore::load(1);
        QVERIFY(order);
        QVERIFY(order->isSent());
        QCOMPARE(order->notes, QStringLiteral("Back door"));
        QCOMPARE(order->lines.size(), 1);
        QCOMPARE(order->lines.first().unitPricePence, qint64(1450));
        QCOMPARE(order->lines.first().productId, 1); // relinked by SKU
        QCOMPARE(order->totalPence(), qint64(2900));

        QVERIFY(q.exec(QStringLiteral("PRAGMA foreign_key_check")));
        QVERIFY(!q.next());

        Database::close();
    }

    void searchTreatsWildcardsLiterally()
    {
        QVERIFY(m_db.open());
        ProductModel products(nullptr);

        products.setSearchText(QStringLiteral("mozz"));
        QCOMPARE(products.count(), 2); // Mozzarella Block + the blend

        products.setSearchText(QStringLiteral("wv-che"));
        QCOMPARE(products.count(), 4); // SKU search, case-insensitive

        products.setSearchText(QStringLiteral("%"));
        QCOMPARE(products.count(), 0);

        products.setSearchText(QStringLiteral("o'brien"));
        QCOMPARE(products.count(), 0); // quotes are bound, not interpolated

        products.setSearchText(QString());
        QCOMPARE(products.count(), 48);
    }

    void categoryFilter()
    {
        QVERIFY(m_db.open());
        ProductModel products(nullptr);

        QSqlQuery q(Database::connection());
        QVERIFY(q.exec(QStringLiteral("SELECT id FROM categories WHERE name = 'Seafood'")) && q.next());
        products.setCategoryFilter(q.value(0).toInt());
        QCOMPARE(products.count(), 4);
        QCOMPARE(products.data(products.index(0), ProductModel::CategoryNameRole).toString(),
                 QStringLiteral("Seafood"));
    }

private:
    TestDatabase m_db;
};

QTEST_GUILESS_MAIN(TestDatabaseSchema)
#include "tst_database.moc"
