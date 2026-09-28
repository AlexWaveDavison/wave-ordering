import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

// One basket row. Role values are injected by the ListView.
Rectangle {
    id: row

    required property int productId
    required property string name
    required property string sku
    required property string unit
    required property string priceText
    required property bool isPoa
    required property int quantity
    required property string lineTotalText

    radius: Theme.radiusMedium
    color: Theme.surface
    border.color: Theme.border
    border.width: 1
    implicitHeight: content.implicitHeight + 24

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: 12
        anchors.rightMargin: 4
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true
                    text: row.name
                    font.pixelSize: Theme.fontSizeBody
                    font.bold: true
                    color: Theme.textPrimary
                    wrapMode: Text.WordWrap
                }

                Label {
                    Layout.fillWidth: true
                    text: (row.sku.length > 0 ? row.sku + " · " : "") + row.unit
                    font.pixelSize: Theme.fontSizeCaption
                    color: Theme.textSecondary
                    elide: Text.ElideRight
                }
            }

            ToolButton {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: -8
                text: "✕"
                font.pixelSize: 14
                onClicked: CartModel.removeProduct(row.productId)
                Accessible.name: qsTr("Remove %1").arg(row.name)
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Remove")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.rightMargin: 8
            spacing: Theme.spacingMedium

            QuantityStepper {
                quantity: row.quantity
                minimumValue: 1
                onIncrease: CartModel.increment(row.productId)
                onDecrease: CartModel.decrement(row.productId)
            }

            Label {
                Layout.fillWidth: true
                text: row.isPoa ? qsTr("Price on application") : qsTr("%1 each").arg(row.priceText)
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.textSecondary
                elide: Text.ElideRight
            }

            Label {
                text: row.lineTotalText
                font.pixelSize: Theme.fontSizeBody
                font.bold: true
                color: row.isPoa ? Theme.textSecondary : Theme.primary
            }
        }
    }
}
