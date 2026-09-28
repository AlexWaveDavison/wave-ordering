#include "appcontroller.h"
#include "cartmodel.h"
#include "database.h"
#include "ordermodel.h"
#include "orderstore.h"
#include "testdatabase.h"

#include <QSignalSpy>
#include <QSqlQuery>
#include <QtTest>

class TestOrders : public QObject
{
    Q_OBJECT

private slots:
    void init() { QVERIFY(m_db.open()); }
    void cleanup() { m_db.close(); }

    void placeOrderSnapshotsBasketAndDetails()
    {
        CartModel cart(nullptr);
        OrderModel orders(nullptr);
        AppController app(&cart, &orders);
        app.setCustomerName(QStringLiteral("  Pizza Palace  "));
        app.setAccountName(QStringLiteral("PP-001"));

        cart.addProduct(productId("WV-CHE-001"), 2); // 2 x £14.50
        cart.addProduct(productId("WV-POT-001"), 1); // 1 x £21.00

        const int orderId = app.placeOrder(QStringLiteral("  Deliver before 11  "));
        QVERIFY(orderId > 0);
        QCOMPARE(cart.lineCount(), 0);
        QCOMPARE(orders.count(), 1);
        QCOMPARE(orders.pendingCount(), 1);

        const std::optional<OrderRecord> order = OrderStore::load(orderId);
        QVERIFY(order);
        QCOMPARE(order->status, OrderStore::StatusPending);
        QVERIFY(!order->isSent());
        QCOMPARE(order->customerName, QStringLiteral("Pizza Palace"));
        QCOMPARE(order->accountName, QStringLiteral("PP-001"));
        QCOMPARE(order->notes, QStringLiteral("Deliver before 11"));
        QCOMPARE(order->lines.size(), 2);
        QCOMPARE(order->totalPence(), qint64(5000));
        QCOMPARE(order->itemCount(), 3);

        // Changing the catalogue later must not rewrite history.
        QSqlQuery q(Database::connection());
        QVERIFY(q.exec(QStringLiteral("UPDATE products SET price_pence = 9999")));
        QCOMPARE(OrderStore::load(orderId)->totalPence(), qint64(5000));
    }

    void emptyBasketDoesNothing()
    {
        CartModel cart(nullptr);
        OrderModel orders(nullptr);
        AppController app(&cart, &orders);
        QCOMPARE(app.placeOrder(QString()), -1);
        QCOMPARE(orders.count(), 0);
    }

    void markSent()
    {
        CartModel cart(nullptr);
        OrderModel orders(nullptr);
        AppController app(&cart, &orders);
        cart.addProduct(productId("WV-CHE-001"), 1);
        const int orderId = app.placeOrder(QString());

        QVERIFY(app.markOrderSent(orderId));
        QVERIFY(OrderStore::load(orderId)->isSent());
        QVERIFY(OrderStore::load(orderId)->sentAt.isValid());
        QCOMPARE(orders.pendingCount(), 0);
        QCOMPARE(orders.data(orders.index(0), OrderModel::StatusTextRole).toString(), QStringLiteral("Sent"));

        QSignalSpy errors(&app, &AppController::errorOccurred);
        QVERIFY(!app.markOrderSent(424242));
        QCOMPARE(errors.count(), 1);
    }

    void reorderAddsAvailableLines()
    {
        CartModel cart(nullptr);
        OrderModel orders(nullptr);
        AppController app(&cart, &orders);
        const int mozzarella = productId("WV-CHE-001");
        const int chips = productId("WV-POT-001");
        cart.addProduct(mozzarella, 2);
        cart.addProduct(chips, 5);
        const int orderId = app.placeOrder(QString());

        // Chips have since been discontinued.
        QSqlQuery q(Database::connection());
        QVERIFY(q.exec(QStringLiteral("UPDATE products SET active = 0 WHERE id = %1").arg(chips)));

        const QVariantMap details = orders.details(orderId);
        QVERIFY(details.value(QStringLiteral("canReorder")).toBool());

        QCOMPARE(app.reorder(orderId), 1);
        QCOMPARE(cart.quantityForProduct(mozzarella), 2);
        QCOMPARE(cart.quantityForProduct(chips), 0);
    }

    void detailsForMissingOrderIsEmpty()
    {
        OrderModel orders(nullptr);
        QVERIFY(orders.details(12345).isEmpty());
    }

    void shareTextFormat()
    {
        OrderRecord order;
        order.id = 42;
        order.createdAt = QDateTime(QDate(2026, 9, 28), QTime(20, 1));
        order.customerName = QStringLiteral("Pizza Palace");
        order.notes = QStringLiteral("Side door please");

        OrderLine priced;
        priced.name = QStringLiteral("Grated Mozzarella & Cheddar Blend");
        priced.sku = QStringLiteral("WV-CHE-003");
        priced.unitPricePence = 4200;
        priced.quantity = 2;

        OrderLine poa;
        poa.name = QStringLiteral("Halloumi Block");
        poa.sku = QStringLiteral("WV-CHE-004");
        poa.unitPricePence = 0;
        poa.quantity = 1;

        order.lines = {priced, poa};

        const QString expected = QStringLiteral(
            "*Wave Ordering - Order #42*\n"
            "28 Sep 2026, 20:01\n"
            "\n"
            "Customer: Pizza Palace\n"
            "Account: -\n"
            "\n"
            "```\n"
            "Ref        Product            Qty    Price\n"
            "WV-CHE-003 Grated Mozzarell..   2   £42.00\n"
            "WV-CHE-004 Halloumi Block       1      POA\n"
            "```\n"
            "\n"
            "*Total: £84.00* (plus POA items)\n"
            "\n"
            "Notes: Side door please");

        QCOMPARE(buildOrderShareText(order), expected);
    }

private:
    static int productId(const char *sku)
    {
        QSqlQuery q(Database::connection());
        q.prepare(QStringLiteral("SELECT id FROM products WHERE sku = ?"));
        q.addBindValue(QString::fromLatin1(sku));
        return q.exec() && q.next() ? q.value(0).toInt() : -1;
    }

    TestDatabase m_db;
};

QTEST_GUILESS_MAIN(TestOrders)
#include "tst_orders.moc"
