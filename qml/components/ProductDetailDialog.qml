import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import WaveOrdering

Dialog {
    id: dialog

    property int productId: -1
    property int initialQuantity: 0

    function openFor(id, productName, productSku, productUnit, productPrice, qty) {
        productId = id
        initialQuantity = qty
        title = productName
        skuLabel.text = productSku.length > 0
                         ? qsTr("SKU: %1  ·  %2").arg(productSku).arg(productUnit)
                         : productUnit
        priceLabel.text = productPrice > 0
                           ? qsTr("£%1 per unit").arg(productPrice.toFixed(2))
                           : qsTr("Price on application")
        stepper.quantity = qty
        open()
    }

    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width * 0.9 : 360, 400)
    standardButtons: Dialog.Close

    contentItem: ColumnLayout {
        spacing: Theme.spacingMedium

        Label {
            id: skuLabel
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.textSecondary
            font.pixelSize: Theme.fontSizeBody
        }

        Label {
            id: priceLabel
            Layout.fillWidth: true
            color: Theme.primary
            font.pixelSize: Theme.fontSizeSubtitle
            font.bold: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingMedium

            Label {
                Layout.fillWidth: true
                text: qsTr("Quantity")
                font.pixelSize: Theme.fontSizeBody
                color: Theme.textPrimary
            }

            QuantityStepper {
                id: stepper
                onIncrease: quantity += 1
                onDecrease: if (quantity > 0) quantity -= 1
            }
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("Save to Basket")
            Material.background: Theme.primary
            Material.foreground: Theme.textOnPrimary
            onClicked: {
                cartModel.setQuantity(dialog.productId, stepper.quantity)
                dialog.close()
            }
        }
    }
}
