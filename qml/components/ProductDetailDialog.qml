import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

// Product sheet: details plus a quantity picker that only touches the basket
// when the user taps the button.
Dialog {
    id: dialog

    property int productId: -1
    property int initialQuantity: 0
    property string productName
    property string sku
    property string unit
    property string categoryName
    property int categoryId: -1
    property string priceText
    property bool isPoa: false

    function openFor(product) {
        productId = product.productId
        productName = product.name
        sku = product.sku
        unit = product.unit
        categoryName = product.categoryName
        categoryId = product.categoryId
        priceText = product.priceText
        isPoa = product.isPoa
        initialQuantity = product.quantityInCart
        stepper.quantity = Math.max(1, product.quantityInCart)
        open()
    }

    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 32 : 360, 420)
    padding: Theme.spacingLarge
    topPadding: Theme.spacingLarge

    contentItem: ColumnLayout {
        spacing: Theme.spacingMedium

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingMedium

            ProductTile {
                Layout.alignment: Qt.AlignTop
                implicitWidth: 56
                implicitHeight: 56
                categoryId: dialog.categoryId
                categoryName: dialog.categoryName
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: dialog.productName
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSubtitle
                    font.bold: true
                    color: Theme.textPrimary
                }

                Label {
                    Layout.fillWidth: true
                    text: dialog.categoryName
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Theme.spacingMedium
            rowSpacing: 6

            Label { text: qsTr("SKU"); color: Theme.textSecondary; font.pixelSize: Theme.fontSizeCaption; visible: dialog.sku.length > 0 }
            Label { text: dialog.sku; color: Theme.textPrimary; visible: dialog.sku.length > 0; Layout.fillWidth: true }

            Label { text: qsTr("Pack size"); color: Theme.textSecondary; font.pixelSize: Theme.fontSizeCaption }
            Label { text: dialog.unit; color: Theme.textPrimary; Layout.fillWidth: true }

            Label { text: qsTr("Price"); color: Theme.textSecondary; font.pixelSize: Theme.fontSizeCaption }
            Label {
                Layout.fillWidth: true
                text: dialog.isPoa ? qsTr("Price on application") : qsTr("%1 per pack").arg(dialog.priceText)
                color: dialog.isPoa ? Theme.textSecondary : Theme.primary
                font.bold: !dialog.isPoa
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.border
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                text: qsTr("Quantity")
                font.pixelSize: Theme.fontSizeBody
                color: Theme.textPrimary
            }

            QuantityStepper {
                id: stepper
                minimumValue: dialog.initialQuantity > 0 ? 0 : 1
                onIncrease: quantity += 1
                onDecrease: if (quantity > minimumValue) quantity -= 1
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            Button {
                Layout.fillWidth: true
                flat: true
                text: qsTr("Cancel")
                onClicked: dialog.close()
            }

            Button {
                Layout.fillWidth: true
                highlighted: true
                text: {
                    if (stepper.quantity === 0)
                        return qsTr("Remove")
                    return dialog.initialQuantity > 0 ? qsTr("Update basket") : qsTr("Add to basket")
                }
                onClicked: {
                    CartModel.setQuantity(dialog.productId, stepper.quantity)
                    dialog.close()
                }
            }
        }
    }
}
