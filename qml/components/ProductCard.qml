import QtQuick
import QtQuick.Layouts
import WaveOrdering

Rectangle {
    id: card

    property int productId: -1
    property string name: ""
    property string sku: ""
    property string unit: ""
    property real price: 0
    property int quantityInCart: 0

    signal openDetails()

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
                text: card.name
                font.pixelSize: Theme.fontSizeBody
                font.bold: true
                color: Theme.textPrimary
                wrapMode: Text.WordWrap
            }

            Text {
                visible: text.length > 0
                text: card.sku.length > 0
                      ? qsTr("SKU %1 · %2").arg(card.sku).arg(card.unit)
                      : card.unit
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.textSecondary
            }

            Text {
                text: card.price > 0
                      ? "£" + card.price.toFixed(2)
                      : qsTr("Price on application")
                font.pixelSize: Theme.fontSizeBody
                font.bold: true
                color: Theme.primary
            }
        }

        QuantityStepper {
            quantity: card.quantityInCart
            onIncrease: cartModel.increment(card.productId)
            onDecrease: cartModel.decrement(card.productId)
        }
    }

    MouseArea {
        anchors.fill: parent
        z: -1
        onClicked: card.openDetails()
    }
}
