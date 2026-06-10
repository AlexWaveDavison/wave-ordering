#include "appcontroller.h"

#include "cartmodel.h"
#include "database.h"
#include "ordermodel.h"
#include "productmodel.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>

AppController::AppController(CartModel *cart, OrderModel *orders, ProductModel *products, QObject *parent)
    : QObject(parent)
    , m_cart(cart)
    , m_orders(orders)
    , m_products(products)
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
