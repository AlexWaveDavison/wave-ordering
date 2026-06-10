import QtQuick
import QtQuick.Controls
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
            text: qsTr("Order History")
            font.pixelSize: Theme.fontSizeTitle
            font.bold: true
            color: Theme.textPrimary
        }

        ListView {
            id: orderList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Theme.spacingSmall
            clip: true
            model: orderModel

            delegate: Rectangle {
                width: orderList.width
                implicitHeight: row.implicitHeight + Theme.spacingMedium * 2
                radius: Theme.radiusMedium
                color: Theme.surface
                border.color: Theme.border
                border.width: 1

                RowLayout {
                    id: row
                    anchors.fill: parent
                    anchors.margins: Theme.spacingMedium
                    spacing: Theme.spacingMedium

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Label {
                            text: qsTr("Order #%1").arg(model.id)
                            font.pixelSize: Theme.fontSizeBody
                            font.bold: true
                            color: Theme.textPrimary
                        }
                        Label {
                            text: model.createdAt + " · " + model.itemCount + " "
                                  + (model.itemCount === 1 ? qsTr("item") : qsTr("items"))
                            font.pixelSize: Theme.fontSizeCaption
                            color: Theme.textSecondary
                        }
                        Label {
                            text: model.status
                            font.pixelSize: Theme.fontSizeCaption
                            font.bold: true
                            color: Theme.primary
                        }
                    }

                    Label {
                        text: "£" + model.total.toFixed(2)
                        font.pixelSize: Theme.fontSizeSubtitle
                        font.bold: true
                        color: Theme.textPrimary
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: orderDetailDialog.openFor(model.id)
                }
            }

            Label {
                anchors.centerIn: parent
                visible: orderModel.count === 0
                text: qsTr("No orders yet.\nPlace your first order from the Basket tab.")
                horizontalAlignment: Text.AlignHCenter
                color: Theme.textSecondary
                font.pixelSize: Theme.fontSizeBody
            }
        }
    }

    OrderDetailDialog {
        id: orderDetailDialog
    }
}
