#include "appcontroller.h"

#include "cartmodel.h"
#include "database.h"
#include "ordermodel.h"
#include "productmodel.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QUrl>

AppController::AppController(CartModel *cart, OrderModel *orders, ProductModel *products, QObject *parent)
    : QObject(parent)
    , m_cart(cart)
    , m_orders(orders)
    , m_products(products)
    , m_customerName(Database::settingValue(QStringLiteral("customer_name")))
    , m_accountName(Database::settingValue(QStringLiteral("account_name")))
{
}

bool AppController::placeOrder(const QString &notes)
{
    if (m_cart->rowCount() == 0)
        return false;

    QSqlDatabase db = QSqlDatabase::database(Database::connectionName());
    if (!db.transaction())
        return false;

    QSqlQuery insertOrder(db);
    insertOrder.prepare(QStringLiteral(
        "INSERT INTO orders (created_at, status, notes, total) VALUES (?, 'Submitted', ?, ?)"));
    insertOrder.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    insertOrder.addBindValue(notes);
    insertOrder.addBindValue(m_cart->total());
    if (!insertOrder.exec()) {
        db.rollback();
        return false;
    }

    const int orderId = insertOrder.lastInsertId().toInt();

    QSqlQuery insertItem(db);
    insertItem.prepare(QStringLiteral(R"(
        INSERT INTO order_items (order_id, product_name, sku, unit, unit_price, quantity, line_total)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )"));

    for (int row = 0; row < m_cart->rowCount(); ++row) {
        const QModelIndex idx = m_cart->index(row);
        insertItem.addBindValue(orderId);
        insertItem.addBindValue(m_cart->data(idx, CartModel::NameRole));
        insertItem.addBindValue(m_cart->data(idx, CartModel::SkuRole));
        insertItem.addBindValue(m_cart->data(idx, CartModel::UnitRole));
        insertItem.addBindValue(m_cart->data(idx, CartModel::PriceRole));
        insertItem.addBindValue(m_cart->data(idx, CartModel::QuantityRole));
        insertItem.addBindValue(m_cart->data(idx, CartModel::LineTotalRole));
        if (!insertItem.exec()) {
            db.rollback();
            return false;
        }
    }

    db.commit();

    m_cart->clear();
    m_orders->refresh();
    m_products->refresh();
    return true;
}

QString AppController::customerName() const
{
    return m_customerName;
}

void AppController::setCustomerName(const QString &name)
{
    if (m_customerName == name)
        return;
    m_customerName = name;
    Database::setSettingValue(QStringLiteral("customer_name"), name);
    emit accountDetailsChanged();
}

QString AppController::accountName() const
{
    return m_accountName;
}

void AppController::setAccountName(const QString &name)
{
    if (m_accountName == name)
        return;
    m_accountName = name;
    Database::setSettingValue(QStringLiteral("account_name"), name);
    emit accountDetailsChanged();
}

QString AppController::buildShareText(const QString &notes) const
{
    auto formatRow = [](const QString &ref, const QString &name, const QString &qty, const QString &price) {
        QString shortName = name;
        if (shortName.size() > 18)
            shortName = shortName.left(16) + QStringLiteral("..");
        return QStringLiteral("%1 %2 %3 %4")
            .arg(ref.left(10), -10)
            .arg(shortName, -18)
            .arg(qty, 3)
            .arg(price, 7);
    };

    QStringList lines;
    lines << QStringLiteral("*Wave Ordering - New Order*");
    lines << QString();
    lines << QStringLiteral("Customer: %1").arg(m_customerName.isEmpty() ? QStringLiteral("-") : m_customerName);
    lines << QStringLiteral("Account: %1").arg(m_accountName.isEmpty() ? QStringLiteral("-") : m_accountName);
    lines << QString();
    lines << QStringLiteral("```");
    lines << formatRow(QStringLiteral("Ref"), QStringLiteral("Product"), QStringLiteral("Qty"), QStringLiteral("Price"));

    for (int row = 0; row < m_cart->rowCount(); ++row) {
        const QModelIndex idx = m_cart->index(row);
        const QString sku = m_cart->data(idx, CartModel::SkuRole).toString();
        const QString name = m_cart->data(idx, CartModel::NameRole).toString();
        const QString qty = QString::number(m_cart->data(idx, CartModel::QuantityRole).toInt());
        const QString price = QStringLiteral("£%1").arg(m_cart->data(idx, CartModel::PriceRole).toDouble(), 0, 'f', 2);
        lines << formatRow(sku, name, qty, price);
    }

    lines << QStringLiteral("```");
    lines << QString();
    lines << QStringLiteral("*Total: £%1*").arg(m_cart->total(), 0, 'f', 2);

    if (!notes.trimmed().isEmpty()) {
        lines << QString();
        lines << QStringLiteral("Notes: %1").arg(notes.trimmed());
    }

    return lines.join(QLatin1Char('\n'));
}

void AppController::shareViaWhatsApp(const QString &text) const
{
    const QString url = QStringLiteral("https://wa.me/?text=") + QString::fromUtf8(QUrl::toPercentEncoding(text));
    QDesktopServices::openUrl(QUrl(url));
}
