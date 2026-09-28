#include "appcontroller.h"

#include "cartmodel.h"
#include "database.h"
#include "ordermodel.h"
#include "orderstore.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>

namespace {
const QString kCustomerNameKey = QStringLiteral("customer_name");
const QString kAccountNameKey = QStringLiteral("account_name");
} // namespace

AppController::AppController(CartModel *cart, OrderModel *orders, QObject *parent)
    : QObject(parent)
    , m_cart(cart)
    , m_orders(orders)
    , m_customerName(Database::settingValue(kCustomerNameKey))
    , m_accountName(Database::settingValue(kAccountNameKey))
{
}

AppController *AppController::create(QQmlEngine *, QJSEngine *jsEngine)
{
    Q_ASSERT_X(s_instance, "AppController::create", "setInstance() must be called before QML loads");
    Q_ASSERT(jsEngine->thread() == s_instance->thread());
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}

void AppController::setCustomerName(const QString &name)
{
    const QString trimmed = name.trimmed();
    if (m_customerName == trimmed)
        return;
    m_customerName = trimmed;
    Database::setSettingValue(kCustomerNameKey, trimmed);
    emit accountDetailsChanged();
}

void AppController::setAccountName(const QString &name)
{
    const QString trimmed = name.trimmed();
    if (m_accountName == trimmed)
        return;
    m_accountName = trimmed;
    Database::setSettingValue(kAccountNameKey, trimmed);
    emit accountDetailsChanged();
}

QString AppController::appVersion() const
{
    return QCoreApplication::applicationVersion();
}

int AppController::placeOrder(const QString &notes)
{
    if (m_cart->lineCount() == 0)
        return -1;

    OrderRecord order;
    order.createdAt = QDateTime::currentDateTime();
    order.status = OrderStore::StatusPending;
    order.customerName = m_customerName;
    order.accountName = m_accountName;
    order.notes = notes.trimmed();

    for (const CartLine &cartLine : m_cart->lines()) {
        OrderLine line;
        line.productId = cartLine.productId;
        line.name = cartLine.name;
        line.sku = cartLine.sku;
        line.unit = cartLine.unit;
        line.unitPricePence = cartLine.pricePence;
        line.quantity = cartLine.quantity;
        order.lines.append(line);
    }

    const int orderId = OrderStore::create(order);
    if (orderId < 0) {
        emit errorOccurred(tr("Your order couldn't be saved. Your basket hasn't been changed — please try again."));
        return -1;
    }

    m_cart->clear();
    m_orders->refresh();
    return orderId;
}

QString AppController::orderShareText(int orderId) const
{
    const std::optional<OrderRecord> order = OrderStore::load(orderId);
    return order ? buildOrderShareText(*order) : QString();
}

bool AppController::shareOrderViaWhatsApp(int orderId)
{
    const QString text = orderShareText(orderId);
    if (text.isEmpty())
        return false;

    const QUrl url(QStringLiteral("https://wa.me/?text=") + QString::fromUtf8(QUrl::toPercentEncoding(text)));
    if (!QDesktopServices::openUrl(url)) {
        emit errorOccurred(tr("Couldn't open WhatsApp on this device."));
        return false;
    }
    return true;
}

bool AppController::markOrderSent(int orderId)
{
    if (!OrderStore::markSent(orderId)) {
        emit errorOccurred(tr("Couldn't update the order status."));
        return false;
    }
    m_orders->refresh();
    return true;
}

int AppController::reorder(int orderId)
{
    const std::optional<OrderRecord> order = OrderStore::load(orderId);
    if (!order)
        return 0;

    int added = 0;
    for (const OrderLine &line : order->lines) {
        if (line.productId < 0)
            continue;
        const int before = m_cart->quantityForProduct(line.productId);
        m_cart->addProduct(line.productId, line.quantity);
        if (m_cart->quantityForProduct(line.productId) != before)
            ++added;
    }
    return added;
}
