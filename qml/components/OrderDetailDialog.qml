import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WaveOrdering

// Full order sheet: status, who it's for, notes, lines and total, plus the
// actions that make sense for its status.
Dialog {
    id: dialog

    property var order: ({})
    readonly property int orderId: order.orderId !== undefined ? order.orderId : -1

    signal reordered(int count)

    function openFor(id) {
        order = OrderModel.details(id)
        if (order.orderId !== undefined)
            open()
    }

    function reload() {
        order = OrderModel.details(orderId)
    }

    modal: true
    anchors.centerIn: parent
    width: Math.min(parent ? parent.width - 24 : 400, 460)
    height: Math.min(parent ? parent.height - 48 : 600, 680)
    padding: 0
    topPadding: 0

    contentItem: ColumnLayout {
        spacing: 0

        // Header
        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: Theme.spacingLarge
            Layout.bottomMargin: Theme.spacingMedium
            spacing: 6

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacingSmall

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Order #%1").arg(dialog.orderId)
                    font.pixelSize: Theme.fontSizeTitle
                    font.bold: true
                    color: Theme.textPrimary
                }

                StatusChip {
                    sent: dialog.order.isSent === true
                    text: dialog.order.statusText || ""
                }
            }

            Label {
                Layout.fillWidth: true
                text: dialog.order.isSent && dialog.order.sentAtText
                      ? qsTr("Placed %1 · Sent %2").arg(dialog.order.createdAtText).arg(dialog.order.sentAtText)
                      : qsTr("Placed %1").arg(dialog.order.createdAtText || "")
                wrapMode: Text.WordWrap
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.textSecondary
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.topMargin: 6
                columns: 2
                columnSpacing: Theme.spacingMedium
                rowSpacing: 4

                Label { text: qsTr("Customer"); color: Theme.textSecondary; font.pixelSize: Theme.fontSizeCaption }
                Label {
                    Layout.fillWidth: true
                    text: dialog.order.customerName || "—"
                    color: Theme.textPrimary
                    elide: Text.ElideRight
                }

                Label { text: qsTr("Account"); color: Theme.textSecondary; font.pixelSize: Theme.fontSizeCaption }
                Label {
                    Layout.fillWidth: true
                    text: dialog.order.accountName || "—"
                    color: Theme.textPrimary
                    elide: Text.ElideRight
                }

                Label {
                    visible: !!dialog.order.notes
                    text: qsTr("Notes")
                    color: Theme.textSecondary
                    font.pixelSize: Theme.fontSizeCaption
                    Layout.alignment: Qt.AlignTop
                }
                Label {
                    visible: !!dialog.order.notes
                    Layout.fillWidth: true
                    text: dialog.order.notes || ""
                    color: Theme.textPrimary
                    wrapMode: Text.WordWrap
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

        // Lines
        ListView {
            id: lines
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: dialog.order.items || []
            boundsBehavior: Flickable.StopAtBounds
            ScrollIndicator.vertical: ScrollIndicator { }

            delegate: Item {
                id: line
                required property var modelData
                width: lines.width
                implicitHeight: lineRow.implicitHeight + 20

                RowLayout {
                    id: lineRow
                    anchors.fill: parent
                    anchors.leftMargin: Theme.spacingLarge
                    anchors.rightMargin: Theme.spacingLarge
                    spacing: Theme.spacingMedium

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            Layout.fillWidth: true
                            text: line.modelData.name
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                            wrapMode: Text.WordWrap
                        }
                        Label {
                            Layout.fillWidth: true
                            text: qsTr("%1 × %2 @ %3")
                                  .arg(line.modelData.quantity)
                                  .arg(line.modelData.unit)
                                  .arg(line.modelData.unitPriceText)
                                  + (line.modelData.available ? "" : qsTr(" · no longer available"))
                            font.pixelSize: Theme.fontSizeCaption
                            color: Theme.textSecondary
                            wrapMode: Text.WordWrap
                        }
                    }

                    Label {
                        text: line.modelData.lineTotalText
                        font.bold: true
                        color: line.modelData.isPoa ? Theme.textSecondary : Theme.textPrimary
                    }
                }
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

        // Total + actions
        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: Theme.spacingLarge
            Layout.topMargin: Theme.spacingMedium
            spacing: Theme.spacingSmall

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true
                    text: dialog.order.hasPoaItems ? qsTr("Total (excl. POA items)") : qsTr("Total")
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                }
                Label {
                    text: dialog.order.totalText || ""
                    font.pixelSize: Theme.fontSizeSubtitle
                    font.bold: true
                    color: Theme.primary
                }
            }

            Button {
                Layout.fillWidth: true
                visible: dialog.order.isSent === false
                highlighted: true
                text: qsTr("Send via WhatsApp")
                onClicked: AppController.shareOrderViaWhatsApp(dialog.orderId)
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacingSmall

                Button {
                    Layout.fillWidth: true
                    visible: dialog.order.isSent === false
                    flat: true
                    text: qsTr("Mark as sent")
                    onClicked: {
                        if (AppController.markOrderSent(dialog.orderId))
                            dialog.reload()
                    }
                }

                Button {
                    Layout.fillWidth: true
                    visible: dialog.order.canReorder === true
                    flat: true
                    text: qsTr("Reorder")
                    onClicked: {
                        const count = AppController.reorder(dialog.orderId)
                        dialog.close()
                        dialog.reordered(count)
                    }
                }

                Button {
                    Layout.fillWidth: true
                    flat: true
                    text: qsTr("Close")
                    onClicked: dialog.close()
                }
            }
        }
    }
}
