#include "appcontroller.h"
#include "cartmodel.h"
#include "categorymodel.h"
#include "ordermodel.h"
#include "productmodel.h"
#include "testdatabase.h"

#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtQml/qqmlextensionplugin.h>
#include <QtTest>

Q_IMPORT_QML_PLUGIN(WaveOrderingPlugin)

// Smoke tests for the QML layer: the singletons QML sees must be the C++
// instances main() creates, and the whole UI must load without errors.
class TestQml : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        QQuickStyle::setStyle(QStringLiteral("Material"));
        QVERIFY(m_db.open());

        m_categories = std::make_unique<CategoryModel>(nullptr);
        m_cart = std::make_unique<CartModel>(nullptr);
        m_products = std::make_unique<ProductModel>(nullptr);
        m_products->setCartModel(m_cart.get());
        m_orders = std::make_unique<OrderModel>(nullptr);
        m_app = std::make_unique<AppController>(m_cart.get(), m_orders.get());

        CategoryModel::setInstance(m_categories.get());
        CartModel::setInstance(m_cart.get());
        ProductModel::setInstance(m_products.get());
        OrderModel::setInstance(m_orders.get());
        AppController::setInstance(m_app.get());
    }

    void cleanupTestCase()
    {
        m_app.reset();
        m_orders.reset();
        m_products.reset();
        m_cart.reset();
        m_categories.reset();
        m_db.close();
    }

    void singletonsAreTheCppInstances()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(R"(
            import QtQml
            import WaveOrdering
            QtObject {
                property QtObject cart: CartModel
                property QtObject products: ProductModel
                property QtObject orders: OrderModel
                property QtObject categories: CategoryModel
                property QtObject app: AppController
            }
        )", QUrl());
        std::unique_ptr<QObject> object(component.create());
        QVERIFY2(object, qPrintable(component.errorString()));

        QCOMPARE(object->property("cart").value<QObject *>(), m_cart.get());
        QCOMPARE(object->property("products").value<QObject *>(), m_products.get());
        QCOMPARE(object->property("orders").value<QObject *>(), m_orders.get());
        QCOMPARE(object->property("categories").value<QObject *>(), m_categories.get());
        QCOMPARE(object->property("app").value<QObject *>(), m_app.get());
    }

    void mainWindowLoadsCleanly()
    {
        QStringList warnings;
        QQmlApplicationEngine engine;
        connect(&engine, &QQmlApplicationEngine::warnings, this, [&](const QList<QQmlError> &errors) {
            for (const QQmlError &e : errors)
                warnings << e.toString();
        });

        engine.loadFromModule("WaveOrdering", "Main");
        QCOMPARE(engine.rootObjects().size(), 1);

        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QVERIFY(window);
        QVERIFY(QTest::qWaitForWindowExposed(window));

        // Exercise the bindings that depend on basket/order state.
        m_cart->addProduct(1, 2);
        QTest::qWait(50);
        m_cart->clear();
        QTest::qWait(50);

        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join(QLatin1Char('\n'))));
    }

private:
    TestDatabase m_db;
    std::unique_ptr<CategoryModel> m_categories;
    std::unique_ptr<CartModel> m_cart;
    std::unique_ptr<ProductModel> m_products;
    std::unique_ptr<OrderModel> m_orders;
    std::unique_ptr<AppController> m_app;
};

QTEST_MAIN(TestQml)
#include "tst_qml.moc"
