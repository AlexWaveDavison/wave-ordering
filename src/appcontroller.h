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

    Q_PROPERTY(QString customerName READ customerName WRITE setCustomerName NOTIFY accountDetailsChanged)
    Q_PROPERTY(QString accountName READ accountName WRITE setAccountName NOTIFY accountDetailsChanged)

public:
    explicit AppController(CartModel *cart, OrderModel *orders, ProductModel *products,
                            QObject *parent = nullptr);

    // Moves the current basket into a new order, clears the basket and
    // refreshes order history. Returns false if the basket is empty or the
    // database write fails.
    Q_INVOKABLE bool placeOrder(const QString &notes);

    QString customerName() const;
    void setCustomerName(const QString &name);

    QString accountName() const;
    void setAccountName(const QString &name);

    // Builds a WhatsApp-friendly summary of the current basket (customer and
    // account details, then a monospace table of products). Call this before
    // placeOrder(), which clears the basket.
    Q_INVOKABLE QString buildShareText(const QString &notes) const;

    // Opens WhatsApp with the given text pre-filled, letting the user pick a
    // contact to send it to.
    Q_INVOKABLE void shareViaWhatsApp(const QString &text) const;

signals:
    void accountDetailsChanged();

private:
    CartModel *m_cart;
    OrderModel *m_orders;
    ProductModel *m_products;
    QString m_customerName;
    QString m_accountName;
};
