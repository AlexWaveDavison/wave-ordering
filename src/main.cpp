#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "appcontroller.h"
#include "cartmodel.h"
#include "categorymodel.h"
#include "database.h"
#include "ordermodel.h"
#include "productmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("Wave Ordering"));
    app.setOrganizationDomain(QStringLiteral("waveordering.app"));
    app.setApplicationName(QStringLiteral("Wave Ordering"));

    if (!Database::initialize())
        return -1;

    QQmlApplicationEngine engine;

    auto *categoryModel = new CategoryModel(&app);
    auto *cartModel = new CartModel(&app);
    auto *productModel = new ProductModel(&app);
    productModel->setCartModel(cartModel);
    auto *orderModel = new OrderModel(&app);
    auto *appController = new AppController(cartModel, orderModel, productModel, &app);

    engine.rootContext()->setContextProperty(QStringLiteral("categoryModel"), categoryModel);
    engine.rootContext()->setContextProperty(QStringLiteral("productModel"), productModel);
    engine.rootContext()->setContextProperty(QStringLiteral("cartModel"), cartModel);
    engine.rootContext()->setContextProperty(QStringLiteral("orderModel"), orderModel);
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), appController);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.loadFromModule("WaveOrdering", "Main");

    return app.exec();
}
