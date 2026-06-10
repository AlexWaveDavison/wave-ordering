import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import WaveOrdering

Rectangle {
    id: delegateRoot

    property int productId: -1
    property string name: ""
    property string sku: ""
    property string unit: ""
    property real price: 0
    property int quantity: 0
    property real lineTotal: 0

    radius: Theme.radiusMedium
    color: Theme.surface
    border.color: Theme.border
    border.width: 1
    implicitHeight: Math.max(96, contentRow.implicitHeight + Theme.spacingMedium * 2)

    RowLayout {
        id: contentRow
        anchors.fill: parent
        anchors.margins: Theme.spacingMedium
        spacing: Theme.spacingMedium

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            Text {
                Layout.fillWidth: true
                text: delegateRoot.name
                font.pixelSize: Theme.fontSizeBody
                font.bold: true
                color: Theme.textPrimary
                wrapMode: Text.WordWrap
            }

            Text {
                text: (delegateRoot.sku.length > 0 ? qsTr("SKU %1 · ").arg(delegateRoot.sku) : "")
                      + delegateRoot.unit + qsTr(" · £%1 each").arg(delegateRoot.price.toFixed(2))
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.textSecondary
            }

            Text {
                text: qsTr("Line total: £%1").arg(delegateRoot.lineTotal.toFixed(2))
                font.pixelSize: Theme.fontSizeBody
                font.bold: true
                color: Theme.primary
            }
        }

        QuantityStepper {
            quantity: delegateRoot.quantity
            onIncrease: cartModel.increment(delegateRoot.productId)
            onDecrease: cartModel.decrement(delegateRoot.productId)
        }

        Button {
            text: qsTr("Remove")
            flat: true
            Material.foreground: Theme.danger
            onClicked: cartModel.removeProduct(delegateRoot.productId)
        }
    }
}
