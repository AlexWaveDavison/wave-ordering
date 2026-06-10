import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import WaveOrdering

Page {
    id: page
    background: Rectangle { color: Theme.background }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingMedium

        Label {
            text: qsTr("Your Basket")
            font.pixelSize: Theme.fontSizeTitle
            font.bold: true
            color: Theme.textPrimary
        }

        ListView {
            id: cartList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacingSmall
            clip: true
            model: cartModel

            delegate: CartItemDelegate {
                width: cartList.width
                productId: model.productId
                name: model.name
                sku: model.sku
                unit: model.unit
                price: model.price
                quantity: model.quantity
                lineTotal: model.lineTotal
            }

            Label {
                anchors.centerIn: parent
                visible: cartList.count === 0
                text: qsTr("Your basket is empty.\nAdd products from the Catalog tab.")
                horizontalAlignment: Text.AlignHCenter
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeBody
            }
        }

        Rectangle {
            Layout.fillWidth: true
            radius: Theme.radiusMedium
            color: Theme.surface
            border.color: Theme.border
            border.width: 1
            implicitHeight: summaryColumn.implicitHeight + Theme.spacingMedium * 2

            ColumnLayout {
                id: summaryColumn
                anchors.fill: parent
                anchors.margins: Theme.spacingMedium
                spacing: Theme.spacingSmall

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Total")
                        font.pixelSize: Theme.fontSizeSubtitle
                        color: Theme.textPrimary
                    }
                    Label {
                        text: "£" + cartModel.total.toFixed(2)
                        font.pixelSize: Theme.fontSizeSubtitle
                        font.bold: true
                        color: Theme.primary
                    }
                }

                TextField {
                    id: notesField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Order notes (optional)")
                }

                Button {
                    Layout.fillWidth: true
                    text: qsTr("Place Order")
                    enabled: cartModel.itemCount > 0
                    Material.background: Theme.primary
                    Material.foreground: Theme.textOnPrimary
                    onClicked: {
                        if (appController.placeOrder(notesField.text)) {
                            notesField.text = ""
                            confirmDialog.open()
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: confirmDialog
        title: qsTr("Order placed")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok

        Label {
            text: qsTr("Your order has been saved. View it in the Orders tab.")
            wrapMode: Text.WordWrap
        }
    }
}
