#pragma once

#include <QObject>
#include <QString>

class CartModel;
class OrderModel;
class ProductModel;

// Top-level actions exposed to QML that span multiple models.
class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(CartModel *cart, OrderModel *orders, ProductModel *products,
                            QObject *parent = nullptr);

    // Moves the current basket into a new order, clears the basket and
    // refreshes order history. Returns false if the basket is empty or the
    // database write fails.
    Q_INVOKABLE bool placeOrder(const QString &notes);

private:
    CartModel *m_cart;
    OrderModel *m_orders;
    ProductModel *m_products;
};
