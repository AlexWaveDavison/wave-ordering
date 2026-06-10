import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import WaveOrdering

Dialog {
    id: dialog

    property int orderId: -1
    property var orderItems: []

    function openFor(id) {
        orderId = id
        title = qsTr("Order #%1").arg(id)
        orderItems = orderModel.itemsForOrder(id)
        open()
    }

    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width * 0.9 : 400, 420)
    height: Math.min(parent ? parent.height * 0.8 : 500, 520)
    standardButtons: Dialog.Close

    contentItem: ListView {
        clip: true
        spacing: Theme.spacingSmall
        model: dialog.orderItems

        delegate: Rectangle {
            width: ListView.view.width
            implicitHeight: row.implicitHeight + Theme.spacingMedium * 2
            radius: Theme.radiusSmall
            color: Theme.background
            border.color: Theme.border
            border.width: 1

            RowLayout {
                id: row
                anchors.fill: parent
                anchors.margins: Theme.spacingSmall
                spacing: Theme.spacingMedium

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        Layout.fillWidth: true
                        text: modelData.name
                        font.bold: true
                        font.pixelSize: Theme.fontSizeBody
                        color: Theme.textPrimary
                        wrapMode: Text.WordWrap
                    }
                    Label {
                        text: qsTr("%1 x %2 @ £%3")
                              .arg(modelData.quantity)
                              .arg(modelData.unit)
                              .arg(modelData.unitPrice.toFixed(2))
                        font.pixelSize: Theme.fontSizeCaption
                        color: Theme.textSecondary
                    }
                }

                Label {
                    text: "£" + modelData.lineTotal.toFixed(2)
                    font.bold: true
                    color: Theme.primary
                }
            }
        }
    }
}
