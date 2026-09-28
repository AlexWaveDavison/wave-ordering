import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

Page {
    id: page

    signal browseRequested()
    signal orderPlaced(int orderId)

    function submitOrder() {
        // Commit the fields explicitly: on mobile, tapping the button doesn't
        // always fire editingFinished first.
        AppController.customerName = customerField.text
        AppController.accountName = accountField.text

        const orderId = AppController.placeOrder(notesField.text)
        if (orderId < 0)
            return

        notesField.clear()
        AppController.shareOrderViaWhatsApp(orderId)
        page.orderPlaced(orderId)
    }

    background: Rectangle { color: Theme.background }

    EmptyState {
        anchors.centerIn: parent
        visible: CartModel.lineCount === 0
        iconSource: "../icons/basket.png"
        title: qsTr("Your basket is empty")
        message: qsTr("Add products from the catalogue and they'll appear here.")
        actionText: qsTr("Browse catalogue")
        onActionTriggered: page.browseRequested()
    }

    ColumnLayout {
        anchors.fill: parent
        visible: CartModel.lineCount > 0
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Theme.spacingMedium
            Layout.rightMargin: Theme.spacingSmall
            Layout.topMargin: Theme.spacingSmall

            Label {
                Layout.fillWidth: true
                text: CartModel.itemCount === 1 ? qsTr("1 item") : qsTr("%1 items").arg(CartModel.itemCount)
                font.pixelSize: Theme.fontSizeBody
                color: Theme.textSecondary
            }

            Button {
                flat: true
                text: qsTr("Clear basket")
                font.pixelSize: Theme.fontSizeCaption + 1
                onClicked: clearDialog.open()
            }
        }

        ListView {
            id: cartList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacingSmall
            clip: true
            model: CartModel
            bottomMargin: Theme.spacingMedium
            ScrollIndicator.vertical: ScrollIndicator { }

            delegate: CartItemDelegate {
                x: Theme.spacingMedium
                width: cartList.width - Theme.spacingMedium * 2
            }
        }

        // Order summary sheet
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: summary.implicitHeight + Theme.spacingMedium * 2
            color: Theme.surface

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Theme.border
            }

            ColumnLayout {
                id: summary
                anchors.fill: parent
                anchors.margins: Theme.spacingMedium
                spacing: Theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spacingSmall

                    TextField {
                        id: customerField
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        placeholderText: qsTr("Customer name")
                        text: AppController.customerName
                        onEditingFinished: AppController.customerName = text
                    }

                    TextField {
                        id: accountField
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        placeholderText: qsTr("Account no.")
                        text: AppController.accountName
                        onEditingFinished: AppController.accountName = text
                    }
                }

                TextField {
                    id: notesField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Order notes (optional)")
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 4

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Label {
                            text: qsTr("Total")
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                        }
                        Label {
                            visible: CartModel.hasPoaItems
                            text: qsTr("POA items are priced by your rep")
                            font.pixelSize: Theme.fontSizeCaption
                            color: Theme.textSecondary
                        }
                    }

                    Label {
                        text: CartModel.totalText
                        font.pixelSize: Theme.fontSizeTitle
                        font.bold: true
                        color: Theme.primary
                    }
                }

                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    highlighted: true
                    enabled: CartModel.lineCount > 0
                    text: qsTr("Place order & send on WhatsApp")
                    onClicked: page.submitOrder()
                }
            }
        }
    }

    Dialog {
        id: clearDialog
        anchors.centerIn: parent
        modal: true
        title: qsTr("Clear basket?")
        standardButtons: Dialog.Cancel | Dialog.Discard
        width: Math.min(page.width - 32, 360)

        Label {
            width: parent.width
            text: qsTr("This removes every item from your basket.")
            wrapMode: Text.WordWrap
            color: Theme.textSecondary
        }

        onDiscarded: {
            CartModel.clear()
            close()
        }
    }
}
