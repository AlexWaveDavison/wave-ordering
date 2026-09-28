#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>

class CartModel;
class OrderModel;

// Actions exposed to QML that span several models: placing, sending and
// reordering orders, plus the customer/account details saved on the device.
// Exposed to QML as the `AppController` singleton.
class AppController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString customerName READ customerName WRITE setCustomerName NOTIFY accountDetailsChanged)
    Q_PROPERTY(QString accountName READ accountName WRITE setAccountName NOTIFY accountDetailsChanged)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)

public:
    AppController(CartModel *cart, OrderModel *orders, QObject *parent = nullptr);

    QString customerName() const { return m_customerName; }
    void setCustomerName(const QString &name);

    QString accountName() const { return m_accountName; }
    void setAccountName(const QString &name);

    QString appVersion() const;

    // Saves the basket as a new "Not sent" order (stamped with the current
    // customer/account details), then clears the basket. Returns the new
    // order id, or -1 if the basket is empty or the save failed (in which case
    // the basket is left untouched and errorOccurred() is emitted).
    Q_INVOKABLE int placeOrder(const QString &notes);

    // The WhatsApp message for an order (empty if the order doesn't exist).
    Q_INVOKABLE QString orderShareText(int orderId) const;

    // Opens WhatsApp with the order summary pre-filled so the user can pick
    // who to send it to. The order stays "Not sent" until markOrderSent().
    Q_INVOKABLE bool shareOrderViaWhatsApp(int orderId);

    Q_INVOKABLE bool markOrderSent(int orderId);

    // Adds the still-available products from a past order back into the
    // basket at the same quantities. Returns how many lines were added.
    Q_INVOKABLE int reorder(int orderId);

signals:
    void accountDetailsChanged();
    void errorOccurred(const QString &message);

public:
    static void setInstance(AppController *instance) { s_instance = instance; }
    static AppController *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

private:
    CartModel *m_cart;
    OrderModel *m_orders;
    QString m_customerName;
    QString m_accountName;

    inline static AppController *s_instance = nullptr;
};
