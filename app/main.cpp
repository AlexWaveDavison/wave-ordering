#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQml/qqmlextensionplugin.h>

#include "appcontroller.h"
#include "cartmodel.h"
#include "categorymodel.h"
#include "database.h"
#include "ordermodel.h"
#include "productmodel.h"

// The QML module (models, pages, components) is built as a static plugin.
Q_IMPORT_QML_PLUGIN(WaveOrderingPlugin)

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName(QStringLiteral("Wave Ordering"));
    QGuiApplication::setOrganizationDomain(QStringLiteral("waveordering.app"));
    QGuiApplication::setApplicationName(QStringLiteral("Wave Ordering"));
    QGuiApplication::setApplicationVersion(QStringLiteral(WAVE_ORDERING_VERSION));

    // Same look on every platform (otherwise desktop/iOS pick their native
    // styles and ignore the Material theming used throughout the UI).
    QQuickStyle::setStyle(QStringLiteral("Material"));

    if (!Database::initialize())
        return EXIT_FAILURE;

    // Created up-front (the database must be open first) and handed to QML as
    // singletons, so QML and C++ share the same instances.
    CategoryModel categoryModel(nullptr);
    CartModel cartModel(nullptr);
    ProductModel productModel(nullptr);
    productModel.setCartModel(&cartModel);
    OrderModel orderModel(nullptr);
    AppController appController(&cartModel, &orderModel);

    CategoryModel::setInstance(&categoryModel);
    CartModel::setInstance(&cartModel);
    ProductModel::setInstance(&productModel);
    OrderModel::setInstance(&orderModel);
    AppController::setInstance(&appController);

    int exitCode = EXIT_FAILURE;
    {
        QQmlApplicationEngine engine;
        QObject::connect(
            &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
            []() { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
        engine.loadFromModule("WaveOrdering", "Main");
        exitCode = QGuiApplication::exec();
    }

    Database::close();
    return exitCode;
}
