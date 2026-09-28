#include "cartmodel.h"
#include "database.h"
#include "productmodel.h"
#include "testdatabase.h"

#include <QSignalSpy>
#include <QSqlQuery>
#include <QtTest>

class TestCart : public QObject
{
    Q_OBJECT

private slots:
    void init() { QVERIFY(m_db.open()); }
    void cleanup() { m_db.close(); }

    void addIncrementDecrementRemove()
    {
        CartModel cart(nullptr);
        const int mozzarella = productId("WV-CHE-001"); // £14.50

        cart.addProduct(mozzarella, 2);
        QCOMPARE(cart.lineCount(), 1);
        QCOMPARE(cart.itemCount(), 2);
        QCOMPARE(cart.totalPence(), qint64(2900));
        QCOMPARE(cart.totalText(), QStringLiteral("£29.00"));

        cart.increment(mozzarella);
        QCOMPARE(cart.quantityForProduct(mozzarella), 3);

        cart.decrement(mozzarella);
        cart.decrement(mozzarella);
        cart.decrement(mozzarella);
        QCOMPARE(cart.lineCount(), 0); // decrementing to zero removes the line
        QCOMPARE(cart.totalPence(), qint64(0));
    }

    void totalsAreExactInPence()
    {
        CartModel cart(nullptr);
        // Classic floating-point trap: 3 x £19.80 + 7 x £8.50 must be exactly £118.90.
        cart.addProduct(productId("WV-PZA-002"), 3);
        cart.addProduct(productId("WV-SEA-003"), 7);
        QCOMPARE(cart.totalPence(), qint64(3 * 1980 + 7 * 850));
    }

    void persistsAcrossReload()
    {
        const int chips = productId("WV-POT-001");
        {
            CartModel cart(nullptr);
            cart.addProduct(chips, 4);
        }
        CartModel reloaded(nullptr);
        QCOMPARE(reloaded.quantityForProduct(chips), 4);
    }

    void priceOnApplication()
    {
        const int halloumi = productId("WV-CHE-004");
        QSqlQuery q(Database::connection());
        QVERIFY(q.exec(QStringLiteral("UPDATE products SET price_pence = 0 WHERE id = %1").arg(halloumi)));

        CartModel cart(nullptr);
        cart.addProduct(halloumi, 1);
        QVERIFY(cart.hasPoaItems());
        QCOMPARE(cart.totalText(), QStringLiteral("POA"));

        cart.addProduct(productId("WV-CHE-001"), 1);
        QCOMPARE(cart.totalText(), QStringLiteral("£14.50 + POA"));

        const QModelIndex first = cart.index(0);
        QCOMPARE(cart.data(first, CartModel::IsPoaRole).toBool(), true);
        QCOMPARE(cart.data(first, CartModel::PriceTextRole).toString(), QStringLiteral("POA"));
    }

    void ignoresUnknownAndInvalid()
    {
        CartModel cart(nullptr);
        cart.addProduct(999999, 1);
        cart.addProduct(productId("WV-CHE-001"), 0);
        cart.addProduct(productId("WV-CHE-001"), -3);
        QCOMPARE(cart.lineCount(), 0);
    }

    // Changing a quantity must update the one catalogue row in place, not
    // reset the whole list (which would jump the user back to the top).
    void catalogueUpdatesRowInPlace()
    {
        CartModel cart(nullptr);
        ProductModel products(nullptr);
        products.setCartModel(&cart);

        QSignalSpy resets(&products, &QAbstractItemModel::modelReset);
        QSignalSpy changes(&products, &QAbstractItemModel::dataChanged);

        const int mozzarella = productId("WV-CHE-001");
        cart.increment(mozzarella);
        cart.increment(mozzarella);

        QCOMPARE(resets.count(), 0);
        QCOMPARE(changes.count(), 2);

        const QModelIndex changed = changes.last().at(0).value<QModelIndex>();
        QCOMPARE(products.data(changed, ProductModel::IdRole).toInt(), mozzarella);
        QCOMPARE(products.data(changed, ProductModel::QuantityInCartRole).toInt(), 2);

        cart.clear();
        QCOMPARE(products.data(changed, ProductModel::QuantityInCartRole).toInt(), 0);
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

QTEST_GUILESS_MAIN(TestCart)
#include "tst_cart.moc"
